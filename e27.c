
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include "hwdefs.h"
#include "swdefs.h"

#ifndef E27_WR_RETRIES
#define E27_WR_RETRIES 25
#endif

void e27_vpp(uint8_t on)
{
	if( on ) {
		E27_VPP_PORT &= ~_BV(E27_VPP_BIT);
	} else {
		E27_VPP_PORT |= _BV(E27_VPP_BIT);
	}
}

void e27_init(void)
{
	// make address ports outputs
	DDR(E27_LOADR_PORT) = 0xff;
	DDR(E27_HIADR_PORT) = 0xff;
	// make OE port an output and put it high
	DDR(E27_OE_PORT) |= _BV(E27_OE_BIT);
	E27_OE_PORT |= _BV(E27_OE_BIT);
	// make CE port an output and put it high
	DDR(E27_CE_PORT) |= _BV(E27_CE_BIT);
	E27_CE_PORT |= _BV(E27_CE_BIT);
	// make VPP port an output and put it low
	DDR(E27_VPP_PORT) |= _BV(E27_VPP_BIT);
	e27_vpp(0);
}

uint8_t e27_rd(uint16_t a)
{
	// make data port an input
	DDR(E27_DATA_PORT) = 0;
	// set address lines
	E27_LOADR_PORT = a;
	E27_HIADR_PORT = a >> 8;
	// put CE low
	E27_CE_PORT &= ~_BV(E27_CE_BIT);
	// put OE low
	E27_OE_PORT &= ~_BV(E27_OE_BIT);
	_delay_us(1);
	// read data
	uint8_t r = PIN(E27_DATA_PORT);
	// return OE high
	E27_OE_PORT |= _BV(E27_OE_BIT);
	// return CE high
	E27_CE_PORT |= _BV(E27_CE_BIT);

	return r;
}

uint8_t e27_wr(uint16_t a, uint8_t d)
{
	// set address lines
	E27_LOADR_PORT = a;
	E27_HIADR_PORT = a >> 8;

	uint8_t r;

	for( uint8_t i = 0; i < E27_WR_RETRIES; ++i ) {
		// make data port an output and set data
		DDR(E27_DATA_PORT) = 0xff;
		E27_DATA_PORT = d;
		_delay_us(1);
		// pulse CE for 100us
		E27_CE_PORT &= ~_BV(E27_CE_BIT);
		_delay_us(100);
		E27_CE_PORT |= _BV(E27_CE_BIT);
		// make data port an input
		DDR(E27_DATA_PORT) = 0;
		E27_DATA_PORT = 0;
		// put OE low
		E27_OE_PORT &= ~_BV(E27_OE_BIT);
		// read data
		r = PIN(E27_DATA_PORT);
		// return OE high
		E27_OE_PORT |= _BV(E27_OE_BIT);
		// check programming success
		if( r == d ) break;
	}

	return r;
}

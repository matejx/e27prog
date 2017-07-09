/**
E27 EPROM programmer


@file		main.c
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	GPL v2
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "serque.h"
#include "hwdefs.h"
#include "e27.h"

// ------------------------------------------------------------------
// --- DEFINES AND GLOBALS ------------------------------------------
// ------------------------------------------------------------------

uint8_t rxbuf[16];
uint8_t txbuf[16];

#define BUFSIZE 64
uint8_t buf1[BUFSIZE];
uint8_t buf2[BUFSIZE];
uint8_t* wbuf = buf1;
uint8_t wlen = 0;
uint8_t* rbuf = buf2;
uint8_t rlen = 0;

char atbuf[16+(2*BUFSIZE)];
uint8_t atbuflen = 0;
uint8_t at_echo = 0;
uint8_t bufdisp = 1;

const char PROGMEM atbufwr[] = "AT+BUFWR=";	// dddddd...
const char PROGMEM atbufrddisp[] = "AT+BUFRDDISP=";

const char PROGMEM ate27rd[] = "AT+E27RD="; // aaaa,ll
const char PROGMEM ate27vpp[] = "AT+E27VPP=";
const char PROGMEM ate27wr[] = "AT+E27WR="; // aaaa
const char PROGMEM ate27blank[] = "AT+E27BLANK="; // len

const char PROGMEM crlf[] = "\r\n";

// ------------------------------------------------------------------
// --- UTILITY FUNCTIONS --------------------------------------------
// ------------------------------------------------------------------

uint32_t udtoi(const char* s) // unsigned decimal string to u32
{
	uint32_t x = 0;

	while( isdigit((int)*s) ) {
		x *= 10;
		x += *s - '0';
		++s;
	}

	return x;
}

uint32_t uhtoi(const char* s, uint8_t n) // unsigned hex string to u32
{
	uint32_t x = 0;

	uint8_t c = toupper((int)*s);

	while( n-- && ( isdigit((int)c) || ( (c >= 'A') && (c <= 'F') ) ) ) {
		if( isdigit((int)c) ) {
			c -= '0';
		} else {
			c -= 'A' - 10;
		}

		x *= 16;
		x += c;

		++s;
		c = toupper((int)*s);
	}

	return x;
}

void hprintbuf(uint8_t* buf, uint16_t len) // hex print buf
{
	uint16_t i;
	for( i = 0; i < len; ++i ) {
		ser_puti_lc(0, buf[i], 16, 2, '0');
	}
	ser_puts_P(0, crlf);
}

// ------------------------------------------------------------------
// --- AT CMD PROCESSING --------------------------------------------
// ------------------------------------------------------------------

uint8_t proc_at_cmd(const char* s)
{
	if( s[0] == 0 ) { return 1; }

// --- general AT commands ----------------------------------------------------

	if( 0 == strcmp_P(s, PSTR("AT")) ) {
		return 0;
	}

	if( 0 == strcmp_P(s, PSTR("ATE0")) ) {
		at_echo = 0;
		return 0;
	}

	if( 0 == strcmp_P(s, PSTR("ATE1")) ) {
		at_echo = 1;
		return 0;
	}

	if( 0 == strcmp_P(s, PSTR("ATI")) ) {
		ser_puts_P(0, PSTR("MK e27prog v1.0\r\n"));
		return 0;
	}

// --- buffer commands --------------------------------------------------------

	if( 0 == strncmp_P(s, atbufwr, strlen_P(atbufwr)) ) {
		s += strlen_P(atbufwr);

		uint16_t len = strlen(s);
		if( len % 2 ) return 1;
		len /= 2;
		if( len > BUFSIZE ) return 2;

		wlen = len;
		uint16_t i;
		for( i = 0; i < wlen; ++i ) {
			wbuf[i] = uhtoi(s, 2);
			s += 2;
		}

		return 0;
	}

	if( 0 == strcmp_P(s, PSTR("AT+BUFRD")) ) {
		if( rlen == 0 ) return 1;

		hprintbuf(rbuf, rlen);

		return 0;
	}

	if( 0 == strcmp_P(s, PSTR("AT+BUFRDLEN")) ) {
		ser_puti(0, rlen, 10);
		ser_puts_P(0, crlf);

		return 0;
	}

	if( 0 == strcmp_P(s, PSTR("AT+BUFSWAP")) ) {
		uint8_t* b = rbuf;
		uint16_t l = rlen;

		rbuf = wbuf;
		rlen = wlen;
		wbuf = b;
		wlen = l;

		return 0;
	}

	if( 0 == strcmp_P(s, PSTR("AT+BUFCMP")) ) {
		if( rlen != wlen ) return 1;

		if( memcmp(rbuf, wbuf, rlen) ) return 1;

		return 0;
	}

	if( 0 == strncmp_P(s, atbufrddisp, strlen_P(atbufrddisp)) ) {
		s += strlen_P(atbufrddisp);
		if( strlen(s) != 1 ) return 1;

		if( s[0] == '1' ) { bufdisp = 1; return 0; }
		if( s[0] == '0' ) { bufdisp = 0; return 0; }

		return 1;
	}

// --- E27 EPROM commands -----------------------------------------------------

	if( 0 == strncmp_P(s, ate27rd, strlen_P(ate27rd)) ) {
		s += strlen_P(ate27rd);

		if( strlen(s) < 6 ) return 1;
		if( s[4] != ',' ) return 1;
		uint16_t adr = uhtoi(s, 4);
		s += 5;
		uint8_t len = udtoi(s);
		if( (len < 1) || (len > BUFSIZE) ) return 1;

		for( uint8_t i = 0; i < len; ++i ) {
			rbuf[i] = e27_rd(adr+i);
		}

		rlen = len;
		if( bufdisp ) hprintbuf(rbuf, rlen);

		return 0;
	}

	if( 0 == strncmp_P(s, ate27vpp, strlen_P(ate27vpp)) ) {
		s += strlen_P(ate27vpp);
		if( strlen(s) != 1 ) return 1;

		if( s[0] == '0' ) { e27_vpp(0); return 0; }
		if( s[0] == '1' ) { e27_vpp(1); return 0; }

		return 0;
	}

	if( 0 == strncmp_P(s, ate27wr, strlen_P(ate27wr)) ) {
		s += strlen_P(ate27wr);

		if( wlen == 0 ) return 1; // nothing to write
		if( strlen(s) != 4 ) return 1;
		uint16_t adr = uhtoi(s, 4);

		e27_vpp(1);
		_delay_ms(1);

		for( uint8_t i = 0; i < wlen; ++i ) {

			uint8_t r = e27_wr(adr+i, wbuf[i]);

			if( r != wbuf[i] ) {
				ser_puts_P(0, PSTR("address "));
				ser_puti_lc(0, adr+i, 16, 4, '0');
				ser_puts_P(0, PSTR(" wrote "));
				ser_puti_lc(0, wbuf[i], 16, 2, '0');
				ser_puts_P(0, PSTR(" read "));
				ser_puti_lc(0, r, 16, 2, '0');
				ser_puts_P(0, crlf);

				e27_vpp(0);
				_delay_ms(1);
				return 1;
			}
		}

		e27_vpp(0);
		_delay_ms(1);

		return 0;
	}

	if( 0 == strncmp_P(s, ate27blank, strlen_P(ate27blank)) ) {
		s += strlen_P(ate27blank);

		uint16_t len = udtoi(s);

		if( (len == 0) || (len > 0x10000) ) return 1;

		for( uint16_t i = 0; i < len; ++i ) {
			if( e27_rd(i) != 0xff ) return 1;
		}

		return 0;
	}

	return 1;
}

// ------------------------------------------------------------------
// --- MAIN ---------------------------------------------------------
// ------------------------------------------------------------------

int main(void)
{
	cli();
	wdt_reset();
	wdt_enable(WDTO_2S);

	ser_init(0, BAUD_115200, txbuf, sizeof(txbuf), rxbuf, sizeof(rxbuf));
	_delay_ms(10);
	e27_init();

	sei();

	while( 1 ) {
		wdt_reset();
		uint8_t d;
		if( ser_getc(0, &d) ) {

			// echo character
			if( at_echo ) { ser_putc(0, d); }

			// buffer overflow guard
			if( atbuflen >= sizeof(atbuf) ) { atbuflen = 0; }

			// execute on enter
			if( (d == '\r') || (d == '\n') ) {
				if( atbuflen ) {
					atbuf[atbuflen] = 0;
					atbuflen = 0;
					uint8_t r = proc_at_cmd(atbuf);
					if( r == 0 ) ser_puts_P(0, PSTR("OK\r\n"));
					if( r == 1 ) ser_puts_P(0, PSTR("ERR\r\n"));
				}
			} else
			if( d == 0x7f ) {	// backspace
				if( atbuflen ) { --atbuflen; }
			} else {			// store character
				atbuf[atbuflen++] = toupper(d);
			}
		}
	}
}

// ------------------------------------------------------------------
// --- INTERRUPTS ---------------------------------------------------
// ------------------------------------------------------------------

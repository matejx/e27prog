#ifndef MAT_HWDEFS_H
#define MAT_HWDEFS_H

	#define DDR(x) (*(&x - 1))
	#define PIN(x) (*(&x - 2))

	#define E27_CE_PORT PORTD
	#define E27_CE_BIT 7

	#define E27_OE_PORT PORTD
	#define E27_OE_BIT 5

	#define E27_VPP_PORT PORTD
	#define E27_VPP_BIT 6

	#define E27_LOADR_PORT PORTC
	#define E27_HIADR_PORT PORTB
	#define E27_DATA_PORT PORTA

#endif

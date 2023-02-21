#ifdef IN_TOKENS

// NOTE: this file gets inserted directly into the initialisation of the token array

/* C0 */	{"port",		12,	0,  -1,	__port},		// "sw+func", portN, ...
														// sw: '+' pullup, '-' pulldown, '#' opendrain
														// func: 'DIN', 'DOUT', 'AIN', 'PWM:freq'
/* C1 */	{"Dout",		12,	0,	-1,	__DOUT},		// 0/1, portN, ...
/* C2 */	{"Dtog",		12,	0,	-1,	__DTOG},		// toggle portN, ...
/* C3 */	{"Din",			10,	0,	1,	__DIN},			// portN ---> 0/1
/* C4 */	{"Ain",			10,	0,	1,	__AIN},			// portN ---> ratio from Vref
/* C5 */	{"Vref",		10,	0,	1,	__Vref},		// "Vref_option"
/* C6 */	{"setPWM",		10,	0,	2,	__setPWM},		// duty (0 to 1), portN
/* C7 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* C8 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* C9 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* CA */	{".UNUSED",		0,	0,	0,	_invalid_},
/* CB */	{".UNUSED",		0,	0,	0,	_invalid_},
/* CC */	{"trmt",		12,	0,	-1,	__trmt},		// "interface", data, ...
														// in I2C cases:
														// "intf:opt", data, ...
														// options:
														// S - start, P - stop, R - repeat start
/* CD */	{"recv",		10,	0,	1,	__recv},		// "interface" ---> data
														// in I2C cases:
														// "intf:opt", data, ...
														// options:
														// S - start, P - stop, R - repeat start
/* CE */	{"enable",		12,	0,	-1,	__enable},		// "interface", parameters, @callback
														// "UART1/2/3/4", "bps, [8/9] [N/E/O] [1/2]", @rx_callback
														// "SPI1/2", "M/S, bps [,0/1/2/3] [,8/16/32]"
														// "IIC1/2", "M/S [, bps]"
/* CF */	{"disable",		10,	0,	1,	__disable},		// "interface"

/* D0 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* D1 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* D2 */	{"clock",		10,	0,	1,	__clock},		// CPU clock frequency
/* D3 */	{"sleep",		10,	0,	1,	__sleep},		// put the system into sleep mode and configure
														// pin 46 as change-of-level wake up input
/* D4 */	{"srctime",		10,	0,	2,	__srctime},		// "LPRC" selects PIC32's internal clock
														// "SOSC" selects an external 32.768kHz oscillator on SOSCI
														// second parameter specifies calibration value
/* D5 */	{"gettime",		10,	0,	0,	__gettime},		// will return a 16-character text yymmddwwhhmmss00
/* D6 */	{"settime",		10,	0,	1,	__settime},		// requires 16-character text yymmddwwhhmmss00
/* D7 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* D8 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* D9 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* DA */	{".UNUSED",		0,	0,	0,	_invalid_},
/* DB */	{".UNUSED",		0,	0,	0,	_invalid_},
/* DC */	{".UNUSED",		0,	0,	0,	_invalid_},
/* DD */	{".UNUSED",		0,	0,	0,	_invalid_},
/* DE */	{".UNUSED",		0,	0,	0,	_invalid_},
/* DF */	{".UNUSED",		0,	0,	0,	_invalid_},

/* E0 */	{"LCD4",		12,	0,	-1,	__LCD4},		// text_cmd [, parameter(s)]
														// "init:L/S", int_cols, int_rows
														// "clear"
														// "scroll:U/D/L/R"
														// "goto", int_x, int_y
														// "print", any_data, ...
														// "char", int_code, big_def
/* E1 */	{"display",		10,	0,	3,	__display},		// enable and initialise external display
														// text_type, int_sizeX, int_sizeY
														// display type:
														// "NULL"
														// "HD44780" or "LCD4" (only LANDSCAPE)
														// "ST77XX"
														// "ILI9341"
/* E2 */	{"touch",		12,	0,	-1,	__touch},		// enable and initialise touch driver (uses IIC2)
														// "model/command" [, @callback]
														// "POINTS"	- return the number of currently touched points
														// "READ"	- return one touched point (X,Y,pressure)
														// "NULL"	- null touch driver
														// "RSPI", @func	- resistive SPI based on the *2046 IC
/* E3 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* E4 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* E5 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* E6 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* E7 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* E8 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* E9 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* EA */	{".UNUSED",		0,	0,	0,	_invalid_},
/* EB */	{".UNUSED",		0,	0,	0,	_invalid_},
/* EC */	{".UNUSED",		0,	0,	0,	_invalid_},
/* ED */	{".UNUSED",		0,	0,	0,	_invalid_},
/* EE */	{".UNUSED",		0,	0,	0,	_invalid_},
/* EF */	{".UNUSED",		0,	0,	0,	_invalid_},

#endif

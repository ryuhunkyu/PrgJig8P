/*--------------------------------------
By CapRhk 2026. 9. 12.

Functions:
void Init(void)
--------------------------------------*/

#include "def.h"

void Good(u8 t)
{
	if(t) GOOD_PORT |=  (1<<PORT_Good);
	else  GOOD_PORT &= ~(1<<PORT_Good);
}

void Beep(u16 count)
{
	sbi(BEEP_PORT, PORT_Beep);
	BeepCount = count;
}

void Init(void)
{
	// IO Port
	// [DDRx]	Data Direction Reg:	0:input / 1:output
	// [PORTx]	Port Data Reg:		0:highz / 1:pullup

	//<Port B>
	//#define	PORT_Mouse			PB1
	//#define	PORT_Beep			PB2
	//#define	PORT_Good			PB6
	DDRB  = 0x46;	// 0:input / 1:output [0100 0110]
	PORTB = 0x02;	// 0:highz / 1:pullup [0000 0010]

	//<Port C>
	//#define	PORT_Select8			PC0
	//#define	PORT_Select7			PC1
	//#define	PORT_Select6			PC2
	//#define	PORT_Latch			PC3
	//#define	PORT_CLK			PC4
	//#define	PORT_SDI			PC5
	DDRC  = 0x3F;	// 0:input / 1:output [0011 1111]
	PORTC = 0x07;	// 0:highz / 1:pullup [0000 0111]

	//<Port D>
	//#define	PORT_Select5			PD0
	//#define	PORT_Select4			PD1
	//#define	PORT_Select3			PD2
	//#define	PORT_Select2			PD3
	//#define	PORT_Select1			PD4
	//#define	PORT_Control			PD5
	//#define	PORT_Program			PD6
	//#define	PORT_Test			PD7
	DDRD  = 0x9F;	// 0:input / 1:output [1001 1111]
	PORTD = 0xFF;	// 0:highz / 1:pullup [1111 1111]

	//timer0(8bit) 1ms timer -- 8M / 64(ps) / 124(counter) = 1ms

	//timer1(16bit) 1sec timer -- 8M / 1024(ps) / 7811(counter) = 1hz = 1sec

	//timer2(8bit) for 100us timer

}


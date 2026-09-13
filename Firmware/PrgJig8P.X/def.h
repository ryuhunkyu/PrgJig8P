#include <inttypes.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include <avr/wdt.h>

#ifndef F_CPU
#define F_CPU				8000000UL		// 8MHz 내부 클럭 사용
#endif

#include <xc.h>
#include <util/delay.h>

#define u8				unsigned char
#define	u16				unsigned int
#define	u32				unsigned long int
#define	i8				char
#define	i16				int
#define	i32				long int
#define	vu8				volatile unsigned char
#define	vu16				volatile unsigned int
#define	vi8				volatile char
#define	vi16				volatile int
                                	
#define	VERSION				0x0090
#define	SYSTEMNAME			"MCU 8Port Programming Jig "
#define	BUILDWEEK			0x2637			// 260910
#define	SERIALNUMBER			0x0001
                                	
#include "PrgJig8P.h"
                                	
//Port Definition               	
#define	PORT_Select5			PD0
#define	PORT_Select4			PD1
#define	PORT_Select3			PD2
#define	PORT_Select2			PD3
#define	PORT_Select1			PD4
#define	PORT_Control			PD5
#define	PORT_Program			PD6
#define	PORT_Test			PD7

#define	PORT_Mouse			PB1
#define	PORT_Beep			PB2
#define	PORT_Good			PB6

#define	PORT_Select8			PC0
#define	PORT_Select7			PC1
#define	PORT_Select6			PC2
#define	PORT_Latch			PC3
#define	PORT_CLK			PC4
#define	PORT_SDI			PC5
                                	
// Register Definition
#define	GOOD_PORT			PORTB
#define	BEEP_PORT			PORTB
#define	MOUSE_PORT			PORTB
#define	PROGRAM_INPORT			PIND
#define	CONTROL_INPORT			PIND
#define	LATCH_PORT			PORTC
#define	TEST_PORT			PORTD
                               	
//<EEPRom Addr> Memory space:0x0000~0x0FFF
#define	EEP_STATE			0x000
#define	EEP_LEVEL			0x002
#define	EEP_COMPARE			0x100

//Constant                      	
#define	LED_ON				1
#define	LED_OFF				0

#define	STATE_IDLE			0
#define	STATE_COMPARE			1
#define	STATE_PERFUSE			2
#define	STATE_REST			3

#define	CHAR_NUMBER			0x00
#define	CHAR_ALPHABET			0x0A
#define	CHAR_DIGIT_ANI			0x24
#define	CHAR_DASH			0x2A
#define	CHAR_TEST			0x2B
#define	CHAR_NONE			0x2C

//Command                       	

//Code define                   	
//#define	sbi(port,bit)		__asm__ __volatile__( "sbi %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))
//#define	cbi(port,bit)		__asm__ __volatile__( "cbi %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))
//#define	sbis(port,bit)		__asm__ __volatile__("sbis %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))

#define sbi(port,bit)			((port) |= (1 << (bit)))
#define cbi(port,bit)			((port) &= ~(1 << (bit)))
#ifndef NOP
#define	NOP				asm( "nop" ::)
#endif

//Microchip 제공 함수 : CPU Clock Delay
#define Delay1us()			__builtin_avr_delay_cycles(8)
#define Delay10us()			__builtin_avr_delay_cycles(80)
#define Delay100us()			__builtin_avr_delay_cycles(800)

//macro
#define	ENABLE_INT0			EIMSK |= (1 << INT0)
#define	DISABLE_INT0			EIMSK &= ~(1 << INT0)
#define	ENABLE_INT1			EIMSK |= (1 << INT1)
#define	DISABLE_INT1			EIMSK &= ~(1 << INT1)
#define	ENABLE_INT2			EIMSK |= (1 << INT2)
#define	DISABLE_INT2			EIMSK &= ~(1 << INT2)

#define	FND_SDI_HIGH			sbi(SDI_PORT, PORT_SDI)
#define	FND_SDI_LOW			cbi(SDI_PORT, PORT_SDI)
#define	FND_CLK_HIGH			sbi(CLK_PORT, PORT_CLK)
#define	FND_CLK_LOW			cbi(CLK_PORT, PORT_CLK)
#define	FND_CLOCK			do{FND_CLK_HIGH;FND_CLK_LOW;} while(0)
#define	FND_LATCH			do{sbi(LATCH_PORT, PORT_Latch); cbi(LATCH_PORT, PORT_Latch);} while(0)

#define	KEY_BUTTON			((~BUTTON_INPORT)&(1<<PORT_Button))
#define	KEY_CONTROL			((~CONTROL_INPORT)&(1<<PORT_Control))

//------------------------------------

extern	u8	MCUState;
extern	u8	OscState;
extern	vu8	Command;
extern	vu8	State;
extern	u8	temp;
extern	u8	Count;
extern	u8	Stamp;
extern	u8	adcChannel;
extern	u16	SerialNumber;

extern	vu8	tToggle;
extern	vu16	Timer;
extern	vu16	TimerDigit;
extern	vi16	SecondCount;
extern	vi16	MiliCount;
extern	vi16	MiliTerm;
extern	vi16	WaitTerm;
extern	vi8	Blink;
extern	vi16	ButtonTime;
extern	vu16	FNDTerm;
extern	vu16	LEDTerm;
extern	vi16	NumbCount;
extern	i16	LEDCount;
extern	vu8	ValveTerm;
extern	vu8	ButtonFlag;
extern	vu8	WindCheckFlag;
extern	vu8	WDFlag;
extern	vu8	ADCFlag;
extern	vu8	TimeOut;

extern	u8	Level;
extern	vi8	PerfuseTerm;
extern	vi8	RestTerm;
extern	u8	ValveCommand;

extern	u8	cfgCompare;
extern	u8	cfgDifference;
extern	u8	cfgPerfuseTerm;
extern	u8	cfgRestTerm;
extern	vi16	ReferenceTemper;

extern	vi16	PowerSum;
extern	vi16	PowerVal;
extern	vi16	WindTemperSum;
extern	vi16	WindTemperVal;
extern	vi16	TemperatureSum;
extern	vi16	TemperatureVal;

extern	i16	ButtonReleaseTerm;
extern	u8	ButtonPressCount;

extern	u8	AS_RecQue[32];
extern	vu8	AS_SendFlag;
extern	vu8	AS_Qwi;
extern	vu8	AS_Qri;


	

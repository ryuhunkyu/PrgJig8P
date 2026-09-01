#include <inttypes.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include <avr/wdt.h>

#ifndef F_CPU
#define F_CPU			8000000UL		// 8MHz 내부 클럭 사용
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
#define	BUILDWEEK			0x2635			// 260828
#define F_CPU				8000000UL		// 8MHz 내부 클럭 사용
#define	SERIALNUMBER			0x0001
                                	
#include "PrgJig8P.h"
                                	
//Port Definition               	
#define	PORT_Select5			PD0
#define	PORT_Select4			PD1
#define	PORT_Select3			PD2
#define	PORT_Select2			PD3
#define	PORT_Select1			PD4
#define	PORT_Control			PD5
#define	PORT_Button			PD6
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
                                	
// port define                  	
#define	GOOD_PORT			PORTB
#define	BEEP_PORT			PORTB
#define	MOUSE_PORT			PORTB
#define	BUTTON_INPORT			PIND
#define	CONTROL_INPORT			PIND
                               	
//<EEPRom Addr> Memory space:0x0000~0x0FFF
#define	EEP_STATE			0x000
#define	EEP_LEVEL			0x002
#define	EEP_COMPARE			0x100
                                	
//Constant                      	
#define	POWER_GOOD			434		//2.8V/6 * (1024/1.1V)
#define	POWER_POOR			387		//2.5V
#define	POWER_WEEK			356		//2.3V
                                	
#define	LED_ON				1
#define	LED_OFF				0
                                	
#define	STATE_IDLE			0
#define	STATE_COMPARE			1
#define	STATE_PERFUSE			2
#define	STATE_REST			3
                                	
//ADC Channel                   	
#define	CHN_POWER			1
#define	CHN_TEMPER_WIND			7
#define	CHN_TEMPERATURE			6
                                	
#define	CHAR_NUMBER			0x00
#define	CHAR_ALPHABET			0x0A
#define	CHAR_DIGIT_ANI			0x24
#define	CHAR_DASH			0x2A
#define	CHAR_TEST			0x2B
#define	CHAR_NONE			0x2C
                                	
#define	TEXT_S				0x6D
#define	TEXT_E				0x79
#define	TEXT_T				0x78
#define	TEXT_B				0x7C
#define	TEXT_O				0x5C
#define	TEXT_D				0x5E
#define	TEXT_Y				0x6E
#define	TEXT_N				0x54
#define	TEXT_F				0x71
#define	TEXT_L				0x38
#define	TEXT_DASH			0x40
#define	TEXT_NONE			0x00
                                	
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


#define	VALVE_CLOSE			do{cbi(SOLBACKWARD_PORT, PORT_SolBackward);sbi(SOLFORWARD_PORT, PORT_SolForward);}while(0)
#define	VALVE_OPEN			do{cbi(SOLFORWARD_PORT, PORT_SolForward);sbi(SOLBACKWARD_PORT, PORT_SolBackward);}while(0)
#define	VALVE_STANDBY			do{cbi(SOLFORWARD_PORT, PORT_SolForward);cbi(SOLBACKWARD_PORT, PORT_SolBackward);}while(0)
#define	BOOST_ON			sbi(BOOST_PORT, PORT_Boost)
#define	BOOST_OFF			cbi(BOOST_PORT, PORT_Boost)
#define	WINDGATE_OPEN			sbi(WINDGATE_PORT, PORT_WindGate)
#define	WINDGATE_CLOSE			cbi(WINDGATE_PORT, PORT_WindGate)
#define	VALVE_SLEEP			cbi(VALVESLEEP_PORT, PORT_ValveSleep)
#define	VALVE_ENABLE			sbi(VALVESLEEP_PORT, PORT_ValveSleep)
                                	
#define	KEY_BUTTON			((~BUTTON_INPORT)&(1<<PORT_Button))
#define	WIND_CHECK			((~WINDCHECK_INPORT)&(1<<PORT_WindCheck))

#define	ADC_REF				1100L	//1.1V * 1000mV
#define	RLOW_THERM			1000L	//분압 고정저항

// WDT 주기: 1초 (WDP2|WDP1)
#define USE_POWER_DOWN_SLEEP		1	// 1이면 power-down, 0이면 power-save(타이머2사용시)

#define	CMD_OPEN			1
#define	CMD_CLOSE			0

//------------------------------------


//extern	u8	LevelPerfuse[];

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


	

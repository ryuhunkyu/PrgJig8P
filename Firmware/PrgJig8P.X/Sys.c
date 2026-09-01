/*--------------------------------------
By CapRhk 2021. 6. 11.

Functions:
void ReadSwitch(void)
void Init(void)
--------------------------------------*/

#include "def.h"

void Good(u8 t)
{
	if(t) GOOD_PORT |=  (1<<PORT_Good);
	else  GOOD_PORT &= ~(1<<PORT_Good);
}

// ───────── 전력 차단 헬퍼 ───────── 
inline void prepare_for_sleep(void)
{
	//필요 없다면 주변장치/ADC OFF, PRR 활용 등
	ADCSRA &= ~(1<<ADEN);		// ADC OFF 예시
}

inline void go_to_sleep(void)
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_bod_disable();		// BOD 끄기(48PA 지원)
	sei();				// SLEEP 이전에 반드시 전역 INT 켜기
	sleep_cpu();			// 여기서 대기
	sleep_disable();		// 깨어난 직후
}

void PowerDownMode(void)
{
	Good(0);
	PortSleep();		

	prepare_for_sleep();
	go_to_sleep();		// WDT 또는 PCINT가 다시 깨움

	Init();
}

void Init(void)
{
	// IO Port
	// [DDRx]	Data Direction Reg:	0:input / 1:output
	// [PORTx]	Port Data Reg:		0:highz / 1:pullup

	//<Port B>
	//#define	PORT_FND_E			PB0
	//#define	PORT_FND_C			PB1
	//#define	PORT_FND_D			PB2
	//#define	PORT_FND_A			PB7
	DDRB  = 0x87;	// 0:input / 1:output [1000 0111]
	PORTB = 0x87;	// 0:highz / 1:pullup [1000 0111]

	//<Port C>
	//#define	PORT_WindCheck			PC0
	//#define	PORT_PowerCheck			PC1
	//#define	PORT_WindGate			PC2
	//#define	PORT_ValveSleep			PC3
	//#define	PORT_Boost			PC4
	//#define	PORT_Button			PC5
	//#define	PORT_Temperature		PC6
	//#define	PORT_TemperWind			PC7
	DDRC  = 0x1D;	// 0:input / 1:output [0001 1101]
	PORTC = 0x20;	// 0:highz / 1:pullup [0010 0000]

	//<Port D>
	//#define	PORT_RxD			PD0
	//#define	PORT_TxD			PD1
	//#define	PORT_Good			PD2
	//#define	PORT_SolBackward		PD3
	//#define	PORT_SolForward			PD4
	//#define	PORT_FND_G			PD5
	//#define	PORT_FND_B			PD6
	//#define	PORT_FND_F			PD7
	DDRD  = 0xFE;	// 0:input / 1:output [1111 1110]
	PORTD = 0xE3;	// 0:highz / 1:pullup [1110 0011]

//	Main에서 시작시 1회 실행
//	#define	WDPS	(1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0)	// 타임아웃 설정: ≒ 8초
//	//[WDT] 설정을 위해 시퀀스 필요 (타이밍 제한)
//	WDTCSR = (1<<WDCE) | (1<<WDE);			// 변경 허용
//	//WDIE=1, WDE=0  → Interrupt only
//	WDTCSR = (1<<WDIE) | WDPS;

	//timer0(8bit) 1ms timer -- 8M / 64(ps) / 124(counter) = 1ms
	TCCR0A = (1<<WGM01);				// CTC mode
	TCCR0B = (1<<CS01) | (1<<CS00);			// Prescaler 64
	OCR0A = 124;
	TIMSK0 = (1<<OCIE0A);				// Enable compare match interrupt

	//timer1(16bit) 1sec timer -- 8M / 1024(ps) / 7811(counter) = 1hz = 1sec
	TCCR1A = 0;
	TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);	// CTC mode, prescaler 1024
	OCR1A = 7811;					// 8 MHz/1024 ≈ 7812.5
	TIMSK1 = (1<<OCIE1A);				// Enable compare match interrupt

	//timer2(8bit) for 100us timer
//	TCCR2A = (1<<WGM21);				// CTC mode
//	TCCR2B = (1<<CS21);				// Prescaler 8
//	OCR2A = 99;					// (8 MHz/8) ×100 µs −1
//	TIMSK2 = (1<<OCIE2A);				// Enable compare match interrupt

	PCMSK1 |= _BV(PCINT8);				// PC0 감시 wind check
	PCMSK1 |= _BV(PCINT13);				// PC5 감시 button
	PCICR  |= _BV(PCIE1);				// 그룹 Enable


	//UART
	UBRR0H = 0;					// 상위 바이트
	UBRR0L = 51;	// 8Mhz: [4800:103, 9600:51, 14400:34, 19200:25, 28800:16, 38400:12, 57600:8]
	UCSR0A = 0;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<TXCIE0);	// 송수신 활성화,enable int
//	UCSR0B |= (1<<RXCIE0) | (1<<TXCIE0);		// 송수신 인터럽트 허용(x송신버퍼 빔)
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);		// 8비트, 1stop, 비동기, no parity
//	UDR0 = 0;

	ADMUX = (1<<REFS0) | (1<<REFS1);		//ref:Internal 1.1V Voltage Reference
	ADCSRA = ((1<<ADEN) | (1<<ADIE) | 6);		//ps:64clk = 125KHz, =:10kSPS
}

void PortSleep(void)
{
	// IO Port
	// [DDRx]	Data Direction Reg:	0:input / 1:output
	// [PORTx]	Port Data Reg:		0:highz / 1:pullup

	//<Port B>
	//#define	PORT_FND_E			PB0
	//#define	PORT_FND_C			PB1
	//#define	PORT_FND_D			PB2
	//#define	PORT_FND_A			PB7
	DDRB  = 0x87;	// 0:input / 1:output [1000 0111]
	PORTB = 0x87;	// 0:highz / 1:pullup [1000 0111]

	//<Port C>
	//#define	PORT_WindCheck			PC0
	//#define	PORT_PowerCheck			PC1
	//#define	PORT_WindGate			PC2
	//#define	PORT_ValveSleep			PC3
	//#define	PORT_Boost			PC4
	//#define	PORT_Button			PC5
	//#define	PORT_Temperature		PC6
	//#define	PORT_TemperWind			PC7
	DDRC  = 0x1D;	// 0:input / 1:output [0001 1101]
	PORTC = 0x24;	// 0:highz / 1:pullup [0010 0100]

	//<Port D>
	DDRD  = 0xF8;	// 0:input / 1:output [1111 1000]
	PORTD = 0xE0;	// 0:highz / 1:pullup [1110 0000]
	//<Port D>
	//#define	PORT_RxD			PD0
	//#define	PORT_TxD			PD1
	//#define	PORT_Good			PD2
	//#define	PORT_SolBackward		PD3
	//#define	PORT_SolForward			PD4
	//#define	PORT_FND_G			PD5
	//#define	PORT_FND_B			PD6
	//#define	PORT_FND_F			PD7
	DDRD  = 0xFC;	// 0:input / 1:output [1111 1100]
	PORTD = 0xE0;	// 0:highz / 1:pullup [1110 0000]

	//timer0(8bit)
	TCCR0A = 0;
	TCCR0B = 0;
	OCR0A =  0;
	TIMSK0 = 0;

	//timer1(16bit)
	TCCR1A = 0;
	TCCR1B = 0;
	OCR1A =  0;
	TIMSK1 = 0;

	//timer2(8bit)
	TCCR2A = 0;
	TCCR2B = 0;
	OCR2A =  0;
	TIMSK2 = 0;

	//UART
	UBRR0H = 0;
	UBRR0L = 0;
	UCSR0A = 0;
	UCSR0B = 0;
	UCSR0C = 0;

	//[ADC]
//	ADMUX =  0;
//	ADCSRA = 0;
//	ADCSRB = 0;
}

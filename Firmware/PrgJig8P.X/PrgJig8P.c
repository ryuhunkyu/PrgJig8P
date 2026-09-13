/*--------------------------------------
Project:
	PrgJig8P Program for MCU Programming Jig
	By CapRhk 2026.08.28 (from Springkler)

	PCB:	v1.0:	260828 for cpu [atmega8,atmega48PA]
			Internal RC Oscillator 8M

Spec:
	1: 2 digit fnd
	2: 2 buttons
	3: 8Port x 7 Analog Switch
	4: Mouse click Emulation
	5: Beep
	6: Good LED

Operation:
	1 ISP signal Distribute for 8 Port
	Mouse Button Emulation
	Time Second Display
	3 Modes: MCU Programming, Stop/Move, Set Time
	3 LED Display:G,R,Y
	Finish Alarm Beep

ISP Signal:
	Pin/Name	ICSP		JTAG		ISP
	1:TVPP		MCLR/VPP	.		.
	2:TVDD		VDD		VTG		VTG
	3:GND		GND		GND		GND
	4:PGD		DAT		TDO		MISO
	5:PGC		CLK		TCK		SCK
	6:TAUX		.		-RESET		-RESET
	7:TTDI		.		TDI		MOSI
	8:TTMS		.		TMS		.
	
[scenario]
260910
	PCB v1.0

260913
	Button:
		Program Button: Program 시작, Time 값 증가
		Control Button: Mode 변경, Port 이동
		
	동작 mode:
	1 Single Program:	
		PrgMode LED(Green)을 켠다.
		Program 키가 눌리면 현재 Port를 프로그램 Signal에 연결하고 Mouse Click한다.
		Mouse Click후 정해진 시간(초) 만큼 기다리고 시간이 되면 현재 Port값을 1증가시키고 Beep.
		Port값이 8이 되면 다시 0을 만든다.
		정해진 시간 이내에 다시 Program 키가 눌리면 바로 Port값을 증가시키고 프로그램한다.
		Port LED는 현재 Port LED를 켠다.

	2 Multi Program:
		PrgMode LED(Green)을 깜박인다.
		Program 키가 눌리면 자동으로 8Port를 프로그램한다.
		Port가 8이 되면 0으로 만들고 Beep 하고 Program 키를 기다린다.
		Control 키가 짧게(3초 이하) 눌리면 정지하고 Stop/Move Mode로 변경한다.
		Control 키가 3초 이상 눌리면 SetTime Mode로 바꾼다.

	3 Stop/Move:
		Program Mode에서 Control 키를 눌러 Stop을 걸면 Port가 멈춘다.
		Mode LED를 Stop/Move LED(Red)로 켠다.
		이 상태에서 Control button이 눌리면 다음 port로 이동한다.
		Program 키가 눌리면 기존 Program Mode로 바꾸고 Program한다.
		Control 키가 3초 이상 눌리면 SetTime Mode로 바꾼다.

	4 Set Time:
		Mode LED를 SetTime LED(Yellow)로 켠다.
		Program 키를 누르면 Time 값을 1초씩 증가시킨다. 100이 되면 0으로 바꾼다. 
		Control 키를 짧게(3초 이하) 누르면 Time 값을 0으로 바꾼다.
		Control 키를 길게 누르면 기존 Program Mode로 바꾼다.
		Control 키를 한번 더 길게 누르면 single - Multi Program Mode로 바꾼다.
	
[version]
260910	dv
	기본 code를 준비.(기본 코드를 GPT에게 시키는 방법을 연구해야 함: 지금은 그냥 내가 하는 것이 빠를 것으로 보임)
	문제가 발생하는 기능은 codex를 이용하여 해결하려고 함.

260913
	Atmega48PA, 8MHz internal RC OSC, 1Khz timer, SPI mode, no WDT, no Brown-out,
	no ckout


	
[AI prompt]
Mplab x IDE 6.30 xc8 개발환경에서, embedded project, atmega48pa 8mhz를 c program으로 개발하고 있다.

--------------------------------------*/

#include "def.h"

FUSES = {
.low =		0xE2,
.high =		0xDF,
.extended =	0xFE,
};
LOCKBITS =	0xFF;	// {LB=ALL_LOCK, BLB0=NO_LOCK, BLB1=NO_LOCK}

/*
FUSES = {
.low =		0xE2,
.high =		0xCF,
.extended =	0xFF,
};

LOCKBITS = 0xFF; // {LB=NO_LOCK, BLB0=NO_LOCK, BLB1=NO_LOCK}

Extended Fuse Byte (0xFE)
Bit No. Name		Description			Default Value
0	SELFPRGEN	Self Programming Enable		1 (unprogrammed)

Fuse High Byte. (0xCF)
Bit No. Name (Reference)  Description						Default Value
7	RSTDISBL	  External Reset Disable				1 (unprogrammed)
6	DWEN 		  debugWIRE Enable					1 (unprogrammed)
5	SPIEN 		  Enable Serial Program and Data Downloading		0 (programmed, SPI prg EN
4	WDTON		  Watchdog Timer Always On 				1 (unprogrammed)
3	EESAVE		  EEPROM memory is preserved through the Chip Erase	1 (unprogrammed), EEPROM not reserved
2	BODLEVEL2	  Brown-out Detector trigger level 			1 (unprogrammed)
1	BODLEVEL1	  Brown-out Detector trigger level 			1 (unprogrammed)
0	BODLEVEL0	  Brown-out Detector trigger level 			1 (unprogrammed)

Fuse Low Byte (0xE2)
Bit No. Name(Reference)	Description				Default Value
7	CKDIV8		Divide clock by 8 			0 (programmed)
6	CKOUT		Clock output				1 (unprogrammed)
5	SUT1		Select start-up time			1 (unprogrammed)
4	SUT0		Select start-up time			0 (programmed)
3	CKSEL3		Select Clock source			0 (programmed)
2	CKSEL2		Select Clock source			0 (programmed)
1	CKSEL1		Select Clock source			1 (unprogrammed)
0	CKSEL0		Select Clock source			0 (programmed)

*/

void PrgStop(void)
{
	while(1)
	{
		if(Timer & 0x0040) Good(LED_ON);
		else Good(LED_OFF);
		wdt_reset();
	}
}

void CheckSystem(void)
{
	while(!TimeOut)			//timeout이 걸리면 Sleep
	{
		PowerDownMode();
	 	if(WDFlag)		//Sleep에서 WatchDog로 8초마다 깨어날 경우 처리하는 내용
		{
			ADMUX = (1<<REFS0) | (1<<REFS1) | adcChannel;	//ref:Internal 1.1V Voltage Reference
			sbi(ADCSRA, ADSC);				//Start Convertion

			ADCFlag = 0;
			WDFlag = 0;
			while(ADCSRA&(1<<ADSC)) ;

			if(ntc_adc_to_T(WindTemperVal) >= cfgCompare) break;
		}
	}
}

void Frame(void)
{
	CheckSystem();
	wdt_reset();
	while(!tToggle) ;
	tToggle = 0;
}

void WaitMili(i16 time)
{
	//Wait Delay time sec
	while(time--) Frame();
}

void StartUp(void)
{
	FNDTerm = TIMEOUT*1000;
	LEDTerm = TIMEOUT*1000;
	SendFND(Level);
}

void GetConfig(u8 level)
{
	--level;
	level <<= 2;
	
	cfgCompare = LevelPerfuse[level++];
	cfgDifference = LevelPerfuse[level++];
	cfgPerfuseTerm = LevelPerfuse[level++];
	cfgRestTerm = LevelPerfuse[level];
}

void GetButton(void)
{
	static u16 ButtonReleaseTerm = 0, ButtonPressCount = 0;

	if(KEY_BUTTON)	//pressed
	{
		if(!ButtonReleaseTerm && ButtonPressCount < 255) ++ButtonPressCount;
	}
	else
	{
		if(ButtonPressCount) ButtonReleaseTerm = 400;	//버튼 재입력 시간 설정
		ButtonPressCount = 0;
		if(ButtonReleaseTerm) --ButtonReleaseTerm;
	}

	if(ButtonPressCount == 250)
	{
		if(FNDTerm) ++Level;
		if(Level > 8) Level = 1;

		SaveData(EEP_LEVEL, (u16)Level);
		SendFND(Level);
		GetConfig(Level);

		FNDTerm = TIMEOUT*1000;
		LEDTerm = TIMEOUT*1000;
	}

//	ButtonFlag = 0;
}

void LightWork(void)
{
	if(!LEDTerm) Good(LED_OFF);
	else if(State == STATE_PERFUSE) Good(LED_ON);
	else if(PowerVal > POWER_GOOD)
	{
		if(Timer & 0x0400) Good(LED_ON);
		else Good(LED_OFF);
	}
	else if(PowerVal > POWER_POOR)
	{
		if(Timer & 0x0100) Good(LED_ON);
		else Good(LED_OFF);
	}
	else if(PowerVal > POWER_WEEK)
	{
		if((Timer & 0x0780) == 0x0080) Good(LED_ON);
		else Good(LED_OFF);
	}
	else
	{
		Good(LED_OFF);
	}

	if(!FNDTerm) SendFND(CHAR_NONE);
	else SendFND(Level);
}

void Initialize(void)
{
	SecondCount = 0;
	AS_SendFlag = 0;
	AS_Qwi = AS_Qri = 0;
	ButtonFlag = 0;
	WindCheckFlag = 0;
	WDFlag = 0;
	ADCFlag = 0;

	ButtonReleaseTerm = 0;
	ButtonPressCount = 0;
	ValveCommand = 0;
	ValveTerm = 0;

	State = STATE_IDLE;
	adcChannel = CHN_POWER;
	SerialNumber = SERIALNUMBER;
	TimeOut = TIMEOUT;		//3*60;	// 3 minute
}

int main(void)
{
	MCUState = MCUSR;
	MCUSR = 0;
	//MCUSR &= ~(1<<WDRF);			// WDT Reset 플래그 클리어
	OscState = OSCCAL;

	cli();
	wdt_disable();
	PRR = 0;

	#define	WDPS	(1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0)	//타임아웃 설정: ≒ 8초
	//[WDT] 설정을 위해 시퀀스 필요 (타이밍 제한)
	WDTCSR = (1<<WDCE) | (1<<WDE);			//변경 허용
	//WDIE=1, WDE=0  → Interrupt only
	WDTCSR = (1<<WDIE) | WDPS;

	Init();
	Initialize();

	Level = (u8)GetData(EEP_LEVEL);
	if(Level > 7 || !Level) Level = 1;

	GetConfig(Level);

	sei();
	WaitMili(250);

	AS_SendMyName();
	AS_SendSN();
	
	StartUp();

//	test();

	while(1)
	{
		Frame();

		GetButton();
		WaterWork();
		ValveWork();
		LightWork();
	}

	return 0;
}

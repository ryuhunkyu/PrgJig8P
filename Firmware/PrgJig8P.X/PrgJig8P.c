/*--------------------------------------
Project:
	PrgJig8P Program for MCU Programming Jig
	By CapRhk 2026.08.28 (from Springkler)

	PCB:	v1.0:	260828 for cpu [atmega8,atmega48PA]
			Internal RC Oscillator 8M

Spec:
	1: 1 digit fnd
	2: Temperature 2 Thermister
	3: Reed Switch(wind sense)
	4: 1 button
	5: 1 Solenoide
	6: VCC:3V
	7: Boost Up to +12V

Operation
	Battery 전원으로 작동한다.(AA 건전지 2개)
	온도에 따라 냉각수를 살수한다.
	온도 1이 level에 이르면 작동 준비한다.
	온도 2에 따라 냉각수를 살수한다
	Light Sense(Switch)는 바람이 부는지 죽었는지 알 수 있다.
	-> Reed Switch로 바람날개(철물+자석)의 움직임을 감지한다.

	LED Light Work:
		Power Good, Power Poor, Power Week, Water Work;

<scenario>
240416
	작동 강도(Level)를 7단계로 
	각 단계마다 세팅갑이 조금씩 바뀐다.
	세팅값은 외기 온도, 온도차, 살수시간(s:10sec), 휴지시간(s:600sec)
	동작 state: IDLE, Compare, refuse, rest
	*보조배터리문제:낮은 소비전력은 보조 배터리가 자동적으로 꺼지는 문제를 야기함.	

250722
	PCB v0.7
	AA전지 4개로 6V로 작동하는 latch형 Sol Valve를 테스트 했을 때 300회 작동시키면 전압 강하가 0.16V였다
	따라서 AA전지 2개로 줄이고 대신 boost를 붙여서 6~12V를 만들어 Sol Valve의 선택폭을 높인다.
	우리나라에서는 여름 한철(약 3개월) 작동하고 새 해에는 새로 전지를 교체하는 것으로 한다.
	
250805
	PCB v0.8
	소소하게 변경됨. 주 요인은 부품 배치문제였음. 기존 개발보드는 디자인상 보기가 불편한 뱌치.
	모든 콘넥터를 SMD로 바꾸었는데 Valve용은 20037 02pin으로 했으나 구하기 어려운 문제가 있음.
	->20022로 바꿀 예정

[version]
240405	dv
240525	v0.7->v0.6(버전 정리 조정)

250611	v0.6
250720	선태 demo용으로 서미스터(10K)가 45도 정도 값 이상이면 약 0.2초동안 solenoide 작동

250722	v0.7
	전원을 AA 전지 2개로 변경
	기존 OutSide temper 삭제하고 Wind temper와 내부 Temperature sensor 두개만으로 작동
	-MCU:Atmega88PA

250801:
	WindCheck Pin처리 문제.
	바람을 감지하는 날개의 움직임으로 Reed Switch가 On/Off되는데 
	명확한 int가 잘 걸리지 않거나 지속적으로 걸리는 문제가 있음.
	기존 입력포트와 high z상태에서 출력포트, low상태로 변경하여 정확한 int가 감지됨
	그러나 switch on시 vcc가 포트핀을 통해 과전류가 흐르므로 이 부분을 해결해야 함

250805	v0.8
	MCU:Atmega48PA

250812	power low로 인한 valve닫힘에 문제가 생길 경우를 대비하여 poor 이하에는 valve를 열지 않는다.

260805:	v0.9
	Vatos(이태원 식당:선태) 설치. Reed Switch 기능 삭제
	Battery 소비 전력은 솔밸브 On/Off 1회당 0.12mA 정도임. 하루8시간 작동 시 약 1개월 만에 소진된다.
	전력소비를 낮추기 위해 FND,LED display time을 줄인다.
	
[AI prompt]
Mplab x IDE 6.25 xc8 개발환경에서, embedded project, atmega48pa 8mhz를 c program으로 개발하고 있다.

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

//--------------------------------------
//Battery소비 전력 문제로 솔밸브 작동을 1분 마다 1회로 한정함
u8 LevelPerfuse[] =
{//	compare,	difference,	perfuseterm,	restterm
	62,		7,		10,		50,		//Level1
	59,		7,		15,		45,		//Level2
	56,		7,		20,		40,		//Level3
	52,		6,		25,		35,		//Level4
	48,		6,		30,		30,		//Level5
	44,		5,		35,		25,		//Level6
	40,		5,		40,		20,		//Level7

	27,		5,		5,		5,		//Level8
};

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

u16 GetPower(u16 power)
{//power(mV) = (ADC/1024)*1.1*6*1000
	return (((u32)power * ADC_REF * 6) / 1024);
}

//PowerVal 0x1E0 = 480
//WindTemperVal 0x210 = 528 = 3V:x = 528/1024*1.1 = 0.567V :실측 0.570V 
//Rth = Rlow * ( (VCC * 1023) / (Vref * ADC값) - 1 )

//주어진 두개의 ADC를 계산하여 Thermistor R값을 도출
//Rth = Rlow * ((VCC*1024)/(VRef*ADC) -1)
u16 GetThermistor(u16 power, u16 tempvolt)
{
	return (((((u32)power*1024) / (ADC_REF * tempvolt)) - 1) * RLOW_THERM);
}

//25'C에서 103(10Kohm), -10'C:55K, -20'C:100k, 10'C:18K, 20'C:11K, 30'C:8K, 40'C:5K, 50'C:3.5K, 60'C:2.3K, 70'c:1.7K, 80'C:1.2K, 90'C:0.9K, 100'C:0.7K, 110'C:0.5K

//10°C ~ 90°C, 1°C당 저항값(Ω) - 오차 ±7% 수준
const u16 NTC_Rtab[81] PROGMEM = 
{
	18365,17503,16687,15914,15182,14489,13832,13208,12617,12056,	// 10~19°C
	11524,11018,10538,10082,9648, 9236, 8844, 8471, 8116, 7779,	// 20~29°C
	7457, 7151, 6860, 6582, 6317, 6064, 5822, 5591, 5370, 5159,	// 30~39°C
	4961, 4769, 4586, 4411, 4243, 4082, 3927, 3778, 3634, 3495,	// 40~49°C
	3385, 3258, 3136, 3018, 2903, 2792, 2684, 2579, 2477, 2379,	// 50~59°C
	2283, 2191, 2102, 2016, 1933, 1853, 1776, 1701, 1629, 1559,	// 60~69°C
	1492, 1426, 1363, 1302, 1243, 1186, 1131, 1077, 1025, 975,	// 70~79°C
	926,  879,  834,  790,  748,  707,  668,  630,  594,  559,	// 80~89°C
	526								// 90?°C
};

// 실측 저항값 r_ohm(Ω) → 온도(°C, 정수) 반환
//   - 반환 범위: 10~90°C
//   - 테이블 경계 밖이면 포화값(10 or 90)을 돌려줌
i8 ntc_R_to_T(u32 r_ohm)
{
	// 하한·상한 먼저 처리
	if(r_ohm >= pgm_read_word(&NTC_Rtab[0]))	return 10;  // ≤10°C
	if(r_ohm <= pgm_read_word(&NTC_Rtab[80]))	return 90;  // ≥90°C
	
	// 이진 탐색: 저항이 내림차순이므로 비교 부등호 주의
	u8 lo = 0, hi = 80;
	while(lo < hi)
	{
		u8 mid = (lo + hi) >> 1;
		u16 r_mid = pgm_read_word(&NTC_Rtab[mid]);
		if(r_ohm > r_mid)	// 아직 더 낮은 °C 구간
			hi = mid;
		else			// 더 높은 °C 구간
			lo = mid + 1;
	}
	// lo == 원하는 구간의 하한 인덱스 → 실제 °C = 10 + lo
	return (i8)(10 + lo);
}

// 상수:  VREF=1.1V, VCC=3.0V, RLOW=1.0kΩ 예 */
// 10 ~ 90°C 정수 반환
i8 ntc_adc_to_T(u16 adc)
{
	if (adc == 0) return 90;	// 에러 가드
	//Vnode(mV) = adc * VREF / 1023
	u32 Vnode_mV = (u32)adc * ADC_REF / 1024;
	
	//Rth = Rlow * (VCC/Vnode - 1)  → 32bit 정수계산
	u32 Rth = (RLOW_THERM * (u32)GetPower(PowerVal) / Vnode_mV) - RLOW_THERM;
	
	return ntc_R_to_T(Rth);
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


void ValveWork(void)
{
	static u8 valvestate = 0xFF;

	if(!ValveTerm)
	{
		VALVE_STANDBY;
		VALVE_SLEEP ;
		BOOST_OFF;
	}

	if(ValveCommand == CMD_OPEN)
	{
		if(valvestate != CMD_OPEN)
		{
			valvestate = CMD_OPEN;
			ValveTerm = 100;

			BOOST_ON;
			VALVE_ENABLE;
		}
		else if(ValveTerm == 1)
		{
			if(!LEDTerm) LEDTerm = 250;
		}
		else if(ValveTerm < 90)
		{
			//250812: power low로 인한 valve닫힘에 문제가 생길 경우를 대비하여 poor 이하에는 valve를 열지 않는다.
			if(PowerVal > POWER_POOR)
			VALVE_OPEN;
		}
	}
	else if(ValveCommand == CMD_CLOSE)
	{
		if(valvestate != CMD_CLOSE)
		{
			valvestate = CMD_CLOSE;
			ValveTerm = 100;

			BOOST_ON;
			VALVE_ENABLE;
		}
		else if(ValveTerm < 90)
		{
			VALVE_CLOSE;
		}
	}
}

void WaterWork(void)
{
	static u16 temperstate = 0, windstate = 0;
	static u16 wind = 20, temper = 20;

	if(windstate != WindTemperVal)
	{
		windstate = WindTemperVal;
		wind = ntc_adc_to_T(WindTemperVal);
	}
	if(temperstate != TemperatureVal)
	{
		temperstate = TemperatureVal;
		temper = ntc_adc_to_T(TemperatureVal);
		if(temper > 40) temper = 40;	//설치 문제로 주변온도가 40도 이상으로 측정될 경우
	}

	if(temper > wind) temper = wind;

	switch(State)
	{
	case STATE_IDLE:
		//wind:토출구 온도, temper:주변온도
		if(wind >= cfgCompare && (wind - temper) >= cfgDifference)
		{
			State = STATE_PERFUSE;
			PerfuseTerm = cfgPerfuseTerm;
			ValveCommand = CMD_OPEN;
		}
		break;

	case STATE_COMPARE:
		if(wind < cfgCompare)
		{
			State = STATE_IDLE;
			break;
		}

		if(wind >= cfgCompare && (wind - temper) >= cfgDifference)
		{
			State = STATE_PERFUSE;
			PerfuseTerm = cfgPerfuseTerm;
			ValveCommand = CMD_OPEN;
		}
		TimeOut = TIMEOUT;//시간연장
		break;

	case STATE_PERFUSE:
		if(!PerfuseTerm)
		{
			State = STATE_REST;
			RestTerm = cfgRestTerm;
			ValveCommand = CMD_CLOSE;
		}
		TimeOut = TIMEOUT;//시간연장
		break;

	case STATE_REST:
		if(!RestTerm)
		{
			State = STATE_COMPARE;
		}
		TimeOut = TIMEOUT;//시간연장
		break;

	default:
		break;
	}
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

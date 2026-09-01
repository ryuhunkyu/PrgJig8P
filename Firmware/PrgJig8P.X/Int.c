//Interrupt Handler

#include "def.h"

ISR(TIMER0_COMPA_vect)	// signal handler for tcnt0 overflow interrupt
{
	// timer0(8bit) is 1ms timer
	++Timer;
	tToggle = 1;

	if(MiliTerm) --MiliTerm;
	if(ValveTerm) --ValveTerm;
	if(FNDTerm) --FNDTerm;
	if(LEDTerm) --LEDTerm;

	sei();
}

ISR(TIMER1_COMPA_vect)	// signal handler for tcnt1 overflow interrupt
{
	// timer1(16bit)는 1sec timer이다

	++SecondCount;
	if(PerfuseTerm) --PerfuseTerm;
	if(RestTerm) --RestTerm;
	if(TimeOut) --TimeOut;

	ADMUX = (1<<REFS0) | (1<<REFS1) | adcChannel;	//ref:Internal 1.1V Voltage Reference
	sbi(ADCSRA, ADSC);				//Start Convertion
}

//ISR(TIMER2_COMPA_vect)	// signal handler for tcnt2 overflow interrupt
//{
//
//}

ISR(WDT_vect)				//WDT 주기 인터럽트
{
	WDFlag = 1;
}

ISR(PCINT0_vect)			//PB4 Event
{
}

//WindCheck
//Button
ISR(PCINT1_vect)			//PC4 Event
{
//	if(WIND_CHECK)			//pin low: reed switch가 vcc와 떨어짐
//	{
//		sbi(WINDCHECK_DDR, PORT_WindCheck);	//pin outport mode:0을 출력해서 확실한 low로 만들고 다음 변화 때까지 noise제거
//
//		WindCheckFlag = 1;	//
//		TimeOut = TIMEOUT;	//3*60
//	}
//	else cbi(WINDCHECK_DDR, PORT_WindCheck);	//pin inport mode: reed switch가 붙으며 vcc와 연결되어 소비전류를 발생시키므로 high z로 만든다.

	if(KEY_BUTTON)
	{
//		ButtonFlag = 1;
		TimeOut = TIMEOUT;	//3*60
	}
}

ISR(ADC_vect)
{
	int adc;

	adc = ADCW;

	switch(adcChannel)
	{
	case CHN_POWER:
		PowerVal = adc - 20;	//WindGate저항에 따른 전압차 보정값
		adcChannel = CHN_TEMPERATURE;
		break;

	case CHN_TEMPERATURE:
		TemperatureVal = adc;
		adcChannel = CHN_TEMPER_WIND;
		break;

	case CHN_TEMPER_WIND:
		WindTemperVal = adc;
		adcChannel = CHN_POWER;
		break;

	default:
		adcChannel = CHN_POWER;
	}

	ADCFlag = 1;
}


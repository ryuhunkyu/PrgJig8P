#include "def.h"

#define	TESTFUNCTION		TEST_TEMPER

#define	TEST_GOOD		0
#define	TEST_VALVE		1
#define	TEST_TEMPER		2
#define	TEST_INT		3
#define	TEST_LIGHT		4
#define	TEST_TIMER		5
#define	TEST_POWER		6
#define	TEST_DUMMY		100



void test(void)
{
	u8 d = 0;
	u16 page = 0;
	temp = TESTFUNCTION;

	switch(temp)
	{
	case TEST_GOOD:

		while(1)
		{
			if(Timer&0x0100)
			{
				Good(LED_ON);
				SendFND(CHAR_NONE);
			}
			else
			{
				Good(LED_OFF);
				SendFND(0x08);
			}
			asm ( "wdr"::);
		}

		break;

	case TEST_VALVE:
		BOOST_ON;
		VALVE_ENABLE;
		while(1)
		{
			if((Timer&0x0FFF) == 0)
			{
				VALVE_OPEN;
				Good(LED_ON);
				
			}
			else if((Timer&0x0FFF) == 0x0800)
			{
				VALVE_CLOSE;
				Good(LED_OFF);
			}
			else if((Timer&0x00FF) == 0)
			{
				VALVE_STANDBY;
			}
			asm ( "wdr"::);
		}

		break;

	case TEST_TEMPER:

		while(1)
		{
			if(!(Timer&0x07FF))
			{
				AS_SendInt(PowerVal);
				AS_Send(':');
				AS_SendCount(GetPower(PowerVal));
				AS_Send(',');
				
				AS_SendInt(WindTemperVal);
				AS_Send(':');
				AS_SendCount(ntc_adc_to_T(WindTemperVal));
				AS_Send(',');

				AS_SendInt(TemperatureVal);
				AS_Send(':');
				AS_SendCount(ntc_adc_to_T(TemperatureVal));
				AS_SendCRLF();
			}
		
			if(ntc_adc_to_T(WindTemperVal) > 32 )Good(LED_ON);
			else Good(LED_OFF);

			wdt_reset();
			while(!tToggle) ;
			tToggle = 0;
		}
		break;
		
	case TEST_INT:
		while(1)
		{
			if(WDFlag)
			{
				SendFND(1);
			}
			else if(ButtonFlag)
			{
				SendFND(2);
			}
			else if(WindCheckFlag)
			{
				SendFND(3);
			}
			DelayMili(500);

			WDFlag = 0;
			ButtonFlag = 0;
			WindCheckFlag = 0;
			
			PowerDownMode();
		}

	case TEST_LIGHT:
		while(1)
		{
			State = STATE_COMPARE;
			MiliTerm = 5000;
			Level = 2;
			while(MiliTerm)
			{
				LightWork();
			}
			State = STATE_IDLE;
			MiliTerm = 10000;
			Level = 3;
			while(MiliTerm)
			{
				LightWork();
			}
		}
		
	case TEST_TIMER:
		while(1)
		{
			Frame();
			if(SecondCount&0x0004)
			{
				AS_SendInt(SecondCount);
				DelayMili(1000);
			}
		}
		break;

	case TEST_POWER:
		while(1)
		{
//			Frame();
			if(PowerVal > POWER_GOOD)
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

			wdt_reset();
			while(!tToggle) ;
			tToggle = 0;
		}
		break;

	case TEST_DUMMY:
		++d;
		++page;
		break;

	default:

	    break;
	}
}


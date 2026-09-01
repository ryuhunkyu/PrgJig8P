#define u8		unsigned char
#define	u16		unsigned int
#define	u32		unsigned long int
#define	i8		char
#define	i16		int
#define	i32		long int
#define	vu8		volatile unsigned char
#define	vu16		volatile unsigned int
#define	vi8		volatile char
#define	vi16		volatile int

//#include <avr/pgmspace.h>

u8	MCUState;
u8	OscState;
vu8	Command;
vu8	State;
u8	temp;
u8	Count;
u8	Stamp;
vu8	adcChannel;
u16	SerialNumber;

vu8	tToggle;
vu16	Timer;
vu16	TimerDigit;
vi16	SecondCount;
vi16	MiliCount;
vi16	MiliTerm;
vi16	WaitTerm;
vi8	Blink;
vi16	ButtonTime;
vu16	FNDTerm;
vu16	LEDTerm;
vi16	NumbCount;
i16	LEDCount;
vu8	ValveTerm;
vu8	ButtonFlag;
vu8	WindCheckFlag;
vu8	WDFlag;
vu8	ADCFlag;
vu8	TimeOut;

u8	Level;
vi8	PerfuseTerm;
vi8	RestTerm;
u8	ValveCommand;

u8	cfgCompare;
u8	cfgDifference;
u8	cfgPerfuseTerm;
u8	cfgRestTerm;
vi16	ReferenceTemper;

vi16	PowerSum;
vi16	PowerVal;
vi16	WindTemperSum;
vi16	WindTemperVal;
vi16	TemperatureSum;
vi16	TemperatureVal;

i16	ButtonReleaseTerm;
u8	ButtonPressCount;

//Uart
u8	AS_RecQue[32];
vu8	AS_SendFlag;
vu8	AS_Qwi;
vu8	AS_Qri;

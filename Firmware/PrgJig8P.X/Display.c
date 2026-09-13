//Display functions

#include "def.h"

u8 FNDSegment[] =
{	//	HGFE DCBA
0x3F,	//0:	0011 1111
0x06,	//1:	0000 0110
0x5B,	//2:	0101 1011
0x4F,	//3:	0100 1111
0x66,	//4:	0110 0110
0x6D,	//5:	0110 1101
0x7D,	//6:	0111 1101
0x07,	//7:	0000 0111
0x7F,	//8:	0111 1111
0x6F,	//9:	0110 1111

//0x0A
0x77,	//A:	0111 0111	A
0x7C,	//B:	0111 1100	b
0x39,	//C:	0011 1001	C
0x5E,	//D:	0101 1110	d
0x79,	//E:	0111 1001	E
0x71,	//F:	0111 0001	F
0x6F,	//G:	0110 1111	->G
0x76,	//H:	0111 0110	H
0x06,	//I:	0000 0110	|
0x0E,	//J:	0000 1110	J
0x75,	//K:	0111 0101	->K
0x38,	//L:	0011 1000	L
0x37,	//M:	0011 0111	->M
0x54,	//N:	0101 0100	n
0x5C,	//o:	0101 1100	o
0x73,	//P:	0111 0011	P
0x6B,	//Q:	0110 1011	->Q
0x50,	//r:	0101 0000	r
0x6D,	//S:	0110 1101	S
0x78,	//T:	0111 1000	t
0x1C,	//U:	0001 1100	u
0x62,	//V:	0110 0010	->V
0x3E,	//W:	0011 1110	->W
0x64,	//X:	0110 0100	->X
0x6E,	//Y:	0110 1110	->Y
0x5B,	//Z:	0101 1011	->Z

//0x24
0x01,	// :	0000 0001
0x02,	// :	0000 0010
0x04,	// :	0000 0100
0x08,	// :	0000 1000
0x10,	// :	0001 0000
0x20,	// :	0010 0000

//0x24+6
0x00,	// :	0000 0000

//Dash
0x40,	//-:	0100 0000
0x01,	//~:	0000 0001
0x08,	//_:	0000 1000


0x00,

0

};

/*
FND	Segment	name	Bit number
	 -	 A	 0
	| |	F B	5 1
	 -	 G 	 6
	| |	E C	4 2
	 -	 D	 3
	   .	   DP	   7
G:
	 -
	| |
	 -
	  |
	 -
K:
	 -
	|
	 -
	| |

M:
	 -
	| |

	| |

Q:
	 -
	| |
	 -

	 -
V:

	| |
	 _


W:

	| |

	| |
	 -
X:

	|
	 -
	  |

Y:

	| |
	 -
	  |
	 -
Z:
	 -
	  |
	 -
	|
	 -
*/

void InitFND(void)
{
	FND_SDI_LOW;

	for(u8 i = 0; i < 16; i++) FND_CLOCK;

	FND_LATCH1;
}

void SetFNDData(u8 data)
{
	u8 buf;

	buf = data;

	for(u8 i = 0; i < 8; i++)
	{
		if(buf&0x80) FND_SDI_HIGH; else FND_SDI_LOW;

		FND_CLOCK;

		buf <<= 1;
	}
//Latch 1~7은 call측에서 할 것.
}

//LED Driver TLC5925
//Bit	0   1   2   3   4   5   6   7   8    9    10   11   12   13   14   15
//	A   B   C   D   E   F   G   DP  UD   Col  D1   D2   D3   D4   x    x
//D1,D2,D3,D4는 low active signal이다. (TLC5925는 반전 출력이다)

void SetFND(u8 digit, u8 col, u8 data)
{
	//digit is buf bit 13,12,11,10 
	if(digit == 3) FND_SDI_LOW; else FND_SDI_HIGH; FND_CLOCK;
	if(digit == 2) FND_SDI_LOW; else FND_SDI_HIGH; FND_CLOCK;
	if(digit == 1) FND_SDI_LOW; else FND_SDI_HIGH; FND_CLOCK;
	if(digit == 0) FND_SDI_LOW; else FND_SDI_HIGH; FND_CLOCK;

	//bit 9 (dummy bit)
	FND_SDI_LOW; FND_CLOCK;

	//colon bit : bit 8 :시계 모드에서 colon을 1초 단위로 깜박인다.
	if(!digit && col) FND_SDI_HIGH; else FND_SDI_LOW; FND_CLOCK;

	//bit 7~0 
	SetFNDData(data);
}
	
void SetLatch(u8 latch)
{
	switch(latch)
	{
		case 0:	FND_LATCH1; break;
		case 1:	FND_LATCH2; break;
		case 2:	FND_LATCH3; break;
		case 3:	FND_LATCH4; break;
		case 4:	FND_LATCH5; break;
		case 5:	FND_LATCH6; break;
		case 6:	FND_LATCH7; break;
		default: break;
	}
}

u8 GetFNDSegment(u8 index)
{
	return (FNDSegment[index&0x7F]);
}

//refresh FND every frame (1ms term)
void RefreshFND(void)
{
	u8 digit = Tic & 0x03;
	u8 col = 0;

	for(u8 fi = 0; fi < 7; fi++)
	{
		if(fi == FND_BUF_CLOCK) col = Colon;
		else if(fi == FND_BUF_MESSAGE)
		{
			if(Work == WORK_NORMAL) col = Colon;
			else col = 0;
		}
		else col = 1;
		SetFND(digit, col, FNDSegBuf[fi][digit]);
		SetLatch(fi);
	}
}

void SetFNDBuf(u8 sel, u8 high, u8 low)
{
	FNDSegBuf[sel][3] = GetFNDSegment(low%10);
	FNDSegBuf[sel][2] = GetFNDSegment(low/10);
	FNDSegBuf[sel][1] = GetFNDSegment(high%10);
	FNDSegBuf[sel][0] = GetFNDSegment(high/10);
}

void ClearFNDBuf(u8 sel)
{
	FNDSegBuf[sel][3] = FNDSegment[CHAR_NONE];
	FNDSegBuf[sel][2] = FNDSegment[CHAR_NONE];
	FNDSegBuf[sel][1] = FNDSegment[CHAR_NONE];
	FNDSegBuf[sel][0] = FNDSegment[CHAR_NONE];
}

void SetClockSegment(void)
{
	FNDSegBuf[FND_BUF_CLOCK][3] = GetFNDSegment(T_Minute%10);
	FNDSegBuf[FND_BUF_CLOCK][2] = GetFNDSegment(T_Minute/10);
	FNDSegBuf[FND_BUF_CLOCK][1] = GetFNDSegment(T_Time%10);
	FNDSegBuf[FND_BUF_CLOCK][0] = GetFNDSegment(T_Time/10);
}

void SetReserve1Segment(void)
{
	FNDSegBuf[FND_BUF_RESERVE1][3] = GetFNDSegment(Reserve1Minute%10);
	FNDSegBuf[FND_BUF_RESERVE1][2] = GetFNDSegment(Reserve1Minute/10);
	FNDSegBuf[FND_BUF_RESERVE1][1] = GetFNDSegment(Reserve1Time%10);
	FNDSegBuf[FND_BUF_RESERVE1][0] = GetFNDSegment(Reserve1Time/10);
}

void SetReserve2Segment(void)
{
	FNDSegBuf[FND_BUF_RESERVE2][3] = GetFNDSegment(Reserve2Minute%10);
	FNDSegBuf[FND_BUF_RESERVE2][2] = GetFNDSegment(Reserve2Minute/10);
	FNDSegBuf[FND_BUF_RESERVE2][1] = GetFNDSegment(Reserve2Time%10);
	FNDSegBuf[FND_BUF_RESERVE2][0] = GetFNDSegment(Reserve2Time/10);
}

void SetReserve3Segment(void)
{
	FNDSegBuf[FND_BUF_RESERVE3][3] = GetFNDSegment(Reserve3Minute%10);
	FNDSegBuf[FND_BUF_RESERVE3][2] = GetFNDSegment(Reserve3Minute/10);
	FNDSegBuf[FND_BUF_RESERVE3][1] = GetFNDSegment(Reserve3Time%10);
	FNDSegBuf[FND_BUF_RESERVE3][0] = GetFNDSegment(Reserve3Time/10);
}

void SetReserve4Segment(void)
{
	FNDSegBuf[FND_BUF_RESERVE4][3] = GetFNDSegment(Reserve4Minute%10);
	FNDSegBuf[FND_BUF_RESERVE4][2] = GetFNDSegment(Reserve4Minute/10);
	FNDSegBuf[FND_BUF_RESERVE4][1] = GetFNDSegment(Reserve4Time%10);
	FNDSegBuf[FND_BUF_RESERVE4][0] = GetFNDSegment(Reserve4Time/10);
}

void SetMessageSegment(u8 state)
{
	switch(state)
	{
	case STATE_POWERSTART:
		Text('S', 'T', 'A', 'T');
		break;
	case STATE_WATCHDOG:
		Text('D', 'O', 'G', ' ');
		break;
	case STATE_BROWNOUT:
		Text('B', 'R', 'O', 'N');
		break;
	case STATE_SETTIME:
		Text('S', 'E', 'T', 'T');
		break;
	case STATE_MANUAL:
		Text('M', 'A', 'N', 'L');
		break;
	case STATE_RESERVE:
		Text('R', 'E', 'S', 'V');
		break;
	case STATE_ERROR:
		Text('E', 'R', 'O', 'R');
		break;
	default:
		break;
	}
}

void SetTermSegment(void)
{
	FNDSegBuf[FND_BUF_TERM][1] = GetFNDSegment(TermTime%10);
	FNDSegBuf[FND_BUF_TERM][0] = GetFNDSegment(TermTime/10);
}

void DisplayCount(i16 count)
{
	if(count > 9999)
	{
		count = 9999;
	}

	FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment(0);
	FNDSegBuf[FND_BUF_MESSAGE][1] = GetFNDSegment(0);
	FNDSegBuf[FND_BUF_MESSAGE][2] = GetFNDSegment(0);
	FNDSegBuf[FND_BUF_MESSAGE][3] = GetFNDSegment(0);

	if(count >= 1000)
	{
		FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment(count/1000);
	}

	if(count >= 100)
	{
		FNDSegBuf[FND_BUF_MESSAGE][1] = GetFNDSegment((count%1000)/100);
	}

	if(count >= 10)
	{
		FNDSegBuf[FND_BUF_MESSAGE][2] = GetFNDSegment((count%100)/10);
	}

	FNDSegBuf[FND_BUF_MESSAGE][3] = GetFNDSegment(count%10);
}

void SetColon(void)
{
	Colon = 1;
}

void ClearColon(void)
{
	Colon = 0;
}


void Text(u8 s1, u8 s2, u8 s3, u8 s4)
{
	FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment(CHAR_ALPHABET + s1-'A');
	FNDSegBuf[FND_BUF_MESSAGE][1] = GetFNDSegment(CHAR_ALPHABET + s2-'A');
	FNDSegBuf[FND_BUF_MESSAGE][2] = GetFNDSegment(CHAR_ALPHABET + s3-'A');
	FNDSegBuf[FND_BUF_MESSAGE][3] = GetFNDSegment(CHAR_ALPHABET + s4-'A');
}

void DigitAni(void)
{
	u8 d = ((Timer>>5) % 16);

	FNDSegBuf[FND_BUF_MESSAGE][0] = FNDSegment[CHAR_NONE];
	FNDSegBuf[FND_BUF_MESSAGE][1] = FNDSegment[CHAR_NONE];
	FNDSegBuf[FND_BUF_MESSAGE][2] = FNDSegment[CHAR_NONE];
	FNDSegBuf[FND_BUF_MESSAGE][3] = FNDSegment[CHAR_NONE];

	if     (d == 0)	FNDSegBuf[FND_BUF_MESSAGE][0] = FNDSegment[CHAR_DIGIT_ANI+0];
	else if(d == 1)	FNDSegBuf[FND_BUF_MESSAGE][1] = FNDSegment[CHAR_DIGIT_ANI+0];
	else if(d == 2)	FNDSegBuf[FND_BUF_MESSAGE][1] = FNDSegment[CHAR_DIGIT_ANI+0];
	else if(d == 3)	FNDSegBuf[FND_BUF_MESSAGE][2] = FNDSegment[CHAR_DIGIT_ANI+0];
	else if(d == 4)	FNDSegBuf[FND_BUF_MESSAGE][2] = FNDSegment[CHAR_DIGIT_ANI+0];
	else if(d == 5)	FNDSegBuf[FND_BUF_MESSAGE][3] = FNDSegment[CHAR_DIGIT_ANI+0];
	else if(d == 6)	FNDSegBuf[FND_BUF_MESSAGE][3] = FNDSegment[CHAR_DIGIT_ANI+1];
	else if(d == 7)	FNDSegBuf[FND_BUF_MESSAGE][3] = FNDSegment[CHAR_DIGIT_ANI+2];
	else if(d == 8)	FNDSegBuf[FND_BUF_MESSAGE][3] = FNDSegment[CHAR_DIGIT_ANI+3];
	else if(d == 9)	FNDSegBuf[FND_BUF_MESSAGE][2] = FNDSegment[CHAR_DIGIT_ANI+3];
	else if(d == 10)FNDSegBuf[FND_BUF_MESSAGE][2] = FNDSegment[CHAR_DIGIT_ANI+3];
	else if(d == 11)FNDSegBuf[FND_BUF_MESSAGE][1] = FNDSegment[CHAR_DIGIT_ANI+3];
	else if(d == 12)FNDSegBuf[FND_BUF_MESSAGE][1] = FNDSegment[CHAR_DIGIT_ANI+3];
	else if(d == 13)FNDSegBuf[FND_BUF_MESSAGE][0] = FNDSegment[CHAR_DIGIT_ANI+3];
	else if(d == 14)FNDSegBuf[FND_BUF_MESSAGE][0] = FNDSegment[CHAR_DIGIT_ANI+4];
	else if(d == 15)FNDSegBuf[FND_BUF_MESSAGE][0] = FNDSegment[CHAR_DIGIT_ANI+5];
}


//FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment(CHAR_ALPHABET + 'E'-'A');

//=================================================================
void DisplayHex(u16 hex)
{
	if(hex >= 0x1000) FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment((u8)((hex>>12) & 0x000F));
	if(hex >= 0x0100) FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment((u8)((hex>>8) & 0x000F));
	if(hex >= 0x0010) FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment((u8)((hex>>4) & 0x000F));
	                  FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment((u8)(hex) & 0x000F);
}

void DisplayTime(void)
{
	u8 time = T_Time, minute = T_Minute;

	if(time >= 10)
	{
		FNDSegBuf[FND_BUF_CLOCK][0] = GetFNDSegment(time/10);
	}
	FNDSegBuf[FND_BUF_CLOCK][1] = GetFNDSegment(time%10);

	FNDSegBuf[FND_BUF_CLOCK][2] = GetFNDSegment(minute%10);
	FNDSegBuf[FND_BUF_CLOCK][3] = GetFNDSegment(minute%10);

	if(T_Second&0x01) ClearColon();
	else SetColon();
}

void DisplayTimeSec(void)
{
	u8 minute = T_Minute, sec = T_Second;

	if(minute >= 10) FNDSegBuf[FND_BUF_MESSAGE][0] = GetFNDSegment(minute/10);
	else 		 FNDSegBuf[FND_BUF_MESSAGE][0] = FNDSegment[CHAR_NONE];
	FNDSegBuf[FND_BUF_MESSAGE][1] = GetFNDSegment(minute%10);

	FNDSegBuf[FND_BUF_MESSAGE][2] = GetFNDSegment(sec/10);
	FNDSegBuf[FND_BUF_MESSAGE][3] = GetFNDSegment(sec%10);
}

void DisplaySetTime(u8 time, u8 minute)
{
	if(time >= 10)
	{
		FNDSegBuf[FND_BUF_CLOCK][0] = GetFNDSegment(time/10);
	}
	FNDSegBuf[FND_BUF_CLOCK][1] = GetFNDSegment(time%10);

	FNDSegBuf[FND_BUF_CLOCK][2] = GetFNDSegment(minute%10);
	FNDSegBuf[FND_BUF_CLOCK][3] = GetFNDSegment(minute%10);

	SetColon();
}


#include "def.h"

u8 Segment[] =
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
0x77,	//A:	0111 0111
0x7C,	//B:	0111 1100
0x39,	//C:	0011 1001
0x5E,	//D:	0101 1110
0x79,	//E:	0111 1001
0x71,	//F:	0111 0001
0x6F,	//G:	0110 1111
0x76,	//H:	0111 0110
0x06,	//I:	0000 0110
0x0E,	//J:	0000 1110
0x76,	//K:	0111 0110
0x38,	//L:	0011 1000
0x49,	//M:	0100 1001
0x54,	//N:	0101 0100
0x5C,	//o:	0101 1100
0x73,	//P:	0111 0011
0x67,	//Q:	0110 0111
0x50,	//r:	0101 0000
0x6D,	//S:	0110 1101
0x78,	//T:	0111 1000
0x1C,	//U:	0011 1110
0x3E,	//V:	0001 1100
0x79,	//W:	0111 1001
0x76,	//X:	0111 0110
0x6E,	//Y:	0110 1110
0x59,	//Z:	0000 1001

//0x24
0x01,	// :	0000 0001
0x02,	// :	0000 0010
0x04,	// :	0000 0100
0x08,	// :	0000 1000
0x10,	// :	0001 0000
0x20,	// :	0010 0000

//Dash
0x40,	//-:	0100 0000

//0x24+7
0xFF,	//8.:	1111 1111

//0x24+8
0x00,	// :	0000 0000
0x00,
0x00,

0

};

void SendFND(u8 seg)
{
	u8 data;
	data = Segment[seg];

	if(data&0x01) cbi(PORTB, PORT_FND_A); else sbi(PORTB, PORT_FND_A);
	if(data&0x02) cbi(PORTD, PORT_FND_B); else sbi(PORTD, PORT_FND_B);
	if(data&0x04) cbi(PORTB, PORT_FND_C); else sbi(PORTB, PORT_FND_C);
	if(data&0x08) cbi(PORTB, PORT_FND_D); else sbi(PORTB, PORT_FND_D);
	if(data&0x10) cbi(PORTB, PORT_FND_E); else sbi(PORTB, PORT_FND_E);
	if(data&0x20) cbi(PORTD, PORT_FND_F); else sbi(PORTD, PORT_FND_F);
	if(data&0x40) cbi(PORTD, PORT_FND_G); else sbi(PORTD, PORT_FND_G);
}


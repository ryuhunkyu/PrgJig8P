#include "def.h"

ISR(USART_RX_vect)
{// signal handler for receive complete interrupt
	AS_RecQue[AS_Qwi++] = UDR0;
	AS_Qwi &= 0x1F;
}

ISR(USART_TX_vect)
{
	AS_SendFlag = 0;
}

ISR(USART_UDRE_vect)              // Tx Data Register Empty
{
	UCSR0B &= ~_BV(UDRIE0);   // 인터럽트 OFF
	AS_SendFlag = 0;
}

u8 MyName[] = SYSTEMNAME;

void AS_SendMyName(void)
{
	int i = 0;
	while(MyName[i])
	{
		AS_Send(MyName[i++]);
	}
	AS_Send('V');
	AS_Send((VERSION>>4)+'0');
	AS_Send('.');
	AS_Send((VERSION&0x0F)+'0');

	AS_Send(0x0D);
	AS_Send(0x0A);
}

void AS_SendSN(void)
{
	AS_Send((u8)(BUILDWEEK/0x100));
	AS_Send((u8)(BUILDWEEK));

	AS_Send(SerialNumber/0x100);
	AS_Send((u8)SerialNumber);
}

u16 AS_GetQri(void)
{
	return AS_Qri;
}

u16 AS_GetQwi(void)
{
	return AS_Qwi;
}

void AS_Send(u8 c)
{
	while(AS_SendFlag) ;

	UDR0 = c;
	AS_SendFlag = 1;
}

void AS_SendMessage(char *msg)
{
	while(*msg)
	{
		AS_Send(*msg++);
	}	
}

u8 Bin2HexCode(u8 bin)
{
	return (bin < 10 ? bin + '0' : bin + 'A' - 10);
}

void AS_SendByte(u8 val)
{
	AS_Send(Bin2HexCode((val>>4)&0x0F));
	AS_Send(Bin2HexCode(val&0x0F));
}

void AS_SendInt(u16 val)
{
	AS_Send(Bin2HexCode(val>>12));
	AS_Send(Bin2HexCode((val>>8)&0x0F));
	AS_Send(Bin2HexCode((val>>4)&0x0F));
	AS_Send(Bin2HexCode(val&0x0F));
}

void AS_SendLongInt(long val)
{
	AS_Send(Bin2HexCode((val>>28)&0x0F));
	AS_Send(Bin2HexCode((val>>24)&0x0F));
	AS_Send(Bin2HexCode((val>>20)&0x0F));
	AS_Send(Bin2HexCode((val>>16)&0x0F));

	AS_Send(Bin2HexCode((val>>12)&0x0F));
	AS_Send(Bin2HexCode((val>>8)&0x0F));
	AS_Send(Bin2HexCode((val>>4)&0x0F));
	AS_Send(Bin2HexCode(val&0x0F));
}

void AS_SendCRLF(void)
{
	AS_Send(0x0D);
	AS_Send(0x0A);
}

void AS_SendCount(int count)
{
	if(count > 9999)
	{
		count = 9999;
	}

	AS_Send((count/1000)+'0');
	AS_Send(((count%1000)/100)+'0');
	AS_Send(((count%100)/10)+'0');
	AS_Send((count%10)+'0');
}

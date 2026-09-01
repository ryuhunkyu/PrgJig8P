/*--------------------------------------
By Co Rhk 2021. 6. 11.

--------------------------------------*/
#include "def.h"

void uDelay(int d)		// call ret: 4 + 4 = 8 clk
{
//	while(--d) ++temp;
	asm("sbiw	r24, 0x01");
	asm("breq	.+14");
	asm("lds	r18, temp");

	asm("subi	r18, 0xFF");
	asm("sbiw	r24, 0x01");
	asm("brne	.-6");

	asm("sts	temp, r18");
}				// sum = 8 + 5 + 2 + [(d-1) * 5] - 1 => 14 + (d-1)*5
//Âü°í:uDelay(100) := 63uS	[14+(100-1)*5 => 509*125nS =63.625uS]

void DelayMili(int delay)
{
	MiliTerm = delay;
	while(MiliTerm) ;
}

//-----------------------------
//EEPRom Command

void EEPRomWrite(u16 addr, u8 data)
{
	// Wait for completion of previous write
	while(EECR & (1<<EEPE))	;
	// Set up address and Data Registers
	EEARL = addr;
	EEDR = data;
	// Write logical one to EEMPE
	EECR |= (1<<EEMPE);
	// Start eeprom write by setting EEPE
	EECR |= (1<<EEPE);
}

u8 EEPRomRead(u16 addr)
{
	// Wait for completion of previous write
	while(EECR & (1<<EEPE)) ;
	// Set up address register
	EEARL = addr;
	// Start eeprom read by writing EERE
	EECR |= (1<<EERE);
	// Return data from Data Register
	return EEDR;
}

void SaveData(u16 addr, u16 data)
{
	EEPRomWrite(addr + 0, (u8)data);
	EEPRomWrite(addr + 1, data>>8);
}

u8 GetData(u16 addr)
{
	return (EEPRomRead(addr) + (EEPRomRead(addr+1)*0x100));
}

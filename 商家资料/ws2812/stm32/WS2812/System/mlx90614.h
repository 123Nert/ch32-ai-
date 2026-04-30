#ifndef __MLX90614_H
#define __MLX90614_H
 
#include "iic.h"
#include "stm32f10x.h"                  // Device header
 
#define ACK	 0
#define	NACK 1  //不应答或否定的应答
#define SA				0x00 //从机地址，单个MLX90614时地址为0x00,多个时地址默认为0x5a
#define RAM_ACCESS		0x00 //RAM access command
#define EEPROM_ACCESS	0x20 //EEPROM access command
#define RAM_TOBJ1		0x07 //To1 address in the eeprom
 
void SMBus_Init(void);

u8 PEC_Calculation(u8 pec[]);
u16 SMBus_ReadMemory(u8 slaveAddress, u8 command);
float SMBus_ReadTemp(void);
 
#endif

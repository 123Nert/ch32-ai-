#include "mlx90614.h"
 
/*******************************************************************************
* 函数名: SMBus_Init
* 功能: SMBus初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SMBus_Init()
{
    MyI2C_Init();
}
/*******************************************************************************
 * 函数名: SMBus_ReadMemory
 * 功能: READ DATA FROM RAM/EEPROM  从RAM和EEPROM中读取数据
 * Input          : slaveAddress, command
 * Return         : Data
 * SMBus_ReadMemory(0x00, 0x07) 0x00 表示IIC设备的从地址 从0x07这个寄存器开始读取
*******************************************************************************/
u16 SMBus_ReadMemory(u8 slaveAddress, u8 command)
{
    u16 data;			// Data storage (DataH:DataL)
    u8 Pec;				// PEC byte storage
    u8 DataL=0;			// Low data byte storage
    u8 DataH=0;			// High data byte storage
    u8 arr[6];			// Buffer for the sent bytes
    u8 PecReg;			// Calculated PEC byte storage
    u8 ErrorCounter;	// Defines the number of the attempts for communication with MLX90614
 
    ErrorCounter=0x00;	// Initialising of ErrorCounter
	slaveAddress <<= 1;	//2-7位表示从机地址 从机地址左移一位，把读写位空出来
	
    do
    {
repeat:
        MyI2C_Stop();			    //If slave send NACK stop comunication
        --ErrorCounter;				    //Pre-decrement ErrorCounter
        if(!ErrorCounter) 			    //ErrorCounter=0?
        {
            break;					    //Yes,go out from do-while{}
        }
 
        MyI2C_Start();				//Start condition
				MyI2C_SendByte(slaveAddress);
        if(MyI2C_ReceiveAck())//Send SlaveAddress 最低位Wr=0表示接下来写命令
        {
            goto	repeat;			    //Repeat comunication again
        }
				MyI2C_SendByte(command);
        if(MyI2C_ReceiveAck())	    //Send command
        {
            goto	repeat;		    	//Repeat comunication again
        }
 
        MyI2C_Start();					//Repeated Start condition
				MyI2C_SendByte(slaveAddress+1);
        if(MyI2C_ReceiveAck())	//Send SlaveAddress 最低位Rd=1表示接下来读数据
        {
            goto	repeat;             	//Repeat comunication again
        }
 
        DataL = MyI2C_ReceiveByte();	//Read low data,master must send ACK
				MyI2C_SendAck(ACK);
        DataH = MyI2C_ReceiveByte(); //Read high data,master must send ACK
				MyI2C_SendAck(ACK);
        Pec = MyI2C_ReceiveByte();	//Read PEC byte, master must send NACK
				MyI2C_SendAck(NACK);
        MyI2C_Stop();				//Stop condition
 
        arr[5] = slaveAddress;		//
        arr[4] = command;			//
        arr[3] = slaveAddress+1;	//Load array arr
        arr[2] = DataL;				//
        arr[1] = DataH;				//
        arr[0] = 0;					//
        PecReg=PEC_Calculation(arr);//Calculate CRC 数据校验
    }
    while(PecReg != Pec);//If received and calculated CRC are equal go out from do-while{}
 
	data = (DataH<<8) | DataL;	//data=DataH:DataL
    return data;
}
 
/*******************************************************************************
* 函数名: PEC_calculation
* 功能 : 数据校验
* Input          : pec[]
* Output         : None
* Return         : pec[0]-this byte contains calculated crc value
*******************************************************************************/
u8 PEC_Calculation(u8 pec[])
{
    u8 	crc[6];//存放多项式
    u8	BitPosition=47;//存放所有数据最高位，6*8=48 最高位就是47位
    u8	shift;
    u8	i;
    u8	j;
    u8	temp;
 
    do
    {
        /*Load pattern value 0x00 00 00 00 01 07*/
        crc[5]=0;
        crc[4]=0;
        crc[3]=0;
        crc[2]=0;
        crc[1]=0x01;
        crc[0]=0x07;
 
        /*Set maximum bit position at 47 ( six bytes byte5...byte0,MSbit=47)*/
        BitPosition=47;
 
        /*Set shift position at 0*/
        shift=0;
 
        /*Find first "1" in the transmited message beginning from the MSByte byte5*/
        i=5;
        j=0;
        while((pec[i]&(0x80>>j))==0 && i>0)
        {
            BitPosition--;
            if(j<7)
            {
                j++;
            }
            else
            {
                j=0x00;
                i--;
            }
        }/*End of while */
 
        /*Get shift value for pattern value*/
        shift=BitPosition-8;
 
        /*Shift pattern value */
        while(shift)
        {
            for(i=5; i<0xFF; i--)
            {
                if((crc[i-1]&0x80) && (i>0))
                {
                    temp=1;
                }
                else
                {
                    temp=0;
                }
                crc[i]<<=1;
                crc[i]+=temp;
            }/*End of for*/
            shift--;
        }/*End of while*/
 
        /*Exclusive OR between pec and crc*/
        for(i=0; i<=5; i++)
        {
            pec[i] ^=crc[i];
        }/*End of for*/
    }
    while(BitPosition>8); /*End of do-while*/
 
    return pec[0];
}
 
 /*******************************************************************************
 * 函数名: SMBus_ReadTemp
 * 功能: 计算并返回温度值
 * Return         : SMBus_ReadMemory(0x00, 0x07)*0.02-273.15
*******************************************************************************/
float SMBus_ReadTemp(void)
{   
	float temp;
	temp = SMBus_ReadMemory(SA, RAM_ACCESS|RAM_TOBJ1)*0.02-273.15;
	return temp;
}

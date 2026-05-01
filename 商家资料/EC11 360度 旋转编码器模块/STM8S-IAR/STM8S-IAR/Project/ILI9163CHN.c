#include "ILI9163.h"
#include "SGP18T_ILI9163B_config.h"
#include "SGP18T_ILI9163B_ascii.h"


void LCD_SetPos(unsigned int x0,unsigned int x1,unsigned int y0,unsigned int y1);


/*********************** 汉字字模的数据结构定义 ******************************/
struct  typFNT_GB16                 /*汉字字模结构体*/
{
 unsigned char  Index[3];               /*汉字内码索引*/
 unsigned char   Msk[28];                        /*点阵码数据 */
};

/****************************************************************************
// 汉字字模表                                                          //
// 汉字库: 宋体16.dot,横向取模左高位,数据排列:从左到右从上到下   
// 宋体12;  此字体下对应的点阵为：宽x高=16x16 阴码
字模工具 PCtoLCD2002
/////////////////////////////////////////////////////////////////////////
**********************************************************************************/
 const struct  typFNT_GB16 codeGB_16[] =          /*数据表 */
{

  /*"深",0x00,0x00,0x27,0xFC,0x14,0x04,0x14,0xA4,0x81,0x10,0x42,0x08,0x40,0x40,0x10,0x40,
0x17,0xFC,0x20,0x40,0xE0,0xE0,0x21,0x50,0x22,0x48,0x2C,0x46,0x20,0x40,0x00,0x40,"深",0*/


"深",0x00,0x02,0x20,0x1F,0x08,0x12,0x14,0x18,0x13,0x18,0x14,0x17,0x00,0x00,0x00,0x10,
0xFE,0x00,0x04,0x88,0x90,0xE0,0xFE,0xE0,0x90,0x88,0x80,0x00,/*"深",0*/
  




};


void LCD_SetPos(unsigned int x0,unsigned int x1,unsigned int y0,unsigned int y1)
{
 ILI9163B_write_command(0x2A);      	        //Colulm addRSTs set
 ILI9163B_write_para8(0x00);			//行起始坐标高8位，始终为零
 ILI9163B_write_para8(x0);			//行起始坐标低8位
 ILI9163B_write_para8(0x00);			//行起始坐标高8位，始终为零
 ILI9163B_write_para8(x1);			//行终止坐标低8位

 ILI9163B_write_command(0x2B);      	        //Colulm addRSTs set
 ILI9163B_write_para8(0x00);			//列起始坐标高8位，始终为零
 ILI9163B_write_para8(y0);			//列起始坐标低8位
 ILI9163B_write_para8(0x00);			//列坐标起始低8位，始终为0
 ILI9163B_write_para8(y1);			//列终止坐标低8位

 ILI9163B_write_command(0x2C);      	        //GRAM接收MCU数据或命令
}

void PutGB1616(unsigned short x, unsigned short  y, unsigned char c[2], unsigned int fColor,unsigned int bColor)
{
 unsigned int i,j,k;
// LCD_SetPos(x,  x+16-1,y, y+14-1);
 for (k=0;k<56;k++) 
  { //64标示自建汉字库中的个数，循环查询内码
   if ((codeGB_16[k].Index[0]==c[0])&&(codeGB_16[k].Index[1]==c[1]))
   { 
    for(i=0;i<28;i++) 
     {
      unsigned short m=codeGB_16[k].Msk[i];
      for(j=0;j<8;j++) 
       {
	if((m&0x80)==0x80) 
        {
	 ILI9163B_write_para16(fColor);
	}
	else
        {
	 ILI9163B_write_para16(bColor);
	}
	m<<=1;
       } 
     }
   }  
  }	
}

void LCD_PutString(unsigned short x, unsigned short y, unsigned char *s, unsigned int fColor, unsigned int bColor)
{
 unsigned char l=0;
 while(*s)
 {
  if( *s < 0x80) 
  {
  // LCD_printc(x,y,*s,fColor,bColor);
   s++;
   l++;
  
  }
  else
  {
   PutGB1616(x+l*8,y,(unsigned char*)s,fColor,bColor);
   s+=2;
   l+=2;
  }
 }
}

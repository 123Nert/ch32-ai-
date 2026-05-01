#include "SGP18T_ILI9163B_figure.h"
#include "ILI9163.h"
//取模方式 水平扫描 从左到右 低位在前
void showimage(const unsigned char *p) //显示40*40 QQ图片
{
  int i; 
  unsigned char picH,picL;
  ILI9163B_display_full(WHITE); //清屏   
  ILI9163B_SetPos(40,40,40+39,40+39);		//坐标设置
  for(i=0;i<40*40;i++)
  {	
    picL=*(p+i*2);	//数据低位在前
    picH=*(p+i*2+1);				
    ILI9163B_write_para16(picH<<8|picL);  						
  }		
}
 /************************************版权申明********************************************
**                             广州大彩光电科技有限公司
**                             http://www.gz-dc.com
**-----------------------------------文件信息--------------------------------------------
** 文件名称:   ulitity.c
** 创建时间:   2012-06-1
** 文件说明:   单片机中断服务处理和其他服务函数
** 技术支持：  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com

** 修改日期: 2013-12-12
** 1) 完善注释
------------------------------------------------------------------------------------  */
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_process.h"
#include "ulitity.h"
extern uint16 idata  timer_tick_count;
extern uint16 idata  screen_id;
extern uint16 idata  current_screen_id; 
//曲线采样点
  code uint8 sine1[144]={
0xF5, 0xF2, 0xF0, 0xEC, 0xE9, 0xE6, 0xE2, 0xDE, 0xD9, 0xD5, 0xD0, 0xCB, 0xC6, 0xC1, 0xBB, 0xB6,
0xB0, 0xAA, 0xA4, 0x9E, 0x98, 0x92, 0x8C, 0x86, 0x80, 0x79, 0x73, 0x6D, 0x67, 0x61, 0x5B, 0x55,
0x4F, 0x49, 0x44, 0x3E, 0x39, 0x34, 0x2F, 0x2A, 0x26, 0x21, 0x1D, 0x19, 0x16, 0x13, 0x0F, 0x0D, 
0x0A, 0x08, 0x06, 0x04, 0x03, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 
0x0A, 0x0D, 0x0F, 0x13, 0x16, 0x19, 0x1D, 0x21, 0x26, 0x2A, 0x2F, 0x34, 0x39, 0x3E, 0x44, 0x49, 
0x4F, 0x55, 0x5B, 0x61, 0x67, 0x6D, 0x73, 0x79, 0x7F, 0x86, 0x8C, 0x92, 0x98, 0x9E, 0xA4, 0xAA, 
0xB0, 0xB6, 0xBB, 0xC1, 0xC6, 0xCB, 0xD0, 0xD5, 0xD9, 0xDE, 0xE2, 0xE6, 0xE9, 0xEC, 0xF0, 0xF2, 
0xF5, 0xF7, 0xF9, 0xFB, 0xFC, 0xFD, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFD, 0xFC, 0xFB, 0xF9, 0xF7, 
0xF5, 0xF2, 0xF0, 0xEC, 0xE9, 0xE6, 0xE2, 0xDE, 0xD9, 0xD5, 0xD0, 0xCB, 0xC6, 0xC1, 0xBB, 0xB6 
};

code uint8  sine2[144]={

0x4F, 0x49, 0x44, 0x3E, 0x39, 0x34, 0x2F, 0x2A, 0x26, 0x21, 0x1D, 0x19, 0x16, 0x13, 0x0F, 0x0D,	 
0x0A, 0x08, 0x06, 0x04, 0x03, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 
0x0A, 0x0D, 0x0F, 0x13, 0x16, 0x19, 0x1D, 0x21, 0x26, 0x2A, 0x2F, 0x34, 0x39, 0x3E, 0x44, 0x49, 
0x4F, 0x55, 0x5B, 0x61, 0x67, 0x6D, 0x73, 0x79, 0x7F, 0x86, 0x8C, 0x92, 0x98, 0x9E, 0xA4, 0xAA, 
0xB0, 0xB6, 0xBB, 0xC1, 0xC6, 0xCB, 0xD0, 0xD5, 0xD9, 0xDE, 0xE2, 0xE6, 0xE9, 0xEC, 0xF0, 0xF2, 
0xF5, 0xF7, 0xF9, 0xFB, 0xFC, 0xFD, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFD, 0xFC, 0xFB, 0xF9, 0xF7, 
0xF5, 0xF2, 0xF0, 0xEC, 0xE9, 0xE6, 0xE2, 0xDE, 0xD9, 0xD5, 0xD0, 0xCB, 0xC6, 0xC1, 0xBB, 0xB6 ,
0xF5, 0xF2, 0xF0, 0xEC, 0xE9, 0xE6, 0xE2, 0xDE, 0xD9, 0xD5, 0xD0, 0xCB, 0xC6, 0xC1, 0xBB, 0xB6,
0xB0, 0xAA, 0xA4, 0x9E, 0x98, 0x92, 0x8C, 0x86, 0x80, 0x79, 0x73, 0x6D, 0x67, 0x61, 0x5B, 0x55

};
 /********************************
*名    称： timer0
*功    能 ：定时器0中断服务程序	10ms
*入口参数： 无
*出口参数： 无
*********************************/

 void timer0(void) interrupt 1
{
 TH0=0xdc;
 TL0=0x32;//重新载入定时值
  timer_tick_count++;

}
 /********************************
*名    称： serial
*功    能 ： 串口中断服务程序
*入口参数： 无
*出口参数： 无
*********************************/

 void serial() interrupt 4 
{
	if(RI)//接收到串口数据
	{
		RI= 0;
		queue_push(SBUF);//接收串口的数据并压入到指令缓冲区
	}
}

/********************************
*名    称： systicket_init
*功    能 ：节拍定时器 10ms
*入口参数： 无
*出口参数： 无
*********************************/
void systicket_init(void)
{
 TMOD=0x9;
 TH0=0xdc;
 TL0=0x32;
 TR0=1;
 ET0=1;
}

/********************************
*名    称： CleanScreen
*功    能 ：清屏命令
*入口参数： 清屏颜色
*出口参数： 无
*********************************/

void CleanScreen(uint16 color)
{
    SetBcolor(color);
	GUI_CleanScreen();
}
/********************************
*名    称： UpDate
*功    能 ：更新页面上的文本控件
*入口参数： 清屏颜色
*出口参数： 无
*********************************/

void UpDate(void)
{
    uint8 dat[10];
    dat[0] = (timer_tick_count/10)%10+0x30;
    dat[1] ='.';
    dat[2] = timer_tick_count%10+0x31;
		GetScreen(); //获取当前的画面编号
    if(current_screen_id == 1) //文本页面
    {
        SetTextValue( 1, 6,dat);//当前电流（画面1控件6）
        SetTextValue( 1, 7,dat);//实时温度（画面1控件7）
        SetTextValue( 1, 8,dat);//实时湿度（画面1控件8）
    }
    else if(current_screen_id == 3) //曲线页面
	{
		// GraphChannelDataInsert(3,1,0);
	  //  for(i=0;i<6;i++)
		  GraphChannelDataInsert(3,1,0,sine1,128); //添加温度曲线的数据(128个点(画面3，控件1))
		  GraphChannelDataInsert(3,1,0,sine2,128); //添加温度曲线的数据(128个点(画面3，控件1))

    }
   else if(current_screen_id == 4)
   {
       SetMeterValue(4,1,timer_tick_count%100);      //仪表A转动数值（画面4控件1）
       SetMeterValue(4,2,timer_tick_count%100);      //仪表B转动数值 （画面4控件2）
   }
   else if(current_screen_id == 5)
   {
      SetProgressValue(5,1,(timer_tick_count%100)); //进度条数值（画面5控件1）
   }
}

void iniuserctr(void)
{
    /*修改文本框显示内容*/
    SetTextValue( 1, 1,"2");       // 显示机组号  （画面1中的文本控件1）
    SetTextValue( 1, 2,"250");     // 设置最高电压（画面1中的文本控件2）
    SetTextValue( 1, 3,"80");      // 设置最高告警（画面1中的文本控件3）
    SetTextValue( 1, 5,"125");     // 设置最高电压/2（画面1中的文本控件5）
    SetTextValue( 1, 6,"1.6");     // 显示当前电流（画面1中的文本控件6）
    SetTextValue( 1, 7,"40");      // 显示实时温度（画面1中的文本控件7）
    SetTextValue( 1, 8,"50");      // 显示实时湿度（画面1中的文本控件8）
    SetButtonValue(2,3,1);         // 把启动运行的图标变为按下去状态(画面2，控件3)
	AnimationPlayFrame(2, 1, 1);   // 图标控件显示运行状态（画面2，控件1，播放到第一帧）
}

/**************************************************************************************************************
* 名    称： delay_ms
* 功    能： 延时n毫秒
* 入口参数： n-延时时间
* 出口参数： 无
***************************************************************************************************************/
void delay_ms(uint32 delay)
{
    uint32 tick = timer_tick_count;
    while(1)
    {
        if(timer_tick_count-tick>delay/10)
            break;
    }
}
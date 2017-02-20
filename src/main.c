/************************************版权申明********************************************
**                             广州大彩光电科技有限公司
**                             http://www.gz-dc.com
**-----------------------------------文件信息--------------------------------------------
** 文件名称:   main.c
** 创建时间:   2012-06-1
** 文件说明:   主程序模块包含程序的基本处理流程，根据需要添加消息的响应处理。
** 技术支持：  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com

** 修改记录
** 日期: 2012-10-08
** 1) 指令队列queue_find_cmd添加帧头判断
** 2) 结构体CTRL_MSG指定按字节对齐，否则可能会出现对齐问题
** 3) 添加uint32基本类型定义，并修改相关指令解析
** 日期: 2013-12-12
** 1) 完善注释
** 2) 硬件平台 STC51单片机 STC89C52RC 外部晶振必须是22.1184M 波特率才能达到115200，如果是11.0592M的晶振
      必须将波特率改为57600
------------------------------------------------------------------------------------  */

#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_process.h"
#include "stdio.h"
#include "DefineID.c"
#include "main.h"
#include "ulitity.h"

 /*
 串口屏返回一条命令的最长长度，在RAM允许的情况下，尽可能设大一些，
 具体指令返回长度请参照指令手册（各系列的串口屏规格书中有附）串口屏能支持的一条命令最大长度是2048字节
*/

#define CMD_MAX_BUFFER 40  
uint8  idata cmd_buffer[CMD_MAX_BUFFER];

uint16 idata  screen_id;
uint16 idata  control_id;
uint16 idata  timer_tick_count;
uint16 idata current_screen_id; //当前屏幕处在的画面编号变量

/**************************************************************************************************************
* 名    称： main
* 功    能： 程序入口
***************************************************************************************************************/
int main(void)
{

    uint8 idata cmd_type = 0;
	uint8 idata msg_type = 0;
    uint8 idata control_type = 0;
	uint8 update_en = 1;
    qsize  size = 0;
    PCTRL_MSG idata msg = 0;
	  /*配置时钟节拍*/
    systicket_init();
	 /*串口初始化,波特率设置为115200,必须22.1184M 外部晶振支持，如果是11.0592M波特率请修改波特率为57600*/
    UartInit();
	 /*清空串口接收缓冲区*/
    queue_reset();
	delay_ms(300);
	/*初始化用户控件*/
    iniuserctr();
    while(1)
    {
        /*****************************************************************************
         100ms更新一次数据
        不能频繁更新串口屏的数据，不然会导致串口屏忙导致指令丢失(标准型串口屏有8K缓冲区，基本型有4.7K缓冲区)
        如果不能确定是否发送太频繁，请判断串口屏的BUSY引脚，为高时不能发送指令给串口屏
        ********************************************************************************/
	   
	    if(timer_tick_count%10 == 0 && update_en)
		{		  
			update_en = 0;
			UpDate();//更新数据（电压，电流，温度，湿度）
		}
		else if(timer_tick_count%10 == 1)
		{
		   update_en = 1;
		}
        size = queue_find_cmd(cmd_buffer,CMD_MAX_BUFFER); //从缓冲区中获取一条指令
        if(size<=0)//没有接收到指令
            continue;
        msg = (PCTRL_MSG)cmd_buffer;			

        cmd_type = msg->cmd_type;//指令类型
		 /*根据接收到的串口屏命令返回，转到对应的分支去处理事件*/
        switch(cmd_type)//指令类型
        {
        case CMD_TOUCH_PRESS_RETURN://按下触摸屏坐标上传
        case CMD_TOUCH_RELEASE_RETURN://松开触摸屏坐标上传
            OnRecvTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
            break;

        case CMD_TYPE_UPDATE_CONTROL://控件更新消息
			 msg_type = msg->ctrl_msg;
        switch(msg_type)//消息类型
        {   
			case kCtrlMsgGetCurrentScreen:					
				OnRecCurrentScreen(msg,size);// 获取当前画面值
				break;
			case kCtrlMsgGetData:
				control_type = msg->control_type;//控件类型
				switch(control_type)
				{
				case 	kCtrlMsgGetCurrentScreen:
					OnRecCurrentScreen(msg,size);// 更新当前画面值
				case kCtrlButton:
						OnRecvButton(msg,size);
						break; //按钮控件处理
				case kCtrlText:
						OnRecvText(msg,size);
						break; //文本控件处理
				case kCtrlProgress:
						OnRecvProgress(msg,size);
						break; //进度条控件处理
				case kCtrlSlider:
						OnRecvSlider(msg,size);
						break; //滑动条控件 处理
				case kCtrlMeter:
						OnRecvMeter(msg,size);
						break; //仪表控件处理
				case kCtrlAnimation:
						OnRecvAnimation(msg,size);
						break; //图标或者动画控件处理
				default:
						break;
				}
			 break;
          }
       }
    }
}

/**************************************************************************************************************
* 名    称： OnRecvChangeScreen
* 功    能： 画面切换消息响应函数
* 入口参数： screen_id-切换后的画面ID
***************************************************************************************************************/
void OnRecvChangeScreen(uint16 screen_id)
{
    //TODO: 用户代码
}

/**************************************************************************************************************
* 名    称： OnRecvTouchXY
* 功    能： 触摸坐标接收响应
* 入口参数： press-1按下触摸屏，3松开触摸屏
* 入口参数： x-触摸X坐标
* 入口参数： y-触摸y坐标
***************************************************************************************************************/
void OnRecvTouchXY(uint8 press,uint16 x,uint16 y)
{
    //TODO: 用户代码
}

/**************************************************************************************************************
* 名    称： OnRecvButton
* 功    能： 按钮消息响应函数
* 入口参数： msg-指令缓冲区，size-指令大小
***************************************************************************************************************/
uint8  ICON_run_status = 0;
uint8  ICON_fans_status = 0;
uint8  ICON_wind_status =0;
void OnRecvButton(PCTRL_MSG msg,qsize size)
{
    uint8  button_type = msg->param[0];   //0画面切换，1按钮，2虚拟按键
    uint8  value = msg->param[1];         //按钮数值
    screen_id = PTR2U16(&msg->screen_id); //画面ID
    control_id = PTR2U16(&msg->control_id);//控件ID
	  /**************************/
	  /*-- SCREEM 0的画面处理 --*/
	  /**************************/
    if(screen_id == 0 && control_id == 1)    //画面0 ID为1的按键被按下了
    {
		GetControlValue(1,2);                //获取最高电压文本控件的值	(画面1，控件2)
    }
	else if(screen_id== 0 && control_id == 3)//画面0 ID为3的按键被按下了
	{
		GraphChannelAdd(3,1,0,RED);           //添加曲线温度曲线通道 红色	(画面3，控件1，通道0)
	
    }
	 /**************************/
	  /*-- SCREEM 2的画面处理 --*/
	  /**************************/
    else if(screen_id == 2 && control_id == 3)//画面2 ID为3的按键被按下了
    {
		ShowControl( 2, 2, VISABLE);            //画面2 ID2的动画控件显示
        AnimationPlayFrame(2, 1, 1);            //画面2 ID1的图标控件播放到第一帧
	    SetButtonValue(2,4,0);                  //把停止运行的图标变为弹起来状态
        AnimationStart(2,2);	 				 //开始播放吹风的动画(画面2，控件2)
    }
   else if(screen_id == 2 && control_id == 4 )//画面2 ID为4的按键被按下了
	{
	     ShowControl( 2, 2, VISABLE);         //画面2 ID2的动画控件显示
	      AnimationPlayFrame(2, 1, 0);        //画面2 ID1的图标控件播放到第0帧
		  SetButtonValue( 2, 3, 0);           //把启动运行的图标变为弹起来状态
		  AnimationStop(2,2);		          //停止播放吹风的动画	(画面2，控件2)
    }
 	 else if(screen_id == 2 && control_id == 6)//画面2 ID为6的按键被按下了
    {
		SetButtonValue(2,4,0);                  //把停止运行的图标变为弹起来状态(画面2，控件4)
		SetButtonValue(2,3,0);                  //把启动运行运行的图标变为弹起来状态(画面2，控件4)
		ShowControl( 2, 2,UNVISABLE);           //动画控件隐藏
        AnimationPlayFrame(2, 1, 2);            //图标文件播放到第三帧（透明帧，即隐藏）
    }

} 

/**************************************************************************************************************
* 名    称： OnRecvText
* 功    能： 文本控件消息响应函数
* 入口参数： msg-指令缓冲区，size-指令大小
***************************************************************************************************************/
void OnRecvText(PCTRL_MSG msg,qsize size)
{
	uint8 buf[25];
	uint16 voltage=0;
	uint16 i=0;
    screen_id = PTR2U16(&msg->screen_id);  //画面ID
    control_id = PTR2U16(&msg->control_id);//控件ID

    //TODO: 用户代码

	//获取系统自带键盘输入
	if(screen_id == 1 &&  control_id == 2)
	{	
		while(((uint8 *)(&msg->param))[i])
        voltage = voltage*10+ (((uint8 *)(&msg->param))[i++] - 0x30);//从接收缓冲区取出一个字节数据，再转换成十进制数字
		//显示到最高电压/2
		voltage  = 		voltage/2;
		//只做到支持3位数
		buf[0] = voltage/100 +0x30;			     	  //百位
		buf[1] = (voltage- voltage/100*100)/10 +0x30; //十位
		buf[2] = (voltage)%10+0X30;					  //个位
        buf[3] = 0;
		if(buf[0] == 0x30 && buf[1] == 0x30)         //百位和十位都为0
	   {
		 buf[0] = buf[2];
		 buf[1] = 0;
		 buf[2] = 0;
	   }
	   else if(buf[0] == 0x30)						//百位为0
	  {
		buf[0] = buf[1];
		buf[1] = buf[2];
		buf[2] = 0;
	  }
		SetTextValue( 1, 5,buf); //显示到最高电压/2文本控件(画面1，控件5)
  }
}

/**************************************************************************************************************
* 名    称： OnRecvProgress
* 功    能： 进度条消息响应函数
* 入口参数： msg-指令缓冲区，size-指令大小
***************************************************************************************************************/
void OnRecvProgress(PCTRL_MSG msg,qsize size)
{
    uint16 value = PTR2U32(msg->param);    //数值
    screen_id = PTR2U16(&msg->screen_id);  //画面ID
    control_id = PTR2U16(&msg->control_id);//控件ID
    //TODO: 用户代码
}

/**************************************************************************************************************
* 名    称： OnRecvSlider
* 功    能： 滑动条消息响应函数
* 入口参数： msg-指令缓冲区，size-指令大小
***************************************************************************************************************/
void OnRecvSlider(PCTRL_MSG msg,qsize size)
{
    uint16 value = PTR2U32(msg->param);  //数值
    screen_id = PTR2U16(&msg->screen_id);//画面ID
    control_id = PTR2U16(&msg->control_id);//控件ID
    //TODO: 用户代码
	if(screen_id ==5 &&  control_id == 2)
   {
     SetBackLight((value*2)%255);//调节背光
   }
}

/**************************************************************************************************************
* 名    称： OnRecvMeter
* 功    能： 仪表消息响应函数
* 入口参数： msg-指令缓冲区，size-指令大小
***************************************************************************************************************/
void OnRecvMeter(PCTRL_MSG msg,qsize size)
{
    uint16 screen_id = PTR2U16(&msg->screen_id); //画面ID
    uint16 control_id = PTR2U16(&msg->control_id);//控件ID
    uint16 value = PTR2U32(msg->param);           //数值

    //TODO: 用户代码
}
 /**************************************************************************************************************
* 名    称： OnRecvMeter
* 功    能： 图标控件响应函数
* 入口参数： msg-指令缓冲区，size-指令大小
***************************************************************************************************************/
void OnRecvAnimation(PCTRL_MSG msg,qsize size)
{
    uint16 screen_id = PTR2U16(&msg->screen_id);  //画面ID
    uint16 control_id = PTR2U16(&msg->control_id);//控件ID
    uint16 value = PTR2U32(msg->param);           
	
    //TODO: 用户代码
}
/**************************************************************************************************************
* 名    称： OnRecCurrentScreen
* 功    能： 画面状态响应函数
* 入口参数： msg-指令缓冲区，size-指令大小
***************************************************************************************************************/

void	OnRecCurrentScreen(PCTRL_MSG msg,qsize size)
{
	 current_screen_id = PTR2U16(&msg->screen_id);
}
/************************************版权申明********************************************
**                             广州大彩光电科技有限公司
**                             http://www.gz-dc.com
**-----------------------------------文件信息--------------------------------------------
** 文件名称:   cmd_queue.c
** 修改时间:   2011-05-18
** 文件说明:   用户MCU串口驱动函数库
** 技术支持：  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
**--------------------------------------------------------------------------------------*/


#include "cmd_queue.h"

typedef struct _QUEUE
{
    qsize _head;
    qsize _tail;
    qdata _data[QUEUE_MAX_SIZE];
} QUEUE;//串口数据接收队列结构体

static QUEUE que = {0,0,0};
static qdata cmd_state = 0;
static qsize cmd_pos = 0;

/********************************
*名    称： queue_reset
*功    能    串口接收队列复位
*入口参数： 无
*出口参数： 无
*********************************/
void queue_reset()
{
    que._head = que._tail = 0;
    cmd_pos = cmd_state = 0;
}

/**********************************************
*名    称： queue_push
*功    能   存入一个数据到串口接收队列（FIFO）中
*入口参数： 无
*出口参数： 无
**********************************************/
void queue_push(qdata _data)
{
    qsize pos = (que._head+1)%QUEUE_MAX_SIZE;
    if(pos!=que._tail)//非满状态
    {
        que._data[que._head] = _data;
        que._head = pos;
    }
}
/**********************************************
*名    称： queue_pop
*功    能   从串口接收队列(FIFO)中取出一个字节数据
*入口参数： 无
*出口参数： 无
**********************************************/
static void queue_pop(qdata* _data)
{
    if(que._tail!=que._head)//非空状态
    {
        *_data = que._data[que._tail];
        que._tail = (que._tail+1)%QUEUE_MAX_SIZE;
    }
}
/**********************************************
*名    称： queue_size
*功    能   获取串口接收队列(FIFO)的长度
*入口参数： 无
*出口参数： 无
**********************************************/
static qsize queue_size()
{
    return ((que._head+QUEUE_MAX_SIZE-que._tail)%QUEUE_MAX_SIZE);
}

/******************************************************************
*名    称： queue_find_cmd
*功    能   获取串口接收队列(FIFO)中一个完整的帧
            一个完整的帧是以EE开头，FF FC FF FF 结尾的一个数据帧
*入口参数： 无
*出口参数： 无
********************************************************************/
qsize queue_find_cmd(qdata *buffer,qsize buf_len)
{
    qsize cmd_size = 0;
    qdata _data = 0;
    while(queue_size()>0)
    {
        //取一个数据
        queue_pop(&_data);

        if(cmd_pos==0&&_data!=0xEE)//帧头出错，跳过
            continue;

        if(cmd_pos<buf_len)//防止缓冲区溢出
            buffer[cmd_pos++] = _data;

        //判断帧尾
        if(_data==0xFF)
        {
            switch (cmd_state)
            {
            case 2:
                cmd_state = 3;
                break; //FF FC FF ?? (最后一个字节不对)
            case 3:
                cmd_state = 4;
                break; //FF FC FF FF 正确的帧尾
            default:
                cmd_state = 1;
                break; //FF ?? ?? ??(最后三个字节不对)
            }
        }
        else if(_data==0xFC)
        {
            switch (cmd_state)
            {
            case 1:
                cmd_state = 2;
                break; //FF FC ?? ??(最后二个字节不对)
            case 3:
                cmd_state = 2;
                break; //FF FC FF FC 正确的帧尾
            default:
                cmd_state = 0;
                break; //?? ?? ?? ??(全部字节不对)
            }
        }
        else
            cmd_state = 0;

        //得到完整的帧尾
        if(cmd_state==4)
        {
            cmd_size = cmd_pos;
            cmd_state = 0;
            cmd_pos = 0;
            return cmd_size;
        }
    }

    return 0;//没有形成完整的一帧
}

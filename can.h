#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include "pthread.h"
#include "socket.h"
#include <iconv.h>

//can config pormeater
#define CAN0_NAME       "can0"
#define CAN0_BITRATE     500000
#define FILTER0_NUM      1

#define CAN1_NAME       "can1"
#define CAN1_BITRATE     500000
#define FILTER1_NUM      1

#define CPU_CAN_ID 0x4f


UINT8 Rce04aCanInit(unsigned int Bitrate, char *CanName, struct can_filter *CanFilter, unsigned int FilterNum);

int CanReadData(char *name, struct can_frame *Frame);

int CanSendData(char *name,struct can_frame *Frame);

void Rce04aCanReadMsgPoll(void);

void* Rce04aCanMsgThread(void* arg);

void TimeStampToDate(INT64 time_stamp,CHAR *date,UINT16 length);

UINT8 UnpackePeriodMsgFromCAN(UINT8 *receive_buffer,UINT16 receive_length);

/*************************************************************************
 * 功能描述: 日期转换为时间戳
 * 输入参数:  CHAR           date
 *          UINT16         length
 * 输出参数: 无
 * 返回值:  long 时间戳
 *************************************************************************/
long DateToTimeStamp(CHAR *date);

/*************************************************************************
 * 功能描述: 根据两点经纬度计算距离
 * 输入参数: double              lat_last       上周期经度
 *          double              lng_last       上周期纬度
 *          double              lat            经度
 *          double              lng            纬度
 * 输出参数: 无
 * 返回值:  double 距离 m
 *************************************************************************/
double GetDistanceByPoint(double lat_last,double lng_last,double lat,double lng);

/*************************************************************************
 * 功能描述: 更新当前公里标
 * 输出参数: 无
 * 返回值:  double 距离 m
 *************************************************************************/
UINT8 GetCurrentDistance();

/*************************************************************************
 * 功能描述: 更新当前计划，当前站，下一
 * 输出参数: 无
 * 返回值:  double 距离 m
 *************************************************************************/
void GetCurrentPlan();

/*************************************************************************
  * 功能描述: 将4字节数据流变为UINT32车组号
  * 输入参数: input 输入
  * 输出参数: 无
  * 返回值:   车组号
  *************************************************************************/
UINT32 TrainNumberFromChar(const UINT8 *input);

/*************************************************************************
  * 功能描述: 将6字节数据流变为UINT32车次号
  * 输入参数: input 输入
  * 输出参数: 无
  * 返回值:   车次号
  *************************************************************************/
UINT32 TrainIDFromChar(const UINT8 *input);

/*************************************************************************
  * 功能描述: 从Byte中提取Bit
  * 输入参数: input 输入
  * 输出参数: 无
  * 返回值:  无
  *************************************************************************/
void TrainBitDataFromChar(const UINT8 input);

/*************************************************************************
  * 功能描述: 从数据流中提取时间
  * 输入参数: input 输入
  * 输出参数: 时间
  * 返回值:  无
  *************************************************************************/
void GetDateFromChar(const UINT8 *input,CHAR *date);

/*************************************************************************
 * 功能描述: GBK转化为UTF-8
 * 输入参数:
 * char         *inbuff         输入
 * int          inlen           输入长度
 * char         *outbuff        输出
 * int          outlen          输出长度
 * 输出参数: 无
 * 返回值:   0：异常 1：正常
 *************************************************************************/
int CodeConvertGBKToUTF8(char* input, size_t charInPutLen, char* output, size_t charOutPutLen);

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "define.h"
#include "init.h"
#include "SpeedPlan.h"
#include <arpa/inet.h>

#define PORT 8888
#define MAX_NUM_CLIENT 5

extern INIT_MSG_TO_APP g_init_msg_to_app;
extern PERIOD_MSG_TO_APP g_period_msg_to_app;
extern PERIOD_MSG_FROM_TRAIN g_period_msg_from_train;
extern PERIOD_MSG_FROM_SIGNAL g_period_msg_from_signal;
extern char g_current_time[20];
extern UINT8 g_serve_app;
extern UINT8 g_direction;
extern UINT8 g_serve_app_send;

int socket_init();
void *socket_manager();
void *server_handle(void * arg);

/*************************************************************************
  * 功能描述: 将2字节数据流变为UINT16
  * 输入参数: input 输入
  * 输出参数: 无
  * 返回值:   UINT16数据
  *************************************************************************/
UINT16 ShortFromChar(const UINT8 *input);

/*************************************************************************
 * 功能描述: 将4字节数据流变为UINT32
 * 输入参数: input 输入
 * 输出参数: 无
 * 返回值:   UINT32数据
 *************************************************************************/
UINT32 LongFromChar(const UINT8 *input);

/*************************************************************************
 * 功能描述: 将UINT16变为2字节数据流
 * 输入参数: input  UINT16数据
 * 输出参数: output 2字节数组
 * 返回值:   无
 *************************************************************************/
void ShortToChar(UINT16 input, UINT8 *output);

/*************************************************************************
 * 功能描述: 将UINT32变为4字节数据流
 * 输入参数: input  UINT32数据
 * 输出参数: output 4字节数组
 * 返回值:   无
 *************************************************************************/
void LongToChar(UINT32 input, UINT8 *output);

/*************************************************************************
 * 功能描述: 解包来自车辆网络的消息
 * 输入参数: UINT8  *receive_buffer  消息存储指针
 *          UINT16 receive_length  消息长度
 * 输出参数: 无
 * 返回值:   UINT8   result          1：解析成功 0：解析失败
 *************************************************************************/
UINT8 UnpackePeriodMsgFromTrainNet(UINT8 *receive_buffer,UINT16 receive_length);

/*************************************************************************
 * 功能描述: 解包来自信号系统消息
 * 输入参数: UINT8  *receive_buffer  消息存储指针
 *          UINT16 receive_length  消息长度
 * 输出参数: 无
 * 返回值:   UINT8   result          1：解析成功 0：解析失败
 *************************************************************************/
UINT8 UnpackePeriodMsgFromSignal(UINT8 *receive_buffer,UINT16 receive_length);

/*************************************************************************
 * 功能描述: 解包来自APP消息
 * 输入参数: UINT8  *receive_buffer  消息存储指针
 *          UINT16 receive_length   消息长度
 * 输出参数: 无
 * 返回值:   UINT8   result          1：解析成功 0：解析失败
 *************************************************************************/
UINT8 UnpackMsgFromAPP(UINT8 *receive_buffer,UINT16 receive_length);

/*************************************************************************
 * 功能描述: 打包发送给APP的初始化消息
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
UINT16 PackInitMsgToAPP(UINT8 *send_buffer);

/*************************************************************************
 * 功能描述: 打包发送给APP的结束服务消息
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
UINT16 PackEndMsgToAPP(UINT8 *send_buffer);

/*************************************************************************
 * 功能描述: 打包发送给APP的周期消息
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
UINT16 PackPeriodMsgToAPP(UINT8 *send_buffer);

/*************************************************************************
 * 功能描述: 发送给APP消息管理
 * 输入参数:
 * 输出参数: 无
 * 返回值:
 *************************************************************************/
void SendAPPMessageManage(int client_fd);

/*************************************************************************
 * 功能描述: 更新发送给APP的周期数据
 * 输入参数: 无
 * 输出参数: 无
 * 返回值: 无
 *************************************************************************/
void RefreshPeriodMsgToAPP();

/*************************************************************************
 * 功能描述: 打包发送给Signal的周期消息
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
UINT16 PackPeriodMsgToSignal(UINT8 *send_buffer);
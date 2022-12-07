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


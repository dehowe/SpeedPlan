#pragma once

#include "stdio.h"
#include "define.h"
#include "SpeedPlan.h"


/*************************************************************************
 * 功能描述: 读取CSV静态数据文件
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   UINT8   1:读取成功 0：读取失败
 *************************************************************************/
UINT8 StaticDataRead();

/*************************************************************************
 * 功能描述: 初始化线路静态数据、列车静态数据
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   UINT8   1:读取成功 0：读取失败
 *************************************************************************/
UINT8 StaticDataInit();

/*************************************************************************
 * 功能描述: 根据模型计算能耗
 * 输入参数: UINT16          speed           速度km/h
 *          UINT8           level_flag      级位标识 1：牵引 4：制动
 *          UINT8           level_output    级位输出0-100
 *          UINT32          distance        走行距离
 * 输出参数: 无
 * 返回值:  UINT32 焦耳
 *************************************************************************/
UINT32 CalEnergyByMode(UINT16 speed,UINT8 level_flag,UINT8 level_output,UINT32 distance);

/*************************************************************************
 * 功能描述: 根据电压电流计算能耗
 * 输入参数: UINT32          voltage           电压v
 *          UINT32          current           电流A
 *          FLOAT32         cycle_time        周期s
 * 输出参数: 无
 * 返回值:  UINT32 焦耳
 *************************************************************************/
UINT32 CalEnergyByUI(UINT32 voltage,UINT32 current,FLOAT32 cycle_time);

/*************************************************************************
 * 功能描述: 根据车头车尾位置计算限速，两者取最小
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   FLOAT32   等效坡度 千分之一
 *************************************************************************/
UINT16 GetSpeedLimit(UINT32 train_head_loc,UINT32 train_tail_loc);

/*************************************************************************
 * 功能描述: 初始化白名单设备MAC地址
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   UINT8   1:初始化成功 0：初始化失败
 *************************************************************************/
UINT8 DeviceMacDataInit();
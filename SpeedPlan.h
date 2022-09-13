#pragma once
#include "define.h"
#include <stdlib.h>
#include <math.h>
#include "log.h"
#include "socket.h"

extern TRAIN_PARAMETER				g_train_param;						// 列车参数
extern UINT16						g_aw_id;							// 载荷
extern LINE_PARAMETER               g_line_param;                       // 线路参数
extern STATIC_DATA_CSV              g_static_data_csv;                  // CSV静态数据
extern SPEED_PLAN_INFO              g_speed_plan_info;                  // 速度规划信息
extern UINT16                      g_speed_curve_offline[MAX_SPEED_CURVE];  //离线优化速度存储数组
extern UINT16                      g_level_output_flag[MAX_SPEED_CURVE];    //离线优化级位存储数组
extern FLOAT32                     g_plan_time[MAX_SPEED_CURVE];            //离线优化运行时分存储数组
/*************************************************************************
* 功能描述: 速度规划主程序
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void SpeedPlanMain();

/*************************************************************************
* 功能描述: 离线求解算法入口
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void *SpeedPlanOffline();

/*************************************************************************
* 功能描述: 数据预处理，整理区间长度、限速、坡度、曲线半径等信息
* 输入参数: 无
* 输出参数: 无
* 返回值:   0:查询失败 1:查询成功
*************************************************************************/
UINT8 GetBaseDataReady();

/*************************************************************************
* 功能描述: 离线计算下一区间优化速度
* 输入参数:
*		UINT16						discrete_size			距离离散大小
*		UINT32						section_length			区间长度cm
*		UINT32						speed_limit_location[]	限速切换点位置
*		UINT16						speed_limit[]			对应切换点限速值
*		UINT16						speed_limit_length		限速切换点个数
*		UINT16						target_time				区间目标运行时分
*		UINT16						solution_num			解空间大小
* 输出参数:
*		UINT16*						speed_curve				速度曲线
*		UINT16*						speed_limit_max			最快速度曲线限制
*		UINT16*						speed_limit_min			最慢速度曲线限制
*		UINT16*						speed_limit_mmax		顶棚速度曲线限制
* 返回值:
*		UINT16						dim						解空间维度
*************************************************************************/
UINT16 GetOptimalSpeedOffline(UINT16* speed_curve, UINT16 discrete_size, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16* speed_limit_mmax, UINT32 section_length, UINT32 speed_limit_location[], UINT16 speed_limit[], UINT16 speed_limit_length, UINT16 target_time, UINT16 solution_num);

/*************************************************************************
* 功能描述: 建模
* 输入参数:
*		UINT16						discrete_size			距离离散大小
*		UINT16						dim						解空间维度
*		UINT16						solution_num			解空间大小
*		UINT32						speed_limit_loc[]		线路限速-对应位置
*		UINT16						speed_limit[]			线路限速-对应限速值
*		UINT16						speed_limit_length		线路限速存储数组大小
*		UINT16						target_time				目标运行时分
* 输出参数:
*		UINT16*						speed_limit_max			最快速度曲线
*		UINT16*						speed_limit_min			最慢速度曲线
*		UINT16*						speed_limit_mmax		顶棚限速曲线
* 返回值:
*************************************************************************/
void Modeling(UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16* speed_limit_mmax, UINT16 discrete_size, UINT16 dim,
              UINT16 solution_num, UINT32 speed_limit_loc[], UINT16 speed_limit[], UINT16 speed_limit_length, UINT16 target_time);

/*************************************************************************
* 功能描述: 根据区间限速计算防护速度
* 输入参数:
*		UINT16						speed_begin				反推开始速度
*		UINT16						speed_end				结束速度
*		UINT16						index					起始索引
*		UINT16						discrete_size			距离离散大小
* 输出参数:
*		UINT16*						speed_limit				防护速度
* 返回值:
*************************************************************************/
void GetEBI(UINT16 speed_begin, UINT16 speed_end, UINT16 index, UINT16* speed_limit, UINT16 discrete_size);

/*************************************************************************
* 功能描述: 计算防护速度曲线终点的顶棚速度
* 输入参数:
*		UINT16						ebi_begin				反推开始速度
*		UINT16						index					起始索引
*		UINT16*						speed_limit				限速
* 输出参数:
* 返回值:
*		UINT16						v_end					顶棚速度
*************************************************************************/
UINT16 GetEbiEnd(UINT16 ebi_begin, UINT16* speed_limit, UINT16 index);

/*************************************************************************
* 功能描述: 灰狼优化算法求解最优工况转换点(离线求解)
* 输入参数:
*		UINT16*						speed_limit_max			最快速度曲线
*		UINT16*						speed_limit_min			最慢速度曲线
*		UINT16						dim						解空间维度
*		UINT16						target_time				目标运行时分
*		UINT16						discrete_size			距离离散大小
* 输出参数:
*		UINT16*						optimal_speed			优化速度
* 返回值:
*************************************************************************/
void GWO_Offline(UINT16* optimal_speed, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16 dim, UINT16 target_time,
                 UINT16 discrete_size, UINT32* lower_bound, UINT32* upper_bound, UINT8* switch_flag, UINT8 solve_dim);

/*************************************************************************
* 功能描述: 根据参数上下界，初始化狼群位置
* 输入参数:
*		UINT16						wolves_num				狼群数量
*		UINT8						solve_dim				解维度
*		UINT32						upper_bound[]			约束上界
*		UINT32						lower_bound[]			约束下界
* 输出参数:
* 返回值:
*		vector<vector<FLOAT32>>		positions				初始解空间
*************************************************************************/
void Initialize(UINT16 wolves_num, UINT8 solve_dim, UINT32 upper_bound[], UINT32 lower_bound[], FLOAT32** Positions);

/*************************************************************************
* 功能描述: 计算适应度函数(离线求解)
* 输入参数:
*		vector<FLOAT32>				position				解空间
*		UINT16						discrete_size			距离离散大小
*		UINT16						dim						解空间维度
*		UINT16*						speed_limit_max			最快速度曲线
*		UINT16*						speed_limit_min			最慢速度曲线
*		UINT16						target_time				目标运行时分
* 输出参数:
*		UINT16*						optimal_speed			优化速度
* 返回值:
*		FLOAT32						fitness					目标函数值
*************************************************************************/
FLOAT32 GetFitnessOffline2(UINT16* optimal_speed, FLOAT32* position, UINT16 discrete_size, UINT16 dim, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16 target_time, UINT8 switch_flag[], UINT8 switch_num);

/*************************************************************************
* 功能描述: 计算列车质量
* 输入参数: 无
* 输出参数: 无
* 返回值:   FLOAT64	train_weight	列车质量, kg
*************************************************************************/
FLOAT64 CalTrainWeight();

/*************************************************************************
* 功能描述: 根据ATO配置数据中的特性曲线计算某一速度下的最大加速度
* 输入参数: FLOAT64	speed		列车速度, km/h
*			  UINT16    num			特性曲线散点数
*			  UINT16*	curve_v		曲线速度项数组, km/h
*			  UINT16*	curve_a		曲线加速度项数组, cm/s/s
* 输出参数: 无
* 返回值:   FLOAT64	acc			最大加速度, cm/s/s
*************************************************************************/
FLOAT64 GetAccBySpeed(FLOAT64 kph, UINT16 num, const UINT16* curve_v, const UINT16* curve_a);

/*************************************************************************
* 功能描述: 根据速度计算牵引加速度
* 输入参数:
*		UINT16						speed					速度 cm/s
* 输出参数:
* 返回值:
*		UINT16						acc_traction		    最大加速度 cm/s^2
*************************************************************************/
FLOAT32 GetTractionAcc(UINT16 speed);

/*************************************************************************
* 功能描述: 根据速度计算制动加速度
* 输入参数:
*		UINT16						speed					速度 cm/s
* 输出参数:
* 返回值:
*		UINT16						acc_break				最大减速度 cm/s^2
*************************************************************************/
FLOAT32 GetBreakAcc(UINT16 speed);

/*************************************************************************
* 功能描述: 根据速度计算基本阻力附加加速度
* 输入参数:
*		UINT16						speed					速度 cm/s
* 输出参数:
* 返回值:
*		FLOAT64						acc_basic_resistance	减速度 cm/s^2
*************************************************************************/
FLOAT64 GetResistanceAcc(UINT16 speed);

/*************************************************************************
* 功能描述: 根据位置计算坡度附加加速度
* 输入参数:
*		UINT32						dispalcement			位移 cm
* 输出参数: 无
* 返回值:
*		FLOAT32						gradient_acc			加速度 cm/s^2
*************************************************************************/
FLOAT32 GetGradientAcc(UINT32 dispalcement);

/*************************************************************************
* 功能描述: 根据位置计算曲线半径附加加速度
* 输入参数:
*		UINT32						dispalcement			位移 cm
* 输出参数: 无
* 返回值:
*		FLOAT32						curve_radius_acc		加速度 cm/s^2
*************************************************************************/
FLOAT32 GetCurveRadiusAcc(UINT32 dispalcement);

/*************************************************************************
* 功能描述: 根据列车当前位置，设置离线优化的上下边界
* 输入参数:
* 输出参数:
*		UINT32*						lower_bound				约束下界
*		UINT32*						upper_bound				约束上界
*		UINT8*						switch_flag				工况切换标识
* 返回值:
*		UINT8												解维度
*************************************************************************/
UINT8 GetBounderOffline(UINT32* lower_bound, UINT32* upper_bound, UINT8* switch_flag);

/*************************************************************************
* 功能描述: 根据优化速度曲线划分阶段
* 输入参数:
* 输出参数:
* 返回值:
*	UINT8  1：正常 0：异常
*************************************************************************/
UINT8 DivideStageByOptimizeSpeed();

/*************************************************************************
* 功能描述: 根据列车当前位置，查询下一阶段推荐速度、推荐工况和生效倒计时
* 输入参数:
*        UINT32         distance          当前公里标
* 输出参数:
*        UINT16          *rec_speed       下一阶段推荐速度
*        UINT8           *rec_work         下一阶段推荐工况
*        UINT16          *rec_cutdown     下一阶段生效倒计时
*        UINT32          *rec_distance    下一阶段生效距离
* 返回值:
*		   UINT8  1：正常 0：异常
*************************************************************************/
UINT8 GetRecSpdAndWorkByDis(UINT32 distance,UINT16 *rec_speed,UINT8 *rec_work,UINT16 *rec_cutdown,UINT32 *rec_distance);



/*************************************************************************
* 功能描述: 根据列车当前位置，查询推荐速度、级位标志、级位输出（实验室验证环节需要）
* 输入参数:
* 输出参数:
* UINT16                    *target_speed        推荐速度cm/s
* UINT8                     *level_flag          级位标识 1：牵引 2：惰行 3：制动
* 返回值:
*************************************************************************/
void GetTargetSpeedByDistance(UINT16 *target_speed,UINT8 *level_flag);




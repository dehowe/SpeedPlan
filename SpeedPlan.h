#pragma once
#include "define.h"
#include <stdlib.h>
#include <math.h>

extern TRAIN_PARAMETER				g_train_param;						// 列车参数
extern UINT16						g_aw_id;							// 载荷
extern LINE_PARAMETER              g_line_param;                       // 线路参数

void SpeedPlanOffline();

UINT8 GetBaseDataReady();

UINT16 GetOptimalSpeedOffline(UINT16* speed_curve, UINT16 discrete_size, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16* speed_limit_mmax, UINT32 section_length, UINT32 speed_limit_location[], UINT16 speed_limit[], UINT16 speed_limit_length, UINT16 target_time, UINT16 solution_num);

void Modeling(UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16* speed_limit_mmax, UINT16 discrete_size, UINT16 dim,
	UINT16 solution_num, UINT32 speed_limit_loc[], UINT16 speed_limit[], UINT16 speed_limit_length, UINT16 target_time);

void GetEBI(UINT16 speed_begin, UINT16 speed_end, UINT16 index, UINT16* speed_limit, UINT16 discrete_size);

UINT16 GetEbiEnd(UINT16 ebi_begin, UINT16* speed_limit, UINT16 index);

void GWO_Offline(UINT16* optimal_speed, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16 dim, UINT16 target_time,
	UINT16 discrete_size, UINT32* lower_bound, UINT32* upper_bound, UINT8* switch_flag, UINT8 solve_dim);

void Initialize(UINT16 wolves_num, UINT8 solve_dim, UINT32 upper_bound[], UINT32 lower_bound[], FLOAT32** Positions);

FLOAT32 GetFitnessOffline2(UINT16* optimal_speed, FLOAT32* position, UINT16 discrete_size, UINT16 dim, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16 target_time, UINT8 switch_flag[], UINT8 switch_num);

FLOAT64 CalTrainWeight();

FLOAT64 GetAccBySpeed(FLOAT64 kph, UINT16 num, const UINT16* curve_v, const UINT16* curve_a);

FLOAT32 GetTractionAcc(UINT16 speed);

FLOAT32 GetBreakAcc(UINT16 speed);

FLOAT64 GetResistanceAcc(UINT16 speed);

FLOAT32 GetGradientAcc(UINT32 dispalcement);

FLOAT32 GetCurveRadiusAcc(UINT32 dispalcement);

UINT8 GetBounderOffline(UINT32* lower_bound, UINT32* upper_bound, UINT8* switch_flag);









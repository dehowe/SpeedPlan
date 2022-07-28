#pragma once

/*基本数据类型定义*/
typedef char                CHAR;
typedef unsigned char       UCHAR;
typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef short int           INT16;
typedef unsigned short int  UINT16;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef float               FLOAT32;
typedef double              FLOAT64;

/*速度规划*/
#define SAFETY_THRESHOLD_EBI		138				// 推荐速度与EBI的最小差值(5km/h)
#define GRAVITY_ACC					9.8				// 重力加速度 9.8m/s^2
#define MAX_SPEED_CURVE				3000			// 优化曲线的最大存储数量
#define MAX_INTERVAL_SAMPLING		5000			// 区间线路数据的最大采样点个数
#define MAX_LIMIT_POINT				30				// 区间线路限速切换点的最大个数
#define SPEED_PLAN_TB_RATIO			0.6				// 速度规划牵引制动输出比率
#define MAX_AW_NUM					4				// 列车载荷细分数量（AW0|AW1|AW2|AW3）
#define MAX_TBCURVE_NUM				200				// 最大的牵引制动特性曲线数量

/*位置结构体*/
typedef struct 
{
	UINT16 link_id;										/*Link id*/
	UINT32 link_offset;									/*Link offset*/
	UINT8  dir;											/*方向*/
}TRAIN_LOACTION_STRU;

/*列车参数结构体*/
typedef struct
{
	/*基本参数*/
	UINT32  train_length;                   /*列车长度, cm*/
	UINT16  init_v;                         /*初始速度, cm/s*/
	UINT16  init_a;                         /*初始加速度, cm/s/s*/
	UINT16  aw[MAX_AW_NUM];                 /*载荷, 吨, aw[0-3]分别为aw0 aw1 aw2(预留) aw3*/
	FLOAT64 a[MAX_AW_NUM];                  /*不同载荷下的阻力系数a*/
	FLOAT64 b[MAX_AW_NUM];                  /*不同载荷下的阻力系数b*/
	FLOAT64 c[MAX_AW_NUM];                  /*不同载荷下的阻力系数c*/
	/*牵引制动特性曲线参数*/
	UINT16  traction_num;                   /*牵引特性曲线散点数量*/
	UINT16  traction_v[MAX_TBCURVE_NUM];    /*牵引特性曲线速度项，km/h*/
	UINT16  traction_a0[MAX_TBCURVE_NUM];   /*牵引特性曲线加速度项-AW0, cm/s/s*/
	UINT16  traction_a1[MAX_TBCURVE_NUM];   /*牵引特性曲线加速度项-AW1, cm/s/s*/
	UINT16  traction_a2[MAX_TBCURVE_NUM];   /*牵引特性曲线加速度项-AW2, cm/s/s*/
	UINT16  traction_a3[MAX_TBCURVE_NUM];   /*牵引特性曲线加速度项-AW3, cm/s/s*/
	UINT16  braking_num;                    /*制动特性曲线散点数量*/
	UINT16  braking_v[MAX_TBCURVE_NUM];     /*制动特性曲线速度项，km/h*/
	UINT16  braking_a0[MAX_TBCURVE_NUM];    /*制动特性曲线加速度项-AW0, cm/s/s*/
	UINT16  braking_a1[MAX_TBCURVE_NUM];    /*制动特性曲线加速度项-AW1, cm/s/s*/
	UINT16  braking_a2[MAX_TBCURVE_NUM];    /*制动特性曲线加速度项-AW2, cm/s/s*/
	UINT16  braking_a3[MAX_TBCURVE_NUM];    /*制动特性曲线加速度项-AW3, cm/s/s*/
	/*紧急制动率*/
	UINT16  eb;                             /*紧急制动率，cm/s/s*/
	/*延时参数*/
	FLOAT64 t_delay_10;                     /*牵引时，从牵引指令发出到列车加速度达到目标加速度的10%的延时，s*/
	FLOAT64 t_delay_90;                     /*牵引时，从牵引指令发出到列车加速度达到目标加速度的90%的延时，s*/
	FLOAT64 tr_delay_10;                    /*切除牵引时，从牵引切除指令发出，到列车加速度降到原加速度的10%的延时，s*/
	FLOAT64 tr_delay_90;                    /*切除牵引时，从牵引切除指令发出，到列车加速度降到原加速度的90%的延时，s*/
	FLOAT64 sb_delay_10;                    /*常用制动时，从常用制动指令发出，到列车制动率达到目标制动率的10%的延时，s*/
	FLOAT64 sb_delay_90;                    /*常用制动时，从常用制动指令发出，到列车制动率达到目标制动率的90%的延时，s*/
	FLOAT64 eb_delay_10;                    /*紧急制动时，从紧急制动指令发出，到列车制动率达到目标制动率的10%的延时，s*/
	FLOAT64 eb_delay_90;                    /*紧急制动时，从紧急制动指令发出，到列车制动率达到目标制动率的90%的延时，s*/
}TRAIN_PARAMETER;

/*线路数据结构体*/
typedef struct
{
	UINT32 interval_length;					/*区间长度 m*/
	UINT16 limit[MAX_INTERVAL_SAMPLING];	/*按照1m离散的线路限速, cm/s*/
	FLOAT32 gradient[MAX_INTERVAL_SAMPLING]; /*按照1m离散线路坡度，‰*/
	UINT32 curve_radius[MAX_INTERVAL_SAMPLING];/*按照1m离散线路曲线半径*/
}LINE_PARAMETER;
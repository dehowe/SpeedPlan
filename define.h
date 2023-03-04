#pragma once
#include <string.h>
#include "stdio.h"
#define _XOPEN_SOURCE
#include <time.h>
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
typedef long int            INT64;
/*速度规划*/
#define SAFETY_THRESHOLD_EBI		138				// 推荐速度与EBI的最小差值(5km/h)
#define GRAVITY_ACC					9.8				// 重力加速度 9.8m/s^2
#define MAX_SPEED_CURVE				1000			// 优化曲线的最大存储数量
#define MAX_INTERVAL_SAMPLING		6000			// 区间线路数据的最大采样点个数
#define MAX_LIMIT_POINT				30				// 区间线路限速切换点的最大个数
#define SPEED_PLAN_TB_RATIO			0.8				// 速度规划牵引制动输出比率
#define MAX_AW_NUM					4				// 列车载荷细分数量（AW0|AW1|AW2|AW3）
#define MAX_TBCURVE_NUM				200				// 最大的牵引制动特性曲线数量
#define MAX_STATION_NUM             30              // 最大车站数量
#define MAX_SPEED_LIMIT_CSV_NUM     100             // 最大线路限速数量
#define MAX_GRADIENT_CSV_NUM        100             // 最大坡度数量
#define MAX_CURVE_RADIUS_CSV_NUM    200             // 最大曲线半径数量
#define MAX_TUNNEL_CSV_NUM          100             // 最大桥梁隧道数量
#define MAX_BASIC_PARAM_CSV_NUM     10              // 最大列车基础参数数量
#define MAX_DYNAMICS_CSV_NUM        200             // 最大列从动力学特性数量
#define MAX_SEPARATE_CSV_NUM        10              // 最大分相区数量
#define MAX_SINGAL_CSV_NUM          100             // 最大信号机数量
#define MAX_TEMPORARY_LIMIT_NUM     10              // 最大临时限速数量
#define MAX_OPTIMIZE_CSV_NUM        5000            // 最大离线优化曲线数量
#define DIRECTION_UP                1               // 上行方向
#define DIRECTION_DOWN              0               // 下行方向
#define MAX_DEVICE_NUM              50              // 白名单设备最大数量
#define PI                          3.1415926       // pai
#define EARTH_RADIUS                6378.137        // 地球半径
#define PERIOD                      0.2             // 计算周期 s
#define SPEED_MODE                  0               // 速度模式
#define LEVEL_MODE                  1               // 级位模式
#define DISTANCE_ERROR              200             // 允许的定位误差 m
#pragma region 曲线优化所需数据结构体定义
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
    UINT32 line_length;					       /*线路长度 m*/
    UINT8 discrete_size;                       /*离散步长 m*/
    UINT16 discrete_num;                       /*离散num*/
    UINT16 limit[MAX_INTERVAL_SAMPLING];	   /*离散的线路限速, cm/s*/
    FLOAT32 gradient[MAX_INTERVAL_SAMPLING];   /*离散线路坡度，‰*/
    UINT32 curve_radius[MAX_INTERVAL_SAMPLING];/*离散线路曲线半径*/
}LINE_PARAMETER;

/*曲线优化结构体*/
typedef struct
{
    UINT8 optimize_station;                     /*优化的区间编号*/
    UINT8 optimize_stage;                       /*所处优化阶段  1：正在优化 2：优化完成*/
    UINT16 optimize_evaluate;                   /*优化评价结果 运行时分误差*/
    UINT8 optimize_mode;                     /*优化模式 1：在线优化曲线 2：使用离线优化数据*/
    UINT8 optimize_send_flag;                /*是否向app发送优化数据  1：未发送 0：已发送*/
    UINT32 interval;                         /*区间长度 m*/
    UINT32 interval_begin_dis;               /*区间开始里程*/
    UINT32 interval_end_dis;                 /*区间结束里程*/
    UINT32 target_time;                      /*目标运行时分*/
    UINT8 aw_id;                             /*载荷*/
    UINT32 current_distance;                 /*列车当前位置*/
    UINT16 target_speed;                     /*当前优化速度*/
    //UINT8 current_direction;                 /*列车当前运行方向*/
    UINT16 current_station_id;               /*当前站编号*/
    UINT16 next_station_id;                  /*下一站编号*/
    CHAR next_station_name[20];              /*下一站名称*/
    UINT8 recommend_change_num;              /*推荐转换数量*/
    UINT32 recommend_distance[20];           /*对应推荐位置*/
    UINT16 recommend_speed[20];              /*对应推荐速度*/
    UINT8 recommend_wrok[20];                /*对应推荐工况*/
    UINT16 recommend_index[20];               /*对应推荐索引*/

}SPEED_PLAN_INFO;

/*站信息结构体*/
typedef struct
{
    CHAR station_name[20];              /*站名称*/
    UINT32 distance;                         /*所在公里标*/
    UINT8 station_id;                        /*站编号*/
    UINT8 jump_flag;                         /*跳停标识 0：停站 1：跳停*/
}PLAN_STATION_INFO;

/*司机输入的运行计划配置结构体*/
typedef struct
{
    UINT8 plan_refresh_flag;                  /*运行计划更新标志 0：无效 1：计划有变化*/
    UINT32 train_id;                          /*列车车次号*/
    UINT8 direction;                          /*运行方向 0：下行 1：上行*/
    UINT8 plan_flag;                          /*运行计划配置标识 0：默认 1：司机输入*/
    UINT8 plan_station_num;                   /*停站数量*/
    PLAN_STATION_INFO plan_station_info[20];  /*停站计划配置*/
}PLAN_CONFIG_INFO;

#pragma endregion

#pragma region 通过读取CSV数据文件初始化的静态数据结构体定义
/*车站静态数据*/
typedef struct
{
    UINT32 length;					          /*车站数量*/
    CHAR station_name[MAX_STATION_NUM][50];   /*车站名称*/
    UINT16 station_id[MAX_STATION_NUM];       /*车站序号*/
    CHAR property[MAX_STATION_NUM][50];       /*属性-读取用*/
    CHAR property_display[MAX_STATION_NUM][50];/*属性-仅显示*/
    FLOAT32 schedule_time[MAX_STATION_NUM];    /*站间计划运行时间 s*/
    FLOAT32 dwell_time[MAX_STATION_NUM];       /*停站时间 s*/
    FLOAT32 traction_energy[MAX_STATION_NUM];  /*累计牵引能耗 kwh*/
    FLOAT32 regenerative_energy[MAX_STATION_NUM];/*累计再生能量 kwh*/
    FLOAT32 operation_energy[MAX_STATION_NUM];  /*实际运行能耗 kwh*/
    UINT32 begin_distance[MAX_STATION_NUM];  /*起始公里标 m*/
    UINT32 end_distance[MAX_STATION_NUM];    /*结束公里标 m*/
    FLOAT32 latitude[MAX_STATION_NUM];       /*纬度*/
    UINT16 latitude_dir[MAX_STATION_NUM];    /*纬度方向*/
    FLOAT32 longitude[MAX_STATION_NUM];       /*经度*/
    UINT16 longitude_dir[MAX_STATION_NUM];    /*经度方向*/
}STATION_CSV;

/*线路限速静态数据*/
typedef struct
{
    UINT16 length;					/*数据数量*/
    UINT32 distance[MAX_SPEED_LIMIT_CSV_NUM];	/*公里标 m*/
    UINT16 speed_limit[MAX_SPEED_LIMIT_CSV_NUM]; /*限速值 km/h*/
}SPEED_LIMIT_CSV;

/*线路坡度静态数据*/
typedef struct
{
    UINT16 length;					/*数据数量*/
    UINT32 distance[MAX_GRADIENT_CSV_NUM];	/*公里标 m*/
    FLOAT32 gradient[MAX_GRADIENT_CSV_NUM]; /*坡度值 千分之一*/
    UINT32 vertical_curve_radius[MAX_GRADIENT_CSV_NUM]; /*竖曲线半径*/
}GRADIENT_CSV;

/*曲线半径静态数据*/
typedef struct
{
    UINT16 length;					/*数据数量*/
    UINT32 distance[MAX_CURVE_RADIUS_CSV_NUM];	/*公里标 m*/
    UINT16 curve_radius[MAX_CURVE_RADIUS_CSV_NUM]; /*曲线半径*/
}CURVE_RADIUS_CSV;

/*桥梁隧道静态数据*/
typedef struct
{
    UINT16 length;					/*数据个数*/
    UINT32 begin_distance[MAX_TUNNEL_CSV_NUM];	/*公里标 m*/
    UINT32 end_distance[MAX_TUNNEL_CSV_NUM];	/*公里标 m*/
    UINT16 tunnel_param[MAX_TUNNEL_CSV_NUM]; /*参数*/
}TUNNEL_CSV;

/*列车基础参数静态数据*/
typedef struct
{
    UINT16 length;					/*数据数量*/
    UINT8 number[MAX_BASIC_PARAM_CSV_NUM];	/*编号*/
    CHAR property[MAX_BASIC_PARAM_CSV_NUM][50]; /*属性-读取用*/
    CHAR property_display[MAX_BASIC_PARAM_CSV_NUM][50]; /*属性-仅显示*/
    CHAR type[MAX_BASIC_PARAM_CSV_NUM][50]; /*车型*/
    UINT16 train_length[MAX_BASIC_PARAM_CSV_NUM]; /*车长 m*/
    UINT16 max_speed[MAX_BASIC_PARAM_CSV_NUM]; /*最高时速 km/h*/
    CHAR capacity[MAX_BASIC_PARAM_CSV_NUM][50]; /*载荷 AW0 AW2...*/
    FLOAT32 train_weight[MAX_BASIC_PARAM_CSV_NUM];/*列车重量 t*/
    FLOAT32 basic_a[MAX_BASIC_PARAM_CSV_NUM];/*基本阻力参数a N/t*/
    FLOAT32 basic_b[MAX_BASIC_PARAM_CSV_NUM];/*基本阻力参数b N/t*/
    FLOAT32 basic_c[MAX_BASIC_PARAM_CSV_NUM];/*基本阻力参数c N/t*/
    FLOAT32 transformer[MAX_BASIC_PARAM_CSV_NUM];/*牵引变压器效率*/
    FLOAT32 converter[MAX_BASIC_PARAM_CSV_NUM];/*牵引变流器效率*/
    FLOAT32 gearboxes[MAX_BASIC_PARAM_CSV_NUM];/*齿轮箱效率*/
}BASIC_PARAM_CSV;

/*列车动力学静态数据*/
typedef struct
{
    UINT16 length;					/*数据数量*/
    UINT16 speed[MAX_DYNAMICS_CSV_NUM];	/*速度 km/h*/
    FLOAT32 traction_aw0[MAX_DYNAMICS_CSV_NUM];       /*AW0牵引力 kN*/
    FLOAT32 traction_motor_aw0[MAX_DYNAMICS_CSV_NUM]; /*AW0牵引状态电机效率*/
    FLOAT32 brake_aw0[MAX_DYNAMICS_CSV_NUM];          /*AW0制动力 kN*/
    FLOAT32 brake_motor_aw0[MAX_DYNAMICS_CSV_NUM];    /*AW0制动状态电机效率*/
    FLOAT32 traction_aw1[MAX_DYNAMICS_CSV_NUM];       /*AW1牵引力 kN*/
    FLOAT32 traction_motor_aw1[MAX_DYNAMICS_CSV_NUM]; /*AW1牵引状态电机效率*/
    FLOAT32 brake_aw1[MAX_DYNAMICS_CSV_NUM];          /*AW1制动力 kN*/
    FLOAT32 brake_motor_aw1[MAX_DYNAMICS_CSV_NUM];    /*AW1制动状态电机效率*/
    FLOAT32 traction_aw2[MAX_DYNAMICS_CSV_NUM];       /*AW2牵引力 kN*/
    FLOAT32 traction_motor_aw2[MAX_DYNAMICS_CSV_NUM]; /*AW2牵引状态电机效率*/
    FLOAT32 brake_aw2[MAX_DYNAMICS_CSV_NUM];          /*AW2制动力 kN*/
    FLOAT32 brake_motor_aw2[MAX_DYNAMICS_CSV_NUM];    /*AW2制动状态电机效率*/
    FLOAT32 traction_aw3[MAX_DYNAMICS_CSV_NUM];       /*AW3牵引力 kN*/
    FLOAT32 traction_motor_aw3[MAX_DYNAMICS_CSV_NUM]; /*AW3牵引状态电机效率*/
    FLOAT32 brake_aw3[MAX_DYNAMICS_CSV_NUM];          /*AW3制动力 kN*/
    FLOAT32 brake_motor_aw3[MAX_DYNAMICS_CSV_NUM];    /*AW3制动状态电机效率*/
    FLOAT32 traction_aw4[MAX_DYNAMICS_CSV_NUM];       /*AW4牵引力 kN*/
    FLOAT32 traction_motor_aw4[MAX_DYNAMICS_CSV_NUM]; /*AW4牵引状态电机效率*/
    FLOAT32 brake_aw4[MAX_DYNAMICS_CSV_NUM];          /*AW4制动力 kN*/
    FLOAT32 brake_motor_aw4[MAX_DYNAMICS_CSV_NUM];    /*AW4制动状态电机效率*/
    FLOAT32 traction_aw5[MAX_DYNAMICS_CSV_NUM];       /*AW5牵引力 kN*/
    FLOAT32 traction_motor_aw5[MAX_DYNAMICS_CSV_NUM]; /*AW5牵引状态电机效率*/
    FLOAT32 brake_aw5[MAX_DYNAMICS_CSV_NUM];          /*AW5制动力 kN*/
    FLOAT32 brake_motor_aw5[MAX_DYNAMICS_CSV_NUM];    /*AW6制动状态电机效率*/
}DYNAMICS_CSV;

/*曲线优化离线数据静态数据*/
typedef struct
{
    UINT16 length;					/*数据数量*/
    UINT32 distance[MAX_OPTIMIZE_CSV_NUM];	/*公里标 m*/
    FLOAT32 speed_down_aw0[MAX_OPTIMIZE_CSV_NUM];           /*优化速度 下行 aw0*/
    UINT8 level_flag_down_aw0[MAX_OPTIMIZE_CSV_NUM];        /*优化级位标识 下行 aw0*/
    UINT8 level_output_down_aw0[MAX_OPTIMIZE_CSV_NUM];     /*优化级位输出 下行 aw0*/
    FLOAT32 speed_down_aw1[MAX_OPTIMIZE_CSV_NUM];           /*优化速度 下行 aw1*/
    UINT8 level_flag_down_aw1[MAX_OPTIMIZE_CSV_NUM];        /*优化级位标识 下行 aw1*/
    UINT8 level_output_down_aw1[MAX_OPTIMIZE_CSV_NUM];     /*优化级位输出 下行 aw1*/
    FLOAT32 speed_down_aw2[MAX_OPTIMIZE_CSV_NUM];           /*优化速度 下行 aw2*/
    UINT8 level_flag_down_aw2[MAX_OPTIMIZE_CSV_NUM];        /*优化级位标识 下行 aw2*/
    UINT8 level_output_down_aw2[MAX_OPTIMIZE_CSV_NUM];     /*优化级位输出 下行 aw2*/
    FLOAT32 speed_down_aw3[MAX_OPTIMIZE_CSV_NUM];           /*优化速度 下行 aw3*/
    UINT8 level_flag_down_aw3[MAX_OPTIMIZE_CSV_NUM];        /*优化级位标识 下行 aw3*/
    UINT8 level_output_down_aw3[MAX_OPTIMIZE_CSV_NUM];     /*优化级位输出 下行 aw3*/
    FLOAT32 speed_up_aw0[MAX_OPTIMIZE_CSV_NUM];             /*优化速度 上行 aw0*/
    UINT8 level_flag_up_aw0[MAX_OPTIMIZE_CSV_NUM];          /*优化级位标识 上行 aw0*/
    UINT8 level_output_up_aw0[MAX_OPTIMIZE_CSV_NUM];       /*优化级位输出 上行 aw0*/
    FLOAT32 speed_up_aw1[MAX_OPTIMIZE_CSV_NUM];             /*优化速度 上行 aw1*/
    UINT8 level_flag_up_aw1[MAX_OPTIMIZE_CSV_NUM];          /*优化级位标识 上行 aw1*/
    UINT8 level_output_up_aw1[MAX_OPTIMIZE_CSV_NUM];       /*优化级位输出 上行 aw1*/
    FLOAT32 speed_up_aw2[MAX_OPTIMIZE_CSV_NUM];             /*优化速度 上行 aw2*/
    UINT8 level_flag_up_aw2[MAX_OPTIMIZE_CSV_NUM];          /*优化级位标识 上行 aw2*/
    UINT8 level_output_up_aw2[MAX_OPTIMIZE_CSV_NUM];       /*优化级位输出 上行 aw2*/
    FLOAT32 speed_up_aw3[MAX_OPTIMIZE_CSV_NUM];             /*优化速度 上行 aw3*/
    UINT8 level_flag_up_aw3[MAX_OPTIMIZE_CSV_NUM];          /*优化级位标识 上行 aw3*/
    UINT8 level_output_up_aw3[MAX_OPTIMIZE_CSV_NUM];       /*优化级位输出 上行 aw3*/
}OPTIMIZE_CSV;

/*所有静态数据结构体*/
typedef struct
{
    STATION_CSV station_csv;           /*车站静态数据*/
    SPEED_LIMIT_CSV speed_limit_csv;   /*线路限速静态数据*/
    GRADIENT_CSV gradient_csv;         /*线路坡度静态数据*/
    CURVE_RADIUS_CSV curve_radius_csv; /*曲线半径静态数据*/
    TUNNEL_CSV tunnel_csv;             /*桥梁隧道静态数据*/
    BASIC_PARAM_CSV basic_param_csv;   /*列车基础参数静态数据*/
    DYNAMICS_CSV dynamics_csv;         /*列车动力学静态数据*/
    OPTIMIZE_CSV optimize_csv;         /*离线优化数据*/
}STATIC_DATA_CSV;

/*白名单设备MAC地址结构体*/
typedef struct
{
    UINT16 device_num;
    CHAR device_mac_list[MAX_DEVICE_NUM][30];/*设备MAC地址*/
}DEVICE_MAC_DATA;

#pragma  endregion

#pragma region 通信协议结构体

//网卡信息结构体1
typedef struct {
    int  sum_n;
    char net_name1[20];
    char net_name2[20];
    char net_name3[20];
    char net_name4[20];
    char net_name5[20];
    char net_ip1[16];
    char net_ip2[16];
    char net_ip3[16];
    char net_ip4[16];
    char net_ip5[16];
    char net_mac1[32];
    char net_mac2[32];
    char net_mac3[32];
    char net_mac4[32];
    char net_mac5[32];
}net_iface;

//网卡信息结构体2
typedef struct  {
    struct net_iface_1 *next; //指向下一个网卡信息结构体的地址
    char net_name[20];          //网卡名字
    char net_ip[16];            //网卡IP
}net_iface_1;

/*车辆网络->主设备周期数据结构体*/
typedef struct
{
    UINT16 train_weight;         /*列车实时载荷 t*/
    UINT8 formation_num;         /*编组数量*/
    UINT16 train_length;         /*列车长度 m*/
    UINT16 traction_voltage_2;     /*2车牵引机组输入电压 V*/
    UINT16 traction_voltage_3;          /*3车牵引机组输入电压 V*/
    UINT16 traction_voltage_side_2;     /*2车牵引机组输入电压 副边 V*/
    UINT16 traction_voltage_side_3;     /*3车牵引机组输入电压 副边 V*/
    UINT16 traction_current_2;          /*2车牵引机组输入电流 A*/
    UINT16 traction_current_3;          /*3车牵引机组输入电流 A*/
    UINT16 traction_current_low_2;     /*2车牵引机组输入低压侧电流 A*/
    UINT16 traction_current_low_3;     /*3车牵引机组输入低压侧电流 A*/
    UINT8 traction_current_sign; /*电流符号 1：正值 2：负值*/
    UINT8 traction_fault_flag;   /*列车牵引故障标志位*/
    UINT8 brake_fault_flag;      /*列车制动故障标志位*/
    UINT8 other_fault_flag;      /*列车其他故障标志位*/
    UINT16 train_fault_code;     /*列车故障代码*/
    //自更新变量
    UINT32 traction_energy_sum;          /*累计牵引能耗 焦耳*/
    UINT32 brake_energy_sum;             /*累计再生能量 焦耳*/
}PERIOD_MSG_FROM_TRAIN;

/*信号系统->主设备周期数据结构体*/
typedef struct
{
    UINT32 traction_energy;      /*累计牵引能耗 100kwh*/
    UINT32 regeneration_energy;  /*累计再生能量能 100kwh*/
    UINT8 train_direction;       /*列车运行方向 1:上行 0:下行*/
    UINT32 train_id;             /*列车车次号*/
    UINT32 train_number;         /*列车车组号*/
    UINT8 arrive_flag;           /*停准停稳标志 1:停准停稳 0:其他*/
    UINT8 leave_flag;            /*允许发车标志 1:允许发车 0:其他*/
    /*bit*/
    UINT8 byte_data;             /*包含以下bit数据的byte*/
    UINT8 handle_brake_flag;     /*手柄制动区*/
    UINT8 handle_traction_flag;  /*手柄牵引区*/
    UINT8 door_flag;             /*列车车门标志 1：锁闭 0：开启*/
    UINT8 door_flag_last;        /*上周期列车车门标志 1：锁闭 0：开启*/
    UINT8 control_mode_flag;     /*0:*/
    UINT8 pantograph_3_flag;     /*3车受电弓状态*/
    UINT8 pantograph_2_flag;     /*2车受电弓状态*/
    UINT8 vcb_3_flag;            /*3车vcb状态*/
    UINT8 vcb_2_flag;            /*2车vcb状态*/
    /**/
    UINT8 train_plan_flag;       /*列车运行计划变更标志 1:计划变更 0:其他*/
    UINT16 train_ebi;            /*ATP防护速度 km/h*/
    FLOAT32 train_speed;          /*列车速度 km/h*/
    UINT16 next_station_id;      /*下一站编号*/
    UINT8 current_station_leave_time[20];   /*当前站出发时间*/
    CHAR current_station_name[100];  /*当前站名称*/
    CHAR next_station_name[100];  /*下一站名称*/
    CHAR dst_station_name[100];  /*终点站名称*/
    UINT8 next_station_arrive_time[20];  /*下一站到达时间*/
    UINT8 next_station_leave_time[20];   /*下一站出发时间*/
    UINT8 train_work_condition;            /*列车工况 1:牵引  2:惰行 3:制动 4:无效*/
    UINT16 train_work_level;              /*列车级位 级位模式有效*/
    UINT16 train_work_speed;              /*目标速度km/h 速度模式有效*/
    UINT32 train_distance;               /*列车公里标 m*/
    FLOAT64 train_distance_double;
    UINT8 train_time[20];                /*列车时间*/
    UINT16 temporary_limit_num;           /*临时限速数量*/
    UINT32 temporary_limit_begin_distance[MAX_TEMPORARY_LIMIT_NUM]; /*临时限速起始公里标 m*/
    UINT32 temporary_limit_end_distance[MAX_TEMPORARY_LIMIT_NUM];   /*临时限速结束公里标 m*/
    UINT16 temporary_limit_value[MAX_TEMPORARY_LIMIT_NUM];          /*临时限速 km/h*/
    UINT32 longitude_value;        /*经度值*/
    UINT8 longitude_direction;      /*经度方向*/
    UINT32 latitude_value;         /*纬度值*/
    UINT8 latitude_direction;       /*纬度方向*/
    //自更新变量
    UINT32 train_distance_last;          /*上周期列车公里标 m*/
    UINT32 longitude_value_last;        /*上周期经度值*/
    UINT8 longitude_direction_last;      /*上周期经度方向*/
    UINT32 latitude_value_last;         /*上周期纬度值*/
    UINT8 latitude_direction_last;       /*上周期纬度方向*/
    FLOAT32 train_speed_last;            /*上周期列车速度 km/h*/
}PERIOD_MSG_FROM_SIGNAL;

/*主设备->APP初始化数据结构体*/
typedef struct
{
    UINT16 gradient_num;                                   /*坡度数量*/
    UINT32 gradient_distance[MAX_GRADIENT_CSV_NUM];        /*坡度公里标 m*/
    UINT16 gradient_value[MAX_GRADIENT_CSV_NUM];           /*坡度值 十万分之一*/
    UINT16 curve_radius_num;                               /*曲线半径数量*/
    UINT32 curve_radius_distance[MAX_CURVE_RADIUS_CSV_NUM];/*曲线半径公里标 m*/
    UINT16 curve_radius_value[MAX_CURVE_RADIUS_CSV_NUM];   /*曲线半径*/
    UINT16 tunnel_num;                                     /*桥梁隧道数量*/
    UINT32 tunnel_begin_distance[MAX_TUNNEL_CSV_NUM];      /*桥梁隧道起始公里标 m*/
    UINT32 tunnel_end_distance[MAX_TUNNEL_CSV_NUM];        /*桥梁隧道结束公里标 m*/
    UINT8 tunnel_flag[MAX_CURVE_RADIUS_CSV_NUM];           /*1:桥梁 2:隧道*/
    UINT16 speed_limit_num;                                /*线路限速数量*/
    UINT32 speed_limit_distance[MAX_SPEED_LIMIT_CSV_NUM];  /*限速公里标 m*/
    UINT16 speed_limit_value[MAX_SPEED_LIMIT_CSV_NUM];     /*限速值 km/h*/
    UINT16 separate_num;                                   /*分相区数量*/
    UINT32 separate_begin_distance[MAX_SEPARATE_CSV_NUM];  /*分相区起始公里标 m*/
    UINT32 separate_end_distance[MAX_SEPARATE_CSV_NUM];    /*分相区结束公里标 m*/
    UINT16 signal_num;                                     /*信号机数量*/
    UINT32 signal_distance[MAX_SINGAL_CSV_NUM];            /*信号机起始公里标 m*/
    UINT32 line_length;                                    /*线路长度 m*/
    UINT8 line_name[20];                                   /*线路名称*/
    UINT16 station_num;                                    /*车站数量*/
    UINT8 station_name[MAX_STATION_NUM][20];               /*车站名称*/
    UINT32 station_distance[MAX_STATION_NUM];              /*车站公里标 m*/
    UINT8 serve_reply;                                     /*服务回复标志 1:请求成功 2:请求无效*/
}INIT_MSG_TO_APP;

/*主设备->APP周期数据结构体*/
typedef struct
{
    UINT8 traction_fault_flag;  /*列车牵引故障标志位*/
    UINT8 brake_fault_flag;     /*列车制动故障标志位*/
    UINT8 other_fault_flag;     /*列车其他故障标志位*/
    UINT32 traction_energy;     /*累计牵引能耗 100kwh*/
    UINT16 regeneration_energy; /*累计再生能量能 100kwh*/
    UINT8 train_direction;      /*列车运行方向 1:上行 0:下行*/
    UINT32 train_id;            /*列车车次号*/
    UINT32 train_number;        /*列车车组号*/
    UINT8 arrive_flag;          /*停准停稳标志 1:停准停稳 0:其他*/
    UINT8 leave_flag;           /*允许发车标志 1:允许发车 0:其他*/
    UINT8 optimize_flag;        /*曲线优化标志 1:正在优化 2:优化完成 3:其他*/
    UINT16 train_ebi;           /*ATP防护速度 km/h*/
    UINT16 train_speed;         /*列车速度 km/h*/
    UINT8 current_station_leave_time[20]; /*下一站出发时间*/
    UINT8 next_station_name[20]; /*下一站名称*/
    UINT8 next_station_arrive_time[20];/*下一站到达时间*/
    UINT8 next_station_leave_time[20]; /*下一站出发时间*/
    UINT8 train_work_condition;        /*列车工况 1:牵引  2:惰行 3:制动 4:无效*/
    UINT8 train_work_level;            /*列车级位*/
    UINT32 train_distance;             /*列车公里标 m*/
    UINT8 train_time[20];              /*列车时间*/
    UINT16 next_speed_recommend;       /*下一推荐速度 km/h*/
    UINT8 next_work_condition_recommend; /*下一推荐工况 1:牵引  2:惰行 3:制动 4:无效*/
    UINT8 next_work_level_recommend; /*下一推荐工况级位 1:牵引  2:惰行 3:制动 4:无效*/
    UINT16 next_recommend_countdown;   /*下一建议生效倒计时 s*/
    UINT32 next_recommend_distance;    /*下一建议生效距离 m*/
    UINT8 temporary_limit_num;         /*临时限速数量*/
    UINT32 temporary_limit_begin_distance[MAX_TEMPORARY_LIMIT_NUM];/*临时限速起始公里标 m*/
    UINT32 temporary_limit_end_distance[MAX_TEMPORARY_LIMIT_NUM];  /*临时限速结束公里标 m*/
    UINT16 temporary_limit_value[MAX_TEMPORARY_LIMIT_NUM];         /*临时限速 km/h*/
}PERIOD_MSG_TO_APP;

#pragma endregion
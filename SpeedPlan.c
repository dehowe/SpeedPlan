#include "SpeedPlan.h"

TRAIN_PARAMETER				g_train_param;						// 列车参数
UINT16						g_aw_id;							// 载荷
LINE_PARAMETER              g_line_param;                       // 线路参数 下行
STATIC_DATA_CSV             g_static_data_csv;                  // CSV静态数据
SPEED_PLAN_INFO             g_speed_plan_info;                  // 速度规划信息
UINT16                      g_speed_curve_offline[MAX_SPEED_CURVE];  //离线优化速度存储数组
UINT16                      g_level_output_flag[MAX_SPEED_CURVE];    //离线优化级位存储数组
FLOAT32                     g_plan_time[MAX_SPEED_CURVE];            //离线优化运行时分存储数组

/*速度规划相关变量*/
FLOAT32* gradient = NULL;						 // 初始化区间坡度存储指针;
UINT32* curve_radius = NULL;					 // 初始化区间曲线半径存储指针;
UINT32* speed_limit_location = NULL;			 // 初始化限速转换点位置存储指针;
UINT16* speed_limit = NULL;						 // 初始化限速转换点限速存储指针;
UINT16* speed_curve_offline = NULL;				 // 初始化离线优化速度存储指针;
UINT16* tartget_speed_curve = NULL;				 // 初始化优化速度存储指针;
UINT16* speed_limit_max = NULL;					 // 初始化最快速度存储指针;
UINT16* speed_limit_min = NULL;					 // 初始化最小速度存储指针;
UINT16* speed_limit_mmax = NULL;				 // 初始化顶棚限速存储指针;
UINT16* level_output_flag = NULL;				 // 初始化级位输出标识1：牵引；2：惰行;3：制动;
FLOAT32* plan_time = NULL;						 // 初始化在离散点需要达到的运行时分;
UINT32	interval_length;						// 下一区间长度m
UINT32	interval_length_cm;						// 下一区间长度cm
UINT32	target_time_offline;					// 离线求解目标运行时分
UINT32	target_time_online;						// 在线优化目标运行时分
UINT16	dim;									// 建模离散维度
UINT8	limit_num;								// 限速切换点个数
UINT16  solution_num = 50;						// 解空间大小
UINT16  discrete_size = 2000;					// 离散步长
FLOAT32 optimal_time_offline = 0;				// 离线优化运行时分
UINT16 remain_section_length;					// 区间进行等间隔离散后的剩余长度

/*************************************************************************
* 功能描述: 速度规划主程序
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void SpeedPlanMain()
{
    /*如果信号系统发送允许计划更新且无正在进行的曲线优任务*/
    if(g_period_msg_from_signal.train_plan_flag==1&&g_speed_plan_info.optimize_stage==2)
    {
        printf("receive plan change and optimize end!\n");
        g_period_msg_from_signal.train_plan_flag=0;
        //参数校验
        UINT8 data_error_flag=0;//数据异常标志 0：无异常 1：数据异常
        g_speed_plan_info.current_direction=g_period_msg_from_signal.train_direction;//列车运行方向
        g_speed_plan_info.interval_begin_dis=g_period_msg_from_signal.train_distance;//区间起始位置为列车头部位置
        //memcpy(g_speed_plan_info.next_station_name,g_period_msg_from_signal.next_staion_name,20);
        g_speed_plan_info.next_station_id=g_period_msg_from_signal.next_station_id;//下一站编号
        for(int i=0;i<g_static_data_csv.station_csv.length;i++)
        {
            //if(strcmp(g_static_data_csv.station_csv.station_name[i],g_speed_plan_info.next_station_name)==0)
            //从车站静态数据中找到下一站
            if (g_static_data_csv.station_csv.station_id[i]==g_speed_plan_info.next_station_id)
            {
                g_speed_plan_info.interval_end_dis=g_static_data_csv.station_csv.begin_distance[i];//区间起结束位置为下一站公里标位置
                //如果运行方向为下行
                if(g_speed_plan_info.current_direction==0)
                {
                    if(g_speed_plan_info.interval_end_dis<g_speed_plan_info.interval_begin_dis)
                    {
                        data_error_flag=1;
                        printf("SPEED_PLAN:error!");
                        break;
                    }
                    if(i!=0)
                    {
                        g_speed_plan_info.target_time=(UINT32)g_static_data_csv.station_csv.schedule_time[i-1];
                    }
                    else
                    {
                        g_speed_plan_info.target_time=0;
                        data_error_flag=1;
                        printf("SPEED_PLAN:error!");
                        break;
                    }
                }
                else
                {
                    g_speed_plan_info.target_time=(UINT32)g_static_data_csv.station_csv.schedule_time[i];
                    if(g_speed_plan_info.interval_end_dis>g_speed_plan_info.interval_begin_dis)
                    {
                        data_error_flag=1;
                        printf("SPEED_PLAN:error!");
                        break;
                    }
                }
                break;
            }
        }

        //test
//        g_speed_plan_info.interval_begin_dis=0;
//        g_speed_plan_info.interval_end_dis=4280;
//        g_speed_plan_info.target_time=200;
//        data_error_flag=0;
        //如果数据无异常
        if (data_error_flag==0)
        {
            printf("speed plan thread on!\n");
            g_speed_plan_info.optimize_stage=1;//曲线优化标志置为：正在曲线优化中
            pthread_t tid_speed_plan;
            /*创建曲线优化线程*/
            if(pthread_create(&tid_speed_plan,NULL,SpeedPlanOffline,NULL))
            {
                perror("Fail to create server thread");
            }
        }



    }
}

/*************************************************************************
* 功能描述: 离线求解算法入口
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void *SpeedPlanOffline()
{
    UINT32 start = clock();
    //基础数据预处理
    gradient = (FLOAT32*)malloc(MAX_INTERVAL_SAMPLING * sizeof(FLOAT32));				// 初始化区间坡度存储指针;
    curve_radius = (UINT32*)malloc(MAX_INTERVAL_SAMPLING * sizeof(UINT32));				// 初始化区间曲线半径存储指针;
    speed_limit_location = (UINT32*)malloc(MAX_LIMIT_POINT * sizeof(UINT32));			// 初始化限速转换点位置存储指针;
    speed_limit = (UINT16*)malloc(MAX_LIMIT_POINT * sizeof(UINT16));					// 初始化限速转换点限速存储指针;
    speed_curve_offline = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));			// 初始化离线优化速度存储指针;
    tartget_speed_curve = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));			// 初始化优化速度存储指针;
    speed_limit_max = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));				// 初始化最快速度存储指针;
    speed_limit_min = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));				// 初始化最小速度存储指针;
    speed_limit_mmax = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));				// 初始化顶棚限速存储指针;
    level_output_flag = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));              // 初始化级位输出标识1：牵引；2：惰行;3：制动；;
    plan_time = (FLOAT32*)malloc(MAX_SPEED_CURVE * sizeof(FLOAT32));					// 初始化在离散点需要达到的运行时分;

    //数据准备
    GetBaseDataReady();

    //离线求解
    //出口参数初始化
    memset(level_output_flag, 0, MAX_SPEED_CURVE * sizeof(UINT16));
    dim = GetOptimalSpeedOffline(speed_curve_offline, discrete_size, speed_limit_max, speed_limit_min, speed_limit_mmax,
                                 interval_length_cm, speed_limit_location, speed_limit, limit_num, target_time_offline, solution_num);
    //sleep(10);//模拟板卡求解时间
    for (int i = 0; i <= dim; i++)
    {
        g_speed_curve_offline[i]=speed_curve_offline[i];
        g_level_output_flag[i]=level_output_flag[i];
        g_plan_time[i]=plan_time[i];
    }

    DivideStageByOptimizeSpeed();
    g_speed_plan_info.optimize_stage=2;
    g_speed_plan_info.optimize_station=g_speed_plan_info.next_station_id;
    g_speed_plan_info.interval=interval_length;
    UINT32 finish = clock();
    UINT32 cal_time=(finish - start) / 1000;
    printf("%s SPEED_PLAN:solve end, use time % d ms!\n",g_current_time, cal_time);
    LogWrite(INFO,"%s:%d%s","end:use_time",cal_time,"ms");



    free(gradient);
    free(curve_radius);
    free(speed_limit_location);
    free(speed_limit);
    free(speed_curve_offline);
    free(tartget_speed_curve);
    free(speed_limit_max);
    free(speed_limit_min);
    free(speed_limit_mmax);
    free(level_output_flag);
    free(plan_time);
    pthread_exit(0);//此线程退出
}


/*************************************************************************
* 功能描述: 数据预处理，整理区间长度、限速、坡度、曲线半径等信息
* 输入参数: 无
* 输出参数: 无
* 返回值:   0:查询失败 1:查询成功
*************************************************************************/
UINT8 GetBaseDataReady()
{
    UINT8  result = 1;						/*函数返回值*/
    UINT16 speed_limit_last = 0;			/*上一限速值临时变量*/
    UINT32 k = 0;							/*循环变量*/
    UINT32 m = 0;							/*循环变量*/
    LINE_PARAMETER* line_param_temp;
    line_param_temp = &g_line_param;

//    //根据运行方向选择静态线路数据
//    if(g_direction==0)
//    {
//        line_param_temp = &g_line_param;
//    }
//    else
//    {
//        line_param_temp = &g_line_param_up;
//    }

    //如果运行方向为上行
    if (g_speed_plan_info.current_direction==0)
    {
        /*曲线优化所需数据准备*/
        if(g_speed_plan_info.interval_begin_dis<=line_param_temp->line_length&&g_speed_plan_info.interval_end_dis<=line_param_temp->line_length)
        {
            target_time_offline = g_speed_plan_info.target_time;
            /*计算区间长度*/
            interval_length=g_speed_plan_info.interval_end_dis-g_speed_plan_info.interval_begin_dis;
            interval_length_cm = interval_length * 100;
            /*遍历所有位置限速，找到限速切换点及对应限速*/
            speed_limit_last = line_param_temp->limit[g_speed_plan_info.interval_begin_dis/line_param_temp->discrete_size];
            for (int i=0;i<line_param_temp->discrete_num;i++)
            {
                if(g_speed_plan_info.interval_begin_dis<=i*line_param_temp->discrete_size
                   &&g_speed_plan_info.interval_end_dis>=i*line_param_temp->discrete_size)
                {
                    if (line_param_temp->limit[i] != speed_limit_last)//限速切换
                    {
                        /*记录切换点位置和限速值*/
                        if(g_speed_plan_info.interval_begin_dis>(i - 1) * line_param_temp->discrete_size)
                        {
                            speed_limit_location[k]=0;
                        }
                        else
                        {
                            speed_limit_location[k] = (i - 1) * line_param_temp->discrete_size*100-g_speed_plan_info.interval_begin_dis*100;
                        }
                        speed_limit[k] = speed_limit_last;
                        /*更新上一限速*/
                        speed_limit_last = line_param_temp->limit[i];
                        k += 1;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            speed_limit_location[k] = g_speed_plan_info.interval_end_dis*100-g_speed_plan_info.interval_begin_dis*100;
            speed_limit[k] = line_param_temp->limit[g_speed_plan_info.interval_end_dis/line_param_temp->discrete_size - 1];
            limit_num = k+1;
//            for(int i=0;i<limit_num;i++)
//            {
//                printf("loc:%d,limit:%d\n", speed_limit_location[i], speed_limit[i]);
//            }

            for (int i=0;i<line_param_temp->discrete_num;i++)
            {
                if(g_speed_plan_info.interval_begin_dis<=i*line_param_temp->discrete_size
                   &&g_speed_plan_info.interval_end_dis>=i*line_param_temp->discrete_size)
                {
                    gradient[m] = line_param_temp->gradient[i]; //坡度
                    curve_radius[m] = line_param_temp->curve_radius[i];//曲线半径
                    m+=1;
                    //printf("loc:%d,gradient:%f,curve:%d\n", i*line_param_temp->discrete_size, gradient[i], curve_radius[i]);
                    //LogWrite(INFO,"%s:%f,%s:%d","gradient",gradient[i],"curve",curve_radius[i]);
                }
            }

        }
        else
        {
            result=0;//线路里程数据超出静态数据
            return result;
        }
    }
    else
    {
        /*曲线优化所需数据准备*/
        if(g_speed_plan_info.interval_begin_dis<=line_param_temp->line_length&&g_speed_plan_info.interval_end_dis<=line_param_temp->line_length)
        {
            target_time_offline = g_speed_plan_info.target_time;
            /*计算区间长度*/
            interval_length=g_speed_plan_info.interval_begin_dis-g_speed_plan_info.interval_end_dis;
            interval_length_cm = interval_length * 100;
            /*遍历所有位置限速，找到限速切换点及对应限速*/
            speed_limit_last = line_param_temp->limit[g_speed_plan_info.interval_begin_dis/line_param_temp->discrete_size];
            for (int i=line_param_temp->discrete_num-1;i>=0;i--)
            {
                if(g_speed_plan_info.interval_begin_dis>=i*line_param_temp->discrete_size
                   &&g_speed_plan_info.interval_end_dis<=i*line_param_temp->discrete_size)
                {
                    if (line_param_temp->limit[i] != speed_limit_last)//限速切换
                    {
                        /*记录切换点位置和限速值*/
                        if(g_speed_plan_info.interval_begin_dis<(i - 1) * line_param_temp->discrete_size)
                        {
                            speed_limit_location[k]=0;
                        }
                        else
                        {
                            speed_limit_location[k] = g_speed_plan_info.interval_begin_dis*100-(i - 1) * line_param_temp->discrete_size*100;
                        }
                        speed_limit[k] = speed_limit_last;
                        /*更新上一限速*/
                        speed_limit_last = line_param_temp->limit[i];
                        k += 1;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            speed_limit_location[k] = g_speed_plan_info.interval_begin_dis*100-g_speed_plan_info.interval_end_dis*100;
            speed_limit[k] = line_param_temp->limit[g_speed_plan_info.interval_end_dis/line_param_temp->discrete_size];
            limit_num = k+1;
            for(int i=0;i<limit_num;i++)
            {
                //printf("loc:%d,limit:%d\n", speed_limit_location[i], speed_limit[i]);
            }

            for (int i=line_param_temp->discrete_num-1;i>=0;i--)
            {
                if(g_speed_plan_info.interval_begin_dis>=i*line_param_temp->discrete_size
                   &&g_speed_plan_info.interval_end_dis<=i*line_param_temp->discrete_size)
                {
                    gradient[m] = -line_param_temp->gradient[i]; //坡度
                    curve_radius[m] = line_param_temp->curve_radius[i];//曲线半径
                    m+=1;
                    //printf("loc:%d,gradient:%f,curve:%d\n", i*line_param_temp->discrete_size, -line_param_temp->gradient[i], line_param_temp->curve_radius[i]);
                    //LogWrite(INFO,"%s:%f,%s:%d","gradient",gradient[i],"curve",curve_radius[i]);
                }
            }

        }
        else
        {
            result=0;//线路里程数据超出静态数据
            return result;
        }

    }
    printf("%s SPEED_PLAN:base data ready!\n",g_current_time);
    /*释放所申请的内存, 防止内存泄露*/
    return result;
}

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
UINT16 GetOptimalSpeedOffline(UINT16* speed_curve, UINT16 discrete_size, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16* speed_limit_mmax, UINT32 section_length, UINT32 speed_limit_location[], UINT16 speed_limit[], UINT16 speed_limit_length, UINT16 target_time, UINT16 solution_num)
{
    //数学建模
    UINT16 dim = (UINT16)ceil(section_length * 1.0 / discrete_size)-1;//按等间隔划分维度
    remain_section_length = section_length - dim * discrete_size;
    Modeling(speed_limit_max, speed_limit_min, speed_limit_mmax, discrete_size, dim, solution_num, speed_limit_location, speed_limit, speed_limit_length, target_time);
    //根据当前位置启发
    UINT32 lower_bound[5];//初始化求解下边界存储指针;
    UINT32 upper_bound[5];//初始化求解上边界存储指针;
    UINT8 switch_flag[5];//初始化工况切换标识存储指针;
    UINT8 bound_size = GetBounderOffline(lower_bound, upper_bound, switch_flag);
    for (INT32 i = 0; i < bound_size; i++)
    {
        //printf("lower_bound:%d;upper_bound:%d\n", lower_bound[i], upper_bound[i]);
        LogWrite(INFO,"%s:%d,%s:%d","lower_bound",lower_bound[i],"upper_bound",upper_bound[i]);
    }
    //灰狼算法求解
    GWO_Offline(speed_curve, speed_limit_max, speed_limit_min, dim, target_time, discrete_size, lower_bound, upper_bound, switch_flag, bound_size);
    return dim;
}




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
              UINT16 solution_num, UINT32 speed_limit_loc[], UINT16 speed_limit[], UINT16 speed_limit_length, UINT16 target_time)
{
    UINT16 j = 0;						//限速变化点索引初始化
    UINT16 p = 0;						//限速下降点索引初始化
    UINT32 location_index;
    UINT16* speed_limit_temp = NULL;	//限速离散存储指针初始化
    UINT16* limit_fall_begin = NULL;	//限速下降开始速度
    UINT16* limit_fall_end = NULL;		//限速下降结束速度
    UINT16* limit_fall_index = NULL;	//限速下降对应离散索引
    speed_limit_temp = (UINT16*)malloc(MAX_INTERVAL_SAMPLING * sizeof(UINT16));
    limit_fall_begin = (UINT16*)malloc(MAX_LIMIT_POINT * sizeof(UINT16));
    limit_fall_end = (UINT16*)malloc(MAX_LIMIT_POINT * sizeof(UINT16));
    limit_fall_index = (UINT16*)malloc(MAX_LIMIT_POINT * sizeof(UINT16));
    /*申请内存无效防护*/
    if (NULL == speed_limit_temp || NULL == limit_fall_begin || NULL == limit_fall_end || NULL == limit_fall_index)
        return;
    /*提取前方限速下降*/
    for (UINT16 i = 0; i <= dim; i++)
    {
        speed_limit_temp[i] = speed_limit[j];
        speed_limit_mmax[i] = speed_limit[j];
        location_index = (i + 1) * discrete_size;
        if (location_index < speed_limit_loc[j])
        {
            continue;
        }
        else
        {
            j++;
            if (j < speed_limit_length)
            {
                if (speed_limit[j] < speed_limit[j - 1])
                {
                    limit_fall_begin[p] = GetEbiEnd(speed_limit[j], speed_limit_temp, i);
                    limit_fall_end[p] = speed_limit[j];
                    limit_fall_index[p] = i;
                    p++;
                }
            }
            else
            {
                limit_fall_begin[p] = GetEbiEnd(0, speed_limit_temp, i);
                limit_fall_end[p] = 0;
                limit_fall_index[p] = i - 1;
                p++;
            }
        }
    }
    speed_limit_temp[dim] = 0;
    speed_limit_mmax[dim] = 0;
    /*根据限速下降计算目标距离防护速度*/
    for (UINT16 i = 0; i < p; i++)
    {
        GetEBI(limit_fall_end[i], limit_fall_begin[i], limit_fall_index[i], speed_limit_temp, discrete_size);
    }
    for (UINT16 i = 0; i <= dim; i++)
    {
        speed_limit_max[i] = speed_limit_temp[i];
        speed_limit_min[i] = 0;
        //printf("最大防护速度%d\n", speed_limit_max[i]);
        //LogWrite(INFO,"%s:%d","max_speed",speed_limit_max[i]);
    }
    /*释放所申请的内存, 防止内存泄露*/
    free(speed_limit_temp);
    free(limit_fall_begin);
    free(limit_fall_end);
    free(limit_fall_index);
}

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
void GetEBI(UINT16 speed_begin, UINT16 speed_end, UINT16 index, UINT16* speed_limit, UINT16 discrete_size)
{
    UINT16 v_index = speed_begin;
    UINT16 spd;
    UINT16 acc_break;//制动减速度
    while (v_index < speed_end)
    {
        acc_break = (UINT16)(GetBreakAcc(v_index) * SPEED_PLAN_TB_RATIO);
        spd = (UINT16)sqrt(v_index * v_index + 2 * acc_break * discrete_size);
        if (spd < speed_end)
        {
            if (index > 0)
            {
                if (spd < speed_limit[index])
                {
                    speed_limit[index] = spd;
                }
                v_index = spd;
                index = index - 1;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

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
UINT16 GetEbiEnd(UINT16 ebi_begin, UINT16* speed_limit, UINT16 index)
{
    UINT16 v_end = ebi_begin;
    for (UINT16 i = 0; i < index - 1; i++)
    {
        if (speed_limit[index - i] >= v_end)
        {
            v_end = speed_limit[index - i];
        }
        else
        {
            break;
        }
    }
    return v_end;
}

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
                 UINT16 discrete_size, UINT32* lower_bound, UINT32* upper_bound, UINT8* switch_flag, UINT8 solve_dim)
{
    UINT16 wolves_num = 50;//狼群大小
    UINT16 iteration = 30;//迭代次数
    //初始化Alpha,Beta,Delta狼位置
    FLOAT32* position_Alpha =(FLOAT32*)malloc(solve_dim * sizeof(FLOAT32));
    FLOAT32* position_Beta = (FLOAT32*)malloc(solve_dim * sizeof(FLOAT32));
    FLOAT32* position_Delta = (FLOAT32*)malloc(solve_dim * sizeof(FLOAT32));
    //初始化Alpha,Beta,Delta狼适应度
    UINT16 score_Alpha = 65535;
    UINT16 score_Beta = 65535;
    UINT16 score_Delta = 65535;
    UINT16 fitness;//适应度
    FLOAT32 convergence_factor = 1;//收敛因子初始值
    //下面为GWO求解过程参数，预先初始化
    FLOAT32 a;
    FLOAT32 A1, A2, A3;
    FLOAT32 C1, C2, C3;
    FLOAT32 X1;
    FLOAT32 X2;
    FLOAT32 X3;
    FLOAT32 D_alpha;
    FLOAT32 D_beta;
    FLOAT32 D_delta;
    UINT16* OptimalSpd = NULL;
    OptimalSpd = (UINT16*)malloc(MAX_SPEED_CURVE * sizeof(UINT16));
    /*申请内存无效防护*/
    if (OptimalSpd == NULL)
        return;
    FLOAT32** Positions = NULL;
    Positions = (FLOAT32**)malloc(wolves_num * sizeof(FLOAT32*));
    for (int i = 0; i < wolves_num; i++)
        Positions[i] = (FLOAT32*)malloc(solve_dim * sizeof(FLOAT32));

    Initialize(wolves_num, solve_dim, upper_bound, lower_bound, Positions);//初始化狼群位置分布

    for (UINT16 i = 0; i < iteration; i++)//循环计数器
    {
        for (UINT16 m = 0; m < wolves_num; m++)//找到本次迭代产生的Alpha、Beta、Delta狼
        {
            //fitness = (UINT16)GetFitnessOffline(OptimalSpd, Positions[m], discrete_size, dim, speed_limit_max,speed_limit_min, target_time);//计算每只狼的适应度值，以目标值最小为目标
            fitness = (UINT16)GetFitnessOffline2(OptimalSpd, Positions[m], discrete_size, dim, speed_limit_max, speed_limit_min, target_time, switch_flag, solve_dim);//计算每只狼的适应度值，以目标值最小为目标

            if (fitness <= score_Alpha)
            {
                score_Alpha = fitness;
                position_Alpha = Positions[m];
            }
            if (fitness > score_Alpha && fitness < score_Beta)
            {
                score_Beta = fitness;
                position_Beta = Positions[m];
            }
            if (fitness > score_Alpha && fitness > score_Beta && fitness < score_Delta)
            {
                score_Delta = fitness;
                position_Delta = Positions[m];
            }
        }
        a = (FLOAT32)(convergence_factor - (2.0 / iteration) * i);//收敛因子随着迭代过程进行，由初始值线性减小到0
        if (a < 0)
        {
            a = 0;
        }
        for (UINT16 m = 0; m < wolves_num; m++)
        {
            for (UINT16 n = 0; n < solve_dim; n++)
            {
                //r2 = rand();
                A1 = 2 * a * (FLOAT32)rand() / RAND_MAX - a;//计算系数A，Equation (3.3)
                C1 = 2 * (FLOAT32)rand() / RAND_MAX;//计算系数C，Equation (3.4)
                //Alpha狼位置更新
                D_alpha = abs(C1 * position_Alpha[n] - Positions[m][n]);
                X1 = position_Alpha[n] - A1 * D_alpha;

                A2 = 2 * a * (FLOAT32)rand() / RAND_MAX - a;
                C2 = 2 * (FLOAT32)rand() / RAND_MAX;
                //Beta狼位置更新
                D_beta = abs(C2 * position_Beta[n] - Positions[m][n]);
                X2 = position_Beta[n] - A2 * D_beta;

                A3 = 2 * a * (FLOAT32)rand() / RAND_MAX - a;
                C3 = 2 * (FLOAT32)rand() / RAND_MAX;
                //Delta狼位置更新
                D_delta = abs(C3 * position_Delta[n] - Positions[m][n]);
                X3 = position_Delta[n] - A3 * D_delta;

                Positions[m][n] = (X1 + X2 + X3) / 3;//位置更新，Equation (3.7)

                if (Positions[m][n] > upper_bound[n])//边界判断
                {
                    Positions[m][n] = (FLOAT32)upper_bound[n];
                }
                if (Positions[m][n] < lower_bound[n])
                {
                    Positions[m][n] = (FLOAT32)lower_bound[n];
                }
                /*if (Positions[m][3] < Positions[m][2])
                {
                    Positions[m][3] = Positions[m][2];
                }*/
            }

        }
    }
    //计算每只狼的适应度值，以目标值最小为目标
    //fitness = (UINT16)GetFitnessOffline(OptimalSpd, position_Alpha, discrete_size, dim, speed_limit_max, speed_limit_min, target_time);
    fitness = (UINT16)GetFitnessOffline2(OptimalSpd, position_Alpha, discrete_size, dim, speed_limit_max, speed_limit_min, target_time, switch_flag, solve_dim);
//    for (UINT16 i = 0; i < solve_dim; i++)
//    {
//        printf("SPEED_PLAN:best_solve:%f\n", position_Alpha[i]);
//    }
    for (UINT16 i = 0; i < dim + 1; i++)//输出最优解
    {
        optimal_speed[i] = OptimalSpd[i];
        //printf("loc:%dm-speed:%dcm/s-switch:%d\n", i*discrete_size/100, OptimalSpd[i], level_output_flag[i]);
        LogWrite(INFO,"%s:%d%s:%d%s:%d","loc",i*discrete_size/100,"m,speed",OptimalSpd[i],"cm/s,switch",level_output_flag[i]);

    }

    printf("%s SPEED_PLAN:next_station:%d,interval_length:%d-target_time:%d-optimize_time：%.3f\n",g_current_time,g_speed_plan_info.next_station_id, interval_length, target_time, plan_time[dim - 1]);
    LogWrite(INFO,"%s:%d%s:%d%s:%f","interval_length",interval_length,"m,target_time",target_time,"s,optimize_time",plan_time[dim - 1]);
    g_speed_plan_info.optimize_evaluate=abs(plan_time[dim - 1] - target_time);
    for (UINT16 i = 0; i < dim + 1; i++)//输出最优解
    {
        optimal_speed[i] = OptimalSpd[i];
    }

    //释放内存
    for (int i = 0; i < wolves_num; i++)
        free(Positions[i]);
    free(Positions);
    free(OptimalSpd);


}


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
void Initialize(UINT16 wolves_num, UINT8 solve_dim, UINT32 upper_bound[], UINT32 lower_bound[],FLOAT32** Positions)
{
    srand((UINT32)(time(0)));//修改种子
    for (UINT16 i = 0; i < wolves_num; i++)
    {
        for (UINT16 j = 0; j < solve_dim; j++)
        {
            Positions[i][j] = (FLOAT32)(rand() % (upper_bound[j] - lower_bound[j] + 1) + lower_bound[j]);
            //printf("%f\n",Positions[i][j]);
        }
    }
}

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
FLOAT32 GetFitnessOffline2(UINT16* optimal_speed, FLOAT32* position, UINT16 discrete_size, UINT16 dim, UINT16* speed_limit_max, UINT16* speed_limit_min, UINT16 target_time, UINT8 switch_flag[], UINT8 switch_num)
{
    FLOAT32 v_index = 0.01f; //迭代速度
    UINT32 train_weight = (UINT32)CalTrainWeight();//车重
    FLOAT32 time_sum = 0;//运行时分总和
    FLOAT32 energy_sum = 0;//能耗总和
    FLOAT32 acc_gradient;//坡度附加加速度
    FLOAT32 acc_qxbj;//曲线半径附加加速度
    FLOAT32 acc_w;//基本阻力附加加速度
    FLOAT32 acc_traction;//牵引加速度
    FLOAT32 acc_index;//等效加速度
    FLOAT32 v_next;//下一速度
    FLOAT32 P = 0.99f;//运行时分惩罚系数
    FLOAT32 fitness;//目标函数值
    UINT8 pos_index = 0;//解索引
    for (UINT32 i = 0; i < dim; i++)
    {
        acc_gradient = GetGradientAcc(i * discrete_size);
        acc_qxbj = GetCurveRadiusAcc(i * discrete_size);
        acc_w = GetResistanceAcc((UINT16)v_index);
        acc_traction = GetTractionAcc((UINT16)v_index) * SPEED_PLAN_TB_RATIO;
        if (i * discrete_size <= position[pos_index])
        {
            if (switch_flag[pos_index] == 1)//牵引-惰行
            {
                acc_index = acc_traction - acc_w - acc_gradient - acc_qxbj;
                v_next = sqrt(v_index * v_index + 2 * acc_index * discrete_size);
                if (v_next >= speed_limit_max[i + 1])//下一规划速度超速
                {
                    acc_index = (speed_limit_max[i + 1] * speed_limit_max[i + 1] - v_index * v_index) / (2 * discrete_size);
                    acc_traction = acc_index + acc_w + acc_gradient + acc_qxbj;
                }
                if (acc_traction > 0)
                {
                    energy_sum = energy_sum + (FLOAT32)train_weight * acc_traction * discrete_size / 10000;
                }
                level_output_flag[i] = 1;
            }
            else if (switch_flag[pos_index] == 3)//制动-惰行
            {
            }
            else
            {
                acc_index = -acc_w - acc_gradient - acc_qxbj;
                if (v_index * v_index + 2 * acc_index * discrete_size > 0)
                {
                    v_next = sqrt(v_index * v_index + 2 * acc_index * discrete_size);
                }
                else
                {
                    v_next = 0.1f;
                }
                level_output_flag[i] = 2;
            }
            //更新下一工况切换点
            if ((i + 1) * discrete_size > position[pos_index] && pos_index < switch_num - 1)
            {
                pos_index += 1;
            }
        }
        else
        {
            acc_index = -acc_w - acc_gradient - acc_qxbj;
            if (v_index * v_index + 2 * acc_index * discrete_size > 0)
            {
                v_next = sqrt(v_index * v_index + 2 * acc_index * discrete_size);
            }
            else
            {
                v_next = 0.1f;
            }
            level_output_flag[i] = 2;
        }
        //边界约束
        if (v_next > speed_limit_max[i + 1])
        {
            v_next = speed_limit_max[i + 1];
            //如果当前处于牵引阶段，规划速度大于防护速度，则驾驶阶段切换为惰行。
            if (switch_flag[pos_index] == 1 && i * discrete_size <= position[pos_index])
            {
                level_output_flag[i] = 2;
            }
                //否则驾驶阶段切换为制动
            else
            {
                level_output_flag[i] = 3;
            }
        }
        /*if (v_next < SpeedLimitMin[i + 1])
        {
            v_next = SpeedLimitMin[i + 1];
        }*/
        if (i == dim - 1)
        {
            time_sum = time_sum + (FLOAT32)(discrete_size + remain_section_length) / ((v_index + v_next) / 2);
        }
        else
        {
            time_sum = time_sum + (FLOAT32)discrete_size / ((v_index + v_next) / 2);
        }
        optimal_speed[i] = (UINT16)v_index;
        plan_time[i] = time_sum;
        v_index = v_next;
    }
    optimal_speed[dim] = (UINT16)v_index;
    level_output_flag[dim] = 3;
    fitness = (1 - P) * energy_sum / 1000000 + P * abs(time_sum - target_time);
    return fitness;
}

/*************************************************************************
* 功能描述: 计算列车质量
* 输入参数: 无
* 输出参数: 无
* 返回值:   FLOAT64	train_weight	列车质量, kg
*************************************************************************/
FLOAT64 CalTrainWeight()
{
    return (FLOAT64)g_train_param.aw[g_aw_id] * 1000.0;	// 质量单位是吨, 乘1000转化为kg
}

/*************************************************************************
* 功能描述: 根据ATO配置数据中的特性曲线计算某一速度下的最大加速度
* 输入参数: FLOAT64	speed		列车速度, km/h
*			  UINT16    num			特性曲线散点数
*			  UINT16*	curve_v		曲线速度项数组, km/h
*			  UINT16*	curve_a		曲线加速度项数组, cm/s/s
* 输出参数: 无
* 返回值:   FLOAT64	acc			最大加速度, cm/s/s
*************************************************************************/
FLOAT64 GetAccBySpeed(FLOAT64 kph, UINT16 num, const UINT16* curve_v, const UINT16* curve_a)
{
    FLOAT64 acc = 0;					// 加速度返回值，cm/s/s
    if (kph <= curve_v[0])
    {
        acc = curve_a[0];
    }
    else if (kph >= curve_v[num - 1])
    {
        acc = curve_a[num - 1];
    }
    else
    {
        for (UINT16 i = 1; i < num; i++)
        {
            if (kph <= curve_v[i])
            {
                FLOAT64 ratio = (FLOAT64)(curve_v[i] - kph) / (FLOAT64)(curve_v[i] - curve_v[i - 1]);
                acc = curve_a[i - 1] * ratio + curve_a[i] * (1 - ratio);
                break;
            }
        }
    }
    return acc;
}

/*************************************************************************
* 功能描述: 根据速度计算牵引加速度
* 输入参数:
*		UINT16						speed					速度 cm/s
* 输出参数:
* 返回值:
*		UINT16						acc_traction		    最大加速度 cm/s^2
*************************************************************************/
FLOAT32 GetTractionAcc(UINT16 speed)
{
    FLOAT64 kph = 1.0 * speed / 100 * 3.6;
    FLOAT64 acc = 0;							// 加速度返回值，cm/s/s
    const TRAIN_PARAMETER* tp = &g_train_param;	// 列车参数指针
    switch (g_aw_id)
    {
        case 0:
            acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a0);
            break;
        case 1:
            acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a1);
            break;
        case 2:
            acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a2);
            break;
        case 3:
            acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a3);
            break;
        default:
            acc = GetAccBySpeed(kph, tp->traction_num, tp->traction_v, tp->traction_a0);
            break;
    }
    tp = NULL;
    return (FLOAT32)acc; // 牵引返回正值
}

/*************************************************************************
* 功能描述: 根据速度计算制动加速度
* 输入参数:
*		UINT16						speed					速度 cm/s
* 输出参数:
* 返回值:
*		UINT16						acc_break				最大减速度 cm/s^2
*************************************************************************/
FLOAT32 GetBreakAcc(UINT16 speed)
{
    FLOAT64 kph = 1.0 * speed / 100 * 3.6;
    FLOAT64 acc = 0;							// 加速度返回值，cm/s/s
    const TRAIN_PARAMETER* tp = &g_train_param;	// 列车参数指针
    switch (g_aw_id)
    {
        case 0:
            acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a0);
            break;
        case 1:
            acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a1);
            break;
        case 2:
            acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a2);
            break;
        case 3:
            acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a3);
            break;
        default:
            acc = GetAccBySpeed(kph, tp->braking_num, tp->braking_v, tp->braking_a0);
            break;
    }
    tp = NULL;
    return (FLOAT32)acc; // 制动返回正值
}
/*************************************************************************
* 功能描述: 根据速度计算基本阻力附加加速度
* 输入参数:
*		UINT16						speed					速度 cm/s
* 输出参数:
* 返回值:
*		FLOAT64						acc_basic_resistance	减速度 cm/s^2
*************************************************************************/
FLOAT64 GetResistanceAcc(UINT16 speed)
{
    FLOAT32 acc_basic_resistance = 0;
    const TRAIN_PARAMETER* tp = &g_train_param;
    double kph = 1.0 * speed / 100 * 3.6;
    UINT16 idx = g_aw_id;
    FLOAT64 rf = (tp->a[idx] + tp->b[idx] * kph + tp->c[idx] * kph * kph) * 1000.0; // 戴维斯公式结果是KN, 转化为N
    tp = NULL;
    acc_basic_resistance = (FLOAT32)(rf / CalTrainWeight() * 100);
    return acc_basic_resistance;
}


/*************************************************************************
* 功能描述: 根据位置计算坡度附加加速度
* 输入参数:
*		UINT32						dispalcement			位移 cm
* 输出参数: 无
* 返回值:
*		FLOAT32						gradient_acc			加速度 cm/s^2
*************************************************************************/
FLOAT32 GetGradientAcc(UINT32 dispalcement)
{
    FLOAT32 gradient_acc = 0;/*坡度附加加速度*/
    UINT16 gradient_front_index, gradient_rear_index;/*前后离散坡度对应索引*/
    FLOAT32 gradient_current_index;
    if (dispalcement>interval_length * 100)/*输入参数检查*/
    {
        /*什么也不做*/
    }
    else
    {
        gradient_front_index = dispalcement / (g_line_param.discrete_size*100);
        gradient_rear_index = gradient_front_index + 1;
        if ((gradient_front_index > 0 && gradient_front_index < MAX_INTERVAL_SAMPLING)
            && (gradient_rear_index > 0 && gradient_rear_index < MAX_INTERVAL_SAMPLING))
        {
            gradient_current_index = (FLOAT32)(((FLOAT64)dispalcement - gradient_front_index * g_line_param.discrete_size*100*1.0) / (g_line_param.discrete_size*100));
            gradient_acc = ((gradient[gradient_front_index] + (gradient[gradient_front_index] - gradient[gradient_rear_index]) * gradient_current_index) * GRAVITY_ACC / 10);
        }
    }
    //printf("%f\n",gradient_acc);
    return gradient_acc;
}

/*************************************************************************
* 功能描述: 根据位置计算曲线半径附加加速度
* 输入参数:
*		UINT32						dispalcement			位移 cm
* 输出参数: 无
* 返回值:
*		FLOAT32						curve_radius_acc		加速度 cm/s^2
*************************************************************************/
FLOAT32 GetCurveRadiusAcc(UINT32 dispalcement)
{
    FLOAT32 curve_radius_acc = 0;/*坡度附加加速度*/
    UINT16 curve_radius_index;
    if (dispalcement>interval_length * 100)/*输入参数检查*/
    {
        /*什么也不做*/
    }
    else
    {
        curve_radius_index = dispalcement / (g_line_param.discrete_size*100);
        if (curve_radius_index > 0 && curve_radius_index < MAX_INTERVAL_SAMPLING && curve_radius[curve_radius_index] != 0)
            curve_radius_acc = (FLOAT32)(600.0 / curve_radius[curve_radius_index] * GRAVITY_ACC / 10);
    }
    return curve_radius_acc;
}

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
UINT8 GetBounderOffline(UINT32* lower_bound, UINT32* upper_bound, UINT8* switch_flag)
{
    UINT32 station_jump_begin = 0;
    UINT32 station_jump_end = 0;
    //如果区间长度小于5000m，不需要多阶段优化
    if (interval_length < 8000)
    {
        lower_bound[0] = 0;
        upper_bound[0] = interval_length_cm;
        switch_flag[0] = 1;//牵引
        return 1;
    }
        //否则按照多阶段优化
    else
    {
        station_jump_begin=interval_length_cm/4;
        station_jump_end=station_jump_begin+interval_length_cm/2;
        lower_bound[0] = 0;
        upper_bound[0] = station_jump_begin; //对半分
        switch_flag[0] = 1;//牵引-惰行

        lower_bound[1] = station_jump_begin;
        upper_bound[1] = station_jump_end;
        switch_flag[1] = 2;//惰行-牵引

        lower_bound[2] = station_jump_end;
        upper_bound[2] = interval_length_cm;
        switch_flag[2] = 1;//牵引-惰行
        return 3;
    }
}

/*************************************************************************
* 功能描述: 根据当前载荷和运行方向得到离线优化曲线索引
* 输入参数:
* 输出参数:
* 返回值:
*	    UINT16     离线优化速度数量
*************************************************************************/
void GetOptimizeSpeedIndex(UINT32 *distance,UINT16 *optimize_speed,UINT8 *level,UINT16 *length)
{
    FLOAT32 *speed_temp=NULL;  //离线优化速度临时指针
    UINT8 *work_temp=NULL;    //离线优化工况临时指针
    //根据运行方向、载荷找到相应离线优化速度和工况
    if(g_speed_plan_info.current_direction==DIRECTION_DOWN)
    {
        switch (g_aw_id)
        {
            //AW0
            case 0:
                speed_temp=g_static_data_csv.optimize_csv.speed_down_aw0;
                work_temp=g_static_data_csv.optimize_csv.level_flag_down_aw0;
                break;
            //AW1
            case 1:
                speed_temp=g_static_data_csv.optimize_csv.speed_down_aw1;
                work_temp=g_static_data_csv.optimize_csv.level_flag_down_aw1;
                break;
            //AW2
            case 2:
                speed_temp=g_static_data_csv.optimize_csv.speed_down_aw2;
                work_temp=g_static_data_csv.optimize_csv.level_flag_down_aw2;
                break;
            //AW3
            case 3:
                speed_temp=g_static_data_csv.optimize_csv.speed_down_aw3;
                work_temp=g_static_data_csv.optimize_csv.level_flag_down_aw3;
                break;
            default:
                break;
        }
    }
    else
    {
        switch (g_aw_id)
        {
            case 0:
                speed_temp=g_static_data_csv.optimize_csv.speed_up_aw0;
                work_temp=g_static_data_csv.optimize_csv.level_flag_up_aw0;
                break;
            case 1:
                speed_temp=g_static_data_csv.optimize_csv.speed_up_aw1;
                work_temp=g_static_data_csv.optimize_csv.level_flag_up_aw1;
                break;
            case 2:
                speed_temp=g_static_data_csv.optimize_csv.speed_up_aw2;
                work_temp=g_static_data_csv.optimize_csv.level_flag_up_aw2;
                break;
            case 3:
                speed_temp=g_static_data_csv.optimize_csv.speed_up_aw3;
                work_temp=g_static_data_csv.optimize_csv.level_flag_up_aw3;
                break;
            default:
                break;
        }
    }
    //判断指针是否有效
    if(speed_temp!=NULL&&work_temp!=NULL)
    {
        UINT16 optimize_num=0;
        //根据运行方向，提取对应当前区间的速度曲线和工况
        if(g_speed_plan_info.current_direction==DIRECTION_DOWN) {
            //遍历静态速度数据
            for (int i = 0; i < g_static_data_csv.optimize_csv.length; i++)
            {
                if (g_static_data_csv.optimize_csv.distance[i] >= g_speed_plan_info.interval_begin_dis &&
                    g_static_data_csv.optimize_csv.distance[i] <= g_speed_plan_info.interval_end_dis) {
                    distance[optimize_num] = g_static_data_csv.optimize_csv.distance[i];
                    optimize_speed[optimize_num] = (UINT16) (speed_temp[i] * 1000 / 36);
                    level[optimize_num] = work_temp[i];
                    optimize_num += 1;
                }

            }
        }
        else
        {
            //遍历静态速度数据
            for (int i = g_static_data_csv.optimize_csv.length-1; i >=0; i--)
            {
                if (g_static_data_csv.optimize_csv.distance[i] <= g_speed_plan_info.interval_begin_dis &&
                    g_static_data_csv.optimize_csv.distance[i] >= g_speed_plan_info.interval_end_dis)
                {
                    distance[optimize_num] = g_static_data_csv.optimize_csv.distance[i];
                    optimize_speed[optimize_num] = (UINT16) (speed_temp[i] * 1000 / 36);
                    level[optimize_num] = work_temp[i];
                    optimize_num += 1;
                }
            }
        }
        *length=optimize_num;
    }
    else
    {
        *length=0;
    }
}

/*************************************************************************
* 功能描述: 根据优化速度划分驾驶阶段
* 输入参数:
* 输出参数:
* 返回值:
*	UINT8  1：正常 0：异常
*************************************************************************/
UINT8 DivideStageByOptimizeSpeed()
{
    UINT16 traction_num;//牵引工况数量
    g_speed_plan_info.recommend_change_num=0;
    //如果在线优化误差大于一定阈值，则切换为离线优化速度
    if (g_speed_plan_info.optimize_evaluate>500)
    {
        UINT32 optimize_distance[500];
        UINT16 optimize_speed[500];
        UINT8 optimize_work[500];
        UINT16 optimize_num=0;
        GetOptimizeSpeedIndex(optimize_distance,optimize_speed,optimize_work,&optimize_num);
        for(int i=0;i <= optimize_num; i++)
        {
            UINT16 level=optimize_work[i];
            if(optimize_work[i]!=1)
            {
                //找到启动牵引阶段结束点，记录需要牵引的数量，后续划分为3个子阶段
                traction_num=i;
                break;
            }
        }
        //下行
        if (g_speed_plan_info.current_direction==DIRECTION_DOWN)
        {
            //启动牵引阶段划分为3个子阶段
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=optimize_distance[traction_num/3];
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=optimize_speed[traction_num/3];
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=1;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=traction_num/3;
            g_speed_plan_info.recommend_change_num+=1;
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=optimize_distance[2*traction_num/3];
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=optimize_speed[2*traction_num/3];
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=1;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=2*traction_num/3;
            g_speed_plan_info.recommend_change_num+=1;
            //中间阶段按照工况切换点填充
            UINT16 work_index=1;
            for (int i = 0; i < optimize_num; i++)
            {
                if(optimize_work[i]!=work_index)
                {
                    g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=optimize_distance[i];
                    g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=optimize_speed[i];
                    g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=optimize_work[i];
                    g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=i;
                    g_speed_plan_info.recommend_change_num+=1;
                    work_index=optimize_work[i];
                }
            }
            //停车点推荐速度为0，推荐工况无效
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=optimize_distance[optimize_num-1];
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=0;
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=5;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=optimize_num-1;
            g_speed_plan_info.recommend_change_num+=1;
        }
        else
        {
            //启动牵引阶段划分为3个子阶段
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=optimize_distance[traction_num/3];
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=optimize_speed[traction_num/3];
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=1;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=traction_num/3;
            g_speed_plan_info.recommend_change_num+=1;
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=optimize_distance[2*traction_num/3];
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=optimize_speed[2*traction_num/3];
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=1;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=2*traction_num/3;
            g_speed_plan_info.recommend_change_num+=1;
            //中间阶段按照工况切换点填充
            UINT16 work_index=1;
            for (int i = 0; i < optimize_num; i++)
            {
                if(optimize_work[i]!=work_index)
                {
                    g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=optimize_distance[i];
                    g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=optimize_speed[i];
                    g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=optimize_work[i];
                    g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=i;
                    g_speed_plan_info.recommend_change_num+=1;
                    work_index=optimize_work[i];
                }
            }
            //停车点推荐速度为0，推荐工况无效
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=optimize_distance[optimize_num-1];
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=0;
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=5;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=optimize_num-1;
            g_speed_plan_info.recommend_change_num+=1;
        }
    }
    else
    {
        for(int i=0;i <= dim; i++)
        {
            UINT16 level=g_level_output_flag[i];
            if(g_level_output_flag[i]!=1)
            {
                //找到启动牵引阶段结束点，记录需要牵引的数量，后续划分为3个子阶段
                traction_num=i;
                break;
            }
        }
        //下行
        if (g_speed_plan_info.current_direction==DIRECTION_DOWN)
        {
            //启动牵引阶段划分为3个子阶段
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=traction_num/3*discrete_size/100+g_speed_plan_info.interval_begin_dis;
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=g_speed_curve_offline[traction_num/3];
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=1;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=traction_num/3;
            g_speed_plan_info.recommend_change_num+=1;
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=2*traction_num/3*discrete_size/100+g_speed_plan_info.interval_begin_dis;
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=g_speed_curve_offline[2*traction_num/3];
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=1;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=2*traction_num/3;
            g_speed_plan_info.recommend_change_num+=1;
            //中间阶段按照工况切换点填充
            UINT16 work_index=1;
            for (int i = 0; i <= dim; i++)
            {
                if(g_level_output_flag[i]!=work_index)
                {
                    g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=i*discrete_size/100+g_speed_plan_info.interval_begin_dis;
                    g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=g_speed_curve_offline[i];
                    g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=g_level_output_flag[i];
                    g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=i;
                    g_speed_plan_info.recommend_change_num+=1;
                    work_index=g_level_output_flag[i];
                }
            }
            //停车点推荐速度为0，推荐工况无效
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=interval_length+g_speed_plan_info.interval_begin_dis;
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=0;
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=5;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=dim;
            g_speed_plan_info.recommend_change_num+=1;
        }
        else
        {
            if(g_speed_plan_info.interval_begin_dis<interval_length)
            {
                printf("SPEED_PLAN:error!\n");
                return 0;//数据异常直接退出
            }
            //启动牵引阶段划分为3个子阶段
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=g_speed_plan_info.interval_begin_dis-traction_num/3*discrete_size/100;
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=g_speed_curve_offline[traction_num/3];
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=1;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=traction_num/3;
            g_speed_plan_info.recommend_change_num+=1;
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=g_speed_plan_info.interval_begin_dis-2*traction_num/3*discrete_size/100;
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=g_speed_curve_offline[2*traction_num/3];
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=1;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=2*traction_num/3;
            g_speed_plan_info.recommend_change_num+=1;
            //中间阶段按照工况切换点填充
            UINT16 work_index=1;
            for (int i = 0; i <= dim; i++)
            {
                if(g_level_output_flag[i]!=work_index)
                {
                    g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=g_speed_plan_info.interval_begin_dis-i*discrete_size/100;
                    g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=g_speed_curve_offline[i];
                    g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=g_level_output_flag[i];
                    g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=i;
                    g_speed_plan_info.recommend_change_num+=1;
                    work_index=g_level_output_flag[i];
                }
            }
            //停车点推荐速度为0，推荐工况无效
            g_speed_plan_info.recommend_distance[g_speed_plan_info.recommend_change_num]=g_speed_plan_info.interval_begin_dis-interval_length;
            g_speed_plan_info.recommend_speed[g_speed_plan_info.recommend_change_num]=0;
            g_speed_plan_info.recommend_wrok[g_speed_plan_info.recommend_change_num]=5;
            g_speed_plan_info.recommend_index[g_speed_plan_info.recommend_change_num]=dim;
            g_speed_plan_info.recommend_change_num+=1;
        }
    }
//    for (int i = 0; i < g_speed_plan_info.recommend_change_num; i++)
//    {
//        printf("%d,%d,%d,%d,%d\n",i,g_speed_plan_info.recommend_distance[i],g_speed_plan_info.recommend_speed[i],
//               g_speed_plan_info.recommend_wrok[i],g_speed_plan_info.recommend_index[i]);
//    }
    return 1;
}

/*************************************************************************
* 功能描述: 根据起始索引和结束索引计算时间
* 输入参数:
*        UINT16           begin_index      开始索引
*        UINT16           end_index        结束索引
*        const UINT16     speed_curve[]    速度曲线
*        UINT16           discrete_dis
* 输出参数:
* 返回值:
*	     UINT16  倒计时
*************************************************************************/
UINT16 GetTimeByIndex(UINT16 begin_index,UINT16 end_index,const UINT16 speed_curve[],UINT16 discrete_dis)
{
    FLOAT32 time_sum=1;
    if (begin_index>=end_index)
    {
        return 0;
    }
    for(int i=begin_index;i<end_index-1;i++)
    {
        if (speed_curve[i]+speed_curve[i+1]!=0)
        {
            time_sum+=(FLOAT32)(2.0*discrete_dis/(speed_curve[i]+speed_curve[i+1]));
        }
        else
        {
            printf("cal time error!");
        }
    }
    return (UINT16)time_sum;
}


/*************************************************************************
* 功能描述: 根据列车当前位置，查询下一阶段推荐速度、推荐工况和生效倒计时
* 输入参数:
*        UINT32         distance          当前公里标
* 输出参数:
*        UINT16          *rec_speed       下一阶段推荐速度 cm/s
*        UINT8           *rec_work         下一阶段推荐工况
*        UINT16          *rec_cutdown     下一阶段生效倒计时 s
*        UINT32          *rec_distance    下一阶段生效距离 m
* 返回值:
*		   UINT8  1：正常 0：异常
*************************************************************************/
UINT8 GetRecSpdAndWorkByDis(UINT32 distance,UINT16 *rec_speed,UINT8 *rec_work,UINT16 *rec_cutdown,UINT32 *rec_distance)
{
    UINT8 result=0;
    UINT16 current_index;//当前位置相对于优化曲线索引
    //参数输入校验
    if((g_direction==DIRECTION_DOWN&&(distance<g_speed_plan_info.interval_begin_dis||distance>g_speed_plan_info.interval_end_dis))||
      (g_direction==DIRECTION_UP&&(distance>g_speed_plan_info.interval_begin_dis||distance<g_speed_plan_info.interval_end_dis)))
    {
        /*输入参数异常*/
        return result;
    }

    /*计算下一阶段推荐速度和推荐工况*/
    UINT8 next_recommend_index=0;
    UINT8 find_flag=0;//索引成功标志
    /*如果是下行*/
    if (g_direction==DIRECTION_DOWN)
    {
        for (int i = 0; i < g_speed_plan_info.recommend_change_num-1; i++)
        {
            //如果当前里程小于初始推荐生效位置，则使用初始推荐
            if (distance < g_speed_plan_info.recommend_distance[0])
            {
                *rec_speed = g_speed_plan_info.recommend_speed[0];
                *rec_work = g_speed_plan_info.recommend_wrok[0];
                next_recommend_index=0;
                find_flag=1;
                break;
            }
            //根据当前里程索引下一推荐
            else if(distance>=g_speed_plan_info.recommend_distance[i]&&distance<g_speed_plan_info.recommend_distance[i+1])
            {
                *rec_speed = g_speed_plan_info.recommend_speed[i+1];
                *rec_work = g_speed_plan_info.recommend_wrok[i+1];
                next_recommend_index=i+1;
                find_flag=1;
                break;
            }
            else
            {
                continue;
            }
        }
        //如果索引失败，推荐置无效
        if(find_flag==0)
        {
            *rec_speed=0;
            *rec_work=5;
            next_recommend_index=0;
        }
        /*计算生效距离*/
        *rec_distance=(g_speed_plan_info.recommend_distance[next_recommend_index]-distance)>0?(g_speed_plan_info.recommend_distance[next_recommend_index]-distance):0;
        /*计算生效倒计时*/
        current_index=100*(distance-g_speed_plan_info.interval_begin_dis)/discrete_size;
        *rec_cutdown=GetTimeByIndex(current_index,g_speed_plan_info.recommend_index[next_recommend_index],g_speed_curve_offline,discrete_size);
    }
    /*如果是上行*/
    else
    {
        for (int i = 0; i < g_speed_plan_info.recommend_change_num-1; i++)
        {
            //如果当前里程大于初始推荐生效位置，则使用初始推荐
            if (distance > g_speed_plan_info.recommend_distance[0])
            {
                *rec_speed = g_speed_plan_info.recommend_speed[0];
                *rec_work = g_speed_plan_info.recommend_wrok[0];
                next_recommend_index=0;
                find_flag=1;
                break;
            }
            //根据当前里程索引下一推荐
            else if(distance<=g_speed_plan_info.recommend_distance[i]&&distance>g_speed_plan_info.recommend_distance[i+1])
            {
                *rec_speed = g_speed_plan_info.recommend_speed[i+1];
                *rec_work = g_speed_plan_info.recommend_wrok[i+1];
                next_recommend_index=i+1;
                find_flag=1;
                break;
            }
            else
            {
                continue;
            }
        }
        //如果索引失败，推荐置无效
        if(find_flag==0)
        {
            *rec_speed=0;
            *rec_work=5;
            next_recommend_index=0;
        }
        /*计算生效距离*/
        *rec_distance=(distance-g_speed_plan_info.recommend_distance[next_recommend_index])>0?(distance-g_speed_plan_info.recommend_distance[next_recommend_index]):0;
        /*计算生效倒计时*/
        current_index=100*(g_speed_plan_info.interval_begin_dis-distance)/discrete_size;
        *rec_cutdown=GetTimeByIndex(current_index,g_speed_plan_info.recommend_index[next_recommend_index],g_speed_curve_offline,discrete_size);
    }

}

/*************************************************************************
* 功能描述: 根据列车当前位置，查询推荐速度、级位标志、级位输出（实验室验证环节需要）
* 输入参数:
* 输出参数:
* UINT16                    *target_speed        推荐速度cm/s
* UINT8                     *level_flag          级位标识 1：牵引 2：惰行 3：制动
* 返回值:
*************************************************************************/
void GetTargetSpeedByDistance(UINT16 *target_speed,UINT8 *level_flag)
{
    int i;
    UINT32 distance_temp;
    //下行
    if (g_speed_plan_info.current_direction==0)
    {
        if(g_period_msg_from_signal.train_distance>g_speed_plan_info.interval_begin_dis)
        {
            distance_temp=(g_period_msg_from_signal.train_distance-g_speed_plan_info.interval_begin_dis)*100;
        }
        else
        {
            distance_temp=0;
        }
    }
    //上行
    else
    {
        if(g_speed_plan_info.interval_begin_dis>g_period_msg_from_signal.train_distance)
        {
            distance_temp=(g_speed_plan_info.interval_begin_dis-g_period_msg_from_signal.train_distance)*100;
        }
        else
        {
            distance_temp=0;
        }
    }
    //如果当前优化完成
    if (g_speed_plan_info.optimize_stage==2&&g_speed_plan_info.optimize_station==g_speed_plan_info.next_station_id)
    {
        for (i = 0; i < dim; i++)
        {
            if (distance_temp>=i*discrete_size&&distance_temp<(i+1)*discrete_size)
            {
                if (i!=0)
                {
                    *target_speed=(UINT16)(1.0*g_speed_curve_offline[i]+1.0*(distance_temp-discrete_size*i)*(g_speed_curve_offline[i+1]-g_speed_curve_offline[i])/discrete_size);
                    *level_flag=g_level_output_flag[i];
                }
                else
                {
                    *target_speed=g_speed_curve_offline[1];
                    *level_flag=g_level_output_flag[1];
                }
                break;
            }
        }
        if(i==dim)
        {
            *target_speed=0;
            *level_flag=3;
        }
    }
    else
    {
        *target_speed=0;
        *level_flag=3;
    }

}
#include "can.h"

static volatile int can0;
static volatile int can1;

/*************************************************************************
 * 功能描述: GBK转化为UTF-8
 * 输入参数: input 输入
 * 输出参数: 无
 * 返回值:   UINT64数据
 *************************************************************************/
//int CodeConvertGBKToUTF8(char* input, size_t charInPutLen, char* output, size_t charOutPutLen)
//{
//    int ret = 0;
//    iconv_t cd;
//    cd = iconv_open("utf-8", "GBK");
//    if(cd==-1)
//    {
//        LogWrite(INFO,"%s-%s",g_current_time,"iconv open error");
//    }
//    ret = iconv(cd, &input, &charInPutLen, &output, &charOutPutLen);
//    if(ret==-1)
//    {
//        LogWrite(INFO,"%s-%s",g_current_time,"iconv utf-8 error");
//    }
//    iconv_close(cd);
//    return ret;
//}

/*************************************************************************
 * 功能描述: 将8字节数据流变为UINT64
 * 输入参数: input 输入
 * 输出参数: 无
 * 返回值:   UINT64数据
 *************************************************************************/
INT64 LongLongFromChar(const UINT8 *input)
{
    INT64 temp;
    temp = (*input);
    temp = (temp << 8) + (*(input + 1));
    temp = (temp << 8) + (*(input + 2));
    temp = (temp << 8) + (*(input + 3));
    temp = (temp << 8) + (*(input + 4));
    temp = (temp << 8) + (*(input + 5));
    temp = (temp << 8) + (*(input + 6));
    temp = (temp << 8) + (*(input + 7));
    return temp;
}

/*************************************************************************
  * 功能描述: 将6字节数据流变为UINT32车次号
  * 输入参数: input 输入
  * 输出参数: 无
  * 返回值:   车次号
  *************************************************************************/
UINT32 TrainIDFromChar(const UINT8 *input)
{
    UINT32 train_id=0;
    train_id=input[0]*100000+input[1]*10000+input[2]*1000+input[3]*100+input[4]*10+input[5];
    return train_id;
}

/*************************************************************************
  * 功能描述: 将4字节数据流变为UINT32车组号
  * 输入参数: input 输入
  * 输出参数: 无
  * 返回值:   车组号
  *************************************************************************/
UINT32 TrainNumberFromChar(const UINT8 *input)
{
    UINT32 train_number=0;
    train_number=input[0]*1000+input[1]*100+input[2]*10+input[3];
    return train_number;
}
/*************************************************************************
  * 功能描述: 从Byte中提取Bit
  * 输入参数: input 输入
  * 输出参数: 无
  * 返回值:  无
  *************************************************************************/
void TrainBitDataFromChar(const UINT8 input)
{
    g_period_msg_from_signal.vcb_2_flag=(input&(0x1<<0))>>0;
    g_period_msg_from_signal.vcb_3_flag=(input&(0x1<<1))>>1;
    g_period_msg_from_signal.pantograph_2_flag=(input&(0x1<<2))>>2;
    g_period_msg_from_signal.pantograph_3_flag=(input&(0x1<<3))>>3;
    g_period_msg_from_signal.control_mode_flag=(input&(0x1<<4))>>4;
    g_period_msg_from_signal.door_flag=(input&(0x1<<5))>>5;
    g_period_msg_from_signal.handle_traction_flag=(input&(0x1<<6))>>6;
    g_period_msg_from_signal.handle_brake_flag=(input&(0x1<<7))>>7;
    //根据牵引手柄区域计算工况
    if(g_period_msg_from_signal.handle_traction_flag==1&&g_period_msg_from_signal.handle_brake_flag==0)
    {
        g_period_msg_from_signal.train_work_condition=1;//牵引工况
    }
    else if (g_period_msg_from_signal.handle_brake_flag==1&&g_period_msg_from_signal.handle_traction_flag==0)
    {
        g_period_msg_from_signal.train_work_condition=3;//制动工况
    }
    else
    {
        g_period_msg_from_signal.train_work_condition=4;//无效
    }
}



/*************************************************************************
  * 功能描述: 从数据流中提取时间
  * 输入参数: input 输入
  * 输出参数: 时间
  * 返回值:  无
  *************************************************************************/
void GetDateFromChar(const UINT8 *input,CHAR *date)
{
    time_t t= 946699200;
    struct tm *p;
    p= gmtime(&t);
    p->tm_year=100+input[0];
    p->tm_mon=input[1]-1;
    p->tm_mday=input[2];
    p->tm_hour=input[3];
    p->tm_min=input[4];
    p->tm_sec=input[5];
    strftime(date,20,"%Y-%m-%d %H:%M:%S",p);
}

/*************************************************************************
 * 功能描述: 时间戳转换为日期
 * 输入参数: INT64          time_stamp           时间戳
 *          CHAR           date
 *          UINT16         length
 * 输出参数: 无
 * 返回值:  无
 *************************************************************************/
void TimeStampToDate(INT64 time_stamp,CHAR *date,UINT16 length)
{
    time_t t= time_stamp+8*3600;
    struct tm *p;
    p= gmtime(&t);
    strftime(date,length,"%Y-%m-%d %H:%M:%S",p);
}

/*************************************************************************
 * 功能描述: 日期转换为时间戳
 * 输入参数:  CHAR           date
 *          UINT16         length
 * 输出参数: 无
 * 返回值:  long 时间戳
 *************************************************************************/
long DateToTimeStamp(CHAR *date)
{
    struct tm tm = {0};  // 初始化 tm 结构
    int year, month, day, hour, minute,second;
    long t = 0;
    // 解析日期时间字符串
    // 注意：依据你的日期时间格式，这里可能需要调整 sscanf 的格式字符串
    if (sscanf(date, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6)
    {
        // struct tm 中的年份是从1900年开始的，月份是从0开始的
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;  // 假设秒是0
        tm.tm_isdst = -1;  // 自动检测夏令时
        t= mktime(&tm);
    }
    else
    {
        t=0;
    }
    return t;
}

/*************************************************************************
 * 功能描述: 求弧度
 * 输入参数: double      d       角度
 * 输出参数: 无
 * 返回值:  double      弧度
 *************************************************************************/
double radian(double d)
{
    return d*PI/180.0;//角度1度=PI/180
}

/*************************************************************************
 * 功能描述: 根据两点经纬度计算距离
 * 输入参数: double              lat_last       上周期纬度
 *          double              lng_last       上周期经度
 *          double              lat            纬度
 *          double              lng            经度
 * 输出参数: 无
 * 返回值:  double 距离 m
 *************************************************************************/
double GetDistanceByPoint(double lat_last,double lng_last,double lat,double lng)
{
    double lat_last_rad= radian(lat_last);
    double lat_rad= radian(lat);
    double a=lat_last_rad-lat_rad;
    double b= radian(lng_last)- radian(lng);
    double dst=2* asin((sqrt(pow(sin(a/2),2)+cos(lat_last_rad)* cos(lat_rad)* pow(sin(b/2),2))));
    dst=dst*EARTH_RADIUS;
    dst= round(dst*10000)/10;
    return dst;
}

/*************************************************************************
 * 功能描述: 根据本周期速度和上周期速度计算距离
 * 输入参数: double              speed_last       上周期速度 km/h
 *          double              speed            本周期速度 km/h
 * 输出参数: 无
 * 返回值:  double 距离 m
 *************************************************************************/
double GetDistanceBySpeed(UINT16 speed_last,UINT16 speed)
{
    double result=1.0*(speed_last+speed)/2*10/36*PERIOD;
    return result;
}

/*************************************************************************
 * 功能描述: 更新当前运行方向
 * 输出参数: 无
 * 返回值:
 *************************************************************************/
UINT8 GetCurrentDirection()
{
    UINT8 result=0;
    g_direction=DIRECTION_DOWN;
    result=1;
    return result;
}


/*************************************************************************
 * 功能描述: 更新当前公里标
 * 输出参数: 无
 * 返回值:  double 距离 m
 *************************************************************************/
UINT8 GetCurrentDistance()
{
    UINT8 result=0;
    double period_dis=0; //m
    //GPS数据有效性检查，如果经纬度不为0,且方向一致
    if (g_period_msg_from_signal.latitude_value!=0&&g_period_msg_from_signal.longitude_value!=0
    &&g_period_msg_from_signal.latitude_value_last!=0&&g_period_msg_from_signal.longitude_value_last!=0
    &&g_period_msg_from_signal.latitude_direction==g_period_msg_from_signal.latitude_direction_last
    &&g_period_msg_from_signal.longitude_direction==g_period_msg_from_signal.longitude_direction_last)
    {
        //1.根据经纬度计算周期走行距离
//        double lat_last = 1.0*g_period_msg_from_signal.latitude_value_last/10000000;
//        double lng_last = 1.0*g_period_msg_from_signal.longitude_value_last/10000000;
//        double lat = 1.0*g_period_msg_from_signal.latitude_value/10000000;
//        double lng = 1.0*g_period_msg_from_signal.longitude_value/10000000;
//        period_dis=GetDistanceByPoint(lat_last,lng_last,lat,lng);
        //2.根据速度计算周期走行距离
        period_dis=GetDistanceBySpeed(g_period_msg_from_signal.train_speed_last,g_period_msg_from_signal.train_speed);
        if (g_direction==DIRECTION_DOWN)
        {
            g_period_msg_from_signal.train_distance_double+=period_dis;//累加周期走行
            g_period_msg_from_signal.train_distance=g_period_msg_from_signal.train_distance_double;
        }
        else
        {
            //如果当前公里标大于周期走行距离
            if(g_period_msg_from_signal.train_distance>period_dis)
            {
                g_period_msg_from_signal.train_distance_double-=period_dis;//累减周期走行
                g_period_msg_from_signal.train_distance=g_period_msg_from_signal.train_distance_double;
            }
            else
            {
                g_period_msg_from_signal.train_distance=0;
                g_period_msg_from_signal.train_distance_double=0;
            }
        }

        //停站阶段，根据GPS定位辅助速度差分，校正公里标，只在开门时进行一次
        if(g_period_msg_from_signal.door_flag_last==1&&g_period_msg_from_signal.door_flag==0&&g_period_msg_from_signal.train_speed==0)
        {
            UINT32 error_dis_gps=65535; //GPS定位误差
            UINT32 error_dis_spd=65535; //速度积分定位误差
            UINT16 station_index_gps=65535; //GPS定位的站索引
            UINT16 station_index_spd=65535; //速度积分定位的站索引
            //1.根据GPS计算最近车站
            double lat = 1.0*g_period_msg_from_signal.latitude_value/100000;
            double lng = 1.0*g_period_msg_from_signal.longitude_value/100000;
            for(int i=0;i<g_static_data_csv.station_csv.length;i++)
            {
                double lat_temp=g_static_data_csv.station_csv.latitude[i];
                double lng_temp=g_static_data_csv.station_csv.longitude[i];
                error_dis_gps=GetDistanceByPoint(lat_temp,lng_temp,lat,lng);
                if(error_dis_gps<DISTANCE_ERROR)
                {
                    station_index_gps=i;
                    break;
                }
            }
            //2.根据速度积分计算最近车站
            for(int i=0;i<g_static_data_csv.station_csv.length;i++)
            {
                UINT32 station_dis_temp=g_static_data_csv.station_csv.begin_distance[i];
                UINT32 current_dis=g_period_msg_from_signal.train_distance;
                error_dis_spd=(current_dis>station_dis_temp)?(current_dis-station_dis_temp):(station_dis_temp-current_dis);
                if(error_dis_spd<DISTANCE_ERROR)
                {
                    station_index_spd=i;
                    break;
                }

            }
            //比较两者定位是否一致
            if(station_index_spd!=65535&&station_index_gps==station_index_spd)
            {
                g_period_msg_from_signal.train_distance=g_static_data_csv.station_csv.begin_distance[station_index_gps];
                g_period_msg_from_signal.train_distance_double=g_period_msg_from_signal.train_distance;
                printf("%s POSITION_CORRECTION:current_station:%d,dis:%d\n",g_current_time,g_static_data_csv.station_csv.station_id[station_index_gps],g_period_msg_from_signal.train_distance);
                LogWrite(INFO,"%s-%s,%d,%d",g_current_time,"POSITION_CORRECTION",g_static_data_csv.station_csv.station_id[station_index_gps],g_period_msg_from_signal.train_distance);
            }
        }

        result=1;
        return result;
    }
    else
    {
        printf("latitude or longitude data error\n");
        return result;
    }




}

/*************************************************************************
 * 功能描述: 更新下一站
 * 输出参数: 无
 * 返回值:  double 距离 m
 *************************************************************************/
void GetCurrentPlan()
{
    //如果当前列车车门打开，且当前速度为0,且计划更新
    if(g_period_msg_from_signal.door_flag==0&&g_period_msg_from_signal.train_speed==0&&g_plan_config_info.plan_refresh_flag==1)
    {
        //根据公里标查询当前站
        UINT8 plan_station_num=g_plan_config_info.plan_station_num;
        UINT8 current_station_index=0;
        UINT32 current_dis=g_period_msg_from_signal.train_distance;
        UINT32 station_dis=0;
        UINT32 error_dis=65535;
        g_period_msg_from_signal.next_station_id=0;//出口参数防护
        for (int i = 0; i < g_plan_config_info.plan_station_num; i++)
        {
            station_dis=g_plan_config_info.plan_station_info[i].distance;
            error_dis=current_dis>station_dis?current_dis-station_dis:station_dis-current_dis;
            if(error_dis<200)
            {
                current_station_index=i;
                break;
            }
        }
        //如果是下行，向后查找非跳停站
        if (g_plan_config_info.direction==DIRECTION_DOWN)
        {
            for (int i = current_station_index+1; i < g_plan_config_info.plan_station_num; i++)
            {
                //找到下一非跳停站
                if (g_plan_config_info.plan_station_info[i].jump_flag==0)
                {
                    g_period_msg_from_signal.next_station_id=g_plan_config_info.plan_station_info[i].station_id;
                    //printf("%s next station:%d!\n",g_current_time,g_period_msg_from_signal.next_station_id);
                    break;
                }
            }

        }
        //如果是上行，向前查找非跳停站
        else if(g_plan_config_info.direction==DIRECTION_UP)
        {
            for (int i = current_station_index-1; i >=0; i--)
            {
                //找到下一非跳停站
                if (g_plan_config_info.plan_station_info[i].jump_flag==0)
                {
                    g_period_msg_from_signal.next_station_id=g_plan_config_info.plan_station_info[i].station_id;
                    //printf("%s next station:%d!\n",g_current_time,g_period_msg_from_signal.next_station_id);
                    break;
                }
            }
        }



    }
}
/*************************************************************************
 * 功能描述: 更新下一站到达时间和发车时间
 * 输出参数: 无
 * 返回值:
 *************************************************************************/
void GetArriveAndLeaveTime()
{
    //arrive
    if (g_period_msg_from_signal.door_flag_last==1&&g_period_msg_from_signal.door_flag==0&&g_period_msg_from_signal.train_speed==0)
    {
        INT32 current_timestamp=DateToTimeStamp(g_current_time);
        INT32 arrive_timestamp=0;
        INT32 leave_timestamp=0;
        UINT16 find_index=65535;
        for(int i=0;i<g_static_data_csv.station_csv.length;i++)
        {
            if(g_period_msg_from_signal.next_station_id==g_static_data_csv.station_csv.station_id[i])
            {
                find_index=i;
                break;
            }
        }
        if (g_direction==DIRECTION_DOWN)
        {
            if(find_index!=0&&find_index!=65535)
            {
                arrive_timestamp=(UINT16)(g_static_data_csv.station_csv.schedule_time[find_index-1])+current_timestamp;
                leave_timestamp=arrive_timestamp+(UINT16)(g_static_data_csv.station_csv.dwell_time[find_index-1]);
            }
        }
        else if(g_direction==DIRECTION_UP)
        {
            if(find_index!=g_static_data_csv.station_csv.length-1&&find_index!=65535)
            {
                arrive_timestamp=(UINT16)(g_static_data_csv.station_csv.schedule_time[find_index])+current_timestamp;
                leave_timestamp=arrive_timestamp+(UINT16)(g_static_data_csv.station_csv.dwell_time[find_index]);
            }
        }
        CHAR date[20];
        TimeStampToDate(arrive_timestamp,date,sizeof(date));
        memcpy(g_period_msg_from_signal.next_station_arrive_time,date,20);
        TimeStampToDate(leave_timestamp,date,sizeof(date));
        memcpy(g_period_msg_from_signal.next_station_leave_time,date,20);
    }
    //leave
    if (g_period_msg_from_signal.door_flag_last==0&&g_period_msg_from_signal.door_flag==1)
    {
        memcpy(g_period_msg_from_signal.current_station_leave_time,g_current_time,20);
    }

}



/*************************************************************************
 * 功能描述: 解包来自车辆网络的can消息
 * 输入参数: UINT8  *receive_buffer  消息存储指针
 *          UINT16 receive_length  消息长度
 * 输出参数: 无
 * 返回值:   UINT8   result          1：解析成功 0：解析失败
 *************************************************************************/
UINT8 UnpackePeriodMsgFromCAN(UINT8 *receive_buffer,UINT16 receive_length)
{
    UINT8 result=0;
    UINT8 *index=receive_buffer;
    index+=1;
    UINT16 message_id = ShortFromChar(index);//解析消息长度
    index+=2;
    UINT16 message_length = *(index++)*8;//解析消息长度
    //消息头校验
    if (message_id==101&&message_length==receive_length)
    {
        g_period_msg_from_train.train_weight=ShortFromChar(index);//解析列车实时载荷
        index+=2;
        g_period_msg_from_train.formation_num = *(index++);//解析列车编组数量
        g_period_msg_from_train.train_length=ShortFromChar(index);//解析列车长度
        index+=2;
        g_period_msg_from_train.traction_voltage= LongFromChar(index);//解析（原边）牵引电压
        index+=4;
//        g_period_msg_from_train.traction_voltage_side= LongFromChar(index);//解析（副边）牵引电压
        g_period_msg_from_train.traction_voltage_side= (UINT32)(1.0*g_period_msg_from_train.traction_voltage / 25000 * 970);
        index+=4;
        g_period_msg_from_train.traction_current_2= ShortFromChar(index);//解析2车（原边）牵引电流
        index+=2;
        g_period_msg_from_train.traction_current_3= ShortFromChar(index);//解析3车（原边）牵引电流
        index+=2;
        g_period_msg_from_train.traction_current_low_2= ShortFromChar(index);//解析2车（原边低压侧）牵引电流
        index+=2;
        g_period_msg_from_train.traction_current_low_3= ShortFromChar(index);//解析3车（原边低压侧）牵引电流
        index+=2;
        g_period_msg_from_train.traction_current_sign=*(index++);//解析列车牵引电流符号
        g_period_msg_from_train.traction_fault_flag=*(index++);//解析列车牵引故障标识
        g_period_msg_from_train.brake_fault_flag=*(index++);//解析列车制动故障标识
        g_period_msg_from_train.other_fault_flag=*(index++);//解析列车其他故障标识
        //自更新变量
        UINT16 traction_current = g_period_msg_from_train.traction_current_2 + g_period_msg_from_train.traction_current_3;
        if (g_period_msg_from_train.traction_current_sign==1)
        {
            g_period_msg_from_train.traction_energy_sum+=CalEnergyByUI(g_period_msg_from_train.traction_voltage,traction_current,0.2f);
        }
        else if (g_period_msg_from_train.traction_current_sign==2)
        {
            g_period_msg_from_train.brake_energy_sum+=CalEnergyByUI(g_period_msg_from_train.traction_voltage,traction_current,0.2f);
        }
//        LogWrite(INFO,"%s:%s,%s-%d,%s-%d,%s-%d,%s-%d","ENERGY:",g_current_time,"work",g_period_msg_from_signal.train_work_condition,"spd",g_period_msg_from_signal.train_speed,
//                 "dis",g_period_msg_from_signal.train_distance,"target_spd",g_speed_plan_info.target_speed);
        //解析实时数据
        g_period_msg_from_signal.traction_energy= LongFromChar(index);//解析列车累积牵引能耗
        index+=4;
        g_period_msg_from_signal.regeneration_energy= LongFromChar(index);//解析列车累积再生能量
        index+=4;
        g_period_msg_from_signal.train_direction=*(index++);//解析列车运行方向
        g_period_msg_from_signal.train_id= TrainIDFromChar(index);//解析车次号
        index+=6;
        g_period_msg_from_signal.train_number= TrainNumberFromChar(index);//解析车组号
        index+=4;
        g_period_msg_from_signal.arrive_flag=*(index++);//解析停准停稳标识
        g_period_msg_from_signal.leave_flag=*(index++);//解析允许发车标识
        g_period_msg_from_signal.door_flag_last=g_period_msg_from_signal.door_flag;//保存上周期车门状态
        g_period_msg_from_signal.byte_data=*(index++);
        TrainBitDataFromChar(g_period_msg_from_signal.byte_data);//解析BYTE
        g_period_msg_from_signal.train_plan_flag=*(index++);//解析列车运行计划更新标识
        g_period_msg_from_signal.train_ebi=ShortFromChar(index);//解析ATP防护速度
        index+=2;
        g_period_msg_from_signal.train_speed_last=g_period_msg_from_signal.train_speed;//保存上周期速度数据
        g_period_msg_from_signal.train_speed=(FLOAT32)ShortFromChar(index)*1.0/100;//解析列车实时速度
        index+=2;
        //g_period_msg_from_signal.next_station_id=ShortFromChar(index);//解析下一站编号
        index+=2;
        CHAR date[20];
        //INT64 arrive_timestamp= LongLongFromChar(index);//解析下一站到达时间
        //TimeStampToDate(arrive_timestamp,date,sizeof(date));
        //memcpy(g_period_msg_from_signal.next_station_arrive_time,date,20);
        index+=8;
        //INT64 leave_timestamp= LongLongFromChar(index);//解析下一站发车时间
        //TimeStampToDate(leave_timestamp,date,sizeof(date));
        //memcpy(g_period_msg_from_signal.next_station_leave_time,date,20);
        index+=8;
        GetDateFromChar(index,date);//解析列车当前时间
        memcpy(g_period_msg_from_signal.train_time,date,20);
        index+=6;
        //根据控制模式解析，速度模式or级位模式
        if(g_period_msg_from_signal.control_mode_flag==SPEED_MODE)
        {
            g_period_msg_from_signal.train_work_speed=ShortFromChar(index);
        }
        else
        {
            g_period_msg_from_signal.train_work_level=ShortFromChar(index)*10;
        }
        index+=2;
        //g_period_msg_from_signal.train_distance_last=g_period_msg_from_signal.train_distance;//保存上周期公里标
        //g_period_msg_from_signal.train_distance= LongFromChar(index);//解析列车公里标
        index+=4;
        g_period_msg_from_signal.longitude_value_last=g_period_msg_from_signal.longitude_value;//保存上周期数据
        g_period_msg_from_signal.longitude_value= LongFromCharLittle(index);//解析经度
        index+=4;
        g_period_msg_from_signal.longitude_direction_last=g_period_msg_from_signal.longitude_direction;//保存上周期数据
        g_period_msg_from_signal.longitude_direction=*(index++);//解析经度方向
        g_period_msg_from_signal.latitude_value_last=g_period_msg_from_signal.latitude_value;//保存上周期数据
        g_period_msg_from_signal.latitude_value= LongFromCharLittle(index);//解析纬度
        index+=4;
        g_period_msg_from_signal.latitude_direction_last=g_period_msg_from_signal.latitude_direction;//保存上周期数据
        g_period_msg_from_signal.latitude_direction=*(index++);//解析纬度方向
        //解析站点名称
        char in[100];
        char dst[100];
        int srclen = 100;
        int dstlen = 100;
        int ret;
        char record[48];
        memcpy(in,index,16);
        memcpy(record,in,16);
        memcpy(g_period_msg_from_signal.current_station_name_GBK,record,16);
//        ret = CodeConvertGBKToUTF8(in,srclen,dst,dstlen);
//        memcpy(g_period_msg_from_signal.current_station_name,dst,dstlen);
        index+=16;
        memcpy(in,index,16);
        memcpy(record+16,in,16);
        memcpy(g_period_msg_from_signal.next_station_name_GBK,in,dstlen);
//        ret = CodeConvertGBKToUTF8(in,srclen,dst,dstlen);
//        memcpy(g_period_msg_from_signal.next_station_name,dst,dstlen);
        index+=16;
        memcpy(in,index,16);
        memcpy(record+32,in,16);
        memcpy(g_period_msg_from_signal.dst_station_name_GBK,in,dstlen);
//        ret = CodeConvertGBKToUTF8(in,srclen,dst,dstlen);
//        memcpy(g_period_msg_from_signal.dst_station_name,dst,dstlen);
        index+=16;
        g_period_msg_from_train.train_fault_code= ShortFromChar(index);
        index+=2;
        //校验位
        index+=4;
        UINT8 check_rec=*(index++);
        UINT32 byte_sum=0;
        for(int i=0;i<receive_length-2;i++)
        {
            byte_sum+=receive_buffer[i];
        }
        UINT8 check_local=byte_sum&0xFF;
        if (check_rec!=check_local)
        {
            printf("CAN receive data check error\n");
            LogWrite(INFO,"%s","CAN receive data check error\n");
            memset(&g_period_msg_from_train,0,sizeof(g_period_msg_from_train));//清空存储结构体
            memset(&g_period_msg_from_signal,0,sizeof(g_period_msg_from_signal));//清空存储结构体
            result = 0;//校验失败
            return result;
        }
        //全局变量更新
        memcpy(g_current_time,g_period_msg_from_signal.train_time,20);//解析列车当前时间
        GetCurrentDirection();//更新当前运行方向
        GetCurrentDistance();//更新当前公里标
        GetCurrentPlan();//更新下一站
        GetArriveAndLeaveTime();//根据运行计划，更新时间
        //printf("dis:%d\n",g_period_msg_from_signal.train_distance);
        printf("CAN data unpack success\n");
        char hexString[sizeof(record)*4+1];
        snprintf(hexString,sizeof (hexString),"%02X%02X",(unsigned char )record[0],(unsigned char )record[1]);
        for(int m=2;m<sizeof(record);m+=2)
        {
            snprintf(hexString+4*(m/2),sizeof (hexString)-4*(m/2),"%02X%02X",(unsigned char )record[m],(unsigned char )record[m+1]);
        }
        char trainID[20],trainNum[20];
        sprintf(trainID, "%06d", g_period_msg_from_signal.train_id);
        sprintf(trainNum, "%04d", g_period_msg_from_signal.train_number);
        LogWrite(INFO,"%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%s,%d,%d,%d,%d,%d,%f,%s,%d,%d,%d,%d,%d,%d,%d,%s","CAN_MSG",g_current_time,g_period_msg_from_train.train_weight,g_period_msg_from_train.formation_num,
                 g_period_msg_from_train.train_length,g_period_msg_from_train.traction_voltage,g_period_msg_from_train.traction_voltage_side,g_period_msg_from_train.traction_current_2,g_period_msg_from_train.traction_current_3,
                 g_period_msg_from_train.traction_current_low_2,g_period_msg_from_train.traction_current_low_3,g_period_msg_from_train.traction_current_sign,g_period_msg_from_train.traction_fault_flag,g_period_msg_from_train.brake_fault_flag,
                 g_period_msg_from_train.other_fault_flag,g_period_msg_from_train.traction_energy_sum,g_period_msg_from_train.brake_energy_sum,g_period_msg_from_signal.traction_energy,g_period_msg_from_signal.regeneration_energy,
                 g_period_msg_from_signal.train_direction,trainID,trainNum,g_period_msg_from_signal.arrive_flag,g_period_msg_from_signal.leave_flag,g_period_msg_from_signal.byte_data,
                 g_period_msg_from_signal.train_plan_flag,g_period_msg_from_signal.train_ebi,g_period_msg_from_signal.train_speed,g_period_msg_from_signal.train_time,g_period_msg_from_signal.train_work_speed,g_period_msg_from_signal.train_work_level,
                 g_period_msg_from_signal.longitude_value,g_period_msg_from_signal.longitude_direction,g_period_msg_from_signal.latitude_value,g_period_msg_from_signal.latitude_direction,g_period_msg_from_train.train_fault_code,hexString);

        result = 1;//解析成功
        return result;
    }
    return result;
}

/*CAN接收初始化*/
UINT8 Rce04aCanInit(unsigned int Bitrate, char *CanName, struct can_filter *CanFilter, unsigned int FilterNum)
{
    struct ifreq ifr;
    struct sockaddr_can addr;
    static volatile int *s;
    int i = 0;

    FILE *fp;
    char CmdBuff[100];
    char CanBitrate[20];

    sprintf(CanBitrate, "%d", Bitrate);

    if(!strcmp(CanName,CAN0_NAME))
    {
        s = &can0;
    }
    else if(!strcmp(CanName,CAN1_NAME))
    {
        s = &can1;
    }

    sprintf(CmdBuff, "ifconfig %s down", CanName);
    printf("%s\n",CmdBuff);
    if ((fp = popen(CmdBuff, "r")) == NULL) {
        printf("Popen %s Down Error\n", CanName);
        return 0;
    }
    pclose(fp);

    sprintf(CmdBuff, "canconfig %s bitrate %s ctrlmode triple-sampling on", CanName, CanBitrate);
    printf("%s\n",CmdBuff);
    if ((fp = popen(CmdBuff, "r")) == NULL) {
        printf("Popen %s Config Error\n", CanName);
        return 0;
    }
    pclose(fp);

    sprintf(CmdBuff, "ifconfig %s up", CanName);
    printf("%s\n",CmdBuff);
    if ((fp = popen(CmdBuff, "r")) == NULL) {
        printf("Popen %s Up Error\n", CanName);
        return 0;
    }
    pclose(fp);

    if (((*s) = socket(29, SOCK_RAW, CAN_RAW)) < 0)
    {
        printf("Create Can Socket Failed\n");
        return 0;
    }


    strcpy(ifr.ifr_name, CanName);
    ioctl((*s), SIOCGIFINDEX, &ifr);
    addr.can_family = 29;
    addr.can_ifindex = ifr.ifr_ifindex;

    printf("addr.can_ifindex = %d\n",addr.can_ifindex);
    if(bind((*s), (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Bind Can Device failed\n");
        close((*s));
        return 0;
    }

    //fcntl((*s),F_SETFL,FNDELAY);//用于设置read函数的非阻塞

    for(i = 0; i<FilterNum; i++)
    {
        if(setsockopt((*s), SOL_CAN_RAW, CAN_RAW_FILTER, &CanFilter[i], sizeof(struct can_filter)) < 0)
        {
            printf("Set Receiving Filter Error Can_Id = %d \n", CanFilter[i].can_id);
            close((*s));
            return 0;
        }
    }
    printf("Can Device Init Ok\n");
    return 1;
}


int CanReadData(char *name, struct can_frame *Frame)
{
    int nbytes = -1;
    int i = 0;

    if((name == NULL) || (Frame == NULL))
    {
        return -1;
    }

    if(!strcmp(name,CAN0_NAME))
    {
        nbytes = read(can0, Frame, sizeof(struct can_frame));
    }
    else if(!strcmp(name,CAN1_NAME))
    {
        nbytes = read(can1, Frame, sizeof(struct can_frame));
    }
    else
    {
        return -1;
    }
    return nbytes;
}

int CanSendData(char *name,struct can_frame *Frame)
{
    int nbytes = -1;
    int i = 0;

    if((name == NULL) || (Frame == NULL))
    {
        return -1;
    }

    if(!strcmp(name,CAN0_NAME))
    {
        nbytes = write(can0, Frame, sizeof(struct can_frame));
    }
    else if(!strcmp(name,CAN1_NAME))
    {
        nbytes = write(can1, Frame, sizeof(struct can_frame));
    }
    else
    {
        return -1;
    }
    return 0;
}

UINT8 frame_index=0;
UINT8 recvbuf[1024];

void Rce04aCanReadMsgPoll(void)
{
    struct can_frame Rce04aCan1Frame;
    struct can_frame Rce04aCan2Frame;
    struct can_frame Frame;

    while(CanReadData(CAN0_NAME, &Rce04aCan1Frame) == sizeof(struct can_frame)) {

        printf("Can Mesg Read = ");
        for(int i = 0; i<8; i++){
            printf(" %d", Rce04aCan1Frame.data[i]);
        }
        printf("\r\n");

        UINT8 *index=Rce04aCan1Frame.data;
        UINT8 header = *(index++);//解析包头
        UINT16 message_id = ShortFromChar(index);//解析消息ID
        index+=2;
        UINT8 frame_num = *(index++);//解析标准帧数量
        //如果识别到包头，且消息ID和帧数量一致，则清空缓存buffer，准备存储新的数据
        if(header==0x55&&message_id==101&&frame_num==19)
        {
            printf("CAN:recognize package header\n");
            frame_index=0;
            memset(recvbuf,0,sizeof(recvbuf));

        }
        memcpy(&recvbuf[frame_index*8],Rce04aCan1Frame.data,8);
        frame_index+=1;
        //识别最后一帧
        if (frame_index==19)
        {
            //解包
            UnpackePeriodMsgFromCAN(recvbuf,frame_index*8);
            frame_index=0;
            memset(recvbuf,0,sizeof(recvbuf));
            LightChangeLed2();//led2
        }

//        Frame.can_id = 0x01;
//        Frame.can_dlc = 0x08;
//        Frame.data[0] = 0xff;
//        Frame.data[1] = 0xff;
//        Frame.data[2] = 0x3;
//        Frame.data[3] = 0x4;
//        Frame.data[4] = 0x5;
//        Frame.data[5] = 0x6;
//        Frame.data[6] = 0x7;
//        Frame.data[7] = 0x8;
//        CanSendData(CAN0_NAME, &Frame);
    }

    memset(&g_period_msg_from_signal,0,sizeof(g_period_msg_from_signal));//清空存储结构体
    printf("%s clear g_period_msg_from_signal!\n",g_current_time);

}


void* Rce04aCanMsgThread(void* arg)
{
    struct timespec req, rem;
    rem.tv_sec = 0;
    rem.tv_nsec = 0;
    req.tv_sec = 0;
    req.tv_nsec = 1000;

    while(1)
    {
        printf("wait for connect!\n");
        Rce04aCanReadMsgPoll();
        nanosleep(&req, &rem);
    }
}


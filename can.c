#include "can.h"

static volatile int can0;
static volatile int can1;

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
        memset(&g_period_msg_from_train,0,sizeof(g_period_msg_from_train));//清空存储结构体
        g_period_msg_from_train.train_weight=ShortFromChar(index);//解析列车实时载荷
        index+=2;
        g_period_msg_from_train.formation_num = *(index++);//解析列车编组数量
        g_period_msg_from_train.train_length=ShortFromChar(index);//解析列车长度
        index+=2;
        g_period_msg_from_train.traction_voltage= LongFromChar(index);//解析列车牵引电压
        index+=4;
        g_period_msg_from_train.traction_voltage_side= LongFromChar(index);//解析列车牵引电压
        index+=4;
        g_period_msg_from_train.traction_current= LongFromChar(index);//解析列车牵引电流
        index+=4;
        g_period_msg_from_train.traction_current_sign=*(index++);//解析列车牵引电流符号
        g_period_msg_from_train.traction_fault_flag=*(index++);//解析列车牵引故障标识
        g_period_msg_from_train.brake_fault_flag=*(index++);//解析列车制动故障标识
        g_period_msg_from_train.other_fault_flag=*(index++);//解析列车其他故障标识
        //自更新变量
        if (g_period_msg_from_train.traction_current_sign==1)
        {
            g_period_msg_from_train.traction_energy_sum+=CalEnergyByUI(g_period_msg_from_train.traction_voltage,g_period_msg_from_train.traction_current,0.2f);
        }
        else
        {
            g_period_msg_from_train.brake_energy_sum+=CalEnergyByUI(g_period_msg_from_train.traction_voltage,g_period_msg_from_train.traction_current,0.2f);
        }
//        LogWrite(INFO,"%s:%s,%s-%d,%s-%d,%s-%d,%s-%d","ENERGY:",g_current_time,"work",g_period_msg_from_signal.train_work_condition,"spd",g_period_msg_from_signal.train_speed,
//                 "dis",g_period_msg_from_signal.train_distance,"target_spd",g_speed_plan_info.target_speed);
        //解析实时数据
        memset(&g_period_msg_from_signal,0,sizeof(g_period_msg_from_signal));//清空存储结构体
        g_period_msg_from_signal.traction_energy= LongFromChar(index);//解析列车当前区间累积牵引能耗
        index+=4;
        g_period_msg_from_signal.regeneration_energy= LongFromChar(index);//解析列车当前区间累积再生能量
        index+=4;
        g_period_msg_from_signal.train_direction=*(index++);//解析列车运行方向
        g_period_msg_from_signal.train_id= LongFromChar(index);//解析列车车次号
        index+=4;
        g_period_msg_from_signal.train_number= LongFromChar(index);//解析列车车组号
        index+=4;
        g_period_msg_from_signal.arrive_flag=*(index++);//解析停准停稳标识
        g_period_msg_from_signal.leave_flag=*(index++);//解析允许发车标识
        g_period_msg_from_signal.door_flag=*(index++);//解析车门状态
        g_period_msg_from_signal.train_plan_flag=*(index++);//解析列车运行计划更新标识
        g_period_msg_from_signal.train_ebi=ShortFromChar(index);//解析ATP防护速度
        index+=2;
        g_period_msg_from_signal.train_speed=ShortFromChar(index);//解析列车实时速度
        index+=2;
//        memcpy(g_period_msg_from_signal.next_staion_name,index,20);//解析下一到达站名称
//        index+=20;
        g_period_msg_from_signal.next_station_id=ShortFromChar(index);//解析下一站编号
        index+=2;
        CHAR date[20];
        INT64 arrive_timestamp= LongLongFromChar(index);//解析下一站到达时间
        TimeStampToDate(arrive_timestamp,date,sizeof(date));
        memcpy(g_period_msg_from_signal.next_station_arrive_time,date,20);
        index+=8;
        INT64 leave_timestamp= LongLongFromChar(index);//解析下一站发车时间
        TimeStampToDate(leave_timestamp,date,sizeof(date));
        memcpy(g_period_msg_from_signal.next_station_leave_time,date,20);
        index+=8;
        INT64 current_timestamp= LongLongFromChar(index);//解析列车当前时间
        TimeStampToDate(current_timestamp,date,sizeof(date));
        memcpy(g_period_msg_from_signal.train_time,date,20);
        index+=8;
        g_period_msg_from_signal.train_work_condition=*(index++);//解析列车实时工况
        g_period_msg_from_signal.train_work_level=*(index++);//解析列车实时级位
        g_period_msg_from_signal.train_distance_last=g_period_msg_from_signal.train_distance;//保存上周期公里标
        g_period_msg_from_signal.train_distance= LongFromChar(index);//解析列车公里标
        index+=4;
        g_period_msg_from_signal.longitude_value= LongFromChar(index);//解析经度
        index+=4;
        g_period_msg_from_signal.longitude_direction=*(index++);//解析经度方向
        g_period_msg_from_signal.latitude_value= LongFromChar(index);//解析纬度
        index+=4;
        g_period_msg_from_signal.latitude_direction=*(index++);//解析纬度方向
        //校验位
        index+=2;
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
            result = 0;//校验失败
            return result;
        }

        //全局变量更新
        memcpy(g_current_time,g_period_msg_from_signal.train_time,20);//解析列车当前时间
        g_direction=g_period_msg_from_signal.train_direction;
        printf("CAN data unpack success\n");
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

        int i = 0;

        printf("Can Mesg Read = ");
        for(i = 0; i<8; i++){
            printf(" %d", Rce04aCan1Frame.data[i]);
        }
        printf("\r\n");

        UINT8 *index=Rce04aCan1Frame.data;
        UINT8 header = *(index++);//解析包头
        UINT16 message_id = ShortFromChar(index);//解析消息ID
        index+=2;
        UINT8 frame_num = *(index++);//解析标准帧数量
        //如果识别到包头，且消息ID和帧数量一致，则清空缓存buffer，准备存储新的数据
        if(header==0x55&&message_id==101&&frame_num==12)
        {
            printf("CAN:recognize package header\n");
            frame_index=0;
            memset(recvbuf,0,sizeof(recvbuf));

        }
        memcpy(&recvbuf[frame_index*8],Rce04aCan1Frame.data,8);
        frame_index+=1;
        //识别最后一帧
        if (frame_index==12)
        {
            //解包
            UnpackePeriodMsgFromCAN(recvbuf,frame_index*8);
            frame_index=0;
            memset(recvbuf,0,sizeof(recvbuf));
        }

        Frame.can_id = 0x01;
        Frame.can_dlc = 0x08;
        Frame.data[0] = 0xff;
        Frame.data[1] = 0xff;
        Frame.data[2] = 0x3;
        Frame.data[3] = 0x4;
        Frame.data[4] = 0x5;
        Frame.data[5] = 0x6;
        Frame.data[6] = 0x7;
        Frame.data[7] = 0x8;
        CanSendData(CAN0_NAME, &Frame);
    }
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
        printf("等待接收\n");
        Rce04aCanReadMsgPoll();
        nanosleep(&req, &rem);
    }
}


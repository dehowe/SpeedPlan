#include "socket.h"

INIT_MSG_TO_APP g_init_msg_to_app;                //发送给APP的静态数据，全局变量
PERIOD_MSG_TO_APP g_period_msg_to_app;            //发送给APP的周期数据，全局变量
PERIOD_MSG_FROM_TRAIN g_period_msg_from_train;    //车辆系统发送的周期数据，全局变量
PERIOD_MSG_FROM_SIGNAL g_period_msg_from_signal;  //信号系统发送的周期数据，全局变量
DEVICE_MAC_DATA g_device_mac_data;                 //白名单设备MAC地址，全局变量
char g_current_time[20]="2000-01-01 00:00:00"; //当前时钟，跟信号系统同步

UINT8 g_direction=0; //列车运行方向 1：上行 0：下行
UINT8 g_speed_plan_flag; //是否进行曲线优化的标志 1：开始 0：未开始
UINT8 g_serve_app=2; //与APP通信状态标识，1：建立服务 2：结束服务
UINT8 g_serve_app_send=0;/*正在周期发送消息给APP标志*/
int client_fds[MAX_NUM_CLIENT]; //建立通信的客户端列表
int server_fd; //本机
struct sockaddr_in client_add; //客户端地址信息
socklen_t client_address_len; //客户端地址长度
int client_num=0; //现有连接客户端数量


/*************************************************************************
* 功能描述: 主程序初始化
* 输入参数: 无
* 输出参数: 无
* 返回值:
*************************************************************************/
void ProgramInit()
{
    memset(g_speed_curve_offline,0,sizeof(g_speed_curve_offline));
    memset(&g_period_msg_from_signal,0,sizeof(g_period_msg_from_signal));//清空存储结构体
    memset(&g_period_msg_from_train,0,sizeof(g_period_msg_from_train));//清空存储结构体
    memset(&g_period_msg_to_app,0,sizeof(g_period_msg_to_app));//清空存储结构体
    memset(&g_speed_plan_info,0,sizeof(g_speed_plan_info));//清空存储结构体
    g_speed_plan_info.optimize_stage=2;//曲线优化状态置位
    memcpy(g_period_msg_from_signal.train_time,g_current_time,20);
    g_aw_id = 0;
    g_speed_plan_flag=0;
}



/*************************************************************************
* 功能描述: socket通信初始化
* 输入参数: 无
* 输出参数: 无
* 返回值:   1:初始化成功，正在监听 0：初始化失败
*************************************************************************/
int socket_init()
{
    struct sockaddr_in serv_addr;
    char buff[1024];
    for(int i=0;i<MAX_NUM_CLIENT;i++)
    {
        client_fds[i]=0;
    }
    //创建socket连接
    server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd==-1)
    {
        perror("create socket error!");
        return 0;
    }
    else
    {
        printf("Success to create socket %d\n",server_fd);
    }

    //设置server地址结构
    bzero(&serv_addr,sizeof(serv_addr));    //初始化结构占用内存
    serv_addr.sin_family=AF_INET;    //设置传输层类型IPv4
    serv_addr.sin_port=htons(PORT);    //设置端口号
    serv_addr.sin_addr.s_addr=htons(INADDR_ANY);    //设置服务器IP地址
    //serv_addr.sin_addr.s_addr=inet_addr("192.168.187.10");    //设置服务器IP地址
    bzero(&(serv_addr.sin_zero),8);

    //绑定端口
    if(bind(server_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))!=0)
    {
        printf("bind address fail %d\n",errno);
        close(server_fd);
        return 0;
    }
    else
    {
        printf("Success to bind address!\n");
    }

    //监听端口
    if(listen(server_fd,MAX_NUM_CLIENT!=0))
    {
        perror("listen socket error!\n");
        close(server_fd);
        return 0;
    }
    else
    {
        printf("Success to listen\n");
    }

    printf("listening...\n");
    return 1;
}

/*************************************************************************
* 功能描述: socket通信客户端监听和多线程管理
* 输入参数: 无
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void *socket_manager()
{
    int client_fd;
    //创建连接客户端的对应套接字
    client_address_len=sizeof(client_add);
    while(1)
    {
        //接收客户端连接，如无连接请求保持阻塞
        client_fd=accept(server_fd,(struct sockaddr*)&client_add,&client_address_len);
        if(client_fd<=0)
        {
            perror("accept error!");
            break;
        }
        else
        {
            //遍历所有已连接的客户端
            for(int i=0;i<MAX_NUM_CLIENT;i++)
            {
                if(client_fds[i]==0)
                {
                    client_fds[i]=client_fd;
                    pthread_t tid;
                    //创建多线程
                    if(pthread_create(&tid,NULL,server_handle,&client_fd))
                    {
                        perror("Fail to create thread");
                        break;
                    }
                    printf("client_fds[%d] join in!\n",i);
                    client_num++;
                    ProgramInit();//程序初始化，变量初始化
                    if(client_num==MAX_NUM_CLIENT-1)
                    {
                        printf("client full!\n");
                    }
                    break;
                }
            }
        }
        sleep(1);
    }
    close(client_fd);
    close(server_fd);
    pthread_exit(0);//此线程退出
}

/*************************************************************************
* 功能描述: socket通信客户端子线程执行，包括消息的接收和发送
* 输入参数: 客户端序号
* 输出参数: 无
* 返回值:   无
*************************************************************************/
void *server_handle(void *arg)
{
    int *fd = arg;
    int client_fd;
    client_fd=*fd;
    int recv_size,num;
    UINT8 recvbuf[1024];
    UINT8 sendbuf[1024];
    //找到自己的客户端序号
    for(int i=0;i<MAX_NUM_CLIENT;i++)
    {
        if(client_fds[i]==client_fd)
        {
            num=i;
            break;
        }
    }
    //不断接收客户端信息
    while((recv_size=recv(client_fd, recvbuf, sizeof(recvbuf), 0))>0)
    {
        //printf("client_fds[%d]:%s\n",num,recvbuf);
        //如果客户端发出quit命令，则断开连接，结束线程
        if(strcmp(recvbuf,"quit") == 0)
        {
            for(int i=0;i<MAX_NUM_CLIENT;i++)
            {
                if(client_fds[i]==client_fd)
                {
                    printf("client_fds[%d] has been left!\n",i);
                    client_fds[i]=0;
                    ProgramInit();
                    break;
                }
            }
            break;
        }

        //解析包头
        UINT8 *index=recvbuf;
        index+=1;
        UINT16 message_id = ShortFromChar(index);//解析消息类型
        UINT8 unpack_result;
        switch (message_id)
        {
            case 103:
                //人机交互可视化模块
//                unpack_result = UnpackMsgFromAPP(recvbuf,recv_size);
//                if(unpack_result==0)
//                {
//                    LogWrite(INFO,"%s","SOCKET:receive message 103 from APP,unpack error!");
//                    printf("SOCKET:receive message 103 from APP,unpack error!\n");
//                }
//                else
//                {
//                    LogWrite(INFO,"%s","SOCKET:receive message 103 from APP,unpack success!");
//                    printf("SOCKET:receive message 103 from APP,unpack success!\n");
//                    SendAPPMessageManage(client_fd);
//                };
                break;
            case 101:
                //车辆网络
                //unpack_result = UnpackePeriodMsgFromTrainNet(recvbuf,recv_size);
                unpack_result = UnpackePeriodMsgFromCAN(recvbuf,recv_size);
                if(unpack_result==0)
                {
                    LogWrite(INFO,"%s","SOCKET:receive message 101 from TRAIN NET,unpack error!");
                    printf("SOCKET:receive message 101 from TRAIN NET,unpack error!\n");
                }
                else
                {
                    //LogWrite(INFO,"%s","SOCKET:receive message 101 from TRAIN NET,unpack success!");
                    //printf("SOCKET:receive message 101 from TRAIN NET,unpack success!\n");
                    //发送推荐速度（实验室仿真测试）
                    UINT16 message_length=PackPeriodMsgToSignal(sendbuf);
                    send(client_fd, sendbuf, message_length, 0);
                }
                break;
//            case 102:
//                //信号系统
//                unpack_result = UnpackePeriodMsgFromSignal(recvbuf,recv_size);
//                if(unpack_result==0)
//                {
//                    LogWrite(INFO,"%s","SOCKET:receive message 102 from SIGNAL,unpack error!");
//                    printf("SOCKET:receive message 102 from SIGNAL,unpack error!\n");
//                }
//                else
//                {
//                    //LogWrite(INFO,"%s","SOCKET:receive message 102 from SIGNAL,unpack success!");
//                    //printf("SOCKET:receive message 102 from SIGNAL,unpack success!\n");
//                    //发送推荐速度（实验室仿真测试）
//                    UINT16 message_length=PackPeriodMsgToSignal(sendbuf);
//                    send(client_fd, sendbuf, message_length, 0);
//                };
//
//                break;
            default:
                break;
        }

        memset(recvbuf,0,sizeof(recvbuf));
    }

    close(client_fd);//关闭与此客户端的socket
    for(int i=0;i<MAX_NUM_CLIENT;i++)
    {
        if(client_fds[i]==client_fd)
        {
            printf("client_fds[%d] has been left!\n",i);
            client_fds[i]=0;
            break;
        }
    }
    ProgramInit();
    pthread_exit(0);//此线程退出
}

/*************************************************************************
  * 功能描述: 将2字节数据流变为UINT16
  * 输入参数: input 输入
  * 输出参数: 无
  * 返回值:   UINT16数据
  *************************************************************************/
UINT16 ShortFromChar(const UINT8 *input)
{
    UINT16 temp;
    temp = (*input);
    temp = (temp << 8) + (*(input + 1));
    return temp;
}

/*************************************************************************
 * 功能描述: 将4字节数据流变为UINT32
 * 输入参数: input 输入
 * 输出参数: 无
 * 返回值:   UINT32数据
 *************************************************************************/
UINT32 LongFromChar(const UINT8 *input)
{
    UINT32 temp;
    temp = (*input);
    temp = (temp << 8) + (*(input + 1));
    temp = (temp << 8) + (*(input + 2));
    temp = (temp << 8) + (*(input + 3));
    return temp;
}

/*************************************************************************
 * 功能描述: 将UINT16变为2字节数据流
 * 输入参数: input  UINT16数据
 * 输出参数: output 2字节数组
 * 返回值:   无
 *************************************************************************/
void ShortToChar(UINT16 input, UINT8 *output)
{
    *output = (UINT8)((input >> 8) & 0xFF);
    *(output + 1) = (UINT8)(input & 0xFF);
}

/*************************************************************************
 * 功能描述: 将UINT32变为4字节数据流
 * 输入参数: input  UINT32数据
 * 输出参数: output 4字节数组
 * 返回值:   无
 *************************************************************************/
void LongToChar(UINT32 input, UINT8 *output)
{
    *output = (UINT8)((input >> 24) & 0xFF);
    *(output + 1) = (UINT8)((input >> 16) & 0xFF);
    *(output + 2) = (UINT8)((input >> 8) & 0xFF);
    *(output + 3) = (UINT8)(input & 0xFF);
}

/*************************************************************************
 * 功能描述: 解包来自车辆网络的消息
 * 输入参数: UINT8  *receive_buffer  消息存储指针
 *          UINT16 receive_length  消息长度
 * 输出参数: 无
 * 返回值:   UINT8   result          1：解析成功 0：解析失败
 *************************************************************************/
UINT8 UnpackePeriodMsgFromTrainNet(UINT8 *receive_buffer,UINT16 receive_length)
{
    UINT8 result=0;
    UINT8 *index=receive_buffer;
    UINT16 message_id = ShortFromChar(index);//解析消息长度
    index+=2;
    UINT16 message_length = ShortFromChar(index);//解析消息长度
    index+=2;
    //消息头校验
    if (message_id==101&&message_length==receive_length)
    {
        memset(&g_period_msg_from_train,0,sizeof(g_period_msg_from_train));//清空存储结构体
        g_period_msg_from_train.train_weight=ShortFromChar(index);//解析列车实时载荷
        index+=2;
        g_period_msg_from_train.formation_num = *(index++);//解析列车编组数量
        g_period_msg_from_train.train_length=ShortFromChar(index);//解析列车长度
        index+=2;
        g_period_msg_from_train.traction_voltage_2= LongFromChar(index);//解析列车牵引电压
        index+=4;
        g_period_msg_from_train.traction_current_2= LongFromChar(index);//解析列车牵引电流
        index+=4;
        g_period_msg_from_train.traction_current_sign=*(index++);//解析列车牵引电流符号
        g_period_msg_from_train.traction_fault_flag=*(index++);//解析列车牵引故障标识
        g_period_msg_from_train.brake_fault_flag=*(index++);//解析列车制动故障标识
        g_period_msg_from_train.other_fault_flag=*(index++);//解析列车其他故障标识
        //自更新变量
        if (g_period_msg_from_train.traction_current_sign==1)
        {
            g_period_msg_from_train.traction_energy_sum+=CalEnergyByUI(g_period_msg_from_train.traction_voltage_2,g_period_msg_from_train.traction_current_2,0.2f);
        }
        else
        {
            g_period_msg_from_train.brake_energy_sum+=CalEnergyByUI(g_period_msg_from_train.traction_voltage_2,g_period_msg_from_train.traction_current_2,0.2f);
        }

        //解析实时数据
        //memset(&g_period_msg_from_signal,0,sizeof(g_period_msg_from_signal));//清空存储结构体
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
//        g_period_msg_from_signal.next_station_id=ShortFromChar(index);//解析下一站编号
        index+=2;
        memcpy(g_period_msg_from_signal.next_station_arrive_time,index,20);//解析下一站到达时间
        index+=20;
        memcpy(g_period_msg_from_signal.next_station_leave_time,index,20);//解析下一站发车时间
        index+=20;
        g_period_msg_from_signal.train_work_condition=*(index++);//解析列车实时工况
        g_period_msg_from_signal.train_work_level=*(index++);//解析列车实时级位
        g_period_msg_from_signal.train_distance_last=g_period_msg_from_signal.train_distance;//保存上周期公里标
        g_period_msg_from_signal.train_distance= LongFromChar(index);//解析列车公里标
        index+=4;
        memcpy(g_period_msg_from_signal.train_time,index,20);//解析列车当前时间
        index+=20;
        g_period_msg_from_signal.longitude_value_last=g_period_msg_from_signal.longitude_value;//保存上周期
        g_period_msg_from_signal.longitude_direction_last=g_period_msg_from_signal.longitude_direction;//保存上周期
        g_period_msg_from_signal.latitude_value_last=g_period_msg_from_signal.latitude_value;//保存上周期
        g_period_msg_from_signal.latitude_direction_last=g_period_msg_from_signal.latitude_direction;//保存上周期

        g_period_msg_from_signal.longitude_value= LongFromChar(index);//解析GPS经度
        index+=4;
        g_period_msg_from_signal.longitude_direction=*(index++);//解析GPS经度方向
        g_period_msg_from_signal.latitude_value= LongFromChar(index);//解析GPS纬度
        index+=4;
        g_period_msg_from_signal.latitude_direction=*(index++);//解析GPS纬度方向

        g_period_msg_from_signal.temporary_limit_num=ShortFromChar(index);//解析临时限速数量
        index+=2;
        for(int i=0;i<g_period_msg_from_signal.temporary_limit_num;i++)
        {
            g_period_msg_from_signal.temporary_limit_begin_distance[i]=LongFromChar(index);//解析临时限速起始公里标
            index+=4;
            g_period_msg_from_signal.temporary_limit_end_distance[i]=LongFromChar(index);//解析临时限速结束公里标
            index+=4;
            g_period_msg_from_signal.temporary_limit_value[i]=ShortFromChar(index);//解析临时限速值
            index+=2;
        }
        //全局变量更新
        if(g_period_msg_from_signal.train_time[0]!=0)
        {
            memcpy(g_current_time,g_period_msg_from_signal.train_time,20);//解析列车当前时间
        }
        g_direction=g_period_msg_from_signal.train_direction;
        GetCurrentDistance();//更新当前公里标
        GetCurrentPlan();//更新下一站
        //printf("dis:%d\n",g_period_msg_from_signal.train_distance);
        LogWrite(INFO,"%s-%d,%s-%d,%s-%d,%s-%d,%s-%d,%s-%d,%s-%d,%s-%d","ENERGY",DateToTimeStamp(g_current_time),"door",g_period_msg_from_signal.door_flag,"spd",g_period_msg_from_signal.train_speed,
                 "dis",g_period_msg_from_signal.train_distance,"lng",g_period_msg_from_signal.longitude_value,"lng_dir",g_period_msg_from_signal.longitude_direction,"lat",g_period_msg_from_signal.latitude_value,
                 "lat_dir",g_period_msg_from_signal.latitude_direction);
        result = 1;//解析成功
        return result;
    }
    return result;
}

/*************************************************************************
 * 功能描述: 解包来自信号系统消息
 * 输入参数: UINT8  *receive_buffer  消息存储指针
 *          UINT16 receive_length  消息长度
 * 输出参数: 无
 * 返回值:   UINT8   result          1：解析成功 0：解析失败
 *************************************************************************/
UINT8 UnpackePeriodMsgFromSignal(UINT8 *receive_buffer,UINT16 receive_length)
{
    UINT8 result=0;
    UINT8 *index=receive_buffer;
    UINT16 message_id = ShortFromChar(index);//解析消息长度
    index+=2;
    UINT16 message_length = ShortFromChar(index);//解析消息长度
    index+=2;
    //消息头校验
    if (message_id==102&&message_length==receive_length)
    {
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
        g_period_msg_from_signal.train_plan_flag=*(index++);//解析列车运行计划更新标识
        g_period_msg_from_signal.train_ebi=ShortFromChar(index);//解析ATP防护速度
        index+=2;
        g_period_msg_from_signal.train_speed=ShortFromChar(index);//解析列车实时速度
        index+=2;
//        memcpy(g_period_msg_from_signal.next_staion_name,index,20);//解析下一到达站名称
//        index+=20;
        g_period_msg_from_signal.next_station_id=ShortFromChar(index);//解析下一站编号
        index+=2;
        memcpy(g_period_msg_from_signal.next_station_arrive_time,index,20);//解析下一站到达时间
        index+=20;
        memcpy(g_period_msg_from_signal.next_station_leave_time,index,20);//解析下一站发车时间
        index+=20;
        g_period_msg_from_signal.train_work_condition=*(index++);//解析列车实时工况
        g_period_msg_from_signal.train_work_level=*(index++);//解析列车实时级位
        g_period_msg_from_signal.train_distance_last=g_period_msg_from_signal.train_distance;//保存上周期公里标
        g_period_msg_from_signal.train_distance= LongFromChar(index);//解析列车公里标
        index+=4;
        memcpy(g_period_msg_from_signal.train_time,index,20);//解析列车当前时间
        index+=20;
        g_period_msg_from_signal.temporary_limit_num=ShortFromChar(index);//解析临时限速数量
        index+=2;
        for(int i=0;i<g_period_msg_from_signal.temporary_limit_num;i++)
        {
            g_period_msg_from_signal.temporary_limit_begin_distance[i]=LongFromChar(index);//解析临时限速起始公里标
            index+=4;
            g_period_msg_from_signal.temporary_limit_end_distance[i]=LongFromChar(index);//解析临时限速结束公里标
            index+=4;
            g_period_msg_from_signal.temporary_limit_value[i]=ShortFromChar(index);//解析临时限速值
            index+=2;
        }
        //全局变量更新
        memcpy(g_current_time,g_period_msg_from_signal.train_time,20);//解析列车当前时间
        g_direction=g_period_msg_from_signal.train_direction;

        result = 1;//解析成功
        return result;
    }
    return result;
}

/*************************************************************************
 * 功能描述: 解包来自APP消息
 * 输入参数: UINT8  *receive_buffer  消息存储指针
 *          UINT16 receive_length   消息长度
 * 输出参数: 无
 * 返回值:   UINT8   result          1：解析成功 0：解析失败
 *************************************************************************/
UINT8 UnpackMsgFromAPP(UINT8 *receive_buffer,UINT16 receive_length)
{
    UINT8 result=0;
    UINT8 *index=receive_buffer;
    UINT16 message_id = ShortFromChar(index);//解析消息长度
    index+=2;
    UINT16 message_length = ShortFromChar(index);//解析消息长度
    index+=2;
    UINT32 device_id = LongFromChar(index);//解析消息长度
    index+=4;
    //消息头校验
    if (message_id==103&&message_length==receive_length&&device_id==1101)
    {
        g_serve_app = *(index++);//1：建立服务 2：结束服务
        result = 1;
        return result;
    }
    return result;
}

/*************************************************************************
 * 功能描述: 打包发送给APP的初始化消息
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
UINT16 PackInitMsgToAPP(UINT8 *send_buffer)
{
    UINT16 index=0;
    send_buffer[index++]=203;//打包消息类型
    UINT16 message_length=42+6*(g_init_msg_to_app.gradient_num+g_init_msg_to_app.curve_radius_num+g_init_msg_to_app.speed_limit_num)+g_init_msg_to_app.tunnel_num*9+g_init_msg_to_app.separate_num*8+g_init_msg_to_app.signal_num*4+24*g_init_msg_to_app.station_num;
    ShortToChar(message_length,send_buffer+index);//打包消息长度
    index+=2;
    ShortToChar(g_init_msg_to_app.gradient_num,send_buffer+index);//打包坡度数量
    index+=2;
    for(int i=0;i<g_init_msg_to_app.gradient_num;i++)
    {
        LongToChar(g_init_msg_to_app.gradient_distance[i],send_buffer+index);//打包坡度公里标
        index+=4;
        ShortToChar(g_init_msg_to_app.gradient_value[i],send_buffer+index);//打包坡度
        index+=2;
    }
    ShortToChar(g_init_msg_to_app.curve_radius_num,send_buffer+index);//打包曲线半径数量
    index+=2;
    for(int i=0;i<g_init_msg_to_app.curve_radius_num;i++)
    {
        LongToChar(g_init_msg_to_app.curve_radius_distance[i],send_buffer+index);//打包曲线半径公里标
        index+=4;
        ShortToChar(g_init_msg_to_app.curve_radius_value[i],send_buffer+index);//打包曲线半径
        index+=2;
    }
    ShortToChar(g_init_msg_to_app.tunnel_num,send_buffer+index);//打包桥梁隧道数量
    index+=2;
    for(int i=0;i<g_init_msg_to_app.tunnel_num;i++)
    {
        LongToChar(g_init_msg_to_app.tunnel_begin_distance[i],send_buffer+index);//打包起始公里标
        index+=4;
        LongToChar(g_init_msg_to_app.tunnel_end_distance[i],send_buffer+index);//打包结束公里标
        index+=4;
        send_buffer[index++]=g_init_msg_to_app.tunnel_flag[i];//1：桥梁 2：隧道
    }
    ShortToChar(g_init_msg_to_app.speed_limit_num,send_buffer+index);//打包线路限速数量
    index+=2;
    for(int i=0;i<g_init_msg_to_app.speed_limit_num;i++)
    {
        LongToChar(g_init_msg_to_app.speed_limit_distance[i],send_buffer+index);//打包限速公里标
        index+=4;
        ShortToChar(g_init_msg_to_app.speed_limit_value[i],send_buffer+index);//打包限速值
        index+=2;
    }
    ShortToChar(g_init_msg_to_app.separate_num,send_buffer+index);//打包分相区数量
    index+=2;
    for(int i=0;i<g_init_msg_to_app.separate_num;i++)
    {
        LongToChar(g_init_msg_to_app.separate_begin_distance[i],send_buffer+index);//打包起始公里标
        index+=4;
        LongToChar(g_init_msg_to_app.separate_end_distance[i],send_buffer+index);//打包结束公里标
        index+=4;
    }
    ShortToChar(g_init_msg_to_app.signal_num,send_buffer+index);//打包信号机数量
    index+=2;
    for(int i=0;i<g_init_msg_to_app.signal_num;i++)
    {
        LongToChar(g_init_msg_to_app.signal_distance[i],send_buffer+index);//打包信号机公里标
        index+=4;
    }
    LongToChar(g_init_msg_to_app.line_length,send_buffer+index);//打包线路长度
    index+=4;
    memcpy(send_buffer+index,g_init_msg_to_app.line_name,20);//打包线路名称
    index+=20;
    ShortToChar(g_init_msg_to_app.station_num,send_buffer+index);//打包车站数量
    index+=2;
    for(int i=0;i<g_init_msg_to_app.station_num;i++)
    {
        memcpy(send_buffer+index,g_init_msg_to_app.station_name[i],20);//打包车站名称
        index+=20;
        LongToChar(g_init_msg_to_app.station_distance[i],send_buffer+index);//打包车站公里标
        index+=4;
    }
    send_buffer[index++]=g_init_msg_to_app.serve_reply;//打包服务回复
    return index;
}

/*************************************************************************
 * 功能描述: 打包发送给APP的结束服务消息
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
UINT16 PackEndMsgToAPP(UINT8 *send_buffer)
{
    UINT16 index=0;
    send_buffer[index++]=203;//打包消息类型
    UINT16 message_length=4;
    ShortToChar(message_length,send_buffer+index);//打包消息长度
    index+=2;
    send_buffer[index++]=1;
    return index;
}



/*************************************************************************
 * 功能描述: 更新发送给APP的周期数据
 * 输入参数: 无
 * 输出参数: 无
 * 返回值: 无
 *************************************************************************/
void RefreshPeriodMsgToAPP()
{
    //车辆网络数据
    g_period_msg_to_app.traction_fault_flag=g_period_msg_from_train.traction_fault_flag;
    g_period_msg_to_app.brake_fault_flag=g_period_msg_from_train.brake_fault_flag;
    g_period_msg_to_app.other_fault_flag=g_period_msg_from_train.other_fault_flag;
    //信号系统数据
    g_period_msg_to_app.traction_energy=g_period_msg_from_signal.traction_energy;
    g_period_msg_to_app.regeneration_energy=g_period_msg_from_signal.regeneration_energy;
    g_period_msg_to_app.train_direction=g_direction;
    g_period_msg_to_app.train_id=g_period_msg_from_signal.train_id;
    g_period_msg_to_app.train_number=g_period_msg_from_signal.train_number;
    g_period_msg_to_app.arrive_flag=g_period_msg_from_signal.arrive_flag;
    g_period_msg_to_app.leave_flag=g_period_msg_from_signal.leave_flag;
    g_period_msg_to_app.train_ebi=g_period_msg_from_signal.train_ebi;
    g_period_msg_to_app.train_speed=(UINT16)g_period_msg_from_signal.train_speed;

    memcpy(g_period_msg_to_app.current_station_leave_time,g_period_msg_from_signal.current_station_leave_time,20);//打包当前站出发时间
    memcpy(g_period_msg_to_app.next_station_name,g_period_msg_from_signal.next_station_name,20);//打包下一站名词
    memcpy(g_period_msg_to_app.next_station_arrive_time,g_period_msg_from_signal.next_station_arrive_time,20);//打包下一站到达时间
    memcpy(g_period_msg_to_app.next_station_leave_time,g_period_msg_from_signal.next_station_leave_time,20);//打包下一站出发时间
    g_period_msg_to_app.train_work_condition=g_period_msg_from_signal.train_work_condition;
    g_period_msg_to_app.train_work_level=g_period_msg_from_signal.train_work_level;
    g_period_msg_to_app.train_distance=g_period_msg_from_signal.train_distance;
    memcpy(g_period_msg_to_app.train_time,g_period_msg_from_signal.train_time,20);//打包列车时间
    g_period_msg_to_app.temporary_limit_num=g_period_msg_from_signal.temporary_limit_num;
    for(int i=0;i<g_period_msg_from_signal.temporary_limit_num;i++)
    {
        g_period_msg_to_app.temporary_limit_begin_distance[i]=g_period_msg_from_signal.temporary_limit_begin_distance[i];
        g_period_msg_to_app.temporary_limit_end_distance[i]=g_period_msg_from_signal.temporary_limit_end_distance[i];
        g_period_msg_to_app.temporary_limit_value[i]=g_period_msg_from_signal.temporary_limit_value[i];
    }
    //曲线优化结果
    g_period_msg_to_app.optimize_flag=g_speed_plan_info.optimize_stage;
    //计算下一阶段推荐速度、推荐工况、生效倒计时和距离
    UINT16 next_speed_temp;//下一推荐速度 cm/s
    GetRecSpdAndWorkByDis(g_period_msg_from_signal.train_distance,&next_speed_temp,&g_period_msg_to_app.next_work_condition_recommend,
                          &g_period_msg_to_app.next_recommend_countdown,&g_period_msg_to_app.next_recommend_distance);
    g_period_msg_to_app.next_speed_recommend=(UINT16)(3.6*next_speed_temp/100);
    //工况级位
    if (g_period_msg_to_app.next_work_condition_recommend==1||g_period_msg_to_app.next_work_condition_recommend==3)
    {
        g_period_msg_to_app.next_work_level_recommend=SPEED_PLAN_TB_RATIO*100;
    }
    else
    {
        g_period_msg_to_app.next_work_level_recommend=0;
    }
    LogWrite(INFO,"%s-%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d","WebSocket",g_current_time,g_period_msg_to_app.current_station_leave_time,g_period_msg_to_app.next_station_arrive_time,g_period_msg_to_app.next_station_leave_time,
             g_period_msg_to_app.train_work_condition,g_period_msg_to_app.train_work_level,g_period_msg_to_app.train_distance,g_period_msg_to_app.optimize_flag,g_period_msg_to_app.next_speed_recommend,
             g_period_msg_to_app.next_work_level_recommend,g_period_msg_to_app.next_recommend_countdown,g_period_msg_to_app.next_recommend_distance);

    //LogWrite(INFO,"%s,%s-%d,%s-%d,%s-%d,%s-%d,%s-%d","APP","target_spd",g_speed_plan_info.target_speed,"next_target",g_period_msg_to_app.next_speed_recommend,
    //        "next_level",g_period_msg_to_app.next_work_condition_recommend,"next_cutdown",g_period_msg_to_app.next_recommend_countdown,"next_dis",g_period_msg_to_app.next_recommend_distance);
    //printf("target:%d,rec:%d,level:%d,cut:%d,dis:%d\n",g_speed_plan_info.target_speed,g_period_msg_to_app.next_speed_recommend,g_period_msg_to_app.next_work_condition_recommend,g_period_msg_to_app.next_recommend_countdown,g_period_msg_to_app.next_recommend_distance);

}

/*************************************************************************
 * 功能描述: 打包发送给APP的周期消息
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
UINT16 PackPeriodMsgToAPP(UINT8 *send_buffer)
{
    /*消息填充*/
    RefreshPeriodMsgToAPP();
    /*消息打包*/
    UINT16 index=0;
    ShortToChar(204,send_buffer+index);//打包消息类型
    index+=2;
    UINT16 message_length=97+g_period_msg_to_app.temporary_limit_num*10;
    ShortToChar(message_length,send_buffer+index);//打包消息长度
    index+=2;
    send_buffer[index++]=g_period_msg_to_app.traction_fault_flag;//打包牵引故障标识
    send_buffer[index++]=g_period_msg_to_app.brake_fault_flag;//打包制动故障标识
    send_buffer[index++]=g_period_msg_to_app.other_fault_flag;//打包其他故障标识
    LongToChar(g_period_msg_to_app.traction_energy,send_buffer+index);//打包牵引累积能耗
    index+=4;
    LongToChar(g_period_msg_to_app.regeneration_energy,send_buffer+index);//打包再生累积再生能量
    index+=4;
    send_buffer[index++]=g_period_msg_to_app.train_direction;//打包运行方向
    LongToChar(g_period_msg_to_app.train_id,send_buffer+index);//打包车次号
    index+=4;
    LongToChar(g_period_msg_to_app.train_number,send_buffer+index);//打包车组号
    index+=4;
    send_buffer[index++]=g_period_msg_to_app.arrive_flag;//打包停准停稳标识
    send_buffer[index++]=g_period_msg_to_app.leave_flag;//打包允许出发标识
    send_buffer[index++]=g_period_msg_to_app.optimize_flag;//打包曲线优化状态标识
    ShortToChar(g_period_msg_to_app.train_ebi,send_buffer+index);//打包ATP防护速度
    index+=2;
    ShortToChar(g_period_msg_to_app.train_speed,send_buffer+index);//打包列车速度
    index+=2;
    memcpy(send_buffer+index,g_period_msg_to_app.next_station_name,20);//打包下一站名称
    index+=20;
    memcpy(send_buffer+index,g_period_msg_to_app.next_station_arrive_time,20);//打包下一站到达时间
    index+=10;
    memcpy(send_buffer+index,g_period_msg_to_app.next_station_leave_time,20);//打包下一站出发时间
    index+=10;
    send_buffer[index++]=g_period_msg_to_app.train_work_condition;//打包列车工况
    send_buffer[index++]=g_period_msg_to_app.train_work_level;//打包列车级位
    LongToChar(g_period_msg_to_app.train_distance,send_buffer+index);//打包列车公里标
    index+=4;
    memcpy(send_buffer+index,g_period_msg_to_app.train_time,20);//打包列车时间
    index+=10;
    ShortToChar(g_period_msg_to_app.next_speed_recommend,send_buffer+index);//打包下一建议速度
    index+=2;
    send_buffer[index++]=g_period_msg_to_app.next_work_condition_recommend;//打包下一建议工况
    send_buffer[index++]=g_period_msg_to_app.next_work_level_recommend;//打包下一建议级位
    ShortToChar(g_period_msg_to_app.next_recommend_countdown,send_buffer+index);//打包下一建议生效倒计时
    index+=2;
    LongToChar(g_period_msg_to_app.next_recommend_distance,send_buffer+index);//打包下一建议生效距离
    index+=4;
    ShortToChar(g_period_msg_to_app.temporary_limit_num,send_buffer+index);//打包临时限速数量
    index+=2;
    for(int i=0;i<g_period_msg_to_app.temporary_limit_num;i++)
    {
        LongToChar(g_period_msg_to_app.temporary_limit_begin_distance[i],send_buffer+index);//打包临时限速起始公里标
        index+=4;
        LongToChar(g_period_msg_to_app.temporary_limit_end_distance[i],send_buffer+index);//打包临时限速结束公里标
        index+=4;
        ShortToChar(g_period_msg_to_app.temporary_limit_value[i],send_buffer+index);//打包临时限速
        index+=2;
    }
    return index;
}

/*************************************************************************
 * 功能描述: 200msAPP消息发送线程
 * 输入参数:
 * 输出参数: 无
 * 返回值:
 *************************************************************************/
void *SendPeriodMessageToApp(void *arg)
{
    UINT8 sendbuf[1024];
    int *fd = arg;
    int client_fd;
    client_fd=*fd;
    UINT16 message_length;
    while (g_serve_app_send==1)
    {
        message_length=PackPeriodMsgToAPP(sendbuf);
        send(client_fd, sendbuf, message_length,0);
        printf("send message 204 to APP success!\n");
        usleep(200000);
    }
    pthread_exit(0);//此线程退出
}

/*************************************************************************
 * 功能描述: 发送给APP消息管理
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
void SendAPPMessageManage(int client_fd)
{
    UINT8 sendbuf[1024];
    UINT16 message_length;
    //建立通信服务
    if(g_serve_app==1&&g_serve_app_send==0)
    {
        g_serve_app_send=1;
        //一次发送静态数据
        message_length=PackInitMsgToAPP(sendbuf);
        send(client_fd, sendbuf, message_length, 0);
        printf("send message 203(init) to APP success!\n");
        //周期发送动态数据
        /*创建通信管理线程*/
        pthread_t tid_app;
        if(pthread_create(&tid_app,NULL,SendPeriodMessageToApp,&client_fd))
        {
            perror("Fail to create server thread");
        }
    }
    else if(g_serve_app==2)
    {
        g_serve_app_send=0;
        //一次发送静态数据
        PackEndMsgToAPP(sendbuf);
        send(client_fd, sendbuf, sizeof(sendbuf), 0);
        printf("send message 203(exit) to APP success!\n");
    }

}


/*************************************************************************
 * 功能描述: 打包发送给Signal的周期消息
 * 输入参数: UINT8    *send_buffer      消息存储指针
 * 输出参数: 无
 * 返回值:   UINT16   send_buffer_length    消息长度
 *************************************************************************/
UINT16 PackPeriodMsgToSignal(UINT8 *send_buffer)
{
    /*消息填充*/
    UINT16 target_speed;
    UINT8 level_flag;
    UINT8 level_output;
    UINT16 limit_speed;
    //根据当前里程计算推荐速度、工况输出
    GetTargetSpeedByDistance(&target_speed,&level_flag,&level_output);
    UINT32 train_head_loc=g_period_msg_from_signal.train_distance;
    UINT32 train_tail_loc;
    if(g_period_msg_from_signal.train_direction==DIRECTION_DOWN)
    {
        train_tail_loc=(g_period_msg_from_signal.train_distance>g_train_param.train_length/100)?g_period_msg_from_signal.train_distance-g_train_param.train_length/100:0;
    }
    else
    {
        train_tail_loc=g_period_msg_from_signal.train_distance+g_train_param.train_length/100;
    }

    limit_speed = GetSpeedLimit(train_head_loc,train_tail_loc);
    LogWrite(INFO,"%s,%s-%d,%s-%d,%s-%f,%s-%d","DATA","dis",train_head_loc,"target_spd",target_speed,"spd",g_period_msg_from_signal.train_speed,"ebi",limit_speed);
    //printf("dis:%d,target_spd-%d,ebi-%d\n",train_head_loc,target_speed,limit_speed);
    g_speed_plan_info.target_speed=target_speed;
    /*消息打包*/
    UINT16 index=0;
    ShortToChar(202,send_buffer+index);//打包消息类型
    index+=2;
    UINT16 message_length=6;
    ShortToChar(message_length,send_buffer+index);//打包消息长度
    index+=2;
    ShortToChar(g_speed_plan_info.target_speed,send_buffer+index);//打包推荐速度
    index+=2;
    send_buffer[index++]=level_flag;//打包工况标志
    send_buffer[index++]=level_output;//打包工况级位
    //printf("target:%d,level:%d,output:%d\n",target_speed,level_flag,level_output);
    return index;
}


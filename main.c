
#include "stdio.h"
#include "SpeedPlan.h"
#include "socket.h"
#include "init.h"
#include "WebServer.h"
#include "can.h"

struct can_filter Can0Filter[FILTER0_NUM] = {{CPU_CAN_ID,CAN_SFF_MASK}};

int main()
{
    UINT8 result;
    printf("Hello ARM!\n");
//    LogWrite(INFO,"%s","Hello World!");
//    LogWrite(DEBUG,"%s","H.e.l.l.o W.o.r.l.d!");
//    LogWrite(WARN,"%s","H e l l o W o r l d!");
//    LogWrite(ERROR,"%s","Hallo World!");


    result=StaticDataRead();
    if (result==0)
    {
        printf("Fail to read CSV files\n");
        return 0;
    }
    result=StaticDataInit();
    if (result==0)
    {
        printf("Fail to initialize static data\n");
        return 0;
    }
    DeviceMacDataInit();
    if (result==0)
    {
        printf("Fail to initialize device mac data\n");
        return 0;
    }
    g_speed_plan_info.optimize_stage=2;
//    g_period_msg_from_signal.train_direction=1;
//    g_period_msg_from_signal.train_plan_flag=1;
//    g_period_msg_from_signal.train_distance=4280;
//    g_period_msg_from_signal.next_station_id=1;
    g_aw_id = 0;
    printf("CAN init...\n");
    /*CAN通信初始化*/
    result = Rce04aCanInit(CAN0_BITRATE, CAN0_NAME, Can0Filter, FILTER0_NUM);
    /*CAN通信初始化成功*/
    if (result==1)
    {
        /*创建CAN通信管理线程*/
        pthread_t tid_can;
        if(pthread_create(&tid_can,NULL,Rce04aCanMsgThread,NULL))
        {
            perror("Fail to create CAN thread");
        }
    }
    printf("SOCKET init...\n");
    /*socket通信初始化*/
    socket_init();
    /*创建Socket通信管理线程*/
    pthread_t tid_server;
    if(pthread_create(&tid_server,NULL,socket_manager,NULL))
    {
        perror("Fail to create socket server thread");
    }
    printf("WEB SOCKET init...\n");
    pthread_t tid_webserver;
    /*创建WebSocket通信管理线程*/
    if(pthread_create(&tid_webserver,NULL,WebSocketServer,NULL))
    {
        perror("Fail to create web socket server thread");
    }


    while (1)
    {
        SpeedPlanMain();

        sleep(1);
    }

    printf("Bye ARM!\n");
    return 1;
}
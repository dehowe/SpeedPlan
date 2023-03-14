
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
    char net_mac_loc[32]="00:04:9f:04:d2:35";  //唯一绑定MAC地址 本地 00:0c:29:7b:87:2c  融创 00:04:9f:04:d2:35 实验室 00:04:9f:04:d2:35
    Rce04aLedEncoderInit();//led初始化
    LightUpLed1();//点亮led1
    result=CheckLocMac(net_mac_loc);//检查MAC地址有效性
    if (result==0)
    {
        printf("Fail to init MAC address");
        LogWrite(INFO,"%s","Fail to init MAC address");
        return 0;
    }

    result=StaticDataRead();
    if (result==0)
    {
        printf("Fail to read CSV files\n");
        LogWrite(INFO,"%s","Fail to read CSV files");
        return 0;
    }
    result=StaticDataInit();
    if (result==0)
    {
        printf("Fail to initialize static data\n");
        LogWrite(INFO,"%s","Fail to initialize static data");
        return 0;
    }
    DeviceMacDataInit();
    if (result==0)
    {
        printf("Fail to initialize device mac data\n");
        LogWrite(INFO,"%s","Fail to initialize device mac data");
        return 0;
    }
    ProgramInit();//程序初始化，变量初始化

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
            LogWrite(INFO,"%s","Fail to create CAN thread");
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
        LogWrite(INFO,"%s","Fail to create socket server thread");
    }
    printf("WEB SOCKET init...\n");
    pthread_t tid_webserver;
    /*创建WebSocket通信管理线程*/
    if(pthread_create(&tid_webserver,NULL,WebSocketServer,NULL))
    {
        perror("Fail to create web socket server thread");
        LogWrite(INFO,"%s","Fail to create web socket server thread");
    }


    while (1)
    {
        SpeedPlanMain();
        LightChangeLed1();//led1闪烁
        sleep(1);
    }

    printf("Bye ARM!\n");
    return 1;
}
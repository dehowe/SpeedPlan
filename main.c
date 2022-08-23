#include "stdio.h"
#include "SpeedPlan.h"
#include "log.h"
#include "socket.h"
#include "init.h"

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

    g_speed_plan_info.optimize_stage=2;
    g_period_msg_from_signal.train_plan_flag=1;
    g_aw_id = 0;
    /*socket通信初始化*/
    socket_init();
    pthread_t tid_server;
    /*创建通信管理线程*/
    if(pthread_create(&tid_server,NULL,socket_manager,NULL))
    {
        perror("Fail to create server thread");
    }
    while (1)
    {
        SpeedPlanMain();

        sleep(1);
    }

    printf("Bye ARM!\n");
    return 1;
}
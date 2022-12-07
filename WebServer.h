#pragma once

#include "WebCom.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h> //非阻塞宏
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "cJSON.h"
#include "socket.h"
#include "SpeedPlan.h"
//收包缓冲区大小,应确保能够一次接收完一整包数据,按需调整
//每接入一个客户端将开辟一块独立缓冲区空间
#define WS_SERVER_PKG (1024 * 100 + 16)

//最大副线程数量(不包括负责accept的主线程)
//计算机线程数量有限,建议限制在300以内(或上网搜索"linux最大线程数量"进一步了解)
#define WS_SERVER_THREAD 10

//每个副线程维护客户端最大数量
#define WS_SERVER_CLIENT_OF_THREAD 500

/*
 *  接入客户端最大数量
 *
 *  限制接入量的"性能因素":
 *      1.普通计算机接入破千之后CPU会逐渐拉满,很难再接入/发起更多客户端
 *      2.服务端表现为接入量上涨变缓
 *      3.客户端表现为connect阶段超时、登录阶段超时等
 *
 *  限制接入量的"参数因素":
 *      1.线程数量上限,上面宏定义 WS_SERVER_THREAD 中有提到
 *      2.文件描述符上限,使用指令“ulimit -a”在"open files"项可见,一般为4096,可尝试用指令"ulimit -n 8192"提高
 */
#define WS_SERVER_CLIENT (WS_SERVER_THREAD * WS_SERVER_CLIENT_OF_THREAD) // 10*500=5000

//bind超时,通常为服务器端口被占用,或者有客户端还连着上次的服务器
#define WS_SERVER_BIND_TIMEOUT_MS 1000

//连接后又不进行websocket握手,5秒超时踢出
#define WS_SERVER_LOGIN_TIMEOUT_MS 5000

//接入客户端数量超出这个数时不在 onMessage 打印,避免卡顿
#define WS_SERVER_CLIENT_OF_PRINTF 500

//断连原因
typedef enum {
    WET_NONE = 0,
    WET_EPOLL, //epoll检测
    WET_SEND,  //发送失败
    WET_LOGIN, //websocket握手检查失败(http请求格式错误或者path值不一致)
    WET_LOGIN_TIMEOUT, //连接后迟迟不发起websocket握手
    WET_DISCONNECT,    //收到断开协议包
} Ws_ExitType;

//先声明结构体,后面可以互相嵌套使用
typedef struct WsClient Ws_Client;
typedef struct WsThread Ws_Thread;
typedef struct WsServer Ws_Server;

//客户端事件回调函数原型
typedef void (*WsOnLogin)(Ws_Client *wsc);
typedef void (*WsOnMessage)(Ws_Client *wsc, char *msg, int msgLen, Ws_DataType dataType);
typedef void (*WsOnExit)(Ws_Client *wsc, Ws_ExitType exitType);

//客户端使用的参数结构体
struct WsClient
{
    int fd;        //accept之后得到的客户端连接描述符
    uint8_t ip[4]; //接入客户端的ip(accpet阶段获得)
    int port;      //接入客户端的端口
    Ws_ExitType exitType;  //断连标志
    bool isLogin;          //是否完成websocket握手验证
    bool isExiting;        //正在退出(防止反复del)
    uint32_t recvBytes;    //总接收字节计数
    uint32_t index;        //接入客户端的历史序号(从1数起)
    uint32_t loginTimeout; //等待websocket握手超时计数
    void *priv;     //用户私有指针
    Ws_Server *wss; //所在服务器指针
    Ws_Thread *wst; //所在副线程指针
    int serveFlag;  //服务状态 0：无服务  1：初始化完成 2：周期服务中
};

//副线程结构体(只要还有一个客户端在维护就不会退出线程)
struct WsThread
{
    int fd_epoll;    //epoll描述符
    int clientCount; //该线程正在维护的客户端数量
    bool isRun;      //线程运行状况
    Ws_Server *wss;
};

//服务器主线程使用的参数结构体
struct WsServer
{
    int fd;          //服务器描述符
    int fd_epoll;    //epoll描述符
    int port;        //服务器端口
    char path[256];  //服务器路径
    void *priv;      //用户私有指针
    int clientCount; //当前接入客户端总数
    bool isExit;     //线程结束标志
    pthread_mutex_t lock;
    WsOnLogin onLogin;
    WsOnMessage onMessage;
    WsOnExit onExit;
    Ws_Thread thread[WS_SERVER_THREAD]; //副线程数组
    Ws_Client client[WS_SERVER_CLIENT]; //全体客户端列表
};

//服务器创建和回收
Ws_Server* ws_server_create(
        int port,         //服务器端口
        const char *path, //服务器路径
        void *priv,       //用户私有指针,回调函数里使用 wsc->priv 取回
        WsOnLogin onLogin,     //客户端接入时(已连上),你要做什么?
        WsOnMessage onMessage, //收到客户端数据时,你要做什么?
        WsOnExit onExit);      //客户端断开时(已断开),你要做什么?

void ws_server_release(Ws_Server **wss);

void *WebSocketServer();

UINT16 PackInitJsonDataToApp(char* json_data);

UINT16 PackPeriodJsonDataToApp(char* json_data);

/*************************************************************************
* 功能描述: 打包发送给APP的触发JSON数据（驾驶建议曲线，停站阶段曲线优化结束后发送）
        * 输入参数: 无
        * 输出参数: 无
        * 返回值: 无
*************************************************************************/
UINT16 PackTriggerJsonDataToApp(char* json_data);

/*************************************************************************
 * 功能描述: 解析来自APP的数据，并进行相应处理
 * 输入参数:   char         *receive_buff       接收信息
 *           Ws_Client    *wsc                web socket client
 * 输出参数: 无
 * 返回值:
 *************************************************************************/
void UnpackJsonDataFromApp(char *receive_buff,Ws_Client *wsc);

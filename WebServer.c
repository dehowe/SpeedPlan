#include "WebServer.h"

#define WSS_INFO(...) fprintf(stdout, "[WSS_INFO] %s(%d): ", __FUNCTION__, __LINE__),fprintf(stdout, __VA_ARGS__)
#define WSS_ERR(...) fprintf(stderr, "[WSS_ERR] %s(%d): ", __FUNCTION__, __LINE__),fprintf(stderr, __VA_ARGS__)
//服务器副线程,负责检测 数据接收 和 客户端断开
static void* server_thread2(void *argv);

//抛线程工具
static void new_thread(void *obj, void* (*callback)(void*))
{
    pthread_t th;
    pthread_attr_t attr;
    int ret;
    //禁用线程同步,线程运行结束后自动释放
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    //抛出线程
    ret = pthread_create(&th, &attr, callback, (void *)obj);
    if (ret != 0)
        WSS_ERR("WEB SOCKET:pthread_create failed !! %s\n", strerror(ret));
    //attr destroy
    pthread_attr_destroy(&attr);
}

//注意在"epoll_wait时"或"目标fd已close()"的情况下会ctrl会失败
static void _epoll_ctrl(int fd_epoll, int fd, uint32_t event, int ctrl, void *ptr)
{
    struct epoll_event ev;
    ev.events = event;
    if (ptr)
        ev.data.ptr = ptr;
    else
        ev.data.fd = fd;
    if (epoll_ctl(fd_epoll, ctrl, fd, &ev) != 0)
        WSS_ERR("WEB SOCKET:epoll ctrl %d error !!\n", ctrl);
}

/*
 *  接收数据,自动连接客户端
 *  返回: 
 *      >0 有数据
 *      =0 无数据
 *      -1 连接异常
 */
static int client_recv(Ws_Client *wsc)
{
    int ret = 0;
    Ws_DataType retPkgType = WDT_NULL;
    char *buff = (char*)calloc(WS_SERVER_PKG, 1);

    if (!buff)
    {
        WSS_ERR("WEB SOCKET:calloc %dbytes failed\n", WS_SERVER_PKG);
        return -1;
    }

    do
    {
        ret = ws_recv(wsc->fd, buff, WS_SERVER_PKG, &retPkgType);
        //这可能是一包客户端请求
        if (!wsc->isLogin &&
            ret < 0 &&
            strncmp(buff, "GET", 3) == 0 &&
            strstr(buff, "Sec-WebSocket-Key"))
        {
            //构建回复
            if (ws_replyClient(wsc->fd, buff, -ret, wsc->wss->path) > 0)
            {
                //这个延时很有必要,否则下面onLogin里面发东西客户端可能收不到
                ws_delayms(5);
                wsc->isLogin = true;
                //回调
                if (wsc->wss->onLogin)
                    wsc->wss->onLogin(wsc);
                ret = 0;
                break;
            }
                //websocket握手失败,标记断开类型
            else
            {
                wsc->exitType = WET_LOGIN;
                ret = -1;
                break;
            }
        }
        //接收数据量统计
        if (ret != 0)
            wsc->recvBytes += ret > 0 ? ret : (-ret);
        //消息回调
        if (wsc->wss->onMessage)
            wsc->wss->onMessage(wsc, buff, ret, retPkgType);
        //断连协议,标记断开类型
        if (retPkgType == WDT_DISCONN)
        {
            wsc->exitType = WET_DISCONNECT;
            ret = -1;
            break;
        }
        //有效的一次数据接收返回1
        if (ret != 0)
            ret = 1;
    } while (0);
    free(buff);
    //正常返回
    return ret;
}

//onMessage异步回调
// static void client_onMessage(void *argv)
// {
//     Ws_Client *wsc = (Ws_Client *)argv;
//     int ret = 1;
//     //收完为止
//     while (ret > 0)
//         ret = client_recv(wsc);
// }

//onExit异步回调
static void* client_onExit(void *argv)
{
    Ws_Client *wsc = (Ws_Client *)argv;
    if (wsc->wss->onExit)
        wsc->wss->onExit(wsc, wsc->exitType);
    //重置结构体,给下次使用
    memset(wsc, 0, sizeof(Ws_Client));
    return NULL;
}

//取得空闲的坑,返回序号
static int client_get(Ws_Server *wss, int fd, uint32_t ip, int port)
{
    int i;
    for (i = 0; i < WS_SERVER_CLIENT; i++)
    {
        if (!wss->client[i].fd &&
            !wss->client[i].isExiting &&
            !wss->client[i].wst)
        {
            memset(&wss->client[i], 0, sizeof(Ws_Client));
            wss->client[i].fd = fd;
            *((uint32_t*)(wss->client[i].ip)) = ip;
            wss->client[i].port = port;
            wss->client[i].wss = wss;
            wss->client[i].priv = wss->priv;
            wss->client[i].index = ++wss->clientCount;
            return i;
        }
    }
    WSS_ERR("WEB SOCKET:failed, out of range(%d) !!\n", WS_SERVER_CLIENT); //满员
    return -1;
}

//共用代码块,完成客户端加人、客户端结构初始化、注册epoll监听
#define COMMON_CODE() \
wsc->wst = wst;\
wst->clientCount += 1;\
_epoll_ctrl(wst->fd_epoll, wsc->fd, EPOLLIN, EPOLL_CTL_ADD, wsc);

//添加客户端
static void client_add(Ws_Server *wss, int fd, uint32_t ip, int port)
{
    int ret;
    Ws_Client *wsc;
    Ws_Thread *wst;

    pthread_mutex_lock(&wss->lock);

    //取得空闲客户端指针
    ret = client_get(wss, fd, ip, port);
    if (ret < 0)
    {
        pthread_mutex_unlock(&wss->lock);
        return;
    }

    //新增客户端及其匹配的线程
    wsc = &wss->client[ret];
    wst = &wss->thread[ret / WS_SERVER_CLIENT_OF_THREAD];

    //线程已开启
    if (wst->isRun && //线程在运行
        wst->fd_epoll) //线程epoll正常
    {
        //共用代码块
        COMMON_CODE();
    }
        //开启新线程
    else
    {
        //参数初始化
        wst->wss = wss;
        wst->fd_epoll = epoll_create(WS_SERVER_CLIENT_OF_THREAD);
        //开线程
        new_thread(wst, &server_thread2);
        //共用代码块
        COMMON_CODE();
    }
    pthread_mutex_unlock(&wss->lock);
}

//移除特定客户端
static void client_del(Ws_Thread *wst, Ws_Client *wsc)
{
    if (wsc->isExiting)
        return;
    //标记,防止反复del
    wsc->isExiting = true;
    //从epoll监听列表中移除
    _epoll_ctrl(wst->fd_epoll, wsc->fd, 0, EPOLL_CTL_DEL, wsc);
    //关闭描述符
    close(wsc->fd);
    //如有需则断连回调
    new_thread(wsc, &client_onExit);
    //减人
    wst->clientCount -= 1;
    wst->wss->clientCount -= 1;
}

//副线程检测异常客户端并移除
static void client_detect(Ws_Thread *wst, bool delAll)
{
    Ws_Client *client = wst->wss->client;
    int i;
    for (i = 0; i < WS_SERVER_CLIENT; i++)
    {
        if (client[i].wst == wst && //客户端属于该线程管辖
            client[i].fd && //这是有效连接
            !client[i].isExiting) //不是正在退出状态
        {
            //有异常错误 || 就是要删除
            if(client[i].exitType || delAll)
                client_del(wst, &client[i]);
                //非登录状态,进行websocket握手超时计数
            else if (!client[i].isLogin)
            {
                //5秒超时(延时不准,只是大概)
                client[i].loginTimeout += 500;
                if (client[i].loginTimeout > WS_SERVER_LOGIN_TIMEOUT_MS)
                {
                    client[i].exitType = WET_LOGIN_TIMEOUT;
                    client_del(wst, &client[i]);
                }
            }
        }
    }
}

//服务器副线程,负责检测 数据接收 和 客户端断开
//只要还有一个客户端在维护就不会退出线程
static void* server_thread2(void *argv)
{
    Ws_Thread *wst = (Ws_Thread *)argv;
    int nfds, count;
    struct epoll_event events[WS_SERVER_CLIENT_OF_THREAD];

    while (!wst->wss->isExit)// && wst->clientCount > 0)
    {
        wst->isRun = true;
        //等待事件发生,-1阻塞,0/非阻塞,其它数值为超时ms
        if ((nfds = epoll_wait(wst->fd_epoll, events, WS_SERVER_CLIENT_OF_THREAD, 500)) < 0)
        {
            WSS_ERR("WEB SOCKET:epoll_wait failed\n");
            break;
        }
        for (count = 0; count < nfds; count++)
        {
            //epoll错误
            if ((events[count].events & EPOLLERR) || (events[count].events & EPOLLHUP))
            {
                //标记异常类型
                ((Ws_Client *)events[count].data.ptr)->exitType = WET_EPOLL;
                //移除
                client_del(wst, (Ws_Client *)events[count].data.ptr);
            }
                //接收数据事件
            else if (events[count].events & EPOLLIN)
                client_recv((Ws_Client *)events[count].data.ptr);
        }
        //异常客户端检查
        client_detect(wst, false);
    }
    wst->isRun = false;
    //关闭epoll描述符
    close(wst->fd_epoll);
    //关闭线程维护的所有客户端(正常情况应该都已经关闭了)
    client_detect(wst, true);
    //清空内存,下次使用
    memset(wst, 0, sizeof(Ws_Thread));
    return NULL;
}

//服务器主线程,负责检测 新客户端接入
static void* server_thread(void *argv)
{
    Ws_Server *wss = (Ws_Server *)argv;
    int ret, count;
    int fd_accept;

    socklen_t socAddrLen;
    struct sockaddr_in acceptAddr;
    struct sockaddr_in serverAddr = {0};

    int nfds;
    struct epoll_event events[WS_SERVER_CLIENT];

    serverAddr.sin_family = AF_INET; //设置为IP通信
    serverAddr.sin_addr.s_addr = INADDR_ANY; //服务器IP地址
    serverAddr.sin_port = htons(wss->port); //服务器端口号
    socAddrLen = sizeof(struct sockaddr_in);

    //socket init
    if ((wss->fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        WSS_ERR("WEB SOCKET:create socket failed\n");
        return NULL;
    }

    //地址可重用设置(有效避免bind超时)
    setsockopt(wss->fd, SOL_SOCKET, SO_REUSEADDR, &ret, sizeof(ret));

    //设置为非阻塞接收
    ret = fcntl(wss->fd, F_GETFL, 0);
    fcntl(wss->fd, F_SETFL, ret | O_NONBLOCK);

    //bind
    count = 0;
    while (bind(wss->fd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) != 0)
    {
        if (++count > WS_SERVER_BIND_TIMEOUT_MS)
        {
            WSS_ERR("WEB SOCKET:bind timeout %d 服务器端口占用中,请稍候再试\n", count);
            goto server_exit;
        }
        ws_delayms(1);
    }

    //listen
    if (listen(wss->fd, 0) != 0)
    {
        WSS_ERR("WEB SOCKET:listen failed\n");
        goto server_exit;
    }

    pthread_mutex_init(&wss->lock, NULL);

    //创建一个epoll描述符
    wss->fd_epoll = epoll_create(WS_SERVER_CLIENT);

    //向epoll注册server_sockfd监听事件
    _epoll_ctrl(wss->fd_epoll, wss->fd, EPOLLIN | EPOLLET, EPOLL_CTL_ADD, NULL);

    //正式开始
    WSS_INFO("WEB SOCKET:server ws://127.0.0.1:%d%s start \n", wss->port, wss->path);

    while (!wss->isExit)
    {
        //等待事件发生,-1阻塞,0/非阻塞,其它数值为超时ms
        if ((nfds = epoll_wait(wss->fd_epoll, events, WS_SERVER_CLIENT, 500)) < 0)
        {
            WSS_ERR("WEB SOCKET:epoll_wait failed\n");
            break;
        }
        for (count = 0; count < nfds; count++)
        {
            //新通道接入事件
            if (events[count].data.fd == wss->fd)
            {
                fd_accept = accept(wss->fd, (struct sockaddr *)&acceptAddr, &socAddrLen);
                //添加客户端
                if (fd_accept >= 0)
                    client_add(wss, fd_accept, acceptAddr.sin_addr.s_addr, acceptAddr.sin_port);
            }
        }
    }

    //移除所有副线程
    wss->isExit = true;
    //关闭epoll描述符
    close(wss->fd_epoll);
    wss->fd_epoll = 0;

    pthread_mutex_destroy(&wss->lock);

    server_exit:
    wss->isExit = true;
    //关闭socket
    close(wss->fd);
    wss->fd = 0;
    return NULL;
}

void ws_server_release(Ws_Server **wss)
{
    if (wss)
    {
        if (*wss)
        {
            (*wss)->isExit = true;
            while ((*wss)->fd)
                ws_delayms(5);
            free(*wss);
            *wss = NULL;
        }
    }
}

Ws_Server* ws_server_create(
        int port,
        const char *path,
        void *priv,
        WsOnLogin onLogin,
        WsOnMessage onMessage,
        WsOnExit onExit)
{
    Ws_Server *wss = (Ws_Server*)calloc(1, sizeof(Ws_Server));
    wss->port = port;
    strcpy(wss->path, path ? path : "/");
    wss->priv = priv;
    wss->onLogin = onLogin;
    wss->onMessage = onMessage;
    wss->onExit = onExit;
    new_thread(wss, &server_thread);
    return wss;
}


/*
 *  接收数据回调
 *  参数:
 *      wsc: 客户端信息结构体指针
 *      msg: 接收数据内容
 *      msgLen: >0时为websocket数据包,<0时为非包数据,没有=0的情况
 *      type： websocket包类型
 */
void onMessage(Ws_Client *wsc, char *msg, int msgLen, Ws_DataType type)
{
    int ret = 0;
    //正常 websocket 数据包
    if (msgLen > 0)
    {
        //客户端过多时不再打印,避免卡顿
        //if (wsc->wss->clientCount <= WS_SERVER_CLIENT_OF_PRINTF)
        //    printf("onMessage: fd/%03d index/%03d total/%03d %d/%dbytes %s\r\n",
        //           wsc->fd, wsc->index, wsc->wss->clientCount, msgLen, wsc->recvBytes, msgLen < 128 ? msg : " ");

        //在这里根据客户端的请求内容, 提供相应的回复
        UnpackJsonDataFromApp(msg,wsc);

    }
    //非 websocket 数据包
    else if (msgLen < 0)
    {
        msgLen = -msgLen;
        printf("WEB SOCKET:onMessage: fd/%03d index/%03d total/%03d %d/%dbytes bad pkg %s\n",
               wsc->fd, wsc->index, wsc->wss->clientCount, msgLen, wsc->recvBytes, msgLen < 128 ? msg : " ");
    }
    //特殊包(不需作任何处理,知道就行)
    else
    {
        if (type == WDT_PING)
            printf("WEB SOCKET:onMessage: fd/%03d index/%03d total/%03d pkg WDT_PING \n", wsc->fd, wsc->index, wsc->wss->clientCount);
        else if (type == WDT_PONG)
            printf("WEB SOCKET:onMessage: fd/%03d index/%03d total/%03d pkg WDT_PONG \n", wsc->fd, wsc->index, wsc->wss->clientCount);
        else if (type == WDT_DISCONN)
            printf("WEB SOCKET:onMessage: fd/%03d index/%03d total/%03d pkg WDT_DISCONN \n", wsc->fd, wsc->index, wsc->wss->clientCount);
    }
}

//客户端接入时(已连上),你要做什么?
void onLogin(Ws_Client *wsc)
{
    printf("WEB SOCKET:onLogin: fd/%03d index/%03d total/%03d addr/%d.%d.%d.%d:%d\n",
           wsc->fd, wsc->index, wsc->wss->clientCount,
           wsc->ip[0], wsc->ip[1], wsc->ip[2], wsc->ip[3], wsc->port);
    //打招呼
    //ws_send(wsc->fd, (char*)"Say hi~ I am server", 19, false, WDT_TXTDATA);
}

//客户端断开时(已断开),你要做什么?
void onExit(Ws_Client *wsc, Ws_ExitType exitType)
{
    //断开原因
    switch (exitType)
    {
        case WET_EPOLL:
            printf("WEB SOCKET:onExit: fd/%03d index/%03d total/%03d disconnect by epoll\n", wsc->fd, wsc->index, wsc->wss->clientCount);
            break;
        case WET_SEND:
            printf("WEB SOCKET:onExit: fd/%03d index/%03d total/%03d disconnect by send\n", wsc->fd, wsc->index, wsc->wss->clientCount);
            break;
        case WET_LOGIN:
            printf("WEB SOCKET:onExit: fd/%03d index/%03d total/%03d disconnect by login failed \n", wsc->fd, wsc->index, wsc->wss->clientCount);
            break;
        case WET_LOGIN_TIMEOUT:
            printf("WEB SOCKET:onExit: fd/%03d index/%03d total/%03d disconnect by login timeout \n", wsc->fd, wsc->index, wsc->wss->clientCount);
            break;
        case WET_DISCONNECT:
            printf("WEB SOCKET:onExit: fd/%03d index/%03d total/%03d disconnect by disconnect \n", wsc->fd, wsc->index, wsc->wss->clientCount);
            break;
        default:
            printf("WEB SOCKET:onExit: fd/%03d index/%03d total/%03d disconnect by unknow \n", wsc->fd, wsc->index, wsc->wss->clientCount);
    }
}

void *WebSocketServer()
{
    int i;
    UINT8 result;
    UINT16 length=0;
    char send_buff[1024*10];
    char send_buff_trigger[1024*20];
    //服务器必须参数
    Ws_Server *wss = ws_server_create(
            9999,       //服务器端口
            "/",        //服务器路径(这样写表示路径为空)
            NULL,        //指向自己的数据的指针,回调函数里使用 wsc->priv 取回
            &onLogin,   //客户端接入时(已连上),你要做什么?
            &onMessage, //收到客户端数据时,你要做什么?
            &onExit);   //客户端断开时(已断开),你要做什么?

    //服务器启动至少先等3秒(有时会bind超时)
    while (!wss->isExit)
    {
        //200ms周期
        ws_delayms(200);
        LightChangeLed3();//led3闪烁
        //打包200ms周期数据
        length=PackPeriodJsonDataToApp(send_buff);
        //每200ms推送信息给所有客户端
        for (i = 0; i < WS_SERVER_CLIENT; i++)
        {
            if (wss->client[i].fd && wss->client[i].isLogin && !wss->client[i].exitType&&wss->client[i].serveFlag==2)
            {
                result = ws_send(wss->client[i].fd, send_buff, length, false, WDT_TXTDATA);
                //发送失败,标记异常(后续会被自动回收)
                if (result < 0)
                    wss->client[i].exitType = WET_SEND;
                else
                    LogWrite(INFO,"%s:%d","WebSocket:send msg 204 to APP!消息长度",length);
                //printf("%s send msg 203 to %d\n",g_current_time,wss->client[i].fd);


//                snprintf(buff, sizeof(buff), "Tips from server fd/%03d index/%03d total/%03d %s",
//                         wss->client[i].fd, wss->client[i].index, wss->clientCount, ws_time());
//
//                if (ws_send(wss->client[i].fd, buff, strlen(buff), false, WDT_TXTDATA) < 0)
//                {
//                    //发送失败,标记异常
//                    wss->client[i].exitType = WET_SEND;
//                }
            }
        }
        //触发数据发送检查
        if(g_speed_plan_info.optimize_send_flag==1)
        {
            g_speed_plan_info.optimize_send_flag=0;
            //打包数据
            length=PackTriggerJsonDataToApp(send_buff_trigger);
            for (i = 0; i < WS_SERVER_CLIENT; i++)
            {
                if (wss->client[i].fd && wss->client[i].isLogin && !wss->client[i].exitType)
                {
                    result = ws_send(wss->client[i].fd, send_buff_trigger, length, false, WDT_TXTDATA);
                    //发送失败,标记异常(后续会被自动回收)
                    if (result < 0)
                        wss->client[i].exitType = WET_SEND;
                    printf("%s send msg 205 to %d,msg length %d\n",g_current_time,wss->client[i].fd,length);
                    LogWrite(INFO,"%s:%d","WebSocket:send msg 205 to APP!消息长度",length);

                }
            }
        }

    }

    ws_server_release(&wss);
    printf("WEB SOCKET:server exit \r\n");
}

/*************************************************************************
 * 功能描述: 打包发送给APP的请求无效数据
 * 输入参数: 无
 * 输出参数: 无
 * 返回值: 无
 *************************************************************************/
UINT16 PackErrorJsonDataToApp(char* json_data)
{
    UINT16 length=0;
    cJSON* pRoot = cJSON_CreateObject();
    cJSON_AddStringToObject(pRoot,"msg_type","203");
    cJSON_AddStringToObject(pRoot,"serve_flag","0");
    char *szJSON = cJSON_Print(pRoot);
    //printf("%s\n", szJSON);
    length=strlen(szJSON);
    memcpy(json_data,szJSON, strlen(szJSON));
    free(szJSON);
    return length;
}

/*************************************************************************
 * 功能描述: 打包发送给APP的请求成功数据
 * 输入参数: 无
 * 输出参数: 无
 * 返回值: 无
 *************************************************************************/
UINT16 PackSuccessJsonDataToApp(char* json_data)
{
    UINT16 length=0;
    cJSON* pRoot = cJSON_CreateObject();
    cJSON_AddStringToObject(pRoot,"msg_type","203");
    cJSON_AddStringToObject(pRoot,"serve_flag","1");
    char *szJSON = cJSON_Print(pRoot);
    //printf("%s\n", szJSON);
    length=strlen(szJSON);
    memcpy(json_data,szJSON, strlen(szJSON));
    free(szJSON);
    //printf("%s\n", json_data);
    return length;
}

/*************************************************************************
 * 功能描述: 打包发送给APP的静态JSON数据
 * 输入参数: 无
 * 输出参数: 无
 * 返回值: 无
 *************************************************************************/
UINT16 PackInitJsonDataToApp(char* json_data)
{
    UINT16 length=0;
    cJSON* pRoot = cJSON_CreateObject();
    cJSON* pArray;
    cJSON* pItem;
    char temp[50];
    cJSON_AddStringToObject(pRoot,"msg_type","203");
    //坡度数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"gradient_info",pArray);
    for(int i=0;i<g_static_data_csv.gradient_csv.length;i++)
    {
        pItem = cJSON_CreateObject();
        sprintf(temp,"%d",g_static_data_csv.gradient_csv.distance[i]);
        cJSON_AddStringToObject(pItem,"distance",temp);
        sprintf(temp,"%f",g_static_data_csv.gradient_csv.gradient[i]);
        cJSON_AddStringToObject(pItem,"gradient",temp);
        cJSON_AddItemToArray(pArray,pItem);
    }
    //曲线半径数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"curve_radius_info",pArray);
    for(int i=0;i<g_static_data_csv.curve_radius_csv.length;i++)
    {
        pItem = cJSON_CreateObject();
        sprintf(temp,"%d",g_static_data_csv.curve_radius_csv.distance[i]);
        cJSON_AddStringToObject(pItem,"distance",temp);
        sprintf(temp,"%d",g_static_data_csv.curve_radius_csv.curve_radius[i]);
        cJSON_AddStringToObject(pItem,"curve_radius",temp);
        cJSON_AddItemToArray(pArray,pItem);
    }
    //桥梁隧道数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"tunnel_info",pArray);
    for(int i=0;i<g_static_data_csv.tunnel_csv.length;i++)
    {
        pItem = cJSON_CreateObject();
        sprintf(temp,"%d",g_static_data_csv.tunnel_csv.begin_distance[i]);
        cJSON_AddStringToObject(pItem,"begin_distance",temp);
        sprintf(temp,"%d",g_static_data_csv.tunnel_csv.end_distance[i]);
        cJSON_AddStringToObject(pItem,"end_distance",temp);
        sprintf(temp,"%d",g_static_data_csv.tunnel_csv.tunnel_param[i]);
        cJSON_AddStringToObject(pItem,"tunnel_param",temp);
        cJSON_AddItemToArray(pArray,pItem);
    }
    //限速数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"speed_limit_info",pArray);
    for(int i=0;i<g_static_data_csv.speed_limit_csv.length;i++)
    {
        pItem = cJSON_CreateObject();
        sprintf(temp,"%d",g_static_data_csv.speed_limit_csv.distance[i]);
        cJSON_AddStringToObject(pItem,"distance",temp);
        sprintf(temp,"%d",g_static_data_csv.speed_limit_csv.speed_limit[i]);
        cJSON_AddStringToObject(pItem,"speed_limit",temp);
        cJSON_AddItemToArray(pArray,pItem);
    }
    //分相区数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"separate_info",pArray);
    pItem = cJSON_CreateObject();
    sprintf(temp,"%d",0);
    cJSON_AddStringToObject(pItem,"begin_distance",temp);
    sprintf(temp,"%d",0);
    cJSON_AddStringToObject(pItem,"end_distance",temp);
    cJSON_AddItemToArray(pArray,pItem);
    //信号机数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"signal_info",pArray);
    pItem = cJSON_CreateObject();
    sprintf(temp,"%d",0);
    cJSON_AddStringToObject(pItem,"distance",temp);
    cJSON_AddItemToArray(pArray,pItem);
    //线路基础数据
    sprintf(temp,"%d",g_static_data_csv.station_csv.end_distance[g_static_data_csv.station_csv.length-2]);
    cJSON_AddStringToObject(pRoot,"line_distance",temp);
    cJSON_AddStringToObject(pRoot,"line_name","成都17号线一期");
    //车站数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"station_info",pArray);
    for(int i=0;i<g_static_data_csv.station_csv.length;i++)
    {
        pItem = cJSON_CreateObject();
        sprintf(temp,"%s",g_static_data_csv.station_csv.station_name[i]);
        cJSON_AddStringToObject(pItem,"name",temp);
        sprintf(temp,"%d",g_static_data_csv.station_csv.begin_distance[i]);
        cJSON_AddStringToObject(pItem,"distance",temp);
        sprintf(temp,"%d",g_static_data_csv.station_csv.station_id[i]);
        cJSON_AddStringToObject(pItem,"station_id",temp);
        cJSON_AddItemToArray(pArray,pItem);
    }
    //列车基本参数
    cJSON* pRootTrain=cJSON_CreateObject();
    sprintf(temp,"%s",g_static_data_csv.basic_param_csv.type[0]);
    cJSON_AddStringToObject(pRootTrain,"type",temp);
    sprintf(temp,"%f",g_static_data_csv.basic_param_csv.basic_a[0]);
    cJSON_AddStringToObject(pRootTrain,"a",temp);
    sprintf(temp,"%f",g_static_data_csv.basic_param_csv.basic_b[0]);
    cJSON_AddStringToObject(pRootTrain,"b",temp);
    sprintf(temp,"%f",g_static_data_csv.basic_param_csv.basic_c[0]);
    cJSON_AddStringToObject(pRootTrain,"c",temp);
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRootTrain,"speed",pArray);
    FLOAT32 value_temp=0;
    for(int i=0;i<g_static_data_csv.dynamics_csv.length;i++)
    {
        cJSON* obj_temp= cJSON_CreateNumber(g_static_data_csv.dynamics_csv.speed[i]);
        cJSON_AddItemToArray(pArray,obj_temp);
    }
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRootTrain,"traction_aw0",pArray);
    for(int i=0;i<g_static_data_csv.dynamics_csv.length;i++)
    {
        cJSON* obj_temp= cJSON_CreateNumber((UINT32)(g_static_data_csv.dynamics_csv.traction_aw0[i]*1000));
        cJSON_AddItemToArray(pArray,obj_temp);
    }
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRootTrain,"brake_aw0",pArray);
    for(int i=0;i<g_static_data_csv.dynamics_csv.length;i++)
    {
        cJSON* obj_temp= cJSON_CreateNumber((UINT32)(g_static_data_csv.dynamics_csv.brake_aw0[i]*1000));
        cJSON_AddItemToArray(pArray,obj_temp);
    }
    cJSON_AddItemToObject(pRoot,"train_info",pRootTrain);
    cJSON_AddStringToObject(pRoot,"serve_flag","1");
    char *szJSON = cJSON_Print(pRoot);
    //printf("%s\n", szJSON);
    length=strlen(szJSON);
    memcpy(json_data,szJSON, strlen(szJSON));
    cJSON_Delete(pRoot);
    free(szJSON);
    return length;
}

/*************************************************************************
 * 功能描述: 打包发送给APP的周期JSON数据
 * 输入参数: 无
 * 输出参数: 无
 * 返回值: 无
 *************************************************************************/
UINT16 PackPeriodJsonDataToApp(char* json_data)
{
    UINT16 length=0;
    RefreshPeriodMsgToAPP();//更新接口数据
    cJSON* pRoot = cJSON_CreateObject();
    cJSON* pArray;
    cJSON* pItem;
    char temp[50];
    //消息标识
    cJSON_AddStringToObject(pRoot,"msg_type","204");
    //牵引故障标识
    sprintf(temp,"%d",g_period_msg_to_app.traction_fault_flag);
    cJSON_AddStringToObject(pRoot,"traction_fault_flag",temp);
    //制动故障标识
    sprintf(temp,"%d",g_period_msg_to_app.brake_fault_flag);
    cJSON_AddStringToObject(pRoot,"brake_fault_flag",temp);
    //其他故障标识
    sprintf(temp,"%d",g_period_msg_to_app.other_fault_flag);
    cJSON_AddStringToObject(pRoot,"other_fault_flag",temp);
    //当前站间累计牵引能耗
    sprintf(temp,"%d",g_period_msg_to_app.traction_energy);
    cJSON_AddStringToObject(pRoot,"traction_energy",temp);
    //当前站间累计再生能
    sprintf(temp,"%d",g_period_msg_to_app.regeneration_energy);
    cJSON_AddStringToObject(pRoot,"regeneration_energy",temp);
    //当前列车运行方向
    sprintf(temp,"%d",g_period_msg_to_app.train_direction);
    cJSON_AddStringToObject(pRoot,"direction",temp);
    //当前车次号
    sprintf(temp,"%d",g_period_msg_to_app.train_id);
    cJSON_AddStringToObject(pRoot,"train_id",temp);
    //车组号
    sprintf(temp,"%d",g_period_msg_to_app.train_number);
    cJSON_AddStringToObject(pRoot,"train_number",temp);
    //停准停稳标志
    sprintf(temp,"%d",g_period_msg_to_app.arrive_flag);
    cJSON_AddStringToObject(pRoot,"arrive_flag",temp);
    //允许发车标志
    sprintf(temp,"%d",g_period_msg_to_app.leave_flag);
    cJSON_AddStringToObject(pRoot,"leave_flag",temp);
    //曲线优化标志
    sprintf(temp,"%d",g_period_msg_to_app.optimize_flag);
    cJSON_AddStringToObject(pRoot,"optimize_flag",temp);
    //当前位置ATP防护速度
    sprintf(temp,"%d",g_period_msg_to_app.train_ebi);
    cJSON_AddStringToObject(pRoot,"ebi",temp);
    //实时速度
    sprintf(temp,"%d",g_period_msg_to_app.train_speed);
    cJSON_AddStringToObject(pRoot,"speed",temp);
    //当前站ID
    sprintf(temp,"%d",g_speed_plan_info.current_station_id);
    cJSON_AddStringToObject(pRoot,"current_station_id",temp);
    //下一站ID
    sprintf(temp,"%d",g_speed_plan_info.next_station_id);
    cJSON_AddStringToObject(pRoot,"next_station_id",temp);
    //当前车站发车时分
    sprintf(temp,"%s",g_period_msg_to_app.current_station_leave_time);
    cJSON_AddStringToObject(pRoot,"station_leave_time",temp);
    //列车计划到达下一车站名称
    sprintf(temp,"%s",g_period_msg_to_app.next_station_name);
    //printf("test:%s\n",temp);
    cJSON_AddStringToObject(pRoot,"next_station_name",temp);
    //列车计划到达下一车车站时分
    sprintf(temp,"%s",g_period_msg_to_app.next_station_arrive_time);
    cJSON_AddStringToObject(pRoot,"arrive_time",temp);
    //列车计划下一车车站发车时分
    sprintf(temp,"%s",g_period_msg_to_app.next_station_leave_time);
    cJSON_AddStringToObject(pRoot,"leave_time",temp);
    //实时工况
    sprintf(temp,"%d",g_period_msg_to_app.train_work_condition);
    cJSON_AddStringToObject(pRoot,"level_flag",temp);
    //实时级位
    sprintf(temp,"%d",g_period_msg_to_app.train_work_level);
    cJSON_AddStringToObject(pRoot,"level_output",temp);
    //实时位置
    sprintf(temp,"%d",g_period_msg_to_app.train_distance);
    cJSON_AddStringToObject(pRoot,"distance",temp);
    //当前运行时分
    sprintf(temp,"%s",g_period_msg_to_app.train_time);
    cJSON_AddStringToObject(pRoot,"time",temp);
    if (g_speed_plan_flag==1)
    {
        //下一阶段推荐速度
        sprintf(temp,"%d",g_period_msg_to_app.next_speed_recommend);
        cJSON_AddStringToObject(pRoot,"next_speed_recommend",temp);
        //下一阶段推荐工况
        sprintf(temp,"%d",g_period_msg_to_app.next_work_condition_recommend);
        cJSON_AddStringToObject(pRoot,"next_level_flag",temp);
        //下一阶段推荐级位
        sprintf(temp,"%d",g_period_msg_to_app.next_work_level_recommend);
        cJSON_AddStringToObject(pRoot,"next_level_output",temp);
        //下一阶段生效倒计时
        sprintf(temp,"%d",g_period_msg_to_app.next_recommend_countdown);
        cJSON_AddStringToObject(pRoot,"next_recommend_countdown",temp);
        //下一阶段生效剩余距离
        sprintf(temp,"%d",g_period_msg_to_app.next_recommend_distance);
        cJSON_AddStringToObject(pRoot,"next_recommend_distance",temp);
    }
    else
    {
        //下一阶段推荐速度
        cJSON_AddStringToObject(pRoot,"next_speed_recommend","无效");
        //下一阶段推荐工况
        cJSON_AddStringToObject(pRoot,"next_level_flag","无效");
        //下一阶段推荐级位
        cJSON_AddStringToObject(pRoot,"next_level_output","无效");
        //下一阶段生效倒计时
        cJSON_AddStringToObject(pRoot,"next_recommend_countdown","无效");
        //下一阶段生效剩余距离
        cJSON_AddStringToObject(pRoot,"next_recommend_distance","无效");
    }

    //临时限速数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"temporary_info",pArray);
    for(int i=0;i<g_period_msg_to_app.temporary_limit_num;i++)
    {
        pItem = cJSON_CreateObject();
        sprintf(temp,"%d",g_period_msg_to_app.temporary_limit_begin_distance[i]);
        cJSON_AddStringToObject(pItem,"begin_distance",temp);
        sprintf(temp,"%d",g_period_msg_to_app.temporary_limit_end_distance[i]);
        cJSON_AddStringToObject(pItem,"end_distance",temp);
        sprintf(temp,"%d",g_period_msg_to_app.temporary_limit_value[i]);
        cJSON_AddStringToObject(pItem,"limit_speed",temp);
        cJSON_AddItemToArray(pArray,pItem);
    }

    //临时限速信息
    //2车牵引机组输入电压
    sprintf(temp,"%d",g_period_msg_from_train.traction_voltage_2);
    cJSON_AddStringToObject(pRoot,"traction_voltage_2",temp);
    //3车牵引机组输入电压
    sprintf(temp,"%d",g_period_msg_from_train.traction_voltage_3);
    cJSON_AddStringToObject(pRoot,"traction_voltage_3",temp);
    //2车牵引机组输入电流
    sprintf(temp,"%d",g_period_msg_from_train.traction_current_2);
    cJSON_AddStringToObject(pRoot,"traction_current_2",temp);
    //3车牵引机组输入电流
    sprintf(temp,"%d",g_period_msg_from_train.traction_current_3);
    cJSON_AddStringToObject(pRoot,"traction_current_3",temp);
    //输入电流符号
    sprintf(temp,"%d",g_period_msg_from_train.traction_current_sign);
    cJSON_AddStringToObject(pRoot,"traction_current_sign",temp);
    //2车受电弓状态
    sprintf(temp,"%d",g_period_msg_from_signal.pantograph_2_flag);
    cJSON_AddStringToObject(pRoot,"pantograph_flag_2",temp);
    //3车受电弓状态
    sprintf(temp,"%d",g_period_msg_from_signal.pantograph_3_flag);
    cJSON_AddStringToObject(pRoot,"pantograph_flag_3",temp);
    char *szJSON = cJSON_Print(pRoot);
    //printf("%s\n", szJSON);
    //LogWrite(INFO,"%s:%s","WebSocketInfo204",szJSON);
    memcpy(json_data,szJSON, strlen(szJSON));
    length= strlen(szJSON);
    cJSON_Delete(pRoot);
    free(szJSON);
    return length;
}

/*************************************************************************
* 功能描述: 打包发送给APP的触发JSON数据（驾驶建议曲线，停站阶段曲线优化结束后发送）
        * 输入参数: 无
        * 输出参数: 无
        * 返回值: 无
*************************************************************************/
UINT16 PackTriggerJsonDataToApp(char* json_data)
{
    UINT16 length=0;
    UINT32 distance_temp=0;
    UINT32 timestamp=0;
    cJSON* pRoot = cJSON_CreateObject();
    cJSON* pArray;
    cJSON* pItem;
    char temp[50];
    //消息标识
    cJSON_AddStringToObject(pRoot,"msg_type","205");
    //当前站
    sprintf(temp,"%d",g_speed_plan_info.current_station_id);
    cJSON_AddStringToObject(pRoot,"current_station_id",temp);
    //下一站
    sprintf(temp,"%d",g_speed_plan_info.next_station_id);
    cJSON_AddStringToObject(pRoot,"next_station_id",temp);
    //运行方向
    sprintf(temp,"%d",g_direction);
    cJSON_AddStringToObject(pRoot,"direction",temp);
    //曲线优化数据
    pArray = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot,"recommend_info",pArray);
    for(int i=0;i<=g_dim;i++)
    {
        if (g_direction==DIRECTION_DOWN)
        {
            distance_temp=g_speed_plan_info.interval_begin_dis+i*g_discrete_size/100;
        }
        else
        {
            distance_temp=g_speed_plan_info.interval_begin_dis-i*g_discrete_size/100;
        }
        pItem = cJSON_CreateObject();
        sprintf(temp,"%d",distance_temp);
        cJSON_AddStringToObject(pItem,"distance",temp);
        sprintf(temp,"%f",3.6*g_speed_curve_offline[i]/100);
        cJSON_AddStringToObject(pItem,"speed",temp);
        timestamp = (UINT32)(g_plan_time[i]*1000);
        sprintf(temp,"%d",timestamp);
        cJSON_AddStringToObject(pItem,"timestamp",temp);
        cJSON_AddItemToArray(pArray,pItem);
    }
    char *szJSON = cJSON_Print(pRoot);
    //LogWrite(INFO,"%s:%s","WebSocketInfo205",szJSON);
    //printf("%s\n", szJSON);
    memcpy(json_data,szJSON, strlen(szJSON));
    length= strlen(szJSON);
    cJSON_Delete(pRoot);
    free(szJSON);
    return length;
}

/*************************************************************************
* 功能描述: 打包发送给APP的位置确认数据
        * 输入参数: 无
        * 输出参数: 无
        * 返回值: 无
*************************************************************************/
UINT16 PackLocationConfirmJsonDataToApp(char* json_data)
{
    UINT16 length=0;
    cJSON* pRoot = cJSON_CreateObject();
    cJSON* pArray;
    cJSON* pItem;
    char temp[50];
    //消息标识
    cJSON_AddStringToObject(pRoot,"msg_type","206");
    //1.根据GPS计算最近车站
    UINT16 station_index_gps=0;
    double error_dis_gps=65535;
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
    if (station_index_gps!=0)
    {
        //当前站ID
        sprintf(temp,"%d",g_static_data_csv.station_csv.station_id[station_index_gps]);
        cJSON_AddStringToObject(pRoot,"current_station_id",temp);
        //站名称
        sprintf(temp,"%s",g_static_data_csv.station_csv.station_name[station_index_gps]);
        cJSON_AddStringToObject(pRoot,"current_station_name",temp);
    }
    else
    {
        //当前站ID
        sprintf(temp,"%d",0);
        cJSON_AddStringToObject(pRoot,"current_station_id",temp);
        //站名称
        sprintf(temp,"%s","Location unknown");
        cJSON_AddStringToObject(pRoot,"current_station_name",temp);
    }

    char *szJSON = cJSON_Print(pRoot);
    //LogWrite(INFO,"%s:%s","WebSocketInfo206",szJSON);
//    printf("%s\n", szJSON);
    memcpy(json_data,szJSON, strlen(szJSON));
    length= strlen(szJSON);
    cJSON_Delete(pRoot);
    free(szJSON);
    return length;
}

/*************************************************************************
 * 功能描述: 200msAPPJSON数据发送线程
 * 输入参数:
 * 输出参数: 无
 * 返回值:
 *************************************************************************/
void *SendPeriodJsonDataToApp(void *arg)
{
    UINT8 result;
    UINT16 length=0;
    char send_buff[1024*10];
    Ws_Client *wsc=arg;
    while (wsc->serveFlag==2&&wsc->fd && wsc->isLogin && !wsc->exitType)
    {
        length=PackPeriodJsonDataToApp(send_buff);
        result = ws_send(wsc->fd, send_buff, length, false, WDT_TXTDATA);
        //发送失败,标记异常(后续会被自动回收)
        if (result < 0)
            wsc->exitType = WET_SEND;
        //printf("%s WEB SOCKET:send message 204 to APP success!\n",g_current_time);
        usleep(200000);
    }
    wsc->serveFlag=1;
    pthread_exit(0);//此线程退出
}

UINT16 DEVICE_ID[4]={1101,1102,1103,1104};// 人机交互设备ID
UINT8 DEVICE_NUM=4;
/*************************************************************************
 * 功能描述: 检验输入设备ID的有效性
 * 输入参数:   UINT16 device_id 设备ID
 * 输出参数:
 * 返回值:  1：设备有效 0：无效
 *************************************************************************/
UINT8 GetDeviceValid(UINT32 device_id)
{
    UINT8 find_result=0;
    for (int i = 0; i < DEVICE_NUM;i++)
    {
        if(device_id==DEVICE_ID[i])
        {
            find_result=1;
            break;
        }
    }
    return find_result;
}

/*************************************************************************
 * 功能描述: 检查输入的设备MAC地址是否在白名单内
 * 输入参数: CHAR*    device_mac     设备mac
 * 输出参数: 无
 * 返回值:   UINT8   0:否 1：是
 *************************************************************************/
UINT8 CheckDeviceMacValidity(CHAR* device_mac)
{
    UINT8 result=0;
    for(int i=0;i<g_device_mac_data.device_num;i++)
    {
        if(strcmp(g_device_mac_data.device_mac_list[i],device_mac)==0)
        {
            result=1;
            break;
        }
    }
    return result;
}

/*************************************************************************
 * 功能描述: 解析来自APP的数据，并进行相应处理
 * 输入参数:   char         *receive_buff       接收信息
 *           Ws_Client    *wsc                web socket client
 * 输出参数: 无
 * 返回值:
 *************************************************************************/
void UnpackJsonDataFromApp(char *receive_buff,Ws_Client *wsc)
{
    UINT8 result;
    UINT16 length=0;
    UINT16 message_id=0;
    UINT8 serve_flag=0;
    CHAR* device_mac;
    cJSON* pRoot;
    char send_buff[1024*20];
    pRoot= cJSON_Parse(receive_buff);
//    char *debug = cJSON_Print(pRoot);
//    printf("%s\n", debug);
    message_id= (UINT16)cJSON_GetObjectItem(pRoot,"msg_type")->valueint;
    device_mac=cJSON_GetObjectItem(pRoot,"device_id")->valuestring;
    result=CheckDeviceMacValidity(device_mac);
    if (1==result)
    {
        switch (message_id)
        {
            case 103:
                LogWrite(INFO,"%s","WebSocket:receive message 103 from APP!");
                serve_flag= (UINT8)cJSON_GetObjectItem(pRoot,"serve_flag")->valueint;
                if(serve_flag==1)
                {
                    wsc->serveFlag=1;
                    length=PackInitJsonDataToApp(send_buff);
                    result = ws_send(wsc->fd, send_buff, length, false, WDT_TXTDATA);
                    //发送失败,标记异常(后续会被自动回收)
                    if (result < 0)
                        wsc->exitType = WET_SEND;
                    else
                    {
                        LogWrite(INFO,"%s","WebSocket:send init message 203 to APP success!");
                        printf("%s WEB SOCKET:send init message 203 to APP success!\n",g_current_time);
                    }
                }
                else if (serve_flag==2)
                {
                    wsc->serveFlag=0;
                    length=PackSuccessJsonDataToApp(send_buff);
                    result = ws_send(wsc->fd, send_buff, length ,false, WDT_TXTDATA);
                    //发送失败,标记异常(后续会被自动回收)
                    if (result < 0)
                        wsc->exitType = WET_SEND;
                    else
                    {
                        LogWrite(INFO,"%s","WebSocket:send end message 203 to APP success!");
                        printf("%s WEB SOCKET:send end message 203 to APP success!\n",g_current_time);
                    }
                }
                else
                {
                    length=PackErrorJsonDataToApp(send_buff);
                    result = ws_send(wsc->fd, send_buff, length, false, WDT_TXTDATA);
                    //发送失败,标记异常(后续会被自动回收)
                    if (result < 0)
                        wsc->exitType = WET_SEND;
                    else
                    {
                        LogWrite(INFO,"%s","WebSocket:send error message 203 to APP success!");
                        printf("%s WEB SOCKET:send error message 203 to APP success!\n",g_current_time);
                    }
                }
                break;
            case 104:
                LogWrite(INFO,"%s","WebSocket:receive message 104 from APP!");
                serve_flag= (UINT8)cJSON_GetObjectItem(pRoot,"serve_flag")->valueint;
                if(serve_flag==1&&wsc->serveFlag==1)
                {
                    wsc->serveFlag=2;
                    /*创建通信管理线程*/
//                    pthread_t tid_app;
//                    if(pthread_create(&tid_app,NULL,SendPeriodJsonDataToApp,wsc))
//                    {
//                        perror("Fail to create server thread");
//                    }
                }
                else
                {
                    length=PackErrorJsonDataToApp(send_buff);
                    result = ws_send(wsc->fd, send_buff, length, false, WDT_TXTDATA);
                    //发送失败,标记异常(后续会被自动回收)
                    if (result < 0)
                        wsc->exitType = WET_SEND;
                    else
                    {
                        LogWrite(INFO,"%s","WebSocket:send error message 203 to APP success!");
                        printf("%s WEB SOCKET:send error message 203 to APP success!\n",g_current_time);
                    }
                }

                break;
            case 105:
                g_plan_config_info.plan_refresh_flag=1;//运行计划更新
                g_plan_config_info.train_id=(UINT32)cJSON_GetObjectItem(pRoot,"train_id")->valueint;
                g_plan_config_info.direction=(UINT8)cJSON_GetObjectItem(pRoot,"direction")->valueint;
                g_plan_config_info.plan_flag=(UINT8)cJSON_GetObjectItem(pRoot,"plan_flag")->valueint;
                cJSON *array=cJSON_GetObjectItem(pRoot,"plan_config");
                g_plan_config_info.plan_station_num= cJSON_GetArraySize(array);
                for(int i=0;i<g_plan_config_info.plan_station_num;i++)
                {
                    cJSON *array_item = cJSON_GetArrayItem(array,i);
                    CHAR* str_temp=cJSON_GetObjectItem(array_item,"name")->valuestring;
                    memcpy(g_plan_config_info.plan_station_info[i].station_name,str_temp,20);//解析站名称
                    g_plan_config_info.plan_station_info[i].distance=(UINT32)cJSON_GetObjectItem(array_item,"distance")->valueint;
                    g_plan_config_info.plan_station_info[i].station_id=(UINT8)cJSON_GetObjectItem(array_item,"station_id")->valueint;
                    g_plan_config_info.plan_station_info[i].jump_flag=(UINT8)cJSON_GetObjectItem(array_item,"is_jump")->valueint;
                }
                LogWrite(INFO,"%s","WebSocket:receive message 105 from APP!");
                printf("%s WEB SOCKET:receive message 105 from APP!\n",g_current_time);
                length= PackLocationConfirmJsonDataToApp(send_buff);
                result = ws_send(wsc->fd, send_buff, length, false, WDT_TXTDATA);
                //发送失败,标记异常(后续会被自动回收)
                if (result < 0)
                    wsc->exitType = WET_SEND;
                else
                {
                    printf("%s WEB SOCKET:send message 206 to APP success!\n",g_current_time);
                    LogWrite(INFO,"%s","WebSocket:send message 206 to APP success!");
                }

                break;
            default:
                break;
        }
    }
    else
    {
        length=PackErrorJsonDataToApp(send_buff);
        result = ws_send(wsc->fd, send_buff, length ,false, WDT_TXTDATA);
        //发送失败,标记异常(后续会被自动回收)
        if (result < 0)
            wsc->exitType = WET_SEND;
        else
        {
            LogWrite(INFO,"%s","WebSocket:send error message 203 to APP success!");
            printf("%s WEB SOCKET:send error message 203 to APP success!\n",g_current_time);
        }
    }

}



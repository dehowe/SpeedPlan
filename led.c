#include "led.h"

static volatile int g_LedFb;
#define LED_PATH_NAME           "/dev/rcxw_leds_ctl"
#define LED_MAX_NUM 4
static int light_flag_led1=0;
static int light_flag_led2=0;
static int light_flag_led3=0;
static int light_flag_led4=0;

static const char *name_buff[LED_MAX_NUM] = {
	"led_powr",
	"led_485",
	"led_error",
	"led_runing",
};

static const char *name_buff_ouput1[LED_MAX_NUM] = {
   	"led_powr->1",
	"led_485->1",
	"led_error->1",
	"led_runing->1",
};

static const char *name_buff_ouput0[LED_MAX_NUM] = {
   	"led_powr->0",
	"led_485->0",
	"led_error->0",
	"led_runing->0",
};


int Rce04aLedEncoderInit(void)
{
    int i = 0;
    char buff[5] = {0};

    if((g_LedFb = open(LED_PATH_NAME, O_RDWR|O_NOCTTY|O_NONBLOCK))<0)
    {   

    }
    return 0;
}

void Rce04aLedPoll(void)
{
    int i = 0;
    
    write(g_LedFb, name_buff_ouput1[0], strlen(name_buff_ouput1[0]));  //第1个指示灯亮
    write(g_LedFb, name_buff_ouput1[1], strlen(name_buff_ouput1[1]));  //第2个指示灯亮
    write(g_LedFb, name_buff_ouput1[2], strlen(name_buff_ouput1[2]));  //第3个指示灯亮
    write(g_LedFb, name_buff_ouput1[3], strlen(name_buff_ouput1[3]));  //第4个指示灯亮


    write(g_LedFb, name_buff_ouput0[0], strlen(name_buff_ouput0[0]));  //第1个指示灯灭
    write(g_LedFb, name_buff_ouput0[1], strlen(name_buff_ouput0[1]));  //第2个指示灯灭
    write(g_LedFb, name_buff_ouput0[2], strlen(name_buff_ouput0[2]));  //第3个指示灯灭
    write(g_LedFb, name_buff_ouput0[3], strlen(name_buff_ouput0[3]));  //第4个指示灯灭
}

//点亮led1
void LightUpLed1()
{
    write(g_LedFb, name_buff_ouput1[0], strlen(name_buff_ouput1[0]));  //第1个指示灯亮
}
//熄灭led1
void LightDownLed1()
{
    write(g_LedFb, name_buff_ouput0[0], strlen(name_buff_ouput0[0]));  //第1个指示灯灭
}
//led1点亮熄灭变化
void LightChangeLed1()
{
    if(light_flag_led1==1)
    {
        LightUpLed1();//点亮
        light_flag_led1=0;
    }
    else
    {
        LightDownLed1();//熄灭
        light_flag_led1=1;
    }
}

//点亮led2，表示收到CAN通信
void LightUpLed2()
{
    write(g_LedFb, name_buff_ouput1[1], strlen(name_buff_ouput1[1]));  //第2个指示灯亮
}
//熄灭led2，表示未收到CAN通信
void LightDownLed2()
{
    write(g_LedFb, name_buff_ouput0[1], strlen(name_buff_ouput0[1]));  //第2个指示灯灭
}
//led2点亮熄灭变化
void LightChangeLed2()
{
    if(light_flag_led2==1)
    {
        LightUpLed2();//点亮
        light_flag_led2=0;
    }
    else
    {
        LightDownLed2();//熄灭
        light_flag_led2=1;
    }
}
//点亮led3，表示前端通信正常通信
void LightUpLed3()
{
    write(g_LedFb, name_buff_ouput1[2], strlen(name_buff_ouput1[2]));  //第3个指示灯亮
}
//熄灭led3，表示前端通信断开
void LightDownLed3()
{
    write(g_LedFb, name_buff_ouput0[2], strlen(name_buff_ouput0[2]));  //第3个指示灯灭
}
//led3点亮熄灭变化
void LightChangeLed3()
{
    if(light_flag_led3==1)
    {
        LightUpLed3();//点亮
        light_flag_led3=0;
    }
    else
    {
        LightDownLed3();//熄灭
        light_flag_led3=1;
    }
}
//点亮led4，表示曲线优化正在进行
void LightUpLed4()
{
    write(g_LedFb, name_buff_ouput1[3], strlen(name_buff_ouput1[3]));  //第4个指示灯亮
}
//熄灭led4，表示未进行曲线优化
void LightDownLed4()
{
    write(g_LedFb, name_buff_ouput0[3], strlen(name_buff_ouput0[3]));  //第4个指示灯灭
}
//led4点亮熄灭变化
void LightChangeLed4()
{
    if(light_flag_led4==1)
    {
        LightUpLed4();//点亮
        light_flag_led4=0;
    }
    else
    {
        LightDownLed4();//熄灭
        light_flag_led4=1;
    }
}

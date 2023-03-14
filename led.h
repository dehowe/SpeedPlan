#include <string.h>
#include <fcntl.h>
#include <unistd.h>

//led1
// 初始点亮：程序开始运行
// 闪烁：正在周期运行
// 常亮或常灭：程序中断或退出
//led2
// 闪烁：正在周期与CAN通信
// 常亮或常灭：通信程序中断或退出
//led3
// 闪烁：正在周期与APP通信
// 常亮或常灭：通信程序中断或退出
//led4
// 常亮：正在进行曲线优化
// 常灭：无曲线优化

//led初始化
int Rce04aLedEncoderInit(void);
//点亮led1
void LightUpLed1();
//熄灭led1
void LightDownLed1();
//点亮led2
void LightUpLed2();
//熄灭led2
void LightDownLed2();
//点亮led3
void LightUpLed3();
//熄灭led3
void LightDownLed3();
//点亮led4
void LightUpLed4();
//熄灭led4
void LightDownLed4();
//led1点亮熄灭变化
void LightChangeLed1();
//led2点亮熄灭变化
void LightChangeLed2();
//led3点亮熄灭变化
void LightChangeLed3();
//led4点亮熄灭变化
void LightChangeLed4();

#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "pwm.h"

// 声明TIM1_PWM_Init函数
void TIM1_PWM_Init(u32 arr, u32 psc);

int main(void)
{ 
    u16 pwmval = 20; // 初始占空比，范围：0-100

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置系统中断优先级组为2
    delay_init(168);  // 初始化延时函数
    uart_init(115200); // 初始化串口波特率为115200

    TIM1_PWM_Init(100-1, 84-1); // 初始化TIM1的PWM，设置频率和占空比精度为100

    while(1) 
    {
        delay_ms(10); // 控制更新速度，可根据需要调整

        // 只需设置主通道的占空比，互补通道会自动输出互补信号
        TIM_SetCompare1(TIM1, pwmval);   // 设置主通道1的占空比
    }
}

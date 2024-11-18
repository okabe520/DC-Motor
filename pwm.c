#include "pwm.h"
#include "led.h"
#include "usart.h"

void TIM1_PWM_Init(u32 arr, u32 psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 使能TIM1和GPIOA、GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // 设置GPIOA8、GPIOB13为TIM1 CH1和CH1N模式，生成PWM输出
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1); // GPIOA8复用为TIM1_CH1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_TIM1); // GPIOB13复用为TIM1_CH1N

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;            // 配置GPIOA8
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;         // 复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   // 速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);               // 初始化GPIOA8

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;           // 配置GPIOB13
    GPIO_Init(GPIOB, &GPIO_InitStructure);               // 初始化GPIOB13

    // 初始化TIM1定时器
    TIM_TimeBaseStructure.TIM_Prescaler = psc;             // 定时器预分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseStructure.TIM_Period = arr;                // 自动重装载值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);        // 初始化TIM1

    // 初始化TIM1通道1的PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;      // PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  // 主输出使能
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; // 互补输出使能
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       // 主输出极性：高
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;      // 互补输出极性：高
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;      // 主输出空闲状态：置位
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;  // 互补输出空闲状态：复位

    TIM_OC1Init(TIM1, &TIM_OCInitStructure);               // 初始化TIM1通道1
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);      // 使能TIM1通道1预装载寄存器

    // 使能TIM1 ARR预装载寄存器
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    // 使能TIM1高级控制定时器输出
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // 启动TIM1
    TIM_Cmd(TIM1, ENABLE);
	}

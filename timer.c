#include "timer.h"
#include "led.h"
#include "usart.h"
 

volatile int pulse_count = 0; // 记录脉冲计数         
volatile int rpm = 0;  // 存储当前转速                 
volatile int pulse_in_one_second = 0;  // 一秒内的脉冲数

void uart_putchar(char c) {
    while (!(USART1->SR & USART_SR_TXE));  
    USART1->DR = c;  
}// 串口发送字符

void uart_puts1(const char* str) {
    while (*str) {  
        uart_putchar(*str++);  
    }
}// 串口发送字符串
void uart_puts(int str)
	{
    char buffer[20];  
    snprintf(buffer, sizeof(buffer), "%d", str);  
    uart_puts1(buffer);  
} // 串口发送整数
 
void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_CC1IF) {  // 检查通道1中断
        pulse_count++; // 增加脉冲计数
        TIM3->SR &= ~TIM_SR_CC1IF;// 清除中断标志  
    }
    if (TIM3->SR & TIM_SR_CC2IF) {  // 检查通道2中断
        pulse_count++; // 增加脉冲计数 
        TIM3->SR &= ~TIM_SR_CC2IF;// 清除中断标志 
    }
}

void TIM9_IRQHandler(void) {// 定时器9中断服务程序
    if (TIM9->SR & TIM_SR_UIF) { // 检查更新中断
        // 计算转速
        int edges_per_revolution = 11 * 2 * 2 * 4.4; // 每转一圈的边沿数
        pulse_in_one_second = pulse_count; // 获取当前脉冲计数
        pulse_count = 0; // 重置脉冲计数以便下次计算

        rpm = (pulse_in_one_second * 60) / edges_per_revolution;// 计算 RPM

        uart_puts(rpm); 

        TIM9->SR &= ~TIM_SR_UIF;
    }
}

void TIM9_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
    TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1; 
    TIM_TimeBaseStructure.TIM_Period = 10000 - 1;    
    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM9, ENABLE);
    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);// 启用 TIM9 更新中断
}

void TIM3_Init(void)
{
	  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // 使能系统时钟

    TIM_TimeBaseStructure.TIM_Period = 8400 - 1;        
    TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;    
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);// 使能定时器中断

    TIM_Cmd(TIM3, ENABLE);// 启动定时器

    NVIC_EnableIRQ(TIM3_IRQn);// 启用全局中断

}


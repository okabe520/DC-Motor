#include "timer.h"
#include "led.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/4
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

volatile int pulse_count = 0;          // ??????
volatile int rpm = 0;                  // ??????
volatile int pulse_in_one_second = 0;  // ???????

void uart_putchar(char c) {
    while (!(USART1->SR & USART_SR_TXE)); // ?????????
    USART1->DR = c; // ??????????
}

void uart_puts1(const char* str) {
    while (*str) { // ????????????
        uart_putchar(*str++); // ???????????????
    }
}
void uart_puts(int str)
	{
    char buffer[20]; // ???????????
    snprintf(buffer, sizeof(buffer), "%d", str); // ?????????
    uart_puts1(buffer); // ?????
}
// ???????


// ?????????
void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_CC1IF) { // ????1??
        pulse_count++; // ??????
        TIM3->SR &= ~TIM_SR_CC1IF; // ??????
    }
    if (TIM3->SR & TIM_SR_CC2IF) { // ????2??
        pulse_count++; // ??????
        TIM3->SR &= ~TIM_SR_CC2IF; // ??????
    }
}

void TIM9_IRQHandler(void) {
    if (TIM9->SR & TIM_SR_UIF) { // ??????
        // ????
        int edges_per_revolution = 11 * 2 * 2 * 4.4; // ????????
        pulse_in_one_second = pulse_count; // ????????
        pulse_count = 0; // ????????????

        // ?? RPM
        rpm = (pulse_in_one_second * 60) / edges_per_revolution;

        // ?????????
        uart_puts(rpm); // ???????

        // ????????
        TIM9->SR &= ~TIM_SR_UIF;
    }
}

void TIM9_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
    TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1; // 84MHz / 8400 = 10kHz
    TIM_TimeBaseStructure.TIM_Period = 10000 - 1;    // 10kHz * 1s = 10000
    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM9, ENABLE);
    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE); // ?? TIM9 ????
}

void TIM3_Init(void)
{
	  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 2. ?????
    
    TIM_TimeBaseStructure.TIM_Period = 8400 - 1;           // 1? (??????)
    TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;       // 84MHz / 8400 = 10kHz (10ms)
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // ????
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ????
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // 3. ???????
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // 4. ?????
    TIM_Cmd(TIM3, ENABLE);
    
    // 5. ??????
    NVIC_EnableIRQ(TIM3_IRQn);

}


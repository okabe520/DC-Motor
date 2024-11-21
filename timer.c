#include "timer.h"
#include "led.h"
#include "usart.h"
 

volatile int pulse_count = 0;          
volatile int rpm = 0;                   
volatile int pulse_in_one_second = 0;  

void uart_putchar(char c) {
    while (!(USART1->SR & USART_SR_TXE));  
    USART1->DR = c;  
}

void uart_puts1(const char* str) {
    while (*str) {  
        uart_putchar(*str++);  
    }
}
void uart_puts(int str)
	{
    char buffer[20];  
    snprintf(buffer, sizeof(buffer), "%d", str);  
    uart_puts1(buffer);  
} 
 
void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_CC1IF) {  
        pulse_count++;  
        TIM3->SR &= ~TIM_SR_CC1IF;  
    }
    if (TIM3->SR & TIM_SR_CC2IF) {  
        pulse_count++;  
        TIM3->SR &= ~TIM_SR_CC2IF; 
    }
}

void TIM9_IRQHandler(void) {
    if (TIM9->SR & TIM_SR_UIF) { 
        
        int edges_per_revolution = 11 * 2 * 2 * 4.4; 
        pulse_in_one_second = pulse_count; 
        pulse_count = 0; 

        rpm = (pulse_in_one_second * 60) / edges_per_revolution;

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
    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);
}

void TIM3_Init(void)
{
	  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = 8400 - 1;        
    TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;    
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM3, ENABLE);

    NVIC_EnableIRQ(TIM3_IRQn);

}


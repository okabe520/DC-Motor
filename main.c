#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "pwm.h"
#include "lcd.h"
#include "adc.h"
#include "timer.h"
#include "stm32f4xx.h"  
// PID 控制器结构体
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float integral;
    float previous_error;
    float dt;
} PID_Controller;
//这里的 Kp、Ki 和 Kd 是 PID 控制器的参数，分别控制比例、积分和微分部分
float target_speed = 200.0f; // 设置目标转速
float measured_speed = 0.0f;   // 实际转速
float measured_current = 0.0f; // 实际电流
float speed_output = 0.0f;     // 速度控制输出
float current_output = 0.0f;   // 电流控制输出
PID_Controller speed_pid;       // 定义速度 PID 控制器
PID_Controller current_pid;     // 定义电流 PID 控制器
// PID 控制器计算输出
float PID_Compute(PID_Controller *pid, float setpoint, float measured_value) {
    float error = setpoint - measured_value;
    float derivative = (error - pid->previous_error) / pid->dt;
    float output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;
    pid->previous_error = error;
	  pid->integral += error * pid->dt;
    return output;
}
// PID 控制器初始化
void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float dt) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral = 0.0f;
    pid->previous_error = 0.0f;
    pid->dt = dt;
}


// 声明TIM1_PWM_Init函数
void TIM1_PWM_Init(u32 arr, u32 psc);
void TIM8_PWM_Init(u32 arr, u32 psc);//设置互补波

char buffer[100]; // 定义一个缓冲区用于存储要发送的数据
extern int rpm;
int k;
int main(void)
{ 
    u16 pwmval = 100; // 初始占空比，范围：0-100
		u16 pwmval1 =100; // 初始占空比，范围：0-100
	
	 	u16 uadc,IU,IV;
		float temp,temp1,temp2,U,I1,I2;
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置系统中断优先级组为2
    delay_init(168);  // 初始化延时函数
    uart_init(115200); // 初始化串口波特率为115200
	
    TIM1_PWM_Init(100-1, 84-1); // 初始化TIM1的PWM，设置频率和占空比精度为100
		TIM8_PWM_Init(100-1, 84-1); // 初始化TIM1的PWM，设置频率和占空比精度为100
	
	  TIM5_CH1_Cap_Init(0xFFFFFFFF, 84 - 1); // 输入捕获，1us计数
    TIM3_Init(10000 - 1, 8400 - 1);        // 定时器统计周期1s
	
		LCD_Init();         //初始化LCD接口
	
		Adc_Init();         //初始化ADC
		Adc1_Init();
		Adc3_Init();  
	
		POINT_COLOR=RED;//设置字体为红色
		LCD_ShowString(30,150,200,16,16,"UADC:0.000V");	//先在固定位置显示小数点   
		LCD_ShowString(30,190,200,16,16,"IU:0.000A");	//先在固定位置显示小数点 
		LCD_ShowString(30,230,200,16,16,"IV:0.000A");	//先在固定位置显示小数点 	
		LCD_ShowString(30,270,200,16,16,"rpm:0");
		
		PID_Init(&speed_pid, 1, 5, 0.01, 0.01);    // 设置速度 PID 参数（Kp, Ki, Kd, dt）
    PID_Init(&current_pid, 1.0, 0.1, 0.01, 0.01);  // 设置电流 PID 参数（Kp, Ki, Kd, dt）
		
    while(1) 
    {
        delay_ms(5); // 控制更新速度，可根据需要调整
			
				Calculate_Speed();  // 计算并打印转速
			
        // 只需设置主通道的占空比，互补通道会自动输出互补信号
        TIM_SetCompare1(TIM1, pwmval);   // 设置主通道1的占空比
				TIM_SetCompare1(TIM8, pwmval1);   // 设置主通道1的占空比
			
				uadc=Get_Adc_Average(ADC_Channel_5,10);//获取通道5的转换值，10次取平均
				IU=Get_Adc1_Average(ADC_Channel_4,10);//获取通道4的转换值，10次取平均
				IV=Get_Adc3_Average(ADC_Channel_4,10);//获取通道4的转换值，10次取平均
					
				temp=(float)uadc*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
				temp1=(float)IU*(3.3/4096);
				temp2=(float)IV*(3.3/4096);
			
				temp1=(1.65-temp1)/2/1.5;
				temp2=(1.65-temp2)/2/1.5;
			
				uadc=temp;                            //赋值整数部分给adcx变量，因为adcx为u16整形
				IU=temp1;
				IV=temp2;
        U=temp;
        I1=temp1;
        I2=temp2;				
			  k=rpm*2;
				LCD_ShowxNum(69,150,uadc,1,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3
				LCD_ShowxNum(55,190,IU,1,16,0);
				LCD_ShowxNum(55,230,IV,1,16,0);
	      LCD_ShowxNum(62,270,k,4,16,0);
				
				temp-=uadc;                           //把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111
				temp1-=IU;
				temp2-=IV;

				temp*=1000;                           //小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。
				temp1*=1000;
				temp2*=1000;
				
				LCD_ShowxNum(85,150,temp,3,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是111.
				LCD_ShowxNum(71,190,temp,3,16,0X80);
				LCD_ShowxNum(71,230,temp,3,16,0X80);
	      snprintf(buffer, sizeof(buffer), "UADC: %.3f, IU: %.3f, IV: %.3f, RPM: %d\r\n", U, I1, I2, k);
        USART_SendString(buffer); // 发送字符串到串口
				
				measured_speed=rpm;
				measured_current=I1;
        // 速度控制
        speed_output = PID_Compute(&speed_pid, target_speed, measured_speed); // 计算速度控制输出
				LCD_ShowxNum(62,300,speed_output,4,16,0);
        speed_output=220-speed_output;
				LCD_ShowxNum(62,330,speed_output,4,16,0);
        // 将电流控制输出转换为 PWM 占空比
        if (speed_output < 10) speed_output = 10; // 限制最小值
        if (speed_output > 70) speed_output = 70; // 限制最大值
				//pwmval1 = speed_output;
							
    }
	}

#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "pwm.h"
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

float target_speed = 1000.0f; // 设置目标转速
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

// 读取转速传感器的函数
float Read_Speed_Sensor(void) {
    // 这里实现读取转速传感器的代码
    return 0.0f; // 返回测量的转速
}

// 读取电流传感器的函数
float Read_Current_Sensor(void) {
    // 这里实现读取电流传感器的代码
    return 0.0f; // 返回测量的电流
}

// 声明TIM1_PWM_Init函数
void TIM1_PWM_Init(u32 arr, u32 psc);

int main(void)
{ 
    u16 pwmval = 20; // 初始占空比，范围：0-100

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置系统中断优先级组为2
    delay_init(168);  // 初始化延时函数
    uart_init(115200); // 初始化串口波特率为115200

    TIM1_PWM_Init(100-1, 84-1); // 初始化TIM1的PWM，设置频率和占空比精度为100
    TIM_SetCompare1(TIM1, pwmval);   // 设置主通道1的占空比
    
    // PID 控制器初始化
    PID_Init(&speed_pid, 1.0, 0.1, 0.01, 0.01);    // 设置速度 PID 参数（Kp, Ki, Kd, dt）
    PID_Init(&current_pid, 1.0, 0.1, 0.01, 0.01);  // 设置电流 PID 参数（Kp, Ki, Kd, dt）

    while(1) 
    {
        delay_ms(10); // 控制更新速度，可根据需要调整

        // 读取传感器，更新 measured_speed 和 measured_current
        measured_speed = Read_Speed_Sensor();     // 读取当前转速
        measured_current = Read_Current_Sensor(); // 读取当前电流

        // 速度控制
        speed_output = PID_Compute(&speed_pid, target_speed, measured_speed); // 计算速度控制输出

        // 电流控制
        current_output = PID_Compute(&current_pid, speed_output, measured_current); // 计算电流控制输出

        // 将电流控制输出转换为 PWM 占空比
        if (current_output < 0) current_output = 0; // 限制最小值
        if (current_output > 100) current_output = 100; // 限制最大值

        // 设置 PWM 占空比
        TIM_SetCompare1(TIM1, (u16)current_output); // 设置主通道1的占空比
    }
}

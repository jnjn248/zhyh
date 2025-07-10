#include "drv_light.h"
#include "iot_gpio.h"
#include "drv_motor.h"
#include "iot_pwm.h"

#define LED_R_GPIO_HANDLE GPIO0_PB5
#define LED_G_GPIO_HANDLE GPIO0_PB4
#define LED_B_GPIO_HANDLE GPIO1_PD0

#define MOTOR_PWM_HANDLE_B EPWMDEV_PWM0_M1

// 舵机PWM参数定义
#define SERVO_FREQ 50        // 舵机PWM频率50Hz
#define SERVO_PERIOD 20000   // 周期20ms (单位:微秒)
#define SERVO_PULSE_0 500    // 0度脉冲宽度0.5ms
#define SERVO_PULSE_15 666  // 15度脉冲宽度1.5ms
#define SERVO_PULSE_90 1500  // 90度脉冲宽度1.5ms

// 舵机角度定义
// 舵机角度定义

#define SERVO_ANGLE_90 90   // 90度位置
#define SERVO_ANGLE_15  15    // 15度位置
#define SERVO_ANGLE_0 0   // 0度位置

static bool g_light_state = false;


/***************************************************************
* 函数名称: light_dev_init
* 说    明: rgb灯设备初始化
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void light_dev_init(void)
{
    /*
    IoTGpioInit(LED_R_GPIO_HANDLE);
    IoTGpioInit(LED_G_GPIO_HANDLE);
    IoTGpioInit(LED_B_GPIO_HANDLE);
    IoTGpioSetDir(LED_R_GPIO_HANDLE, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(LED_G_GPIO_HANDLE, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(LED_B_GPIO_HANDLE, IOT_GPIO_DIR_OUT);
    */
    IoTPwmInit(MOTOR_PWM_HANDLE_B);

}

/***************************************************************
* 函数名称: light_set_state
* 说    明: 控制灯状态
* 参    数: bool state true：打开 false：关闭
* 返 回 值: 无
***************************************************************/
void light_set_state(bool state)
{

    if (state == g_light_state)
    {
        return;
    }
    if (state)
    {
        B_motor_set_pwm(90);
        LOS_Msleep(500);
        B_motor_set_pwm(0);
        LOS_Msleep(500);

        /*
        IoTGpioSetOutputVal(LED_R_GPIO_HANDLE, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(LED_G_GPIO_HANDLE, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(LED_B_GPIO_HANDLE, IOT_GPIO_VALUE1);
        */
    }
    else
    {
        B_motor_set_pwm(90);
        LOS_Msleep(500);
        B_motor_set_pwm(0);
        LOS_Msleep(500);
        /*
        IoTGpioSetOutputVal(LED_R_GPIO_HANDLE, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(LED_G_GPIO_HANDLE, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(LED_B_GPIO_HANDLE, IOT_GPIO_VALUE0);
        */
    }
    g_light_state = state;

}



int get_light_state(void)
{
    return g_light_state;
}






void B_motor_set_pwm(unsigned int angle)
{
// 只允许0度或90度
    float pulse_width;
    if (angle >= SERVO_ANGLE_90) {
        pulse_width = SERVO_PULSE_90;  // 90度
    } else {
        pulse_width = SERVO_PULSE_0;   // 0度
    }
    
    // 计算占空比 (占空比 = 脉冲宽度 / 周期)
    float duty = (pulse_width / SERVO_PERIOD) * 100.0f;

    
    IoTPwmStart(MOTOR_PWM_HANDLE_B, (unsigned int)duty, SERVO_FREQ);
}


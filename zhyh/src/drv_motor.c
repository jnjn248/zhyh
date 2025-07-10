#include "drv_motor.h"
#include "iot_pwm.h"
#include "ohos_init.h"
#include "iot_gpio.h"

static bool g_motor_state = false;
static bool g_motor_state_C = false;
static bool g_motor_state_D = false;
 int g_motor_state_auto = 0;

#define MOTOR_PWM_HANDLE EPWMDEV_PWM6_M0
#define MOTOR_PWM_HANDLE_1 EPWMDEV_PWM1_M1
#define MOTOR_PWM_HANDLE_2 EPWMDEV_PWM7_M1

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

/***************************************************************
* 函数名称: motor_dev_init
* 说    明: 电机初始化
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void motor_dev_init(void)
{
    IoTPwmInit(MOTOR_PWM_HANDLE);
    IoTPwmInit(MOTOR_PWM_HANDLE_1);
    IoTPwmInit(MOTOR_PWM_HANDLE_2);

    /* 初始化引脚为GPIO */
    IoTGpioInit(GPIO0_PB6);
    /* 引脚配置为输入 */
    IoTGpioSetDir(GPIO0_PB6, IOT_GPIO_DIR_IN);

}


/***************************************************************
* 函数名称: motor_set_pwm
* 说    明: 设置电机pwm占空比
* 参    数: unsigned int duty 占空比
* 返 回 值: 无
***************************************************************/
void motor_set_pwm(unsigned int angle)
{
// 只允许0度或90度
    float pulse_width;
    if (angle >= SERVO_ANGLE_15) {
        pulse_width = SERVO_PULSE_15;  // 90度
    } else {
        pulse_width = SERVO_PULSE_0;   // 0度
    }
    
    // 计算占空比 (占空比 = 脉冲宽度 / 周期)
    float duty = (pulse_width / SERVO_PERIOD) * 100.0f;

    //IoTPwmStart(MOTOR_PWM_HANDLE, duty, 1000);
    IoTPwmStart(MOTOR_PWM_HANDLE, (unsigned int)duty, SERVO_FREQ);
}



/***************************************************************
* 函数名称: motor_set_state
* 说    明: 控制电机状态
* 参    数: bool state true：打开 false：关闭
* 返 回 值: 无
***************************************************************/
void motor_set_state(bool state)
{

    if (state == g_motor_state)
    {
        return;
    }

    if (state)
    {
        motor_set_pwm(0);
        LOS_Msleep(500);
         motor_set_pwm(15);
         LOS_Msleep(500);

    }
    else
    {
        motor_set_pwm(0);
        LOS_Msleep(500);
        motor_set_pwm(15);
        LOS_Msleep(500);
    } 
    g_motor_state = state;
 
}

int get_motor_state(void)
{
    return g_motor_state;
}

/***************************************************************
* 函数名称: motor_set_pwm_C
* 说    明: 设置电机pwm占空比
* 参    数: unsigned int duty 占空比
* 返 回 值: 无
***************************************************************/
void motor_set_pwm_C(unsigned int angle)
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

    
    IoTPwmStart(MOTOR_PWM_HANDLE_1, (unsigned int)duty, SERVO_FREQ);
}



/***************************************************************
* 函数名称: motor_set_state_C
* 说    明: 控制电机状态
* 参    数: bool state true：打开 false：关闭
* 返 回 值: 无
***************************************************************/
void motor_set_state_C(bool state)
{

    if (state == g_motor_state_C)
    {
        return;
    }

    if (state)
    {
        motor_set_pwm_C(90);
        LOS_Msleep(500);
         motor_set_pwm_C(0);
        LOS_Msleep(500);

    }
    else
    {
        motor_set_pwm_C(90);
        LOS_Msleep(500);
        motor_set_pwm_C(0);
        LOS_Msleep(500);
    } 
    g_motor_state_C = state;
 
}



int get_motor_state_C(void)
{
    return g_motor_state_C;
}

/***************************************************************
* 函数名称: motor_set_pwm_D
* 说    明: 设置电机pwm占空比
* 参    数: unsigned int duty 占空比
* 返 回 值: 无
***************************************************************/
void motor_set_pwm_D(unsigned int angle)
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

    
    IoTPwmStart(MOTOR_PWM_HANDLE_2, (unsigned int)duty, SERVO_FREQ);
}



/***************************************************************
* 函数名称: motor_set_state_D
* 说    明: 控制电机状态
* 参    数: bool state true：打开 false：关闭
* 返 回 值: 无
***************************************************************/
void motor_set_state_D(bool state)
{

    if (state == g_motor_state_D)
    {
        return;
    }

    if (state)
    {
        motor_set_pwm_D(90);
        LOS_Msleep(500);
        motor_set_pwm_D(0);
        LOS_Msleep(500);

    }
    else
    {
        motor_set_pwm_D(90);
        LOS_Msleep(500);
        motor_set_pwm_D(0);
        LOS_Msleep(500);
    } 
    g_motor_state_D = state;
 
}



int get_motor_state_D(void)
{
    return g_motor_state_D;
}



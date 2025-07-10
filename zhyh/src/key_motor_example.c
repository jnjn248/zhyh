#include <stdio.h>
#include <stdbool.h>
#include <string.h>  // 添加string.h头文件
#include "los_task.h"
#include "ohos_init.h"
#include "pwm.h"
#include "iot_adc.h"
#include "iot_errno.h"
#include "iot_pwm.h"
#include "lcd.h"

// PWM定义
#define PWM_HANDLE_0 0  // PWM 0
#define PWM_HANDLE_1 1  // PWM 1
#define PWM_HANDLE_2 2  // PWM 2

// 舵机PWM参数定义
#define SERVO_FREQ 50        // 舵机PWM频率50Hz
#define SERVO_PERIOD 20000   // 周期20ms (单位:微秒)
#define SERVO_PULSE_0 500    // 0度脉冲宽度0.5ms
#define SERVO_PULSE_90 1500  // 90度脉冲宽度1.5ms

// 舵机角度定义
#define SERVO_ANGLE_0  0    // 0度位置(关闭位置)
#define SERVO_ANGLE_90 90   // 90度位置(打开位置)

#define KEY_ADC_CHANNEL 7   // ADC 通道 7

// 药盒状态定义
typedef enum {
    BOX_STATUS_EMPTY = 0,    // 空
    BOX_STATUS_NORMAL = 1,   // 正常
    BOX_STATUS_WARNING = 2   // 警告
} BoxStatus;

// 服药时间定义
typedef enum {
    MED_TIME_MORNING = 0,    // 早上
    MED_TIME_NOON = 1,       // 中午
    MED_TIME_EVENING = 2     // 晚上
} MedTime;

// 服药状态定义
typedef struct {
    bool morning;            // 早上是否已服药
    bool noon;              // 中午是否已服药
    bool evening;           // 晚上是否已服药
} MedStatus;

// 药盒数据结构
typedef struct {
    int pill_count;         // 剩余药片数
    BoxStatus status;       // 药盒状态
    MedTime current_time;   // 当前服药时间
    MedStatus med_status;   // 服药状态
    int dose_setting;       // 每次使用剂量设置(0-5)
    int auto_hour;      // 新增：自动吃药小时
    int auto_minute;    // 新增：自动吃药分钟
    int countdown_sec;  // 新增：倒计时剩余秒
} PillBox;

// 长按时间阈值（毫秒）
#define LONG_PRESS_THRESHOLD 1000

// 菜单状态定义
typedef enum {
    MENU_MAIN = 0,           // 主菜单
    MENU_SETTINGS = 1,       // 设置菜单
    MENU_DETAIL_SETTINGS = 2,// 详细设置菜单
    MENU_BOX_SELECT = 3,     // 药盒选择菜单
    MENU_DOSE_SETTING = 4,   // 剂量设置菜单
    MENU_TIME_SETTING = 5,   // 时间设置菜单
    MENU_STATUS_VIEW = 6     // 状态查看菜单
} MenuState;

// 设置类型定义
typedef enum {
    SETTING_DOSE = 0,        // 剂量设置
    SETTING_TIME = 1,        // 时间设置
    SETTING_AUTO = 2,        // 自动模式设置
    SETTING_RESET = 3        // 重置设置
} SettingType;

// 按键状态定义
typedef struct {
    bool is_pressed;
    unsigned long press_start_time;
    bool is_long_press;
} ButtonState;

// 菜单状态结构
typedef struct {
    MenuState current_state;
    SettingType setting_type;
    int current_box_index;
    ButtonState button_state;
    bool auto_mode;
} MenuContext;

// 全局变量
static PillBox g_pill_boxes[3] = {
    {
        .pill_count = 20,       // 初始药片数
        .status = BOX_STATUS_NORMAL,
        .current_time = MED_TIME_MORNING,
        .med_status = {false, false, false},
        .dose_setting = 1,       // 默认每次使用1片
        .auto_hour = 8,
        .auto_minute = 0,
        .countdown_sec = 8 * 3600
    },
    {
        .pill_count = 20,
        .status = BOX_STATUS_NORMAL,
        .current_time = MED_TIME_MORNING,
        .med_status = {false, false, false},
        .dose_setting = 1,
        .auto_hour = 8,
        .auto_minute = 0,
        .countdown_sec = 8 * 3600
    },
    {
        .pill_count = 20,
        .status = BOX_STATUS_NORMAL,
        .current_time = MED_TIME_MORNING,
        .med_status = {false, false, false},
        .dose_setting = 1,
        .auto_hour = 8,
        .auto_minute = 0,
        .countdown_sec = 8 * 3600
    }
};

// PWM状态定义
typedef struct {
    bool is_running;
    unsigned int duty;
    bool is_initialized;  // 添加初始化状态标志
} PwmState;

// 全局PWM状态
static PwmState g_pwm_states[3] = {
    {false, 0, false},  // PWM 0
    {false, 0, false},  // PWM 1
    {false, 0, false}   // PWM 2
};

// 在全局变量区域添加状态缓存
static struct {
    int pill_count;
    BoxStatus status;
    MedTime current_time;
    bool morning;
    bool noon;
    bool evening;
    int dose_setting;
    MenuState menu_state;
    SettingType setting_type;
    int box_index;
} g_last_status = {0};

// 全局菜单上下文
static MenuContext g_menu_context = {
    .current_state = MENU_MAIN,
    .setting_type = SETTING_DOSE,
    .current_box_index = 0,
    .button_state = {false, 0, false},
    .auto_mode = false
};

#define LCD_MAX_LINES 20
static char last_lines[LCD_MAX_LINES][64] = {0};

// 添加LCD显示缓存结构
typedef struct {
    char box_status[3][32];    // 三个药盒状态
    char time_str[32];         // 时间显示
    char dose_str[32];         // 剂量显示
    char record_str[32];       // 记录显示
    char warning_str[32];      // 警告信息
    int selected_box;          // 当前选中的药盒
} LcdCache;

static LcdCache g_lcd_cache = {0};

static void update_lcd_line(int y, const char* new_str, char* cache_str, uint16_t color) {
    if (strcmp(new_str, cache_str) != 0) {
        // 只有当内容变化时才更新
        lcd_clear_line(y, 20);  // 清除这一行
        lcd_show_string(8, y, (const uint8_t*)new_str, color, LCD_BLACK, 16, 0);
        strcpy(cache_str, new_str);
    }
}

// 函数前向声明
static void update_box_status(void);
static void update_pill_count(int delta);
static void switch_med_time(void);
static void record_med_taken(void);
static void display_status(void);
static void handle_button_press(float voltage);
static void handle_menu_navigation(float voltage);
static void pwm_sequence(void);
static void pwm_set_angle(unsigned int handle, int angle);
static void execute_pill_sequence(unsigned int box_index);
void lcd_clear_line(int y, int line_height);
void draw_menu_header(const char* title, uint16_t color);
void draw_status_icon(int x, int y, BoxStatus status);
void draw_footer_tips(const char* tips, uint16_t color);

void lcd_force_refresh(void) {
    for (int i = 0; i < LCD_MAX_LINES; i++) {
        last_lines[i][0] = 0;
    }
}

/***************************************************************
 * 函数名称: pwm_dev_init
 * 说    明: 初始化所有PWM设备
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void pwm_dev_init(void)
{
    // 只初始化未初始化的PWM通道
    for (int i = 0; i < 3; i++) {
        if (!g_pwm_states[i].is_initialized) {
            IoTPwmInit(i);
            g_pwm_states[i].is_initialized = true;
            // 初始化后立即设置到0度位置
            pwm_set_angle(i, SERVO_ANGLE_0);
        }
    }
}

/***************************************************************
 * 函数名称: pwm_set_angle
 * 说    明: 设置舵机角度
 * 参    数: unsigned int handle PWM句柄
 *          int angle 目标角度(0或90)
 * 返 回 值: 无
 ***************************************************************/
static void pwm_set_angle(unsigned int handle, int angle)
{
    if (handle >= 3) {
        return;
    }

    // 确保PWM已初始化
    if (!g_pwm_states[handle].is_initialized) {
        pwm_dev_init();
    }

    // 只允许0度或90度
    float pulse_width;
    if (angle >= SERVO_ANGLE_90) {
        pulse_width = SERVO_PULSE_90;  // 90度
    } else {
        pulse_width = SERVO_PULSE_0;   // 0度
    }
    
    // 计算占空比 (占空比 = 脉冲宽度 / 周期)
    float duty = (pulse_width / SERVO_PERIOD) * 100.0f;

    // 先停止当前PWM输出
    if (g_pwm_states[handle].is_running) {
        IoTPwmStop(handle);
        g_pwm_states[handle].is_running = false;
    }

    // 启动PWM输出
    IoTPwmStart(handle, (unsigned int)duty, SERVO_FREQ);
    g_pwm_states[handle].is_running = true;
    g_pwm_states[handle].duty = (unsigned int)duty;
}

/***************************************************************
 * 函数名称: update_box_status
 * 说    明: 更新药盒状态
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void update_box_status(void)
{
    if (g_pill_boxes[g_menu_context.current_box_index].pill_count == 0) {
        g_pill_boxes[g_menu_context.current_box_index].status = BOX_STATUS_EMPTY;
        // 空状态：所有舵机关闭
        pwm_set_angle(PWM_HANDLE_0, SERVO_ANGLE_0);
        pwm_set_angle(PWM_HANDLE_1, SERVO_ANGLE_0);
        pwm_set_angle(PWM_HANDLE_2, SERVO_ANGLE_0);
    } else if (g_pill_boxes[g_menu_context.current_box_index].pill_count <= 5) {
        g_pill_boxes[g_menu_context.current_box_index].status = BOX_STATUS_WARNING;
        // 警告状态：所有舵机关闭
        pwm_set_angle(PWM_HANDLE_0, SERVO_ANGLE_0);
        pwm_set_angle(PWM_HANDLE_1, SERVO_ANGLE_0);
        pwm_set_angle(PWM_HANDLE_2, SERVO_ANGLE_0);
    } else {
        g_pill_boxes[g_menu_context.current_box_index].status = BOX_STATUS_NORMAL;
        // 正常状态：所有舵机关闭
        pwm_set_angle(PWM_HANDLE_0, SERVO_ANGLE_0);
        pwm_set_angle(PWM_HANDLE_1, SERVO_ANGLE_0);
        pwm_set_angle(PWM_HANDLE_2, SERVO_ANGLE_0);
    }
}

/***************************************************************
 * 函数名称: update_pill_count
 * 说    明: 更新药片数量
 * 参    数: int delta 药片数量变化值
 * 返 回 值: 无
 ***************************************************************/
static void update_pill_count(int delta)
{
    g_pill_boxes[g_menu_context.current_box_index].pill_count += delta;
    if (g_pill_boxes[g_menu_context.current_box_index].pill_count < 0) {
        g_pill_boxes[g_menu_context.current_box_index].pill_count = 0;
    }
    update_box_status();
}

/***************************************************************
 * 函数名称: switch_med_time
 * 说    明: 切换服药时间
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void switch_med_time(void)
{
    g_pill_boxes[g_menu_context.current_box_index].current_time = (g_pill_boxes[g_menu_context.current_box_index].current_time + 1) % 3;
}

/***************************************************************
 * 函数名称: execute_pill_sequence
 * 说    明: 执行单次服药动作序列
 * 参    数: unsigned int box_index 药盒索引
 * 返 回 值: 无
 ***************************************************************/
static void execute_pill_sequence(unsigned int box_index)
{
    if (box_index >= 3) {
        return;
    }

    // 确保PWM已初始化并处于0度位置
    pwm_set_angle(box_index, SERVO_ANGLE_0);
    LOS_Msleep(200);  // 等待稳定

    // 1. 转到90度位置打开
    pwm_set_angle(box_index, SERVO_ANGLE_90);
    LOS_Msleep(1000);  // 等待1秒

    // 2. 返回0度位置关闭
    pwm_set_angle(box_index, SERVO_ANGLE_0);
    LOS_Msleep(500);   // 等待0.5秒

    // 确保PWM输出停止
    IoTPwmStop(box_index);
    g_pwm_states[box_index].is_running = false;
}

/***************************************************************
 * 函数名称: record_med_taken
 * 说    明: 记录服药状态
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void record_med_taken(void)
{
    // 检查所有药盒的药片数量是否足够
    for (int i = 0; i < 3; i++) {
        if (g_pill_boxes[i].pill_count < g_pill_boxes[i].dose_setting) {
            printf("警告：药盒%d药片不足！需要%d片，但只有%d片\n", 
                   i, g_pill_boxes[i].dose_setting, g_pill_boxes[i].pill_count);
            return;
        }
    }

    // 更新所有药盒的服药状态
    for (int i = 0; i < 3; i++) {
        switch (g_pill_boxes[i].current_time) {
            case MED_TIME_MORNING:
                g_pill_boxes[i].med_status.morning = true;
                break;
            case MED_TIME_NOON:
                g_pill_boxes[i].med_status.noon = true;
                break;
            case MED_TIME_EVENING:
                g_pill_boxes[i].med_status.evening = true;
                break;
        }
    }
    
    // 执行所有药盒的服药动作
    for (int i = 0; i < 3; i++) {
        printf("执行药盒%d的服药动作\n", i);
        
        // 对每个药盒执行设定次数的服药动作
        for (int j = 0; j < g_pill_boxes[i].dose_setting; j++) {
            printf("药盒%d执行第%d次服药动作\n", i, j + 1);
            execute_pill_sequence(i);
            LOS_Msleep(1000);  // 动作间隔1秒
        }
        
        // 减少对应剂量的药片
        g_pill_boxes[i].pill_count -= g_pill_boxes[i].dose_setting;
        printf("药盒%d已服用%d片药，剩余%d片\n", 
               i, g_pill_boxes[i].dose_setting, g_pill_boxes[i].pill_count);
        
        // 更新药盒状态
        update_box_status();
    }
    lcd_force_refresh();
}

void lcd_clear_line(int y, int line_height) {
    lcd_fill(0, y, LCD_W - 1, y + line_height - 1, LCD_BLACK);
}

// 新增：增加菜单子状态
typedef enum {
    TIME_SET_HOUR = 0,
    TIME_SET_MINUTE = 1,
    TIME_COUNTDOWN = 2
} TimeSetSubState;
static TimeSetSubState g_time_set_substate = TIME_SET_HOUR;
static int g_temp_hour = 8;   // 临时变量用于设置
static int g_temp_minute = 0;

// 添加设置界面显示函数
static void display_settings(void)
{
    char buf[64];
    int y = 24;
    int line_height = 20;
    
    // 显示设置菜单标题
    lcd_fill(0, 0, LCD_W - 1, 19, LCD_DARKBLUE);
    lcd_show_string(8, 4, (const uint8_t*)"SETTINGS MENU", LCD_WHITE, LCD_DARKBLUE, 16, 0);
    
    if (g_menu_context.current_state == MENU_SETTINGS) {
        // 显示设置选项
        const char* setting_names[] = {
            "Dose Setting",
            "Time Setting",
            "Auto Mode",
            "Reset All"
        };
        
        for (int i = 0; i < 4; i++) {
            lcd_clear_line(y, line_height);
            uint16_t color = (i == g_menu_context.setting_type) ? LCD_GREEN : LCD_WHITE;
            snprintf(buf, sizeof(buf), "%s%s", 
                    (i == g_menu_context.setting_type) ? "> " : "  ",
                    setting_names[i]);
            lcd_show_string(8, y, (const uint8_t*)buf, color, LCD_BLACK, 16, 0);
            y += line_height;
        }
        
        // 显示当前选中项的详细信息
        y += line_height;
        lcd_clear_line(y, line_height);
        switch (g_menu_context.setting_type) {
            case SETTING_DOSE:
                snprintf(buf, sizeof(buf), "Set pills per dose");
                break;
            case SETTING_TIME:
                snprintf(buf, sizeof(buf), "Set medication time");
                break;
            case SETTING_AUTO:
                snprintf(buf, sizeof(buf), "Configure auto mode");
                break;
            case SETTING_RESET:
                snprintf(buf, sizeof(buf), "Reset all settings");
                break;
        }
        lcd_show_string(8, y, (const uint8_t*)buf, LCD_CYAN, LCD_BLACK, 16, 0);
        
    } else if (g_menu_context.current_state == MENU_DETAIL_SETTINGS) {
        // 显示详细设置界面
        switch (g_menu_context.setting_type) {
            case SETTING_DOSE:
                lcd_clear_line(y, line_height);
                snprintf(buf, sizeof(buf), "Box %d Dose Setting", g_menu_context.current_box_index + 1);
                lcd_show_string(8, y, (const uint8_t*)buf, LCD_YELLOW, LCD_BLACK, 16, 0);
                y += line_height;
                
                lcd_clear_line(y, line_height);
                snprintf(buf, sizeof(buf), "Current: %d pills", 
                        g_pill_boxes[g_menu_context.current_box_index].dose_setting);
                lcd_show_string(8, y, (const uint8_t*)buf, LCD_WHITE, LCD_BLACK, 16, 0);
                y += line_height;
                
                lcd_clear_line(y, line_height);
                lcd_show_string(8, y, (const uint8_t*)"Range: 1-5 pills", LCD_CYAN, LCD_BLACK, 16, 0);
                break;
                
            case SETTING_TIME:
                if (g_time_set_substate == TIME_SET_HOUR) {
                    lcd_clear_line(y, line_height);
                    lcd_show_string(8, y, (const uint8_t*)"Set Hour", LCD_YELLOW, LCD_BLACK, 16, 0);
                    y += line_height;
                    
                    lcd_clear_line(y, line_height);
                    snprintf(buf, sizeof(buf), "Hour: %02d:XX", g_temp_hour);
                    lcd_show_string(8, y, (const uint8_t*)buf, LCD_GREEN, LCD_BLACK, 16, 0);
                    
                } else if (g_time_set_substate == TIME_SET_MINUTE) {
                    lcd_clear_line(y, line_height);
                    lcd_show_string(8, y, (const uint8_t*)"Set Minute", LCD_YELLOW, LCD_BLACK, 16, 0);
                    y += line_height;
                    
                    lcd_clear_line(y, line_height);
                    snprintf(buf, sizeof(buf), "Time: %02d:%02d", g_temp_hour, g_temp_minute);
                    lcd_show_string(8, y, (const uint8_t*)buf, LCD_GREEN, LCD_BLACK, 16, 0);
                    
                } else if (g_time_set_substate == TIME_COUNTDOWN) {
                    lcd_clear_line(y, line_height);
                    lcd_show_string(8, y, (const uint8_t*)"Auto Time Set", LCD_YELLOW, LCD_BLACK, 16, 0);
                    y += line_height;
                    
                    lcd_clear_line(y, line_height);
                    int hours = g_pill_boxes[g_menu_context.current_box_index].countdown_sec / 3600;
                    int minutes = (g_pill_boxes[g_menu_context.current_box_index].countdown_sec % 3600) / 60;
                    snprintf(buf, sizeof(buf), "Next: %02d:%02d", hours, minutes);
                    lcd_show_string(8, y, (const uint8_t*)buf, LCD_GREEN, LCD_BLACK, 16, 0);
                }
                break;
                
            case SETTING_AUTO:
                lcd_clear_line(y, line_height);
                lcd_show_string(8, y, (const uint8_t*)"Auto Mode Setting", LCD_YELLOW, LCD_BLACK, 16, 0);
                y += line_height;
                
                lcd_clear_line(y, line_height);
                snprintf(buf, sizeof(buf), "Status: %s", 
                        g_menu_context.auto_mode ? "ENABLED" : "DISABLED");
                lcd_show_string(8, y, (const uint8_t*)buf, 
                              g_menu_context.auto_mode ? LCD_GREEN : LCD_RED, 
                              LCD_BLACK, 16, 0);
                break;
                
            case SETTING_RESET:
                lcd_clear_line(y, line_height);
                lcd_show_string(8, y, (const uint8_t*)"Reset All Settings", LCD_RED, LCD_BLACK, 16, 0);
                y += line_height;
                
                lcd_clear_line(y, line_height);
                lcd_show_string(8, y, (const uint8_t*)"Press UP to confirm", LCD_YELLOW, LCD_BLACK, 16, 0);
                y += line_height;
                
                lcd_clear_line(y, line_height);
                lcd_show_string(8, y, (const uint8_t*)"Press DOWN to cancel", LCD_CYAN, LCD_BLACK, 16, 0);
                break;
        }
    }
    
    // 显示底部提示
    lcd_fill(0, LCD_H - 20, LCD_W - 1, LCD_H - 1, LCD_DARKBLUE);
    if (g_menu_context.current_state == MENU_SETTINGS) {
        lcd_show_string(4, LCD_H - 18, 
                       (const uint8_t*)"UP:SELECT  DOWN:BACK  L/R:MOVE", 
                       LCD_WHITE, LCD_DARKBLUE, 16, 0);
    } else {
        lcd_show_string(4, LCD_H - 18, 
                       (const uint8_t*)"UP:SAVE  DOWN:BACK  L/R:ADJUST", 
                       LCD_WHITE, LCD_DARKBLUE, 16, 0);
    }
}

static void display_status(void)
{
    if (g_menu_context.current_state == MENU_SETTINGS || 
        g_menu_context.current_state == MENU_DETAIL_SETTINGS) {
        display_settings();
        return;
    }

    static bool first_run = true;
    char buf[64];

    if (first_run) {
        // 首次运行时完整初始化屏幕
        lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, LCD_BLACK);
        lcd_show_string(8, 4, (const uint8_t*)"SMART PILLBOX", LCD_WHITE, LCD_BLUE, 16, 0);
        first_run = false;
    }

    int y = 24;
    int line_height = 20;

    // 更新药盒状态显示
    for (int i = 0; i < 3; i++) {
        const char *status_str[] = {"EMPTY", "NORMAL", "WARNING"};
        snprintf(buf, sizeof(buf), "Box %d: %s [%d]", i + 1, 
                status_str[g_pill_boxes[i].status], 
                g_pill_boxes[i].pill_count);
        
        // 检查是否需要更新这一行
        if (strcmp(buf, g_lcd_cache.box_status[i]) != 0 || 
            i == g_menu_context.current_box_index || 
            i == g_lcd_cache.selected_box) {
            lcd_clear_line(y, line_height);
            uint16_t color = (i == g_menu_context.current_box_index) ? LCD_GREEN : LCD_WHITE;
            lcd_show_string(8, y, (const uint8_t*)buf, color, LCD_BLACK, 16, 0);
            strcpy(g_lcd_cache.box_status[i], buf);
        }
        y += line_height;
    }
    g_lcd_cache.selected_box = g_menu_context.current_box_index;

    // 更新时间显示
    const char *time_str[] = {"MORNING", "NOON", "EVENING"};
    snprintf(buf, sizeof(buf), "Time: %s", time_str[g_pill_boxes[g_menu_context.current_box_index].current_time]);
    update_lcd_line(y, buf, g_lcd_cache.time_str, LCD_CYAN);
    y += line_height;

    // 更新剂量显示
    snprintf(buf, sizeof(buf), "Dose: %d pills", g_pill_boxes[g_menu_context.current_box_index].dose_setting);
    update_lcd_line(y, buf, g_lcd_cache.dose_str, LCD_YELLOW);
    y += line_height;

    // 更新服药记录
    snprintf(buf, sizeof(buf), "Record: M[%c] N[%c] E[%c]",
             g_pill_boxes[g_menu_context.current_box_index].med_status.morning ? 'Y' : 'N',
             g_pill_boxes[g_menu_context.current_box_index].med_status.noon ? 'Y' : 'N',
             g_pill_boxes[g_menu_context.current_box_index].med_status.evening ? 'Y' : 'N');
    update_lcd_line(y, buf, g_lcd_cache.record_str, LCD_MAGENTA);
    y += line_height;

    // 更新警告信息
    const char* warning = "";
    uint16_t warning_color = LCD_BLACK;
    if (g_pill_boxes[g_menu_context.current_box_index].status == BOX_STATUS_EMPTY) {
        warning = "WARNING: BOX EMPTY!";
        warning_color = LCD_RED;
    } else if (g_pill_boxes[g_menu_context.current_box_index].status == BOX_STATUS_WARNING) {
        warning = "NOTICE: LOW PILLS!";
        warning_color = LCD_YELLOW;
    }
    
    if (strcmp(warning, g_lcd_cache.warning_str) != 0) {
        lcd_clear_line(y, line_height);
        if (warning[0] != '\0') {
            lcd_show_string(8, y, (const uint8_t*)warning, warning_color, LCD_BLACK, 16, 0);
        }
        strcpy(g_lcd_cache.warning_str, warning);
    }

    // 底部提示信息固定显示
    static bool footer_shown = false;
    if (!footer_shown) {
        lcd_fill(0, LCD_H - 20, LCD_W - 1, LCD_H - 1, LCD_DARKBLUE);
        lcd_show_string(4, LCD_H - 18, (const uint8_t*)"UP:TAKE  DOWN:SET  L/R:SWITCH", 
                       LCD_WHITE, LCD_DARKBLUE, 16, 0);
        footer_shown = true;
    }
}

/***************************************************************
 * 函数名称: adc_dev_init
 * 说    明: 初始化ADC
 * 参    数: 无
 * 返 回 值: 0为成功，反之为失败
 ***************************************************************/
static unsigned int adc_dev_init()
{
    unsigned int ret = 0;

    /* 初始化ADC */
    ret = IoTAdcInit(KEY_ADC_CHANNEL);

    if(ret != IOT_SUCCESS)
    {
        printf("%s, %s, %d: ADC Init fail\n", __FILE__, __func__, __LINE__);
    }

    return ret;
}

/***************************************************************
 * 函数名称: adc_get_voltage
 * 说    明: 获取ADC电压值
 * 参    数: 无
 * 返 回 值: 电压值
 ***************************************************************/
static float adc_get_voltage()
{
    unsigned int ret = IOT_SUCCESS;
    unsigned int data = 0;

    /* 获取ADC值 */
    ret = IoTAdcGetVal(KEY_ADC_CHANNEL, &data);

    if (ret != IOT_SUCCESS)
    {
        printf("%s, %s, %d: ADC Read Fail\n", __FILE__, __func__, __LINE__);
        return 0.0;
    }

    return (float)(data * 3.3 / 1024.0);
}

/***************************************************************
 * 函数名称: pwm_sequence
 * 说    明: 执行PWM序列
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void pwm_sequence(void)
{
    // 设置所有PWM为45%
    pwm_set_angle(PWM_HANDLE_0, 45);
    pwm_set_angle(PWM_HANDLE_1, 45);
    pwm_set_angle(PWM_HANDLE_2, 45);
    LOS_Msleep(2000);  // 保持2秒

    // 设置所有PWM为70%
    pwm_set_angle(PWM_HANDLE_0, 70);
    pwm_set_angle(PWM_HANDLE_1, 70);
    pwm_set_angle(PWM_HANDLE_2, 70);
    LOS_Msleep(2000);  // 保持2秒

    // 设置所有PWM为45%
    pwm_set_angle(PWM_HANDLE_0, 45);
    pwm_set_angle(PWM_HANDLE_1, 45);
    pwm_set_angle(PWM_HANDLE_2, 45);
    LOS_Msleep(2000);  // 保持2秒

    // 停止所有PWM输出，但保持初始化状态
    pwm_set_angle(PWM_HANDLE_0, 0);
    pwm_set_angle(PWM_HANDLE_1, 0);
    pwm_set_angle(PWM_HANDLE_2, 0);
}

// 修改按键电压阈值定义
#define KEY_NO_PRESS  3.0    // 无按键按下阈值（原3.2）
#define KEY_LEFT      1.50   // 左键阈值
#define KEY_DOWN      1.0    // 下键阈值
#define KEY_RIGHT     0.5    // 右键阈值
#define KEY_UP        0.01   // 上键阈值

/***************************************************************
 * 函数名称: handle_button_press
 * 说    明: 处理按键按下事件
 * 参    数: float voltage 按键电压值
 * 返 回 值: 无
 ***************************************************************/
static void handle_button_press(float voltage)
{
    unsigned long current_time = LOS_TickCountGet();
    
    // 检测按键状态
    if (voltage > 3.2) {
        // 无按键按下
        if (g_menu_context.button_state.is_pressed) {
            // 按键释放
            if (current_time - g_menu_context.button_state.press_start_time >= LONG_PRESS_THRESHOLD) {
                g_menu_context.button_state.is_long_press = true;
            }
            g_menu_context.button_state.is_pressed = false;
        }
    } else {
        // 有按键按下
        if (!g_menu_context.button_state.is_pressed) {
            g_menu_context.button_state.is_pressed = true;
            g_menu_context.button_state.press_start_time = current_time;
            g_menu_context.button_state.is_long_press = false;
        }
    }
}

/***************************************************************
 * 函数名称: handle_menu_navigation
 * 说    明: 处理菜单导航
 * 参    数: float voltage 按键电压值
 * 返 回 值: 无
 ***************************************************************/
static void handle_menu_navigation(float voltage)
{
    if (voltage > 3.2) return; // No button pressed

    if (voltage > 1.50) {
        // LEFT key
        if (g_menu_context.current_state == MENU_MAIN) {
            g_menu_context.current_box_index = (g_menu_context.current_box_index + 1) % 3;
            lcd_force_refresh();
        } else if (g_menu_context.current_state == MENU_SETTINGS) {
            g_menu_context.setting_type = (g_menu_context.setting_type + 1) % 4;
            lcd_force_refresh();
        } else if (g_menu_context.current_state == MENU_DETAIL_SETTINGS) {
            if (g_menu_context.setting_type == SETTING_DOSE) {
                if (g_pill_boxes[g_menu_context.current_box_index].dose_setting > 0) {
                    g_pill_boxes[g_menu_context.current_box_index].dose_setting--;
                    lcd_force_refresh();
                }
            } else if (g_menu_context.setting_type == SETTING_TIME) {
                if (g_time_set_substate == TIME_SET_HOUR) {
                    if (g_temp_hour > 0) g_temp_hour--;
                    lcd_force_refresh();
                } else if (g_time_set_substate == TIME_SET_MINUTE) {
                    if (g_temp_minute > 0) g_temp_minute--;
                    lcd_force_refresh();
                }
            }
        }
    } else if (voltage > 1.0) {
        // DOWN key
        if (g_menu_context.current_state == MENU_MAIN) {
            g_menu_context.current_state = MENU_SETTINGS;
            lcd_force_refresh();
        } else if (g_menu_context.current_state == MENU_SETTINGS) {
            // 从设置菜单返回主菜单时，重置设置相关状态
            g_menu_context.current_state = MENU_MAIN;
            g_menu_context.setting_type = SETTING_DOSE;  // 重置为默认设置类型
            g_time_set_substate = TIME_SET_HOUR;  // 重置时间设置状态
            lcd_force_refresh();
        } else if (g_menu_context.current_state == MENU_DETAIL_SETTINGS) {
            if (g_menu_context.setting_type == SETTING_TIME) {
                if (g_time_set_substate == TIME_SET_HOUR) {
                    g_menu_context.current_state = MENU_SETTINGS;
                    g_time_set_substate = TIME_SET_HOUR;  // 重置时间设置状态
                    lcd_force_refresh();
                } else if (g_time_set_substate == TIME_SET_MINUTE) {
                    g_time_set_substate = TIME_SET_HOUR;
                    lcd_force_refresh();
                } else if (g_time_set_substate == TIME_COUNTDOWN) {
                    g_menu_context.current_state = MENU_SETTINGS;
                    g_time_set_substate = TIME_SET_HOUR;  // 重置时间设置状态
                    lcd_force_refresh();
                }
            } else {
                g_menu_context.current_state = MENU_SETTINGS;
                lcd_force_refresh();
            }
        }
    } else if (voltage > 0.5) {
        // RIGHT key
        if (g_menu_context.current_state == MENU_MAIN) {
            g_menu_context.current_box_index = (g_menu_context.current_box_index + 1) % 3;
            lcd_force_refresh();
        } else if (g_menu_context.current_state == MENU_SETTINGS) {
            g_menu_context.setting_type = (g_menu_context.setting_type + 1) % 4;
            lcd_force_refresh();
        } else if (g_menu_context.current_state == MENU_DETAIL_SETTINGS) {
            if (g_menu_context.setting_type == SETTING_DOSE) {
                if (g_pill_boxes[g_menu_context.current_box_index].dose_setting < 5) {
                    g_pill_boxes[g_menu_context.current_box_index].dose_setting++;
                    lcd_force_refresh();
                }
            } else if (g_menu_context.setting_type == SETTING_TIME) {
                if (g_time_set_substate == TIME_SET_HOUR) {
                    if (g_temp_hour < 23) g_temp_hour++;
                    lcd_force_refresh();
                } else if (g_time_set_substate == TIME_SET_MINUTE) {
                    if (g_temp_minute < 59) g_temp_minute++;
                    lcd_force_refresh();
                }
            }
        }
    } else {
        // UP key
        if (g_menu_context.current_state == MENU_MAIN) {
            record_med_taken();
            lcd_force_refresh();
        } else if (g_menu_context.current_state == MENU_SETTINGS) {
            g_menu_context.current_state = MENU_DETAIL_SETTINGS;
            if (g_menu_context.setting_type == SETTING_TIME) {
                g_time_set_substate = TIME_SET_HOUR;
                g_temp_hour = g_pill_boxes[g_menu_context.current_box_index].auto_hour;
                g_temp_minute = g_pill_boxes[g_menu_context.current_box_index].auto_minute;
            }
            lcd_force_refresh();
        } else if (g_menu_context.current_state == MENU_DETAIL_SETTINGS) {
            if (g_menu_context.setting_type == SETTING_TIME) {
                if (g_time_set_substate == TIME_SET_HOUR) {
                    g_time_set_substate = TIME_SET_MINUTE;
                    lcd_force_refresh();
                } else if (g_time_set_substate == TIME_SET_MINUTE) {
                    PillBox *box = &g_pill_boxes[g_menu_context.current_box_index];
                    box->auto_hour = g_temp_hour;
                    box->auto_minute = g_temp_minute;
                    box->countdown_sec = g_temp_hour * 3600 + g_temp_minute * 60;
                    g_time_set_substate = TIME_COUNTDOWN;
                    lcd_force_refresh();
                } else if (g_time_set_substate == TIME_COUNTDOWN) {
                    PillBox *box = &g_pill_boxes[g_menu_context.current_box_index];
                    box->countdown_sec = box->auto_hour * 3600 + box->auto_minute * 60;
                    lcd_force_refresh();
                }
            } else if (g_menu_context.setting_type == SETTING_RESET) {
                for (int i = 0; i < 3; i++) {
                    g_pill_boxes[i].dose_setting = 1;
                    g_pill_boxes[i].current_time = MED_TIME_MORNING;
                    g_pill_boxes[i].med_status.morning = false;
                    g_pill_boxes[i].med_status.noon = false;
                    g_pill_boxes[i].med_status.evening = false;
                }
                g_menu_context.current_state = MENU_SETTINGS;
                lcd_force_refresh();
            } else {
                g_menu_context.current_state = MENU_SETTINGS;
                lcd_force_refresh();
            }
        }
    }
}

/***************************************************************
 * 函数名称: key_motor_thread
 * 说    明: 主线程函数
 * 参    数: void *arg 线程参数
 * 返 回 值: 无
 ***************************************************************/
void key_motor_thread(void *arg)
{
    float voltage;
    int tick = 0;
    
    lcd_init();
    pwm_dev_init();
    adc_dev_init();
    
    while (1)
    {
        voltage = adc_get_voltage();
        handle_button_press(voltage);
        handle_menu_navigation(voltage);
        display_status();
        
        // 每秒刷新倒计时
        if (++tick >= 10) {
            for (int i = 0; i < 3; i++) {
                if (g_pill_boxes[i].countdown_sec > 0) {
                    g_pill_boxes[i].countdown_sec--;
                    if (g_pill_boxes[i].countdown_sec == 0) {
                        execute_pill_sequence(i);
                        g_pill_boxes[i].countdown_sec = g_pill_boxes[i].auto_hour * 3600 + g_pill_boxes[i].auto_minute * 60;
                    }
                }
            }
            tick = 0;
        }
        
        LOS_Msleep(100);  // 使用标准的100ms采样间隔
    }
}

/***************************************************************
 * 函数名称: key_motor_example
 * 说    明: 示例入口函数
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void key_motor_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    printf("System Starting...\n");

    // Initialize LCD
    printf("Init LCD...\n");
    lcd_init();
    lcd_fill(0, 0, LCD_W-1, LCD_H-1, LCD_BLACK);
    lcd_show_string(8, LCD_H/2-16, (const uint8_t*)"SMART PILLBOX", LCD_GREEN, LCD_BLACK, 16, 0);
    lcd_show_string(8, LCD_H/2+8, (const uint8_t*)"STARTING...", LCD_WHITE, LCD_BLACK, 16, 0);

    // Initialize PWM
    printf("Init PWM...\n");
    for (int i = 0; i < 3; i++) {
        ret = IoTPwmInit(i);
        if (ret != IOT_SUCCESS) {
            printf("PWM%d Init Failed: 0x%x\n", i, ret);
            return;
        }
        g_pwm_states[i].is_initialized = true;
        pwm_set_angle(i, SERVO_ANGLE_0);
    }

    // Initialize ADC
    printf("Init ADC...\n");
    ret = adc_dev_init();
    if (ret != IOT_SUCCESS) {
        printf("ADC Init Failed: 0x%x\n", ret);
        return;
    }

    // Initialize box status
    printf("Init Box Status...\n");
    for (int i = 0; i < 3; i++) {
        g_pill_boxes[i].pill_count = 20;
        g_pill_boxes[i].status = BOX_STATUS_NORMAL;
        g_pill_boxes[i].current_time = MED_TIME_MORNING;
        g_pill_boxes[i].med_status.morning = false;
        g_pill_boxes[i].med_status.noon = false;
        g_pill_boxes[i].med_status.evening = false;
        g_pill_boxes[i].dose_setting = 1;
        update_box_status();
    }

    // Create main thread
    printf("Create Thread...\n");
    task.pfnTaskEntry = (TSK_ENTRY_FUNC)key_motor_thread;
    task.uwStackSize = 8192;
    task.pcName = "key_motor_thread";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK) {
        printf("Thread Create Failed: 0x%x\n", ret);
        return;
    }

    printf("System Ready!\n");
    
    // Show welcome screen
    lcd_fill(0, 0, LCD_W-1, LCD_H-1, LCD_BLACK);
    lcd_show_string(8, LCD_H/2-16, (const uint8_t*)"SMART PILLBOX", LCD_GREEN, LCD_BLACK, 16, 0);
    lcd_show_string(8, LCD_H/2+8, (const uint8_t*)"READY!", LCD_WHITE, LCD_BLACK, 16, 0);
    LOS_Msleep(2000);
    lcd_force_refresh();
}

APP_FEATURE_INIT(key_motor_example);

void draw_menu_header(const char* title, uint16_t color) {
    lcd_fill(0, 0, LCD_W-1, 19, color); // 顶部20像素标题条
    lcd_show_string(4, 2, (const uint8_t*)title, LCD_WHITE, color, 16, 0);
}

void draw_status_icon(int x, int y, BoxStatus status) {
    uint16_t color = (status == BOX_STATUS_NORMAL) ? LCD_GREEN : (status == BOX_STATUS_WARNING) ? LCD_YELLOW : LCD_RED;
    lcd_draw_circle(x, y, 6, color);
    lcd_draw_circle(x, y, 6, LCD_WHITE); // 白色边框
}

void draw_footer_tips(const char* tips, uint16_t color) {
    lcd_fill(0, LCD_H-20, LCD_W-1, LCD_H-1, color);
    lcd_show_string(4, LCD_H-18, (const uint8_t*)tips, LCD_WHITE, color, 16, 0);
} 
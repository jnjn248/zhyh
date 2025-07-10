/*
 * Copyright (c) 2024 iSoftStone Education Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "smart_home.h"

#include <stdio.h>
#include <stdbool.h>

#include "iot_errno.h"

#include "iot_pwm.h"
#include "iot_gpio.h"
#include "su_03t.h"
#include "iot.h"
#include "lcd.h"
#include "picture.h"
#include "adc_key.h"
#include "components.h"
#include "lcd.h"
#include "string.h"

// static bool auto_state = false;
static bool network_state = false;

void light_menu_entry(lcd_menu_t *menu);
void fan_menu_entry(lcd_menu_t *menu);
void fan_menu_entry_C(lcd_menu_t *menu);
void fan_menu_entry_D(lcd_menu_t *menu);
void fan_menu_entry_auto(lcd_menu_t *menu);

int A = 20, B = 21, C = 22, D = 23;

// 声明外部全局变量，与iot.c保持一致
extern double A_1;		   // 药盒1使用量
extern double B_1;		   // 药盒2使用量
extern double C_1;		   // 药盒3使用量
extern double D_1;		   // 药盒4使用量
// 声明外部全局变量，与iot.c保持一致
double A_2;		   // 药盒1使用量//
double B_2;		   // 药盒2使用量//
double C_2;		   // 药盒3使用量//
double D_2;		   // 药盒4使用量//
extern double temperature; // 温度
extern double humidity;	   // 湿度

extern double yh1;		   // 药盒1剩余量
extern double yh2;		   // 药盒2剩余量
extern double yh3;		   // 药盒3剩余量
extern double yh4;		   // 药盒4剩余量

/* 风扇菜单的数据初始化*/
// 修改为自动出药逻辑初始化
lcd_menu_t fan_menu_auto = {
	.img = {
		//.img=img_fan_off,
		//.img=NULL,
		//.height=64,
		//.width=64,
	},
	.is_selected = false,
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "自动出药",
	},
	.enterFunc = fan_menu_entry_auto,
	.exitFunc = NULL,
	.base_x = 20,
	.base_y = 32,

};

/* 风扇菜单的数据初始化*/
// 修改为A出药逻辑初始化
lcd_menu_t fan_menu = {
	.img = {
		//.img=img_fan_off,
		//.img=NULL,
		//.height=64,
		//.width=64,
	},
	.is_selected = false,
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "A出药",
	},
	.enterFunc = fan_menu_entry,
	.exitFunc = NULL,
	.base_x = 20,
	.base_y = 64,

};

/* 风扇菜单的数据初始化*/
// 修改为C出药逻辑初始化
lcd_menu_t fan_menu_C = {
	.img = {
		//.img=img_fan_off,
		//.img=NULL,
		//.height=64,
		//.width=64,
	},
	.is_selected = false,
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "C出药",
	},
	.enterFunc = fan_menu_entry_C,
	.exitFunc = NULL,
	.base_x = 20,
	.base_y = 128,

};

/* 风扇菜单的数据初始化*/
// 修改为D出药逻辑初始化
lcd_menu_t fan_menu_D = {
	.img = {
		//.img=img_fan_off,
		//.img=NULL,
		//.height=64,
		//.width=64,
	},
	.is_selected = false,
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "D出药",
	},
	.enterFunc = fan_menu_entry_D,
	.exitFunc = NULL,
	.base_x = 20,
	.base_y = 160,

};

/* 照明灯的菜单初始化数据*/
// 修改为B出药逻辑初始化
lcd_menu_t light_menu = {
	.img = {
		//.img= img_light_off,
		//.height=64,
		//.width=64,
	},
	.is_selected = false,
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "B出药",
	},
	.base_x = 20,
	.base_y = 96,
	.enterFunc = light_menu_entry,
	.exitFunc = NULL,
};

/* 温度面板的初始化数据*/
lcd_display_board_t temp_db = {
	.img = {
		.img = img_temp_normal,
		.height = 48,
		.width = 48,
	},
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "25.5°C",
	},
	.base_x = 180,
	.base_y = 64,
};

/* 湿度面板的初始化数据*/
lcd_display_board_t humi_db = {
	.img = {
		.img = img_humi,
		.height = 48,
		.width = 48,
	},
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "25.5°C",
	},
	.base_x = 180,
	.base_y = 120,
};

/* 亮度面板的初始化数据*/
lcd_display_board_t lum_db = {
	.img = {
		.img = img_lum,
		.height = 48,
		.width = 48,
	},
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "1234Lx",
	},
	.base_x = 180,
	.base_y = 174,
};

/* 亮度面板的初始化数据*/
// 改为A药量数据
lcd_display_board_t lum_db_A = {
	.img = {
		//.img= img_lum,
		//.height=48,
		//.width=48,
	},
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "20颗",
	},
	.base_x = 130,
	.base_y = 64,
};

/* 亮度面板的初始化数据*/
// 改为B药量数据
lcd_display_board_t lum_db_B = {
	.img = {
		//.img= img_lum,
		//.height=48,
		//.width=48,
	},
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "21片",
	},
	.base_x = 130,
	.base_y = 96,
};

/* 亮度面板的初始化数据*/
// 改为C量数据
lcd_display_board_t lum_db_C = {
	.img = {
		//.img= img_lum,
		//.height=48,
		//.width=48,
	},
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "22颗",
	},
	.base_x = 130,
	.base_y = 128,
};

/* 亮度面板的初始化数据*/
// 改为D药量数据
lcd_display_board_t lum_db_D = {
	.img = {
		//.img= img_lum,
		//.height=48,
		//.width=48,
	},
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "23片",
	},
	.base_x = 130,
	.base_y = 160,
};

/* 亮度面板的初始化数据*/
// 盒子开关逻辑
lcd_display_board_t lum_db_is_in = {
	.img = {
		//.img= img_lum,
		//.height=48,
		//.width=48,
	},
	.text = {
		.fc = LCD_MAGENTA,
		.bc = LCD_WHITE,
		.font_size = 24,
		.name = "药盒关闭",
	},
	.base_x = 20,
	.base_y = 192,
};

/* 所有的面板集合数组,方便遍历查询*/
lcd_display_board_t *lcd_dbs[] = {&temp_db, &humi_db, &lum_db, &lum_db_A, &lum_db_B, &lum_db_C, &lum_db_D, &lum_db_is_in};
/* 所有的菜单集合数组,方便遍历查询*/
lcd_menu_t *lcd_menus[] = {&fan_menu_auto, &fan_menu, &light_menu, &fan_menu_C, &fan_menu_D};
/* 菜单的个数*/
static int lcd_menu_number = sizeof(lcd_menus) / sizeof(lcd_menu_t *);
/* 菜单的当前选中索引,在数组中的位置*/
static int menu_select_index = 0;

// 菜单左和右的处理,需要考虑菜单个数的边界
void lcd_menu_selected_move_left()
{
	if (menu_select_index > 0)
	{
		menu_select_index--;
	}
}

void lcd_menu_selected_move_right()
{
	if (menu_select_index < lcd_menu_number - 1)
	{
		menu_select_index++;
	}
}

/**
 * @brief 照明灯菜单按下确认按键//改为B出药菜单按下确认按键
 *
 * @param menu
 */
void light_menu_entry(lcd_menu_t *menu)
{
	int light_state = get_light_state();
	if (light_state)
	{
		light_set_state(false);
		lcd_set_light_state(false);
	}
	else
	{
		light_set_state(true);
		lcd_set_light_state(true);
	}

	// 确保剩余量不会变为负数
	if (B > 0)
		B -= 1;
	su03t_send_double_msg(1, 0x00);
}

/**
 * @brief  风扇菜单按下确认按键//改为自动出药菜单按下确认按键
 *
 * @param menu
 */
void fan_menu_entry_auto(lcd_menu_t *menu)
{
	// 处理药盒A的自动出药
	A_2 = A_1;
	while (A_2 > 0 && A > 0)
	{
		int motor_state = get_motor_state();
		if (motor_state)
		{
			motor_set_state(false);
			lcd_set_motor_state(false);
		}
		else
		{
			motor_set_state(true);
			lcd_set_motor_state(true);
		}

		if (A > 0)
			A -= 1; // 确保剩余量不会变为负数
		A_2 -= 1;	// 减少使用量

		// 如果剩余量为0，停止出药
		if (A <= 0)
			break;
	}
	B_2 = B_1;
	// 处理药盒B的自动出药
	while (B_2 > 0 && B > 0)
	{
		int light_state = get_light_state();
		if (light_state)
		{
			light_set_state(false);
			lcd_set_light_state(false);
		}
		else
		{
			light_set_state(true);
			lcd_set_light_state(true);
		}

		if (B > 0)
			B -= 1; // 确保剩余量不会变为负数
		B_2 -= 1;	// 减少使用量

		// 如果剩余量为0，停止出药
		if (B <= 0)
			break;
	}
	C_2 = C_1;
	// 处理药盒C的自动出药
	while (C_2 > 0 && C > 0)
	{
		int motor_state = get_motor_state_C();
		if (motor_state)
		{
			motor_set_state_C(false);
			lcd_set_motor_state_C(false);
		}
		else
		{
			motor_set_state_C(true);
			lcd_set_motor_state_C(true);
		}

		if (C > 0)
			C -= 1; // 确保剩余量不会变为负数
		C_2 -= 1;	// 减少使用量

		// 如果剩余量为0，停止出药
		if (C <= 0)
			break;
	}
	D_2 = D_1;
	// 处理药盒D的自动出药
	while (D_2 > 0 && D > 0)
	{
		int motor_state = get_motor_state_D();
		if (motor_state)
		{
			motor_set_state_D(false);
			lcd_set_motor_state_D(false);
		}
		else
		{
			motor_set_state_D(true);
			lcd_set_motor_state_D(true);
		}

		if (D > 0)
			D -= 1; // 确保剩余量不会变为负数
		D_2 -= 1;	// 减少使用量

		// 如果剩余量为0，停止出药
		if (D <= 0)
			break;
	}
	su03t_send_double_msg(1, 0x00);
}

/**
 * @brief  风扇菜单按下确认按键//改为A出药菜单按下确认按键
 *
 * @param menu
 */
void fan_menu_entry(lcd_menu_t *menu)
{
	int motor_state = get_motor_state();
	if (motor_state)
	{
		motor_set_state(false);
		lcd_set_motor_state(false);
	}
	else
	{
		motor_set_state(true);
		lcd_set_motor_state(true);
	}

	// 确保剩余量不会变为负数
	if (A > 0)
		A -= 1;
	su03t_send_double_msg(1, 0x00);
}

/**
 * @brief  风扇菜单按下确认按键//改为C出药菜单按下确认按键
 *
 * @param menu
 */
void fan_menu_entry_C(lcd_menu_t *menu)
{
	int motor_state = get_motor_state_C();
	if (motor_state)
	{
		motor_set_state_C(false);
		lcd_set_motor_state_C(false);
	}
	else
	{
		motor_set_state_C(true);
		lcd_set_motor_state_C(true);
	}

	// 确保剩余量不会变为负数
	if (C > 0)
		C -= 1;
	su03t_send_double_msg(1, 0x00);
}

/**
 * @brief  风扇菜单按下确认按键//改为D出药菜单按下确认按键
 *
 * @param menu
 */
void fan_menu_entry_D(lcd_menu_t *menu)
{
	int motor_state = get_motor_state_D();
	if (motor_state)
	{
		motor_set_state_D(false);
		lcd_set_motor_state_D(false);
	}
	else
	{
		motor_set_state_D(true);
		lcd_set_motor_state_D(true);
	}

	// 确保剩余量不会变为负数
	if (D > 0)
		D -= 1;
	su03t_send_double_msg(1, 0x00);
}

/***************************************************************
 * 函数名称: lcd_dev_init
 * 说    明: lcd初始化
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void lcd_dev_init(void)
{
	lcd_init();
	lcd_fill(0, 0, LCD_W, LCD_H, LCD_WHITE);
}

/**
 * @brief 按键处理函数
 *
 * @param key_no 按键号
 */
void smart_home_key_process(int key_no)
{
	printf("smart_home_key_process:%d\n", key_no);
	if (key_no == KEY_UP)
	{
	}
	else if (key_no == KEY_DOWN)
	{
		lcd_menu_entry(lcd_menus[menu_select_index]);
	}
	else if (key_no == KEY_LEFT)
	{

		lcd_menu_selected_move_left();
	}
	else if (key_no == KEY_RIGHT)
	{
		lcd_menu_selected_move_right();
	}
}
/**
 * @brief 物联网的指令处理函数
 *
 * @param iot_cmd iot的指令
 */
void smart_home_iot_cmd_process(int iot_cmd)
{
	switch (iot_cmd)
	{
	case IOT_CMD_LIGHT_ON:
		light_set_state(true);
		lcd_set_light_state(true);
		break;
	case IOT_CMD_LIGHT_OFF:
		light_set_state(false);
		lcd_set_light_state(false);
		break;
	case IOT_CMD_MOTOR_ON:
		motor_set_state(true);
		lcd_set_motor_state(true);
		break;
	case IOT_CMD_MOTOR_OFF:
		motor_set_state(false);
		lcd_set_motor_state(false);
		break;
	}
}

/**
 * @brief 语音管家发出的指令
 *
 * @param su03t_cmd 语音管家的指令
 */
void smart_home_su03t_cmd_process(int su03t_cmd)
{
	switch (su03t_cmd)
	{
	case light_state_on:
		light_set_state(true);
		lcd_set_light_state(true);
		break;
	case light_state_off:
		light_set_state(false);
		lcd_set_light_state(false);
		break;
	case motor_state_on:
		motor_set_state(true);
		lcd_set_motor_state(true);
		break;
	case motor_state_off:
		motor_set_state(false);
		lcd_set_motor_state(false);
		break;
	case temperature_get:
	{
		double temp, humi;

		sht30_read_data(&temp, &humi);
		su03t_send_double_msg(1, temp);
	}

	break;
	case humidity_get:
	{
		double temp, humi;
		sht30_read_data(&temp, &humi);

		su03t_send_double_msg(2, humi);
	}
	break;
	case illumination_get:
	{
		double lum;
		bh1750_read_data(&lum);
		su03t_send_double_msg(3, lum);
	}

	break;
	default:
		break;
	}
}

/***************************************************************
 * 函数名称: lcd_load_ui
 * 说    明: 加载lcd ui
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void lcd_show_ui(void)
{

	lcd_show_chinese(96, 0, "智慧药盒", LCD_RED, LCD_WHITE, 32, 0);
	// lcd_show_picture(280,0, 32,32, network_state? img_wifi_on : img_wifi_off);

	lcd_menu_update(lcd_menus, lcd_menu_number, menu_select_index);

	lcd_menu_show(lcd_menus, lcd_menu_number);
	lcd_db_show(lcd_dbs, sizeof(lcd_dbs) / sizeof(lcd_display_board_t *));

	// lcd_show_picture(0,176,177,58, img_logo);
}

/***************************************************************
 * 函数名称: lcd_set_temperature
 * 说    明: 设置温度显示
 * 参    数: double temperature 温度
 * 返 回 值: 无
 ***************************************************************/
void lcd_set_temperature(double temperature)
{
	sprintf(temp_db.text.name, "%.01f℃ ", temperature);
	/* 对温度做高温和正常的区分*/
	if (temperature > 35)
	{
		temp_db.text.fc = LCD_RED;
		temp_db.img.img = img_temp_high;
	}
	else
	{
		temp_db.text.fc = LCD_MAGENTA;
		temp_db.img.img = img_temp_normal;
	}
}

/***************************************************************
 * 函数名称: lcd_set_humidity
 * 说    明: 设置湿度显示
 * 参    数: double humidity 湿度
 * 返 回 值: 无
 ***************************************************************/
void lcd_set_humidity(double humidity)
{
	sprintf(humi_db.text.name, "%.01f%% ", humidity);
}

/***************************************************************
 * 函数名称: lcd_set_illumination
 * 说    明: 设置光照强度显示
 * 参    数: double illumination 光照强度
 * 返 回 值: 无
 ***************************************************************/
void lcd_set_illumination(double illumination)
{
	sprintf(lum_db.text.name, "%.01fLx ", illumination);
}

void lcd_set_illumination_A(double illumination)
{
	sprintf(lum_db_A.text.name, "%d颗 ", illumination);
}

void lcd_set_illumination_B(double illumination)
{
	sprintf(lum_db_B.text.name, "%d片 ", illumination);
}

void lcd_set_illumination_C(double illumination)
{
	sprintf(lum_db_C.text.name, "%d颗 ", illumination);
}

void lcd_set_illumination_D(double illumination)
{
	sprintf(lum_db_D.text.name, "%d片 ", illumination);
}

void lcd_set_illumination_is_in(bool illumination)
{
	if (illumination)
	{
		sprintf(lum_db_is_in.text.name, "药盒关闭", illumination);
	}
	else
	{
		sprintf(lum_db_is_in.text.name, "药盒打开", illumination);
	}
}

void lcd_set_network_state(int state)
{
	network_state = state;
}

/***************************************************************
 * 函数名称: lcd_set_light_state
 * 说    明: 设置灯状态显示
 * 参    数: bool state true：显示"打开" false：显示"关闭"
 * 返 回 值: 无
 ***************************************************************/
void lcd_set_light_state(bool state)
{

	strcpy(light_menu.text.name, state ? "B出药" : "B出药");
	light_menu.img.img = state ? img_light_on : img_light_off;
}

/***************************************************************
 * 函数名称: lcd_set_motor_state
 * 说    明: 设置电机状态显示
 * 参    数: bool state true：显示"打开" false：显示"关闭"
 * 返 回 值: 无
 ***************************************************************/
/*void lcd_set_motor_state(bool state)
{

	strcpy(fan_menu.text.name,state? "风扇开" :"风扇关");
	fan_menu.img.img = state? img_fan_on : img_fan_off;

}*/

void lcd_set_motor_state(bool state)
{

	strcpy(fan_menu.text.name, state ? "A出药" : "A出药");
	fan_menu.img.img = state ? img_fan_on : img_fan_off;
}

void lcd_set_motor_state_C(bool state)
{

	strcpy(fan_menu_C.text.name, state ? "C出药" : "C出药");
	fan_menu.img.img = state ? img_fan_on : img_fan_off;
}

void lcd_set_motor_state_D(bool state)
{

	strcpy(fan_menu_D.text.name, state ? "D出药" : "D出药");
	fan_menu.img.img = state ? img_fan_on : img_fan_off;
}

void lcd_set_motor_state_auto(bool state)
{

	strcpy(fan_menu_D.text.name, state ? "自动出药" : "自动出药");
	fan_menu.img.img = state ? img_fan_on : img_fan_off;
}

/***************************************************************
 * 函数名称: lcd_set_auto_state
 * 说    明: 设置自动模式状态显示
 * 参    数: bool state true：显示"打开" false：显示"关闭"
 * 返 回 值: 无
 ***************************************************************/
void lcd_set_auto_state(bool state)
{

	// if (state)
	// {
	//     lcd_show_chinese(77, 204, "开启", LCD_RED, LCD_WHITE, 24, 0);
	// }
	// else
	// {
	//     lcd_show_chinese(77, 204, "关闭", LCD_RED, LCD_WHITE, 24, 0);
	// }
}

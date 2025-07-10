#ifndef _IOT_H_
#define _IOT_H_

#include <stdbool.h>

typedef struct
{
	double illumination;
	double temperature;
	double humidity;
	double yh1;
	double yh2;
	double yh3;
	double yh4;
	double yh1use;
	double yh2use;
	double yh3use;
	double yh4use;
	bool motor_state;
	bool light_state;
	bool auto_state;
} e_iot_data;

#define IOT_CMD_LIGHT_ON 0x01
#define IOT_CMD_LIGHT_OFF 0x02
#define IOT_CMD_MOTOR_ON 0x03
#define IOT_CMD_MOTOR_OFF 0x04
#define IOT_CMD_AUTO_ON 0x05
#define IOT_CMD_AUTO_OFF 0x06

// 全局变量声明
extern double A_1; // 药盒1使用量
extern double B_1; // 药盒2使用量
extern double C_1; // 药盒3使用量
extern double D_1; // 药盒4使用量

// 新增全局变量声明
extern double temperature;	// 温度
extern double humidity;		// 湿度
extern double yh1;			// 药盒1剩余量
extern double yh2;			// 药盒2剩余量
extern double yh3;			// 药盒3剩余量
extern double yh4;			// 药盒4剩余量
extern double illumination; // 光照强度

extern int eat;	 
extern int door; 

int wait_message();
void mqtt_init();
unsigned int mqtt_is_connected();
void send_msg_to_mqtt(e_iot_data *iot_data);

// 新增函数声明
void process_udp_data(void);

#endif // _IOT_H_
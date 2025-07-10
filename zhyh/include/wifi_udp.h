#ifndef _WIFI_UDP_H_
#define _WIFI_UDP_H_

#include <stdbool.h>

// UDP通信数据结构
typedef struct
{
	double illumination; // 光照强度
	double temperature;	 // 温度
	double humidity;	 // 湿度
	double box1_remain;	 // 药盒1剩余量
	double box2_remain;	 // 药盒2剩余量
	double box3_remain;	 // 药盒3剩余量
	double box4_remain;	 // 药盒4剩余量

	// B设备发送给A设备的数据 - 使用与iot.c一致的变量名
	double yh1use; // 药盒1使用量 (A_1)
	double yh2use; // 药盒2使用量 (B_1)
	double yh3use; // 药盒3使用量 (C_1)
	double yh4use; // 药盒4使用量 (D_1)
	int eat;	   // 接收ESP32的吃药指令
	int door;	   // 发送给ESP32的开门状态
} udp_data_t;

// UDP通信配置
#define UDP_SERVER_PORT 6666
#define UDP_CLIENT_PORT 8888
#define UDP_BUFFER_LEN 512

// UDP初始化和处理函数声明
void wifi_udp_init(void);
int udp_send_data(udp_data_t *data);
int udp_receive_data(udp_data_t *data);

#endif // _WIFI_UDP_H_
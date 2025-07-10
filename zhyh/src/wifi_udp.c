#include <stdio.h>
#include <string.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "los_task.h"
#include "config_network.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/stats.h"
#include "wifi_udp.h"
#include "cJSON.h"

#define LOG_TAG "UDP"
#define UDP_LOG(fmt, ...) printf("[UDP] " fmt "\n", ##__VA_ARGS__)

static WifiLinkedInfo wifi_info;
static int udp_server_fd = -1;
static int udp_client_fd = -1;
static bool is_wifi_connected = false;

// 数据缓冲区
static udp_data_t received_data = {0};
static udp_data_t send_data_buf = {0};
static osMutexId_t data_mutex = NULL;

// 声明外部全局变量，与iot.c保持一致
extern double A_1;		   // 药盒1使用量
extern double B_1;		   // 药盒2使用量
extern double C_1;		   // 药盒3使用量
extern double D_1;		   // 药盒4使用量
extern double temperature; // 温度
extern double humidity;	   // 湿度
extern double yh1;		   // 药盒1剩余量
extern double yh2;		   // 药盒2剩余量
extern double yh3;		   // 药盒3剩余量
extern double yh4;		   // 药盒4剩余量

// UDP服务器处理函数
static void udp_server_task(void *arg)
{
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_len = sizeof(client_addr);
	char buffer[UDP_BUFFER_LEN];

	while (1)
	{
		if (!is_wifi_connected)
		{
			LOS_Msleep(1000);
			continue;
		}

		if (udp_server_fd < 0)
		{
			udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (udp_server_fd < 0)
			{
				UDP_LOG("Failed to create server socket");
				LOS_Msleep(1000);
				continue;
			}

			// 设置SO_REUSEADDR选项
			int reuse = 1;
			setsockopt(udp_server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			server_addr.sin_port = htons(UDP_SERVER_PORT);

			if (bind(udp_server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				UDP_LOG("Failed to bind server socket");
				close(udp_server_fd);
				udp_server_fd = -1;
				LOS_Msleep(1000);
				continue;
			}

			UDP_LOG("UDP server started on port %d", UDP_SERVER_PORT);
		}

		// 接收数据
		memset(buffer, 0, UDP_BUFFER_LEN);
		int recv_len = recvfrom(udp_server_fd, buffer, UDP_BUFFER_LEN, 0,
								(struct sockaddr *)&client_addr, &addr_len);

		if (recv_len > 0)
		{
			UDP_LOG("Received %d bytes from %s:%d", recv_len,
					inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			UDP_LOG("Data: %s", buffer);

			// 解析JSON数据
			cJSON *root = cJSON_Parse(buffer);
			if (root)
			{
				cJSON *yh1use = cJSON_GetObjectItem(root, "yh1use");
				cJSON *yh2use = cJSON_GetObjectItem(root, "yh2use");
				cJSON *yh3use = cJSON_GetObjectItem(root, "yh3use");
				cJSON *yh4use = cJSON_GetObjectItem(root, "yh4use");
				cJSON *eat_item = cJSON_GetObjectItem(root, "eat");

				// 处理药盒使用量
				if (yh1use && yh2use && yh3use && yh4use)
				{
					osMutexAcquire(data_mutex, osWaitForever);
					received_data.yh1use = yh1use->valuedouble;
					received_data.yh2use = yh2use->valuedouble;
					received_data.yh3use = yh3use->valuedouble;
					received_data.yh4use = yh4use->valuedouble;
					osMutexRelease(data_mutex);

					A_1 = received_data.yh1use;
					B_1 = received_data.yh2use;
					C_1 = received_data.yh3use;
					D_1 = received_data.yh4use;

					UDP_LOG("Received usage data: %.2f, %.2f, %.2f, %.2f",
							received_data.yh1use, received_data.yh2use,
							received_data.yh3use, received_data.yh4use);
				}

				// 处理eat指令
				if (eat_item)
				{
					osMutexAcquire(data_mutex, osWaitForever);
					received_data.eat = eat_item->valueint;
					osMutexRelease(data_mutex);

					extern int eat;
					eat = received_data.eat;
					UDP_LOG("Received eat: %d", received_data.eat);
				}
				UDP_LOG("Raw UDP JSON: %s", buffer);
				cJSON_Delete(root);
			}
		}
		LOS_Msleep(100);
	}
}

// UDP客户端处理函数
static void udp_client_task(void *arg)
{
	struct sockaddr_in client_addr;
	char buffer[UDP_BUFFER_LEN];
	udp_data_t local_data;

	while (1)
	{
		if (!is_wifi_connected)
		{
			LOS_Msleep(1000);
			continue;
		}

		if (udp_client_fd < 0)
		{
			udp_client_fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (udp_client_fd < 0)
			{
				UDP_LOG("Failed to create client socket");
				LOS_Msleep(1000);
				continue;
			}

			// 设置广播权限
			int broadcast = 1;
			setsockopt(udp_client_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

			UDP_LOG("UDP client initialized");
		}

		// 获取当前数据
		osMutexAcquire(data_mutex, osWaitForever);
		memcpy(&local_data, &send_data_buf, sizeof(udp_data_t));
		osMutexRelease(data_mutex);

		// 创建JSON数据
		cJSON *root = cJSON_CreateObject();
		if (root)
		{
			cJSON_AddNumberToObject(root, "illumination", local_data.illumination); // 新增光照
			cJSON_AddNumberToObject(root, "temperature", local_data.temperature);
			cJSON_AddNumberToObject(root, "humidity", local_data.humidity);
			cJSON_AddNumberToObject(root, "box1_remain", local_data.box1_remain);
			cJSON_AddNumberToObject(root, "box2_remain", local_data.box2_remain);
			cJSON_AddNumberToObject(root, "box3_remain", local_data.box3_remain);
			cJSON_AddNumberToObject(root, "box4_remain", local_data.box4_remain);
			cJSON_AddNumberToObject(root, "door", local_data.door);

			char *json_str = cJSON_PrintUnformatted(root);
			if (json_str)
			{
				memset(&client_addr, 0, sizeof(client_addr));
				client_addr.sin_family = AF_INET;
				client_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST); // 广播地址
				client_addr.sin_port = htons(UDP_CLIENT_PORT);

				int sent_bytes = sendto(udp_client_fd, json_str, strlen(json_str), 0,
										(struct sockaddr *)&client_addr, sizeof(client_addr));

				if (sent_bytes > 0)
				{
					UDP_LOG("Sent %d bytes to broadcast:%d - Data: illum=%.2f, temp=%.2f, hum=%.2f, box1=%.2f, box2=%.2f, box3=%.2f, box4=%.2f, door=%d",
							sent_bytes, UDP_CLIENT_PORT, local_data.illumination, local_data.temperature, local_data.humidity,
							local_data.box1_remain, local_data.box2_remain, local_data.box3_remain, local_data.box4_remain, local_data.door);
				}

				free(json_str);
			}
			cJSON_Delete(root);
		}

		LOS_Msleep(1000); // 每秒发送一次数据
	}
}

// WiFi连接状态检查
static void wifi_status_check_task(void *arg)
{
	while (1)
	{
		WifiLinkedInfo info;
		if (GetLinkedInfo(&info) == WIFI_SUCCESS)
		{
			if (info.connState == WIFI_CONNECTED && info.ipAddress != 0)
			{
				is_wifi_connected = true;
				wifi_info = info;
				UDP_LOG("WiFi connected, IP: %s", inet_ntoa(info.ipAddress));
			}
			else
			{
				if (is_wifi_connected)
				{
					UDP_LOG("WiFi disconnected");
				}
				is_wifi_connected = false;
			}
		}
		LOS_Msleep(1000);
	}
}

// 初始化UDP通信
void wifi_udp_init(void)
{
	unsigned int thread_id;
	TSK_INIT_PARAM_S task_param = {0};

	// 创建互斥锁
	osMutexAttr_t mutex_attr = {0};
	data_mutex = osMutexNew(&mutex_attr);
	if (data_mutex == NULL)
	{
		UDP_LOG("Failed to create data mutex");
		return;
	}

	// 创建WiFi状态检查任务
	task_param.pfnTaskEntry = (TSK_ENTRY_FUNC)wifi_status_check_task;
	task_param.uwStackSize = 4096;
	task_param.pcName = "wifi_check";
	task_param.usTaskPrio = 25;
	LOS_TaskCreate(&thread_id, &task_param);

	// 创建UDP服务器任务
	task_param.pfnTaskEntry = (TSK_ENTRY_FUNC)udp_server_task;
	task_param.pcName = "udp_server";
	LOS_TaskCreate(&thread_id, &task_param);

	// 创建UDP客户端任务
	task_param.pfnTaskEntry = (TSK_ENTRY_FUNC)udp_client_task;
	task_param.pcName = "udp_client";
	LOS_TaskCreate(&thread_id, &task_param);

	UDP_LOG("UDP communication initialized");
}

// 发送数据接口
int udp_send_data(udp_data_t *data)
{
	if (!is_wifi_connected || !data)
	{
		return -1;
	}

	osMutexAcquire(data_mutex, osWaitForever);
	memcpy(&send_data_buf, data, sizeof(udp_data_t));
	osMutexRelease(data_mutex);

	return 0;
}

// 接收数据接口
int udp_receive_data(udp_data_t *data)
{
	if (!is_wifi_connected || !data)
	{
		return -1;
	}

	osMutexAcquire(data_mutex, osWaitForever);
	memcpy(data, &received_data, sizeof(udp_data_t));
	osMutexRelease(data_mutex);

	return 0;
}
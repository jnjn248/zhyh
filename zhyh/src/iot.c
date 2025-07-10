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

#include <stdio.h>
#include <stdlib.h>

#include "MQTTClient.h"
#include "cJSON.h"
#include "cmsis_os2.h"
#include "config_network.h"
#include "iot.h"
#include "los_task.h"
#include "ohos_init.h"
#include "smart_home_event.h"
#include "wifi_udp.h"

#define MQTT_DEVICES_PWD "df9426b14936bc707c9efc90176be95efb79b601d2a7aa14d7f605652ddabe49"

#define HOST_ADDR "c70a91618a.st1.iotda-device.cn-south-1.myhuaweicloud.com"

#define DEVICE_ID "68298f570e587e2293165b50_znyh_0_0_2025070512"

#define PUBLISH_TOPIC "$oc/devices/" DEVICE_ID "/sys/properties/report"
#define SUBCRIB_TOPIC \
	"$oc/devices/" DEVICE_ID "/sys/commands/+" /// request_id={request_id}"
#define RESPONSE_TOPIC \
	"$oc/devices/" DEVICE_ID "/sys/commands/response" /// request_id={request_id}"

#define MAX_BUFFER_LENGTH 512
#define MAX_STRING_LENGTH 64

static unsigned char sendBuf[MAX_BUFFER_LENGTH];
static unsigned char readBuf[MAX_BUFFER_LENGTH];

Network network;
MQTTClient client;

static char mqtt_devid[64] = DEVICE_ID;
static char mqtt_pwd[128] = MQTT_DEVICES_PWD;
static char mqtt_username[128] = "68298f570e587e2293165b50_znyh";
static char mqtt_hostaddr[128] = HOST_ADDR;

static char publish_topic[128] = PUBLISH_TOPIC;
static char subcribe_topic[128] = SUBCRIB_TOPIC;
static char response_topic[128] = RESPONSE_TOPIC;

static unsigned int mqttConnectFlag = 0;

extern bool motor_state;
extern bool light_state;
extern bool auto_state;

// 全局变量定义
double A_1 = 0; // 药盒1使用量
double B_1 = 0; // 药盒2使用量
double C_1 = 0; // 药盒3使用量
double D_1 = 0; // 药盒4使用量

// 新增全局变量定义
double temperature = 0;	 // 温度
double humidity = 0;	 // 湿度
double yh1 = 0;			 // 药盒1剩余量
double yh2 = 0;			 // 药盒2剩余量
double yh3 = 0;			 // 药盒3剩余量
double yh4 = 0;			 // 药盒4剩余量
double illumination = 0; // 光照强度

int eat = 0;  // ESP32发来，RK2206接收
int door = 0; // RK2206发给ESP32

/***************************************************************
 * 函数名称: send_msg_to_mqtt
 * 说    明: 发送信息到iot
 * 参    数: e_iot_data *iot_data：数据
 * 返 回 值: 无
 ***************************************************************/
void send_msg_to_mqtt(e_iot_data *iot_data)
{
	int rc;
	MQTTMessage message;
	char payload[MAX_BUFFER_LENGTH] = {0};
	char str[MAX_STRING_LENGTH] = {0};

	if (mqttConnectFlag == 0)
	{
		printf("mqtt not connect\n");
		return;
	}

	cJSON *root = cJSON_CreateObject();
	if (root != NULL)
	{
		cJSON *serv_arr = cJSON_AddArrayToObject(root, "services");
		cJSON *arr_item = cJSON_CreateObject();
		cJSON_AddStringToObject(arr_item, "service_id", "smartHome");
		cJSON *pro_obj = cJSON_CreateObject();
		cJSON_AddItemToObject(arr_item, "properties", pro_obj);

		memset(str, 0, MAX_BUFFER_LENGTH);
		// 光照强度
		sprintf(str, "%5.2fLux", iot_data->illumination);
		cJSON_AddStringToObject(pro_obj, "illumination", str);
		// cJSON_AddNumberToObject(pro_obj, "illumination", iot_data->illumination);
		// 温度
		sprintf(str, "%5.2f℃", iot_data->temperature);
		cJSON_AddStringToObject(pro_obj, "temperature", str);
		// 湿度
		sprintf(str, "%5.2f%%", iot_data->humidity);
		cJSON_AddStringToObject(pro_obj, "humidity", str);
		// 药盒1
		sprintf(str, "%5.2f", iot_data->yh1);
		cJSON_AddStringToObject(pro_obj, "yh1", str);
		// 药盒2
		sprintf(str, "%5.2f", iot_data->yh2);
		cJSON_AddStringToObject(pro_obj, "yh2", str);
		// 药盒3
		sprintf(str, "%5.2f", iot_data->yh3);
		cJSON_AddStringToObject(pro_obj, "yh3", str);
		// 药盒4
		sprintf(str, "%5.2f", iot_data->yh4);
		cJSON_AddStringToObject(pro_obj, "yh4", str);
		// 药盒1使用状态 (A_1)
		sprintf(str, "%5.2f", A_1);
		cJSON_AddStringToObject(pro_obj, "yh1use", str);
		// 药盒2使用状态 (B_1)
		sprintf(str, "%5.2f", B_1);
		cJSON_AddStringToObject(pro_obj, "yh2use", str);
		// 药盒3使用状态 (C_1)
		sprintf(str, "%5.2f", C_1);
		cJSON_AddStringToObject(pro_obj, "yh3use", str);
		// 药盒4使用状态 (D_1)
		sprintf(str, "%5.2f", D_1);
		cJSON_AddStringToObject(pro_obj, "yh4use", str);
		// 电机状态
		if (iot_data->motor_state == true)
		{
			cJSON_AddStringToObject(pro_obj, "motorStatus", "ON");
		}
		else
		{
			cJSON_AddStringToObject(pro_obj, "motorStatus", "OFF");
		}
		// 灯光状态
		if (iot_data->light_state == true)
		{
			cJSON_AddStringToObject(pro_obj, "lightStatus", "ON");
		}
		else
		{
			cJSON_AddStringToObject(pro_obj, "lightStatus", "OFF");
		}
		// 自动状态模式
		if (iot_data->auto_state == true)
		{
			cJSON_AddStringToObject(pro_obj, "autoStatus", "ON");
		}
		else
		{
			cJSON_AddStringToObject(pro_obj, "autoStatus", "OFF");
		}

		cJSON_AddItemToArray(serv_arr, arr_item);

		char *palyload_str = cJSON_PrintUnformatted(root);
		strcpy(payload, palyload_str);

		cJSON_free(palyload_str);
		cJSON_Delete(root);
	}

	message.qos = 0;
	message.retained = 0;
	message.payload = payload;
	message.payloadlen = strlen(payload);

	sprintf(publish_topic, "$oc/devices/%s/sys/properties/report", mqtt_devid);
	if ((rc = MQTTPublish(&client, publish_topic, &message)) != 0)
	{
		printf("Return code from MQTT publish is %d\n", rc);
		mqttConnectFlag = 0;
	}
	else
	{
		printf("mqtt publish success:%s\n", payload);
	}
}

/***************************************************************
 * 函数名称: set_light_state
 * 说    明: 设置灯状态
 * 参    数: cJSON *root
 * 返 回 值: 无
 ***************************************************************/
void set_light_state(cJSON *root)
{
	cJSON *para_obj = NULL;
	cJSON *status_obj = NULL;
	char *value = NULL;

	event_info_t event = {0};
	event.event = event_iot_cmd;

	para_obj = cJSON_GetObjectItem(root, "paras");
	status_obj = cJSON_GetObjectItem(para_obj, "onoff");
	if (status_obj != NULL)
	{
		value = cJSON_GetStringValue(status_obj);
		printf("Light control command: %s\n", value);
		if (!strcmp(value, "ON"))
		{
			event.data.iot_data = IOT_CMD_LIGHT_ON;
			// light_state = true;
			printf("Setting light ON, sending event: %d\n", IOT_CMD_LIGHT_ON);
		}
		else if (!strcmp(value, "OFF"))
		{
			event.data.iot_data = IOT_CMD_LIGHT_OFF;
			// light_state = false;
			printf("Setting light OFF, sending event: %d\n", IOT_CMD_LIGHT_OFF);
		}
		printf("Sending light control event: %d\n", event.data.iot_data);
		smart_home_event_send(&event);
		printf("Light control event sent successfully\n");
	}
}

/***************************************************************
 * 函数名称: set_motor_state
 * 说    明: 设置电机状态
 * 参    数: cJSON *root
 * 返 回 值: 无
 ***************************************************************/
void set_motor_state(cJSON *root)
{
	cJSON *para_obj = NULL;
	cJSON *status_obj = NULL;
	char *value = NULL;

	event_info_t event = {0};
	event.event = event_iot_cmd;

	para_obj = cJSON_GetObjectItem(root, "paras");
	status_obj = cJSON_GetObjectItem(para_obj, "onoff");
	if (status_obj != NULL)
	{
		value = cJSON_GetStringValue(status_obj);
		printf("Motor control command: %s\n", value);
		if (!strcmp(value, "ON"))
		{
			// motor_state = true;
			event.data.iot_data = IOT_CMD_MOTOR_ON;
			printf("Setting motor ON, sending event: %d\n", IOT_CMD_MOTOR_ON);
		}
		else if (!strcmp(value, "OFF"))
		{
			// motor_state = false;
			event.data.iot_data = IOT_CMD_MOTOR_OFF;
			printf("Setting motor OFF, sending event: %d\n", IOT_CMD_MOTOR_OFF);
		}
		printf("Sending motor control event: %d\n", event.data.iot_data);
		smart_home_event_send(&event);
		printf("Motor control event sent successfully\n");
	}
}

/***************************************************************
 * 函数名称: set_auto_state
 * 说    明: 设置自动模式状态
 * 参    数: cJSON *root
 * 返 回 值: 无
 ***************************************************************/
void set_auto_state(cJSON *root)
{
	cJSON *para_obj = NULL;
	cJSON *status_obj = NULL;
	char *value = NULL;

	para_obj = cJSON_GetObjectItem(root, "paras");
	status_obj = cJSON_GetObjectItem(para_obj, "onoff");
	if (status_obj != NULL)
	{
		value = cJSON_GetStringValue(status_obj);
		if (!strcmp(value, "ON"))
		{
			// auto_state = true;
		}
		else if (!strcmp(value, "OFF"))
		{
			// auto_state = false;
		}
	}
}

/***************************************************************
 * 函数名称: set_medicine_box_update
 * 说    明: 处理药盒更新命令
 * 参    数: cJSON *root
 * 返 回 值: 无
 ***************************************************************/
void set_medicine_box_update(cJSON *root)
{
	cJSON *para_obj = NULL;
	cJSON *item = NULL;

	printf("处理药盒更新命令\n");

	para_obj = cJSON_GetObjectItem(root, "paras");
	if (para_obj != NULL)
	{
		// 检查药盒1使用状态
		item = cJSON_GetObjectItem(para_obj, "yh1use");
		if (item != NULL)
		{
			char *value = cJSON_GetStringValue(item);
			A_1 = atof(value);
			printf("接收到 yh1use: %f\n", A_1);
		}

		// 检查药盒2使用状态
		item = cJSON_GetObjectItem(para_obj, "yh2use");
		if (item != NULL)
		{
			char *value = cJSON_GetStringValue(item);
			B_1 = atof(value);
			printf("接收到 yh2use: %f\n", B_1);
		}

		// 检查药盒3使用状态
		item = cJSON_GetObjectItem(para_obj, "yh3use");
		if (item != NULL)
		{
			char *value = cJSON_GetStringValue(item);
			C_1 = atof(value);
			printf("接收到 yh3use: %f\n", C_1);
		}

		// 检查药盒4使用状态
		item = cJSON_GetObjectItem(para_obj, "yh4use");
		if (item != NULL)
		{
			char *value = cJSON_GetStringValue(item);
			D_1 = atof(value);
			printf("接收到 yh4use: %f\n", D_1);
		}
	}
}

/***************************************************************
 * 函数名称: mqtt_message_arrived
 * 说    明: 接收mqtt数据
 * 参    数: MessageData *data
 * 返 回 值: 无
 ***************************************************************/
void mqtt_message_arrived(MessageData *data)
{
	int rc;
	cJSON *root = NULL;
	cJSON *cmd_name = NULL;
	char *cmd_name_str = NULL;
	char *request_id_idx = NULL;
	char request_id[20] = {0};
	MQTTMessage message;
	char payload[MAX_BUFFER_LENGTH];
	char rsptopic[128] = {0};

	printf("Message arrived on topic %.*s: %.*s\n",
		   data->topicName->lenstring.len, data->topicName->lenstring.data,
		   data->message->payloadlen, data->message->payload);

	// get request id
	request_id_idx = strstr(data->topicName->lenstring.data, "request_id=");
	strncpy(request_id, request_id_idx + 11, 19);
	// printf("request_id = %s\n", request_id);

	// create response topic
	sprintf(response_topic, "$oc/devices/%s/sys/commands/response", mqtt_devid);
	sprintf(rsptopic, "%s/request_id=%s", response_topic, request_id);
	// printf("rsptopic = %s\n", rsptopic);

	// response message
	message.qos = 0;
	message.retained = 0;
	message.payload = payload;
	sprintf(payload, "{ \
    \"result_code\": 0, \
    \"response_name\": \"COMMAND_RESPONSE\", \
    \"paras\": { \
        \"result\": \"success\" \
    } \
    }");
	message.payloadlen = strlen(payload);

	// publish the msg to responese topic
	if ((rc = MQTTPublish(&client, rsptopic, &message)) != 0)
	{
		printf("Return code from MQTT publish is %d\n", rc);
		mqttConnectFlag = 0;
	}

	/*{"command_name":"cmd","paras":{"cmd_value":"1"},"service_id":"server"}*/
	root = cJSON_ParseWithLength(data->message->payload, data->message->payloadlen);
	if (root != NULL)
	{
		cmd_name = cJSON_GetObjectItem(root, "command_name");
		if (cmd_name != NULL)
		{
			cmd_name_str = cJSON_GetStringValue(cmd_name);
			if (!strcmp(cmd_name_str, "light_control"))
			{
				set_light_state(root);
			}
			else if (!strcmp(cmd_name_str, "motor_control"))
			{
				set_motor_state(root);
			}
			else if (!strcmp(cmd_name_str, "auto_control"))
			{
				set_auto_state(root);
			}
			else if (!strcmp(cmd_name_str, "medicine_box_update"))
			{
				set_medicine_box_update(root);
			}
			else
			{
				printf("未知命令: '%s'\n", cmd_name_str);
			}
		}
	}

	cJSON_Delete(root);
}

/***************************************************************
 * 函数名称: wait_message
 * 说    明: 等待信息
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int wait_message()
{
	uint8_t rec = MQTTYield(&client, 5000);
	if (rec != 0)
	{
		printf("MQTTYield error: %d\n", rec);
		mqttConnectFlag = 0;
		return 0;
	}
	if (mqttConnectFlag == 0)
	{
		printf("MQTT connection lost, need to reconnect\n");
		return 0;
	}
	return 1;
}

/***************************************************************
 * 函数名称: mqtt_init
 * 说    明: mqtt初始化
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void mqtt_init()
{
	int rc;

	printf("Starting MQTT...\n");
	printf("Device ID: %s\n", mqtt_devid);
	printf("Username: %s\n", mqtt_username);
	printf("Host: %s\n", mqtt_hostaddr);
	printf("Port: 1883\n");

	/*网络初始化*/
	NetworkInit(&network);

begin:
	/* 连接网络*/
	printf("NetworkConnect  ...\n");
	rc = NetworkConnect(&network, HOST_ADDR, 1883);
	if (rc != 0)
	{
		printf("NetworkConnect failed: %d\n", rc);
		osDelay(200);
		goto begin;
	}
	printf("Network connected successfully\n");

	printf("MQTTClientInit  ...\n");
	/*MQTT客户端初始化*/
	MQTTClientInit(&client, &network, 2000, sendBuf, sizeof(sendBuf), readBuf,
				   sizeof(readBuf));

	MQTTString clientId = MQTTString_initializer;
	clientId.cstring = mqtt_devid;

	MQTTString userName = MQTTString_initializer;
	userName.cstring = mqtt_username;

	MQTTString password = MQTTString_initializer;
	password.cstring = mqtt_pwd;

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.clientID = clientId;
	data.username = userName;
	data.password = password;
	data.willFlag = 0;
	data.MQTTVersion = 4;
	data.keepAliveInterval = 60;
	data.cleansession = 1;

	printf("MQTTConnect  ...\n");
	printf("Client ID: %s\n", mqtt_devid);
	printf("Username: %s\n", mqtt_username);
	printf("Password length: %d\n", strlen(mqtt_pwd));
	printf("Password: %s\n", mqtt_pwd);
	rc = MQTTConnect(&client, &data);
	if (rc != 0)
	{
		printf("MQTTConnect failed: %d\n", rc);
		printf("Error details:\n");
		printf("- rc = -1: Network error\n");
		printf("- rc = 1: Unacceptable protocol version\n");
		printf("- rc = 2: Identifier rejected\n");
		printf("- rc = 3: Server unavailable\n");
		printf("- rc = 4: Bad user name or password\n");
		printf("- rc = 5: Not authorized\n");
		NetworkDisconnect(&network);
		MQTTDisconnect(&client);
		osDelay(200);
		goto begin;
	}
	printf("MQTT connected successfully\n");

	printf("MQTTSubscribe  ...\n");
	sprintf(subcribe_topic, "$oc/devices/68298f570e587e2293165b50_znyh/sys/commands/#");
	printf("Subscribing to topic: %s\n", subcribe_topic);
	rc = MQTTSubscribe(&client, subcribe_topic, 0, mqtt_message_arrived);
	if (rc != 0)
	{
		printf("MQTTSubscribe failed: %d\n", rc);
		osDelay(200);
		goto begin;
	}
	printf("MQTT subscribed successfully to: %s\n", subcribe_topic);

	mqttConnectFlag = 1;
	printf("MQTT initialization completed successfully\n");
}

/***************************************************************
 * 函数名称: mqtt_is_connected
 * 说    明: mqtt连接状态
 * 参    数: 无
 * 返 回 值: unsigned int 状态
 ***************************************************************/
unsigned int mqtt_is_connected() { return mqttConnectFlag; }

/***************************************************************
 * 函数名称: process_udp_data
 * 说    明: 处理UDP数据
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void process_udp_data(void)
{
	static udp_data_t udp_data = {0};

	// 准备发送给ESP32的数据
	udp_data.illumination = illumination;
	udp_data.temperature = temperature;
	udp_data.humidity = humidity;
	udp_data.box1_remain = yh1;
	udp_data.box2_remain = yh2;
	udp_data.box3_remain = yh3;
	udp_data.box4_remain = yh4;

	// 发送数据给ESP32
	udp_send_data(&udp_data);

	// 接收来自ESP32的数据
	if (udp_receive_data(&udp_data) == 0)
	{
		// 更新药盒使用量全局变量，确保不超过剩余量
		extern int A, B, C, D; // 声明外部变量

		// 限制使用量不超过剩余量
		if (udp_data.yh1use > A)
			udp_data.yh1use = A;
		if (udp_data.yh2use > B)
			udp_data.yh2use = B;
		if (udp_data.yh3use > C)
			udp_data.yh3use = C;
		if (udp_data.yh4use > D)
			udp_data.yh4use = D;

		// 更新全局变量
		A_1 = udp_data.yh1use;
		B_1 = udp_data.yh2use;
		C_1 = udp_data.yh3use;
		D_1 = udp_data.yh4use;

		printf("从ESP32接收到药盒使用量数据: %.2f, %.2f, %.2f, %.2f\n",
			   A_1, B_1, C_1, D_1);
	}
}

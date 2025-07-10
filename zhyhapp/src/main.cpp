#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "web_page.h"
#include <time.h>
#include <Preferences.h>

// WiFi配置
const char *ssid = "EICA";
const char *password = "003010413";

// UDP配置
#define UDP_PORT_LOCAL 8888
#define UDP_PORT_REMOTE 6666

WiFiUDP udp;
WebServer server(80);

// 用药历史记录结构
struct MedicineRecord
{
	time_t timestamp;
	int boxId;
	int amount;
	String medicineName;
	bool confirmed;		  // 是否确认已吃药
	bool forgotten;		  // 是否忘记吃药
	String scheduledTime; // 新增：计划服药时间
	String actualTime;	  // 新增：实际服药时间
	String notes;		  // 新增：备注信息
	int status;			  // 新增：状态码 (0:等待确认, 1:已确认, 2:忘记吃药, 3:已删除)
} medicineHistory[100];	  // 最多记录100条历史

int historyCount = 0;

// 数据结构
struct UdpData
{
	float temperature;
	float humidity;
	float illumination;
	int box1_remain;
	int box2_remain;
	int box3_remain;
	int box4_remain;
	int yh1use;
	int yh2use;
	int yh3use;
	int yh4use;
	int door; // 新增
} g_data = {0};

char udpBuffer[512];
IPAddress lastRemoteIp;
unsigned long lastUdpSend = 0;

// 新增：忘记吃药检测相关变量
unsigned long lastEatTimestamp[4] = {0, 0, 0, 0};	   // 记录每个药盒最后发送eat的时间戳
bool waitingForDoor[4] = {false, false, false, false}; // 是否在等待door确认
const unsigned long DOOR_TIMEOUT = 5000;			   // 5秒超时，如果5秒内没收到door=0，则记录为忘记吃药

Preferences prefs;
void sendEatUdp(int boxId);
void handleApiEat();
void recordMedicineHistory(int boxId, int amount);
void handleApiHistory();
void handleApiReport();
void handleApiAdvice();
void sendBoxMedicineAmount(int boxId);
void handleApiConfirm();
void handleApiDelete(); // 新增：删除历史记录API

// 格式化时间
String formatTime(time_t timestamp)
{
	struct tm timeinfo;
	localtime_r(&timestamp, &timeinfo);
	char timeStr[64];
	sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d",
			timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
			timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	return String(timeStr);
}

// 1. parseUdpData 只要收到door=0就立即标记最近的未确认用药为已确认
void parseUdpData(const char *json)
{
	StaticJsonDocument<512> doc;
	DeserializationError err = deserializeJson(doc, json);
	if (err)
		return;

	int oldDoor = g_data.door; // 保存旧的door状态
	g_data.temperature = doc["temperature"] | 0.0;
	g_data.humidity = doc["humidity"] | 0.0;
	g_data.illumination = doc["illumination"] | 0.0;
	g_data.box1_remain = doc["box1_remain"] | 0;
	g_data.box2_remain = doc["box2_remain"] | 0;
	g_data.box3_remain = doc["box3_remain"] | 0;
	g_data.box4_remain = doc["box4_remain"] | 0;
	g_data.door = doc["door"] | 0; // 新增

	// 只要收到door=0就立即标记最近的未确认用药为已确认
	if (g_data.door == 0)
	{
		for (int i = historyCount - 1; i >= 0; i--)
		{
			if (!medicineHistory[i].confirmed && !medicineHistory[i].forgotten)
			{
				medicineHistory[i].confirmed = true;
				medicineHistory[i].status = 1;
				medicineHistory[i].actualTime = formatTime(time(NULL));
				// 保存到Preferences
				String historyKey = "history_" + String(i);
				prefs.putBool((historyKey + "_confirmed").c_str(), true);
				prefs.putInt((historyKey + "_status").c_str(), 1);
				prefs.putString((historyKey + "_actual").c_str(), medicineHistory[i].actualTime);
				break;
			}
		}
	}
	// 不覆盖使用量，使用量只由网页设置
}

// UDP发送数据到RK2206
void sendUdpData(IPAddress remoteIp)
{
	StaticJsonDocument<256> doc;
	doc["yh1use"] = g_data.yh1use;
	doc["yh2use"] = g_data.yh2use;
	doc["yh3use"] = g_data.yh3use;
	doc["yh4use"] = g_data.yh4use;
	char out[128];
	size_t len = serializeJson(doc, out, sizeof(out));
	udp.beginPacket(remoteIp, UDP_PORT_REMOTE);
	udp.write((uint8_t *)out, len);
	udp.endPacket();
}

// 网页API
void handleRoot()
{
	server.send_P(200, "text/html", MAIN_page);
}

void handleApiData()
{
	StaticJsonDocument<256> doc;
	doc["temperature"] = g_data.temperature;
	doc["humidity"] = g_data.humidity;
	doc["illumination"] = g_data.illumination;
	doc["box1_remain"] = g_data.box1_remain;
	doc["box2_remain"] = g_data.box2_remain;
	doc["box3_remain"] = g_data.box3_remain;
	doc["box4_remain"] = g_data.box4_remain;
	doc["door"] = g_data.door; // 新增
	doc["time"] = time(NULL);  // 当前NTP时间戳
	String out;
	serializeJson(doc, out);
	server.send(200, "application/json", out);
}

void handleApiSet()
{
	bool updated = false;
	// 支持POST JSON
	if (server.hasArg("plain") && server.arg("plain").length() > 0)
	{
		StaticJsonDocument<512> doc;
		DeserializationError err = deserializeJson(doc, server.arg("plain"));
		if (!err)
		{
			if (doc.containsKey("yh1use"))
			{
				g_data.yh1use = doc["yh1use"];
				updated = true;
			}
			if (doc.containsKey("yh2use"))
			{
				g_data.yh2use = doc["yh2use"];
				updated = true;
			}
			if (doc.containsKey("yh3use"))
			{
				g_data.yh3use = doc["yh3use"];
				updated = true;
			}
			if (doc.containsKey("yh4use"))
			{
				g_data.yh4use = doc["yh4use"];
				updated = true;
			}
			for (int i = 1; i <= 4; ++i)
			{
				String key = "box" + String(i) + "_times";
				if (doc.containsKey(key))
				{
					JsonArray arr = doc[key].as<JsonArray>();
					String times = "";
					for (JsonVariant v : arr)
					{
						if (times.length())
							times += ",";
						times += String((const char *)v);
					}
					prefs.putString(key.c_str(), times); // 以逗号分隔存储
				}
			}
		}
	}
	// 支持GET参数
	if (server.hasArg("yh1use"))
	{
		g_data.yh1use = server.arg("yh1use").toInt();
		updated = true;
	}
	if (server.hasArg("yh2use"))
	{
		g_data.yh2use = server.arg("yh2use").toInt();
		updated = true;
	}
	if (server.hasArg("yh3use"))
	{
		g_data.yh3use = server.arg("yh3use").toInt();
		updated = true;
	}
	if (server.hasArg("yh4use"))
	{
		g_data.yh4use = server.arg("yh4use").toInt();
		updated = true;
	}

	if (updated)
	{
		prefs.putInt("yh1use", g_data.yh1use);
		prefs.putInt("yh2use", g_data.yh2use);
		prefs.putInt("yh3use", g_data.yh3use);
		prefs.putInt("yh4use", g_data.yh4use);
		server.send(200, "text/plain", "OK");
	}
	else
	{
		server.send(400, "text/plain", "No valid parameter");
	}
}

void handleApiEat()
{
	if (server.method() == HTTP_GET)
	{
		sendEatUdp(1);
		server.send(200, "text/plain", "OK (GET)");
		return;
	}
	if (server.hasArg("plain") && server.arg("plain").length() > 0)
	{
		StaticJsonDocument<64> doc;
		DeserializationError err = deserializeJson(doc, server.arg("plain"));
		if (!err && doc["eat"] == 1)
		{
			sendEatUdp(1);
			server.send(200, "text/plain", "OK");
			return;
		}
	}
	server.send(400, "text/plain", "Invalid request");
}

// 记录用药历史
void recordMedicineHistory(int boxId, int amount)
{
	if (historyCount >= 100)
	{
		// 如果历史记录满了，删除最旧的记录
		for (int i = 0; i < 99; i++)
		{
			medicineHistory[i] = medicineHistory[i + 1];
		}
		historyCount = 99;
	}

	medicineHistory[historyCount].timestamp = time(NULL);
	medicineHistory[historyCount].boxId = boxId;
	medicineHistory[historyCount].amount = amount;
	medicineHistory[historyCount].medicineName = "药盒" + String(boxId) + "药品";
	medicineHistory[historyCount].confirmed = false;				   // 初始未确认
	medicineHistory[historyCount].forgotten = false;				   // 初始未忘记
	medicineHistory[historyCount].scheduledTime = "";				   // 计划时间待设置
	medicineHistory[historyCount].actualTime = formatTime(time(NULL)); // 实际时间
	medicineHistory[historyCount].notes = "";						   // 备注为空
	medicineHistory[historyCount].status = 0;						   // 等待确认状态
	historyCount++;

	// 保存到Preferences
	String historyKey = "history_" + String(historyCount - 1);
	prefs.putLong((historyKey + "_time").c_str(), medicineHistory[historyCount - 1].timestamp);
	prefs.putInt((historyKey + "_box").c_str(), medicineHistory[historyCount - 1].boxId);
	prefs.putInt((historyKey + "_amount").c_str(), medicineHistory[historyCount - 1].amount);
	prefs.putString((historyKey + "_name").c_str(), medicineHistory[historyCount - 1].medicineName);
	prefs.putBool((historyKey + "_confirmed").c_str(), false);
	prefs.putBool((historyKey + "_forgotten").c_str(), false);
	prefs.putString((historyKey + "_scheduled").c_str(), "");
	prefs.putString((historyKey + "_actual").c_str(), medicineHistory[historyCount - 1].actualTime);
	prefs.putString((historyKey + "_notes").c_str(), "");
	prefs.putInt((historyKey + "_status").c_str(), 0);
	prefs.putInt("history_count", historyCount);

	Serial.printf("记录用药历史: 药盒%d, 数量%d, 时间%s\n",
				  boxId, amount, formatTime(medicineHistory[historyCount - 1].timestamp).c_str());
}

// 获取用药历史API
void handleApiHistory()
{
	StaticJsonDocument<2048> doc;
	JsonArray historyArray = doc.createNestedArray("history");

	for (int i = 0; i < historyCount; i++)
	{
		JsonObject record = historyArray.createNestedObject();
		record["timestamp"] = medicineHistory[i].timestamp;
		record["time"] = formatTime(medicineHistory[i].timestamp);
		record["boxId"] = medicineHistory[i].boxId;
		record["amount"] = medicineHistory[i].amount;
		record["medicineName"] = medicineHistory[i].medicineName;
		record["confirmed"] = medicineHistory[i].confirmed;
		record["forgotten"] = medicineHistory[i].forgotten;
		record["scheduledTime"] = medicineHistory[i].scheduledTime;
		record["actualTime"] = medicineHistory[i].actualTime;
		record["notes"] = medicineHistory[i].notes;
		record["status"] = medicineHistory[i].status;
		record["index"] = i; // 添加索引用于删除操作
	}

	doc["count"] = historyCount;

	String out;
	serializeJson(doc, out);
	server.send(200, "application/json", out);
}

// 获取用药报告API
void handleApiReport()
{
	StaticJsonDocument<1024> doc;

	// 统计各药盒使用情况
	int boxUsage[4] = {0, 0, 0, 0};
	int totalUsage = 0;

	for (int i = 0; i < historyCount; i++)
	{
		if (medicineHistory[i].boxId >= 1 && medicineHistory[i].boxId <= 4)
		{
			boxUsage[medicineHistory[i].boxId - 1] += medicineHistory[i].amount;
			totalUsage += medicineHistory[i].amount;
		}
	}

	// 计算最近7天的使用情况
	time_t now = time(NULL);
	time_t weekAgo = now - (7 * 24 * 3600);
	int weeklyUsage = 0;

	for (int i = 0; i < historyCount; i++)
	{
		if (medicineHistory[i].timestamp >= weekAgo)
		{
			weeklyUsage += medicineHistory[i].amount;
		}
	}

	doc["totalUsage"] = totalUsage;
	doc["weeklyUsage"] = weeklyUsage;
	doc["totalRecords"] = historyCount;

	JsonArray boxStats = doc.createNestedArray("boxStats");
	for (int i = 0; i < 4; i++)
	{
		JsonObject box = boxStats.createNestedObject();
		box["boxId"] = i + 1;
		box["usage"] = boxUsage[i];
		box["remain"] = g_data.box1_remain + i; // 简化处理
	}

	String out;
	serializeJson(doc, out);
	server.send(200, "application/json", out);
}

// 获取健康建议API
void handleApiAdvice()
{
	StaticJsonDocument<1024> doc;

	// 分析用药规律
	int morningCount = 0, afternoonCount = 0, eveningCount = 0;
	int regularCount = 0;
	int confirmedCount = 0;
	int forgottenCount = 0;

	for (int i = 0; i < historyCount; i++)
	{
		struct tm timeinfo;
		localtime_r(&medicineHistory[i].timestamp, &timeinfo);
		int hour = timeinfo.tm_hour;

		if (hour >= 6 && hour < 12)
			morningCount++;
		else if (hour >= 12 && hour < 18)
			afternoonCount++;
		else if (hour >= 18 && hour < 22)
			eveningCount++;

		// 检查是否按时服药（简化判断）
		if (hour >= 7 && hour <= 9 || hour >= 12 && hour <= 14 || hour >= 18 && hour <= 20)
		{
			regularCount++;
		}

		// 统计确认和忘记的用药
		if (medicineHistory[i].confirmed)
			confirmedCount++;
		if (medicineHistory[i].forgotten)
			forgottenCount++;
	}

	// 生成建议
	JsonArray adviceArray = doc.createNestedArray("advice");

	if (historyCount > 0)
	{
		float regularity = (float)regularCount / historyCount * 100;
		float compliance = (float)confirmedCount / historyCount * 100;

		if (regularity >= 80)
		{
			adviceArray.add("您的用药规律性很好，继续保持！");
		}
		else if (regularity >= 60)
		{
			adviceArray.add("建议您更严格地按时服药，提高用药规律性。");
		}
		else
		{
			adviceArray.add("请务必按时服药，建议设置手机提醒。");
		}

		// 根据确认率给出建议
		if (compliance >= 90)
		{
			adviceArray.add("您的用药依从性很好，继续保持！");
		}
		else if (compliance >= 70)
		{
			adviceArray.add("建议您提高用药依从性，确保按时取药。");
		}
		else
		{
			adviceArray.add("您的用药依从性较低，建议设置提醒并确保及时取药。");
		}

		// 如果有忘记吃药的记录
		if (forgottenCount > 0)
		{
			adviceArray.add("检测到有忘记吃药的情况，建议设置多重提醒。");
		}

		// 根据环境数据给出建议
		if (g_data.temperature > 30)
		{
			adviceArray.add("当前温度较高，请注意药品储存条件。");
		}
		if (g_data.humidity > 70)
		{
			adviceArray.add("当前湿度较大，请确保药品防潮。");
		}
		if (g_data.illumination > 200)
		{
			adviceArray.add("避免阳光直射药品，建议存放在阴凉处。");
		}

		// 根据剩余量给出建议
		for (int i = 1; i <= 4; i++)
		{
			int remain = 0;
			switch (i)
			{
			case 1:
				remain = g_data.box1_remain;
				break;
			case 2:
				remain = g_data.box2_remain;
				break;
			case 3:
				remain = g_data.box3_remain;
				break;
			case 4:
				remain = g_data.box4_remain;
				break;
			}

			if (remain <= 5)
			{
				adviceArray.add("药盒" + String(i) + "剩余量较少，请及时补充。");
			}
		}
	}
	else
	{
		adviceArray.add("暂无用药记录，建议开始记录您的用药情况。");
	}

	doc["regularity"] = historyCount > 0 ? (float)regularCount / historyCount * 100 : 0;
	doc["compliance"] = historyCount > 0 ? (float)confirmedCount / historyCount * 100 : 0;
	doc["morningCount"] = morningCount;
	doc["afternoonCount"] = afternoonCount;
	doc["eveningCount"] = eveningCount;
	doc["confirmedCount"] = confirmedCount;
	doc["forgottenCount"] = forgottenCount;

	String out;
	serializeJson(doc, out);
	server.send(200, "application/json", out);
}

void setup()
{
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());

	// NTP时间同步 - 增加重试次数和更多NTP服务器
	configTime(8 * 3600, 0, "ntp.aliyun.com", "cn.pool.ntp.org", "pool.ntp.org");
	Serial.print("Waiting for NTP time sync");
	time_t now = 0;
	int retry = 0;
	const int retry_count = 30; // 增加重试次数
	while (now < 8 * 3600 * 2 && ++retry < retry_count)
	{
		delay(1000); // 增加延迟时间
		Serial.print(".");
		now = time(nullptr);
	}
	Serial.println();
	if (now > 8 * 3600 * 2)
	{
		struct tm timeinfo;
		localtime_r(&now, &timeinfo);
		Serial.printf("Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
					  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
					  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	}
	else
	{
		Serial.println("Failed to get NTP time, using default time.");
		// 设置一个默认时间，避免后续逻辑出错
		time_t defaultTime = 8 * 3600 * 2 + 1; // 确保大于检查阈值
											   // 注意：这里无法直接设置系统时间，但可以避免后续检查失败
	}

	udp.begin(UDP_PORT_LOCAL);

	server.on("/", handleRoot);
	server.on("/api/data", handleApiData);
	server.on("/api/set", HTTP_POST, handleApiSet);
	server.on("/api/set", HTTP_GET, handleApiSet);
	server.on("/api/eat", HTTP_POST, handleApiEat);
	server.on("/api/eat", HTTP_GET, handleApiEat);
	server.on("/api/history", HTTP_GET, handleApiHistory);
	server.on("/api/report", HTTP_GET, handleApiReport);
	server.on("/api/advice", HTTP_GET, handleApiAdvice);
	server.on("/api/confirm", HTTP_POST, handleApiConfirm);
	server.on("/api/delete", HTTP_POST, handleApiDelete); // 新增：删除历史记录API
	server.begin();

	prefs.begin("medbox", false); // 命名空间"medbox"
	g_data.yh1use = prefs.getInt("yh1use", 0);
	g_data.yh2use = prefs.getInt("yh2use", 0);
	g_data.yh3use = prefs.getInt("yh3use", 0);
	g_data.yh4use = prefs.getInt("yh4use", 0);

	// 初始化时间设置，避免Preferences错误
	for (int i = 1; i <= 4; ++i)
	{
		String key = "box" + String(i) + "_times";
		// 尝试读取，如果返回空字符串且键不存在，则设置默认值
		String times = prefs.getString(key.c_str(), "");
		if (times.length() == 0)
		{
			prefs.putString(key.c_str(), "08:00"); // 设置默认时间
			Serial.print("Set default time for ");
			Serial.println(key);
		}
	}

	// 加载用药历史记录
	historyCount = prefs.getInt("history_count", 0);
	for (int i = 0; i < historyCount && i < 100; i++)
	{
		String historyKey = "history_" + String(i);
		medicineHistory[i].timestamp = prefs.getLong((historyKey + "_time").c_str(), 0);
		medicineHistory[i].boxId = prefs.getInt((historyKey + "_box").c_str(), 0);
		medicineHistory[i].amount = prefs.getInt((historyKey + "_amount").c_str(), 0);
		medicineHistory[i].medicineName = prefs.getString((historyKey + "_name").c_str(), "未知药品");
		medicineHistory[i].confirmed = prefs.getBool((historyKey + "_confirmed").c_str(), false);									 // 加载确认状态
		medicineHistory[i].forgotten = prefs.getBool((historyKey + "_forgotten").c_str(), false);									 // 加载忘记状态
		medicineHistory[i].scheduledTime = prefs.getString((historyKey + "_scheduled").c_str(), "");								 // 加载计划时间
		medicineHistory[i].actualTime = prefs.getString((historyKey + "_actual").c_str(), formatTime(medicineHistory[i].timestamp)); // 加载实际时间
		medicineHistory[i].notes = prefs.getString((historyKey + "_notes").c_str(), "");											 // 加载备注
		medicineHistory[i].status = prefs.getInt((historyKey + "_status").c_str(), 0);												 // 加载状态
	}
	Serial.printf("加载了%d条用药历史记录\n", historyCount);
}

unsigned long lastCheck = 0;
String lastEatTime[4];

// 新增：自动发送最近15分钟药量的功能
unsigned long lastAutoSend = 0;
const unsigned long AUTO_SEND_INTERVAL = 60000; // 每分钟检查一次

void sendEatUdp(int boxId);

// 计算最近15分钟内应该服用的药量
int calculateRecentMedicineAmount(int boxId)
{
	int totalAmount = 0;
	time_t now = time(NULL);
	time_t fifteenMinutesAgo = now - (15 * 60); // 15分钟前

	// 获取该药盒的服药时间设置
	String key = "box" + String(boxId) + "_times";
	String times = prefs.getString(key.c_str(), "");

	if (times.length() > 0)
	{
		for (int p = 0; p < times.length();)
		{
			int q = times.indexOf(',', p);
			if (q < 0)
				q = times.length();
			String timeStr = times.substring(p, q);

			int colonPos = timeStr.indexOf(':');
			if (colonPos > 0)
			{
				int hour = timeStr.substring(0, colonPos).toInt();
				int minute = timeStr.substring(colonPos + 1).toInt();

				struct tm timeinfo;
				localtime_r(&now, &timeinfo);
				timeinfo.tm_hour = hour;
				timeinfo.tm_min = minute;
				timeinfo.tm_sec = 0;
				time_t scheduledTime = mktime(&timeinfo);

				// 检查是否在最近15分钟内
				if (scheduledTime >= fifteenMinutesAgo && scheduledTime <= now)
				{
					int amount = 0;
					switch (boxId)
					{
					case 1:
						amount = g_data.yh1use;
						break;
					case 2:
						amount = g_data.yh2use;
						break;
					case 3:
						amount = g_data.yh3use;
						break;
					case 4:
						amount = g_data.yh4use;
						break;
					}
					if (amount > 0)
						totalAmount += amount;
				}
			}
			p = q + 1;
		}
	}
	// 如果没有任何时间设置，返回0
	return totalAmount;
}

// sendBoxMedicineAmount: 发送所有药盒的amount字段，未到时间的为0
void sendBoxMedicineAmount(int boxId)
{
	if (!lastRemoteIp)
		return;
	StaticJsonDocument<128> doc;
	int totalAmount = 0;
	for (int i = 1; i <= 4; ++i)
	{
		int amount = calculateRecentMedicineAmount(i);
		doc["box" + String(i) + "_amount"] = amount;
		if (i == boxId)
			totalAmount = amount;
	}
	doc["total_amount"] = totalAmount;
	char out[128];
	size_t len = serializeJson(doc, out, sizeof(out));
	Serial.printf("发送药盒%d的用药量: %d粒\n", boxId, totalAmount);
	udp.beginPacket(lastRemoteIp, UDP_PORT_REMOTE);
	udp.write((uint8_t *)out, len);
	udp.endPacket();
}

// sendAutoMedicineAmount: 发送所有药盒的amount字段，未到时间的为0
void sendAutoMedicineAmount()
{
	if (!lastRemoteIp)
		return;
	StaticJsonDocument<256> doc;
	doc["auto_medicine"] = 1;
	int totalAmount = 0;
	for (int i = 1; i <= 4; ++i)
	{
		int amount = calculateRecentMedicineAmount(i);
		doc["box" + String(i) + "_amount"] = amount;
		totalAmount += amount;
	}
	doc["total_amount"] = totalAmount;
	char out[256];
	size_t len = serializeJson(doc, out, sizeof(out));
	Serial.print("自动发送最近15分钟药量: ");
	Serial.println(out);
	udp.beginPacket(lastRemoteIp, UDP_PORT_REMOTE);
	udp.write((uint8_t *)out, len);
	udp.endPacket();
	Serial.printf("已发送总药量: %d粒\n", totalAmount);
}

// 手动确认用药API
void handleApiConfirm()
{
	if (server.method() != HTTP_POST)
	{
		server.send(405, "text/plain", "Method not allowed");
		return;
	}

	if (server.hasArg("plain") && server.arg("plain").length() > 0)
	{
		StaticJsonDocument<128> doc;
		DeserializationError err = deserializeJson(doc, server.arg("plain"));
		if (!err && doc.containsKey("index"))
		{
			int index = doc["index"];
			if (index >= 0 && index < historyCount)
			{
				// 确认指定的用药记录
				medicineHistory[index].confirmed = true;
				medicineHistory[index].forgotten = false;					// 清除忘记状态
				medicineHistory[index].status = 1;							// 设置为已确认状态
				medicineHistory[index].actualTime = formatTime(time(NULL)); // 更新实际时间

				// 保存到Preferences
				String historyKey = "history_" + String(index);
				prefs.putBool((historyKey + "_confirmed").c_str(), true);
				prefs.putBool((historyKey + "_forgotten").c_str(), false);
				prefs.putInt((historyKey + "_status").c_str(), 1);
				prefs.putString((historyKey + "_actual").c_str(), medicineHistory[index].actualTime);
				prefs.putString((historyKey + "_scheduled").c_str(), medicineHistory[index].scheduledTime);
				prefs.putString((historyKey + "_notes").c_str(), medicineHistory[index].notes);

				Serial.printf("手动确认药盒%d的用药记录（索引%d）\n", medicineHistory[index].boxId, index);

				// 返回成功响应
				StaticJsonDocument<64> response;
				response["success"] = true;
				response["message"] = "确认成功";
				String out;
				serializeJson(response, out);
				server.send(200, "application/json", out);
				return;
			}
		}
	}

	// 返回失败响应
	StaticJsonDocument<64> response;
	response["success"] = false;
	response["message"] = "确认失败";
	String out;
	serializeJson(response, out);
	server.send(400, "application/json", out);
}

// 新增：删除历史记录API
void handleApiDelete()
{
	if (server.method() != HTTP_POST)
	{
		server.send(405, "text/plain", "Method not allowed");
		return;
	}

	if (server.hasArg("plain") && server.arg("plain").length() > 0)
	{
		StaticJsonDocument<128> doc;
		DeserializationError err = deserializeJson(doc, server.arg("plain"));
		if (!err && doc.containsKey("index"))
		{
			int index = doc["index"];
			if (index >= 0 && index < historyCount)
			{
				// 删除指定的用药记录
				Serial.printf("删除药盒%d的用药记录（索引%d）\n", medicineHistory[index].boxId, index);

				// 移动后面的记录向前
				for (int i = index; i < historyCount - 1; i++)
				{
					medicineHistory[i] = medicineHistory[i + 1];
				}
				historyCount--;

				// 重新保存所有记录到Preferences
				for (int i = 0; i < historyCount; i++)
				{
					String historyKey = "history_" + String(i);
					prefs.putLong((historyKey + "_time").c_str(), medicineHistory[i].timestamp);
					prefs.putInt((historyKey + "_box").c_str(), medicineHistory[i].boxId);
					prefs.putInt((historyKey + "_amount").c_str(), medicineHistory[i].amount);
					prefs.putString((historyKey + "_name").c_str(), medicineHistory[i].medicineName);
					prefs.putBool((historyKey + "_confirmed").c_str(), medicineHistory[i].confirmed);
					prefs.putBool((historyKey + "_forgotten").c_str(), medicineHistory[i].forgotten);
					prefs.putString((historyKey + "_scheduled").c_str(), medicineHistory[i].scheduledTime);
					prefs.putString((historyKey + "_actual").c_str(), medicineHistory[i].actualTime);
					prefs.putString((historyKey + "_notes").c_str(), medicineHistory[i].notes);
					prefs.putInt((historyKey + "_status").c_str(), medicineHistory[i].status);
				}

				// 清除多余的Preferences键
				for (int i = historyCount; i < 100; i++)
				{
					String historyKey = "history_" + String(i);
					prefs.remove((historyKey + "_time").c_str());
					prefs.remove((historyKey + "_box").c_str());
					prefs.remove((historyKey + "_amount").c_str());
					prefs.remove((historyKey + "_name").c_str());
					prefs.remove((historyKey + "_confirmed").c_str());
					prefs.remove((historyKey + "_forgotten").c_str());
					prefs.remove((historyKey + "_scheduled").c_str());
					prefs.remove((historyKey + "_actual").c_str());
					prefs.remove((historyKey + "_notes").c_str());
					prefs.remove((historyKey + "_status").c_str());
				}

				prefs.putInt("history_count", historyCount);

				// 返回成功响应
				StaticJsonDocument<64> response;
				response["success"] = true;
				response["message"] = "删除成功";
				String out;
				serializeJson(response, out);
				server.send(200, "application/json", out);
				return;
			}
		}
	}

	// 返回失败响应
	StaticJsonDocument<64> response;
	response["success"] = false;
	response["message"] = "删除失败";
	String out;
	serializeJson(response, out);
	server.send(400, "application/json", out);
}

void loop()
{
	server.handleClient();

	// 接收UDP数据
	int len = udp.parsePacket();
	if (len > 0 && len < sizeof(udpBuffer))
	{
		int n = udp.read(udpBuffer, sizeof(udpBuffer) - 1);
		udpBuffer[n] = 0;
		lastRemoteIp = udp.remoteIP();
		parseUdpData(udpBuffer);
		Serial.print("UDP received from ");
		Serial.print(lastRemoteIp);
		Serial.print(": ");
		Serial.println(udpBuffer);
	}

	// 检查忘记吃药超时
	for (int i = 0; i < 4; i++)
	{
		if (waitingForDoor[i] && (millis() - lastEatTimestamp[i] > 300000)) // 5分钟
		{
			Serial.printf("药盒%d超时未收到door=0确认，标记为忘记吃药\n", i + 1);
			waitingForDoor[i] = false;
			for (int j = historyCount - 1; j >= 0; j--)
			{
				if (medicineHistory[j].boxId == i + 1 && !medicineHistory[j].confirmed && !medicineHistory[j].forgotten)
				{
					medicineHistory[j].forgotten = true;
					medicineHistory[j].status = 2;
					String historyKey = "history_" + String(j);
					prefs.putBool((historyKey + "_forgotten").c_str(), true);
					prefs.putInt((historyKey + "_status").c_str(), 2);
					break;
				}
			}
		}
	}

	// 定时向RK2206发送药盒使用量
	if (lastRemoteIp && millis() - lastUdpSend > 1000)
	{
		sendUdpData(lastRemoteIp);
		lastUdpSend = millis();
	}

	// 自动发送最近15分钟的药量（每分钟检查一次）
	if (millis() - lastAutoSend > AUTO_SEND_INTERVAL)
	{
		lastAutoSend = millis();
		sendAutoMedicineAmount();
	}

	// 每分钟检查一次
	if (millis() - lastCheck > 1000 * 10)
	{ // 10秒测试，实际可用60秒
		lastCheck = millis();
		time_t now = time(NULL);
		if (now < 8 * 3600 * 2) // 如果时间同步失败，跳过时间检查
		{
			Serial.println("Time not synced, skipping time check");
			return;
		}

		struct tm t;
		localtime_r(&now, &t);
		char hhmm[6];
		sprintf(hhmm, "%02d:%02d", t.tm_hour, t.tm_min);
		Serial.print("Current time: ");
		Serial.println(hhmm);

		for (int i = 1; i <= 4; ++i)
		{
			String key = "box" + String(i) + "_times";
			String times = prefs.getString(key.c_str(), "08:00"); // 提供默认值
			if (times.length() > 0)
			{
				int idx = 0;
				for (int p = 0; p < times.length();)
				{
					int q = times.indexOf(',', p);
					if (q < 0)
						q = times.length();
					String tstr = times.substring(p, q);

					// 解析时间
					int colonPos = tstr.indexOf(':');
					if (colonPos > 0)
					{
						int hour = tstr.substring(0, colonPos).toInt();
						int minute = tstr.substring(colonPos + 1).toInt();

						// 计算15分钟前的时间
						time_t scheduledTime = now;
						struct tm scheduledTm = t;
						scheduledTm.tm_hour = hour;
						scheduledTm.tm_min = minute;
						scheduledTm.tm_sec = 0;
						scheduledTime = mktime(&scheduledTm);

						time_t fifteenMinutesBefore = scheduledTime - (15 * 60);

						// 检查是否在15分钟前的时间点（发送用药量）
						char fifteenMinBeforeStr[6];
						struct tm fifteenMinBeforeTm;
						localtime_r(&fifteenMinutesBefore, &fifteenMinBeforeTm);
						sprintf(fifteenMinBeforeStr, "%02d:%02d", fifteenMinBeforeTm.tm_hour, fifteenMinBeforeTm.tm_min);

						if (strcmp(hhmm, fifteenMinBeforeStr) == 0)
						{
							// 提前15分钟发送用药量
							Serial.printf("提前15分钟发送药盒%d的用药量\n", i);
							sendBoxMedicineAmount(i);
						}

						// 检查是否在准确的时间点（发送eat命令）
						if (strcmp(hhmm, tstr.c_str()) == 0 && lastEatTime[i - 1] != tstr)
						{
							// 触发出药
							Serial.print("Time match! Triggering eat for box ");
							Serial.println(i);
							sendEatUdp(i); // 发送UDP
							lastEatTime[i - 1] = tstr;
						}
					}
					p = q + 1;
				}
			}
		}
	}
}

// UDP发送eat=1到RK2206
void sendEatUdp(int boxId)
{
	if (!lastRemoteIp)
	{
		Serial.println("No remote IP, cannot send eat UDP");
		return;
	}
	StaticJsonDocument<64> doc;
	// 修改为RK2206期望的格式，只发送eat字段
	doc["eat"] = 1;
	char out[64];
	size_t len = serializeJson(doc, out, sizeof(out));
	Serial.print("UDP send eat: ");
	Serial.println(out); // 打印实际发送内容
	udp.beginPacket(lastRemoteIp, UDP_PORT_REMOTE);
	udp.write((uint8_t *)out, len);
	udp.endPacket();
	Serial.print("Eat UDP sent to ");
	Serial.println(lastRemoteIp);

	// 设置等待door确认
	waitingForDoor[boxId - 1] = true;
	lastEatTimestamp[boxId - 1] = millis();
	Serial.printf("等待药盒%d的door=0确认信号\n", boxId);

	// 记录用药历史 - 使用实际配药数量
	int amount = 1; // 默认1粒
	switch (boxId)
	{
	case 1:
		amount = g_data.yh1use;
		break;
	case 2:
		amount = g_data.yh2use;
		break;
	case 3:
		amount = g_data.yh3use;
		break;
	case 4:
		amount = g_data.yh4use;
		break;
	}
	if (amount <= 0)
		amount = 1; // 如果配药数量为0，默认1粒

	recordMedicineHistory(boxId, amount);
}
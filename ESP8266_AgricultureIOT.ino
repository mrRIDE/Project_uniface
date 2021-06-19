/*
 Name:		ESP8266_AgricultureIOT.ino
 Created:	2021-06-15 8:54:36 PM
 Author:	MrRide
*/
/*
--------------------プロジェクトの概略内容 (Begin)------------------
--テーマ：イチゴファーム管理システム
--要求：
-温度[17-20*C]以内を制御
-湿度[80-90%]以内を制御
--インタネットで監視・制御できること

※システム分析
--使用Hardware
-Board Control, NetworkComunicate: NODE MCU ESP8266
-Display: OLED 1.25inch
-Sensor: DHT, moil
-Load Device: Pump, Fan, Light
-Software: C, C++

--Server
-Cloud Azure
-Webserver: NodeJS, javascript, html, css
--------------------プロジェクトの概略内容 (End)--------------------
*/

/*-------------------------------------------------------------*/
/* Include Library */
#include <SSD1306.h>
#include <Wire.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

/*-------------------------------------------------------------*/
/* Define IO Port */
#define OLED_SCL	9			//GPIO 9
#define OLED_SDA	10			//GPIO 10
#define OLED_ID		0x3C		//I2c Address

#define DHT_OUT		12			//GPIO 12
#define DHT_TYPE	DHT11

/*-------------------------------------------------------------*/
/* Config Device */
SSD1306 displayOLED(OLED_ID, OLED_SDA, OLED_SCL);
DHT sensorDHT(DHT_OUT, DHT_TYPE);

Ticker ticker10ms;
Ticker ticker100ms;
/* Global Variable Declare */
struct sensor_dht_data
{
	float humd;
	float temp;
} DHTDATA;

/*-------------------------------------------------------------*/
/* Function Prototype */
void systemInit();
void variableInit();
void task10ms_Schedule();
void task100ms_Schedule();

void readSensorDHT();
void displayTempHumd();
void blinkLED();

/*-------------------------------------------------------------*/
/* Task List */
void task10ms_Schedule()
{
	//wait for read 
	//Serial.printf("task10ms: %d \n", millis());

}

void task100ms_Schedule()
{
	//Serial.printf("task100ms: %d \n", millis());
	blinkLED();
	readSensorDHT();
	displayTempHumd();

}

/*-------------------------------------------------------------*/
// the setup function runs once when you press reset or power the board
void setup() {
	systemInit();
	variableInit();
	ticker10ms.attach_ms_scheduled(10, task10ms_Schedule);
	ticker100ms.attach_ms_scheduled(100, task100ms_Schedule);
}

// the loop function runs over and over again until power down or reset
void loop() {
	//Use Ticker 10ms. 100ms for control OS
	
}

/*-------------------------------------------------------------*/
void variableInit()
{
	DHTDATA.humd = 0;
	DHTDATA.temp = 0;

}

void displayTempHumd()
{
	displayOLED.clear();
	displayOLED.drawString(0, 5, "Farm System");
	displayOLED.drawString(0, 20, "Temp: " + String(DHTDATA.temp, 1) + " *C");
	displayOLED.drawString(0, 35, "Humd: " + String(DHTDATA.humd, 1) + " %");
	displayOLED.display();
	
}

void systemInit()
{
	// Init Serial for debug
	Serial.begin(115200);
	Serial.println("Init Serial");

	// Init Port IO
	pinMode(LED_BUILTIN, OUTPUT);
	delay(50);
	digitalWrite(LED_BUILTIN, HIGH);		//Set OFF
	Serial.println("Init IO port");

	// Init OLED
	displayOLED.init();
	delay(50);
	displayOLED.flipScreenVertically();
	displayOLED.setFont(ArialMT_Plain_16);
	displayOLED.drawString(0, 5, "_WELCOME_");
	displayOLED.drawString(0, 20, "_Farm_");
	displayOLED.drawString(0, 35, "_System_");
	displayOLED.display();
	Serial.println("Init OLED");

	// Init Sensor DHT
	sensorDHT.begin();
	delay(50);
	readSensorDHT();
	Serial.println("Init Sensor DHT");
	Serial.printf("Frist read: Temp: %.1f(*C) - Humd: %.1f(%%)", DHTDATA.temp, DHTDATA.humd);

	delay(2000);

}

void readSensorDHT()
{
	float temp_t;
	float humd_t;
	
	temp_t = sensorDHT.readTemperature();
	humd_t = sensorDHT.readHumidity();

	if (isnan(temp_t))
	{
		Serial.println("Failled to read Temp");
	}
	else
	{
		DHTDATA.temp = temp_t;
	}

	if (isnan(humd_t))
	{
		Serial.println("Failed to read Humd");
	}
	else
	{
		DHTDATA.humd = humd_t;
	}
}

void blinkLED()
{
	digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
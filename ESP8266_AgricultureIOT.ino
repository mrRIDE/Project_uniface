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
#define OLED_SCL	9			//GPIO9
#define OLED_SDA	10			//GPIO10
#define OLED_ID		0x3C		//I2c Address

#define DHT_PIN		12			//GPIO12
#define DHT_TYPE	DHT11

#define CTR_PUMP	5			//GPIO5
#define CTR_LIGHT	0			//GPIO0
#define CTR_BUTTON	4			//GPIO4

#define CTR_MODE_AUTO		0
#define CTR_MODE_MANU_PUMP	1
#define CTR_MODE_MANU_LIGH	2

#define CTR_TEMP_MIN	17
#define CTR_TEMP_MAX	20
#define CTR_HUMD_MIN	70
#define CTR_HUMD_MAX	80

#define ON				1
#define OFF				0

/*-------------------------------------------------------------*/
/* Config Device */
SSD1306 displayOLED(OLED_ID, OLED_SDA, OLED_SCL);
DHT sensorDHT(DHT_PIN, DHT_TYPE);

Ticker ticker10ms;
Ticker ticker100ms;
/* Global Variable Declare */
struct sensor_dht_data
{
	float humd;
	float temp;
} DHTDATA;

struct control_mode
{
	uint8_t ctrMode;			//0: Auto, 1: Manual
	uint8_t pumpSts;
	uint8_t lightSts;
} CTRLSYSTEM;
/*-------------------------------------------------------------*/
/* Function Prototype */
void systemInit();
void variableInit();
void task10ms_Schedule();
void task100ms_Schedule();

void readSensorDHT();
void displaySystemInfor();
void blinkLED();
bool isButtonPress_3000ms();
bool isButtonPress_150ms();
void controlAutoMode();
void controlManualMode(uint8_t manu_mode);
void checkChangeMode();
void selectControlMode();

/*-------------------------------------------------------------*/
/* Task List */
void task10ms_Schedule()
{
	//Serial.printf("task10ms: %d \n", millis());
	mainControl();

}

void task100ms_Schedule()
{
	//Serial.printf("task100ms: %d \n", millis());
	blinkLED();
	readSensorDHT();
	displaySystemInfor();

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
void mainControl()
{
	checkChangeMode();
	selectControlMode();

}

void selectControlMode()
{
	switch (CTRLSYSTEM.ctrMode)
	{
	case CTR_MODE_AUTO:
		controlAutoMode();
		break;
	case CTR_MODE_MANU_PUMP:
		controlManualMode(CTR_MODE_MANU_PUMP);
		break;
	case CTR_MODE_MANU_LIGH:
		controlManualMode(CTR_MODE_MANU_LIGH);
		break;
	default:
		break;
	}
}

void checkChangeMode()
{
	if (isButtonPress_3000ms())
	{
		switch (CTRLSYSTEM.ctrMode)
		{
		case CTR_MODE_AUTO:
			CTRLSYSTEM.ctrMode = CTR_MODE_MANU_PUMP;
			break;
		case CTR_MODE_MANU_PUMP:
			CTRLSYSTEM.ctrMode = CTR_MODE_MANU_LIGH;
			break;
		case CTR_MODE_MANU_LIGH:
			CTRLSYSTEM.ctrMode = CTR_MODE_AUTO;	//return auto
			break;
		default:
			break;
		}
	}

}

void controlAutoMode()
{
	if (DHTDATA.temp < CTR_TEMP_MIN)
	{
		digitalWrite(CTR_LIGHT, LOW);		//turn on light
		CTRLSYSTEM.lightSts = digitalRead(CTR_LIGHT);
	}
	else if (DHTDATA.temp > CTR_LIGHT)
	{
		digitalWrite(CTR_PUMP, HIGH);		//turn off light
		CTRLSYSTEM.lightSts = digitalRead(CTR_LIGHT);
	}

	if (DHTDATA.humd < CTR_HUMD_MIN)
	{
		digitalWrite(CTR_PUMP, LOW);		//turn on pump
		CTRLSYSTEM.pumpSts = digitalRead(CTR_PUMP);
	}
	else if (DHTDATA.humd > CTR_PUMP)
	{
		digitalWrite(CTR_PUMP, HIGH);		//turn off pump
		CTRLSYSTEM.pumpSts = digitalRead(CTR_PUMP);
	}

}

void controlManualMode(uint8_t manu_mode)
{
	if (isButtonPress_150ms())
	{
		switch (manu_mode)
		{
		case CTR_MODE_MANU_PUMP:
			digitalWrite(CTR_PUMP, !digitalRead(CTR_PUMP));
			CTRLSYSTEM.pumpSts = digitalRead(CTR_PUMP);
			break;
		case CTR_MODE_MANU_LIGH:
			digitalWrite(CTR_LIGHT, !digitalRead(CTR_LIGHT));
			CTRLSYSTEM.lightSts = digitalRead(CTR_LIGHT);
			break;
		default:
			break;
		}
	}
}

bool isButtonPress_150ms()
{
	static uint16_t pressCnt = 0;

	//Serial.printf("press100ms: %d, mili: %d \n", pressCnt, millis());
	if (digitalRead(CTR_BUTTON) == 0)
	{
		pressCnt++;
		//press time 15*os10ms=150ms
		if (pressCnt > 15)
		{
			pressCnt = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		pressCnt = 0;
		return false;
	}

}

bool isButtonPress_3000ms()
{
	static uint16_t pressCnt = 0;

	//Serial.printf("press3000ms: %d, mili: %d \n", pressCnt, millis());
	if (digitalRead(CTR_BUTTON) == 0)
	{
		pressCnt++;
		//press time 300*os10ms=3000ms
		if (pressCnt > 300)
		{
			pressCnt = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		pressCnt = 0;
		return false;
	}

}

void displaySystemInfor()
{
	displayOLED.clear();
	displayOLED.setFont(ArialMT_Plain_10);
	displayOLED.drawString(0, 0, "__FARM SYSTEM__");
	displayOLED.drawString(0, 10, "Mode: " + returnStrCtrMode(CTRLSYSTEM.ctrMode));
	displayOLED.drawString(0, 20, "Temp:" + String(DHTDATA.temp, 1) + "*C");
	displayOLED.drawString(0, 30, "Humd:" + String(DHTDATA.humd, 1) + "%");
	displayOLED.drawString(0, 40, "PumpSts: " + String(!CTRLSYSTEM.pumpSts) + "LighSts: " + String(!CTRLSYSTEM.lightSts));
	displayOLED.display();

}

String returnStrCtrMode(uint8_t control_mode)
{
	String rteString;

	switch (control_mode)
	{
	case CTR_MODE_AUTO:
		rteString = "Auto";
		break;
	case CTR_MODE_MANU_LIGH:
		rteString = "Manu-Light";
		break;
	case CTR_MODE_MANU_PUMP:
		rteString = "Manu-Pump";
		break;
	default:
		break;
	}

	return rteString;
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

void variableInit()
{
	DHTDATA.humd = 0;
	DHTDATA.temp = 0;

	CTRLSYSTEM.ctrMode = CTR_MODE_AUTO;
}

void systemInit()
{
	// Init Serial for debug
	Serial.begin(115200);
	Serial.println("Init Serial");

	// Init Port IO
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(CTR_PUMP, OUTPUT);
	pinMode(CTR_LIGHT, OUTPUT);
	pinMode(CTR_BUTTON, INPUT);

	digitalWrite(LED_BUILTIN, HIGH);		//Set OFF
	digitalWrite(CTR_LIGHT, HIGH);
	digitalWrite(CTR_PUMP, HIGH);
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
#include <Arduino.h>

#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "ESP8266WebServer.h"
#include <DNSServer.h>
#include <FSEWifiManager.h>
#include <NTPClient.h>

#include <FSEDHT.h>
//#include "FSEThingSpeak.h"
const long utcOffsetInSeconds = -18000; // -4 hours
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 2, /* data=*/ 0, /* reset=*/ U8X8_PIN_NONE);

FSEDHT dht(3);

#define IOT_URL_PARAM_NAME "iot_thingspeak_url"
#define IOT_API_KEY_PARAM_NAME "iot_api_Key"
#define IOT_TEMP_PARAM_NAME "field1" // humidity
#define IOT_HUM_PARAM_NAME "field2" // temperarure

#define MYSERIAL

FSEWifiManager myWifiManager;
void writeOled(float number);
void writeOled(String text);
void waitingWifiConfig(WiFiManager *wifiManager);
void wifiFailedMessage();
void readDHT();
void writeDataToOled();
void setData(FSEDHT *dht);
void error(FSEDHT_error_t error);
volatile float humidity=0.0, temperature=0.0;
void saveConfig();

void setup() {
	//reseting wifi
	myWifiManager.resetSettings();
#ifdef MYSERIAL
	Serial.begin(115200);
	Serial.setTimeout(2000);
	// Wait for serial to initialize.
	while(!Serial) { }

#endif
	pinMode(3, FUNCTION_3); // setting RX pin to FUNCTION_3
	u8g2.begin();

	u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
	writeOled("Connecting Wifi...");
	myWifiManager.addParameter(IOT_URL_PARAM_NAME, "Iot Server URL", "https://api.thingspeak.com");
	myWifiManager.addParameter(IOT_API_KEY_PARAM_NAME, "API Key", "");

	myWifiManager.setAPCallback(waitingWifiConfig);
	myWifiManager.setSaveConfigCallback(saveConfig);
	myWifiManager.begin("FSEWEATHER_REC");

	if (WiFi.status() != WL_CONNECTED) {
		wifiFailedMessage();
	}

	dht.afterRead(setData);
	dht.onError(error);
}

void saveConfig() {
	Serial.println("If we wish to do something after saving");
}

void error(FSEDHT_error_t error) {
	Serial.println("Error");
	Serial.println(error.errorNum);
	Serial.println(error.errorMsg);
}

void setData(FSEDHT *dht) {
	humidity = dht->getHumidity();
	temperature = dht->toCelsius();
	writeDataToOled();
}

String getTime() {
	//only will get next time if passed more than 30 secs
	static unsigned long last_update = 50000;
	static String lastTime;

	if (millis() - last_update > 30000) {
		timeClient.update();
		char buff[5];
		sprintf(buff, "%.2d:%.2d", timeClient.getHours(),timeClient.getMinutes());
		lastTime = buff;
	}
	return lastTime;
}

void readDHT() {
	dht.read();
}

String toString(float f) {
	char buff[100];
	dtostrf(f, 2, 2, buff);
	return buff;
}
void writeDataToOled() {
	u8g2.clearBuffer();
	u8g2.drawStr(100,10,getTime().c_str());
	u8g2.drawStr(0,30,"T:");
	u8g2.drawUTF8(40, 30, toString(temperature).c_str());
	u8g2.drawStr(0,50,"H:");
	u8g2.drawStr(40,50, toString(humidity).c_str());
	u8g2.sendBuffer();
}
void wifiFailedMessage() {
	for (int i = 5; i > 0; i--) {
		u8g2.clearBuffer();
		u8g2.drawStr(0,10,"WIFI Failed");
		u8g2.drawStr(0,30,"Retrying in:");
		char buff[100];
		dtostrf(i, 2, 2, buff);
		u8g2.drawStr(0,50,buff);
		u8g2.sendBuffer();
		delay(1000);
	}
	ESP.reset();
}

void waitingWifiConfig(WiFiManager *myWiFiManager) {
	u8g2.clearBuffer();
	u8g2.drawStr(0,30,"Connect to SSID:");
	u8g2.drawStr(0,50,myWiFiManager->getConfigPortalSSID().c_str());
	u8g2.sendBuffer();
}

void loop() {
	readDHT();
	writeDataToOled();
	delay(10000);
}
void writeOled(String text, int x, int y){
	u8g2.clearBuffer();          // clear the internal memory
	u8g2.drawStr(x,y,text.c_str());  // write something to the internal memory
	u8g2.sendBuffer();
}


void writeOled(String text){
	u8g2.clearBuffer();          // clear the internal memory
	u8g2.drawStr(0,10,text.c_str());  // write something to the internal memory
	u8g2.sendBuffer();
}

void writeOled(float number){
	char buff[100];
	dtostrf(number, 2, 2, buff);
	u8g2.clearBuffer();          // clear the internal memory
	u8g2.drawStr(0,10,buff);  // write something to the internal memory
	u8g2.sendBuffer();
}

/**
 * Example: SMAReader_Demo.ino
 *
 */


#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#endif


#include <ArduinoJson.h>

#include <SMAReader.h>

// Fill in appropriate values
#define WLANSSID "..."
#define WLANPWD "..."
#define INVERTERPWD "..."
IPAddress inverterIP(192,168,0,100);

#define DEBUG_LEVEL 3
#define DEBUG_ERROR 1
#define DEBUG_MIN_INFO 2
#define DEBUG_MAX_INFO 3
#define DEBUG_OUT(level, fmt, ...) if(DEBUG_LEVEL>=level) Serial.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ )

SMAReader smaReader(inverterIP, SMAREADER_USER, INVERTERPWD, 5); 


void setup() {
  Serial.begin(9600);
  connectWifi(true);
}

int connectWifi(bool firstConnect) {
  unsigned long startwifi=micros();
  unsigned int retryCount = 0;

  if(!firstConnect) {
    WiFi.reconnect();
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WLANSSID, WLANPWD); // Start WiFI
  }

  DEBUG_OUT(DEBUG_MIN_INFO, "%sonnecting to %s\n", firstConnect?"C":"Rec", WLANSSID);

  while ((WiFi.status() != WL_CONNECTED) && (retryCount < 40)) {
    delay(500);
    DEBUG_OUT(DEBUG_MIN_INFO, ".");
    retryCount++;
  }
  int success=WiFi.status();
  if(success==WL_CONNECTED) {
    DEBUG_OUT(DEBUG_MIN_INFO, "WiFi connected\nIP address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    DEBUG_OUT(DEBUG_ERROR, "Failed to connect\n");
  }
  DEBUG_OUT(DEBUG_MIN_INFO, "Number of tries: %d\n", retryCount);
  DEBUG_OUT(DEBUG_MIN_INFO, "Connecting time (microseconds): %lu\n", micros()-startwifi);

  return success;
}






void loop() {
    // wait for WiFi connection
    if(WiFi.status() == WL_CONNECTED) {
      // getValues int example
      String keys[2]={KEY_POWER, KEY_ENERGY_TODAY};
      int values[2];
      bool isSuccess=smaReader.getValues(2, keys, values);
      Serial.printf("Getting values: %s\n", isSuccess?"success":"fail");
      if(isSuccess) {
         for(int val: values) {
            Serial.printf("Value: %d\n", val);
         }
      }
      // getValues String example
      String keys_string[2]={KEY_WLAN_IP, KEY_WLAN_DNS_IP};
      String values_string[2];
      isSuccess=smaReader.getValues(2, keys_string, values_string);
      Serial.printf("Getting values: %s\n", isSuccess?"success":"fail");
      if(isSuccess) {
         for(String& val: values_string) {
            Serial.printf("Value: %s\n", val.c_str());
         }
      }
      // getLog example
      tm startTime;
      strptime("2020-10-19 16:00:00", "%Y-%m-%d %T", &startTime);
      uint32_t startTimestamp = mktime(&startTime);
      Serial.printf("Timestamp: %lu\n", startTimestamp);

      uint32_t values_log[100];
      uint32_t timestamps[100];
      int numValues=smaReader.getLog(startTimestamp, startTimestamp+1000, values_log, timestamps);
      Serial.printf("Logger: %s\n", numValues==-1?"fail":"success");
      for(int i=0;i<numValues;i++) {
        Serial.printf("Timestamp: %sTotal energy production: %lu Wh\n", asctime(localtime((time_t *)&timestamps[i])), values_log[i]);
      }
      //Serial.printf("All values, succes: %d\n", smaReader.getAllValues());
    }

    
    delay(10000);
}

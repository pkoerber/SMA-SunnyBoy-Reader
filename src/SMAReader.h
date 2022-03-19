/*
  SMAReader.h - Library for reading data from an SMA SunnyBoy Inverter.
  Info on SMA SunnyBoy API based on https://github.com/Dymerz/SMA-SunnyBoy (Python)
  and https://github.com/martijndierckx/sunnyboy-influxdb (Javascript)
*/

#ifndef SMAReader_h
#define SMAReader_h

//#define DEBUG_SMAREADER_ON 1
#define DEBUG_SMAREADER_PORT Serial

#include "Arduino.h"

#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#endif

#ifdef ESP32
#include <HTTPClient.h>
#endif

#include <ArduinoJson.h>
#include <StreamUtils.h>


#if defined(DEBUG_SMAREADER_ON) && defined(DEBUG_SMAREADER_PORT)
#define DEBUG_SMAREADER(fmt, ...) DEBUG_SMAREADER_PORT.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ )
#endif

#ifndef DEBUG_SMAREADER
#define DEBUG_SMAREADER(...) do { (void)0; } while (0)
#endif

#define SMAREADER_USER "usr"
#define SMAREADER_INSTALLER "istl"

#define KEY_POWER "6100_40263F00" // W
#define KEY_ENERGY_TODAY "6400_00262200" // Wh
#define KEY_ENERGY_TOTAL "6400_00260100" // Wh

/* AC values */
#define KEY_AC_L1_POWER "6100_40464000" // W (only one if only one phase)
#define KEY_AC_L1_VOLTAGE "6100_00464800" // 1e-2 V (only one if only one phase)
#define KEY_AC_L1_CURRENT "6100_40465300" // mA (only one if only one phase)
#define KEY_AC_L2_POWER "6100_40464100" // W (only if multiple phases)
#define KEY_AC_L2_VOLTAGE "6100_00464900" // 1e-2 V (only if multiple phases)
#define KEY_AC_L2_CURRENT "6100_40465400" // mA (only if multiple phases)
#define KEY_AC_L3_POWER "6100_40464200" // W (only if multiple phases)
#define KEY_AC_L3_VOLTAGE "6100_00464A00" // 1e-2 V (only if multiple phases)
#define KEY_AC_L3_CURRENT "6100_40465500" // mA (only if multiple phases)
#define KEY_AC_L1L2_VOLTAGE "6100_00464B00" // 1e-2 V (only if multiple phases)
#define KEY_AC_L2L3_VOLTAGE "6100_00464C00" // 1e-2 V (only if multiple phases)
#define KEY_AC_L3L1_VOLTAGE "6100_00464D00" // 1e-2 V (only if multiple phases)
#define KEY_AC_FREQUENCY "6100_00465700" // 1e-2 Hz 

/* DC values */
#define KEY_DC_POWER "6380_40251E00" // W
#define KEY_DC_VOLTAGE "6380_40451F00" // 1e-2 V
#define KEY_DC_CURRENT "6380_40452100" // mA

#define KEY_OPERATING_TIME "6400_00462E00" // s
#define KEY_FEED_IN_TIME "6400_00462F00" // s

/* Device info */
#define KEY_ETHERNET_IP "6180_104A9A00" // String
#define KEY_ETHERNET_DNS_IP "6180_104A9D00" // String
#define KEY_ETHERNET_NETMASK "6180_104A9B00" // String
#define KEY_ETHERNET_GATEWAY_IP "6180_104A9C00" // String

#define KEY_WLAN_IP "6180_104AB700" // String
#define KEY_WLAN_DNS_IP "6180_104ABA00" // String
#define KEY_WLAN_NETMASK "6180_104AB800" // String
#define KEY_WLAN_GATEWAY_IP "6180_104AB900" // String
#define KEY_WLAN_STRENGTH "6100_004AB600" // percentage

#define KEY_DEVICE_WARNING "6100_00411F00"
#define KEY_DEVICE_ERROR "6100_00412000"
#define KEY_DEVICE_OK "6100_00411E00" // Gives nominal power (W) if ok

class SMAReader {
  public:
  
     /* Create an SMAReader object,
      *  inverterAddress: IP address of the inverter, e.g. 192.168.0.xxx
      *  accountType: one of the predefined values SMAREADER_USER or SMAREADER_INSTALLER
      *  passwd: password corresponding to the accountType (USER or INSTALLER) 
      *  numTries: how many times to try to connect: often does not work from the first time
      */
     SMAReader(IPAddress inverterAddress, const char* accountType, const char* passwd, byte numTries=5) { 
        _inverterAddress=inverterAddress; 
        _accountType=accountType;
        _passwd=passwd;
        _numTries=numTries;
     }

     void setNumTries(byte numTries) { _numTries=numTries; }

     /* Get the values corresponding to the keys
      * Does not support keys with String values: they get -1
      * keys: String array specifying which values to get, use the predefined KEY values in this header like KEY_POWER_CURRENT 
      * values: int array to store the resulting values
      * returns: true if success (after the specified number of tries) else false
      * Note: if the operation succeeded, but there is an error in getting one of the keys, it gets value -1 
      */
     bool getValues(int numKeys, const String* keys, int* values);
     
     /* Get the values corresponding to the keys
      * Same as the previous, but supports String values
      * keys: String array specifying which values to get, use the predefined KEY values in this header like KEY_POWER_CURRENT 
      * values: String array to store the resulting values, everything value is converted to a String
      * returns: true if success (after the specified number of tries) else false
      * Note: if the operation succeeded, but there is an error in getting one of the keys, it gets value "" 
      */
     bool getValues(int numKeys, const String* keys, String* values);
     
     /* Get the log of total energy production since start (in Wh) between certain times with intervals of 5 min
      * startTime: start time of the interval as unix timestamp (seconds after 01/01/1970, midnight UTC) 
      * endTime: end time of the interval as unix timestamp  
      * values: uint32_t array to store the values
      * timestamps: unint32_t array to store the timestamps of the values, if not needed supply null pointer
      * returns: number of values if success, otherwise -1
      */
     int getLog(uint32_t startTime, uint32_t endTime, uint32_t* values, uint32_t* timestamps=nullptr);
     bool getAllValues();
     void setInverterIP(IPAddress inverterAddress);
     
  private:
     IPAddress _inverterAddress;
     const char* _passwd;
     const char* _accountType;
     String _sid="";
     byte _numTries;

     HTTPClient http;
     bool getValuesAux(int numKeys, const String* keys, DynamicJsonDocument& doc, JsonVariant& result);
     bool postSMA(const char* postURL, const char* postMessage, DynamicJsonDocument& jsonResult);
     bool authorize();
     bool logout();
     

     
};


#endif

#include "SMAReader.h"

void SMAReader::setInverterIP(IPAddress inverterAddress) {
	_inverterAddress = inverterAddress;
}

bool SMAReader::postSMA(const char* postURL, const char* postMessage, DynamicJsonDocument& jsonResult) {
    DEBUG_SMAREADER("[HTTP] POST... URL: %s\n", postURL);
    DEBUG_SMAREADER("[HTTP] POST... message: %s\n", postMessage);

    http.useHTTP10(true);
    WiFiClient client;
    http.begin(client, postURL);
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.0; WOW64; rv:24.0) Gecko/20100101 Firefox/24.0");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json, text/plain, */*");

    int httpCode = http.POST(postMessage);
    if(httpCode!=HTTP_CODE_OK) {
        DEBUG_SMAREADER("[HTTP] POST... failed, error: %d: %s\n", httpCode, http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }

    #if defined(DEBUG_SMAREADER_ON) && defined(DEBUG_SMAREADER_PORT)
    ReadLoggingStream loggingStream(http.getStream(), DEBUG_SMAREADER_PORT);
    DeserializationError error=deserializeJson(jsonResult, loggingStream);
    #else
    DeserializationError error=deserializeJson(jsonResult, http.getStream());
    #endif
    DEBUG_SMAREADER("\n");
        
    http.end();
    if (error) {
      DEBUG_SMAREADER("Parsing failed: %s\n", error.c_str());
      return false;
    }
    return true;
}

bool SMAReader::authorize() {
    char authURL[40];
    sprintf(authURL, "http://%d.%d.%d.%d/dyn/login.json", _inverterAddress[0], _inverterAddress[1], _inverterAddress[2], _inverterAddress[3]);
    char postText[40];
    sprintf(postText, "{\"right\":\"%s\", \"pass\":\"%s\"}", _accountType, _passwd);

    /* Stream is considered read-only input, so the strings have to be stored too, the minimum size is then 2*JSON_OBJECT_SIZE(1)+28 */
    DynamicJsonDocument jsonResult(2*JSON_OBJECT_SIZE(1)+35);
    bool isSuccess=postSMA(authURL, postText, jsonResult);
    if(isSuccess) {
        const char* res=jsonResult["result"]["sid"].as<const char*>();
        if(res==nullptr) {
           isSuccess=false;
           DEBUG_SMAREADER("Unexpected JSON format\n");
        }
        else {
          _sid=res;
          DEBUG_SMAREADER("Successful authorization, sid: %s\n", _sid.c_str());
        }
    }
    return isSuccess;
}

bool SMAReader::logout() {
    DEBUG_SMAREADER("Logging out\n");
    if(_sid=="") {
      DEBUG_SMAREADER("failed, empty sid\n");
      return false;
    }
    char logoutURL[128];
    sprintf(logoutURL, "http://%d.%d.%d.%d/dyn/logout.json?sid=%s", _inverterAddress[0], _inverterAddress[1], _inverterAddress[2], _inverterAddress[3], _sid.c_str());
    DynamicJsonDocument jsonResult(100);
    bool isSuccess=postSMA(logoutURL, "{}", jsonResult);
    return isSuccess;
}

bool SMAReader::getValuesAux(int numKeys, const String* keys, DynamicJsonDocument& doc, JsonVariant& result) {
    char scriptURL[128];
    char postText[30+numKeys*17];
    char keyString[numKeys*17];
    strcpy(keyString, "\"");
    for(int i=0;i<numKeys;i++) {
      if(keys[i].length()>17) {
        DEBUG_SMAREADER("Invalid key %d: %s, too long\n", i, keys[i].c_str());
        return false;
      }
      if(i>0) strcat(keyString, "\",\"");
      strcat(keyString, keys[i].c_str());
    }
    strcat(keyString, "\"");
    sprintf(postText, "{\"keys\":[%s], \"destDev\":[]}", keyString);
    
    for(byte i=0;i<_numTries;i++) {
      DEBUG_SMAREADER("Try: %d\n", i);
      if(authorize()) {
        sprintf(scriptURL, "http://%d.%d.%d.%d/dyn/getValues.json?sid=%s", _inverterAddress[0], _inverterAddress[1], _inverterAddress[2], _inverterAddress[3], _sid.c_str());
        bool postSuccess=postSMA(scriptURL, postText, doc);
        logout();
        if(postSuccess) {
           JsonObject result0=doc["result"];
           for(JsonPair p: result0) {
              result=p.value();
              DEBUG_SMAREADER("Parsed JSON successfully, try: %d\n", i+1);
              return true;
           }
           DEBUG_SMAREADER("Unexpected JSON format, try: %d\n", i+1);
        }
      }
    }
    return false;
}

bool SMAReader::getValues(int numKeys, const String* keys, int* values) {
    /* Stream is considered read-only input, so the strings have to be stored too, the minimum size is then 
    // numKeys*JSON_ARRAY_SIZE(1)+(2*numKeys+2)*JSON_OBJECT_SIZE(1)+JSON_OBJECT_SIZE(numKeys)+21+20*numkeys */
    DynamicJsonDocument doc(numKeys*JSON_ARRAY_SIZE(1)+(2*numKeys+2)*JSON_OBJECT_SIZE(1)+JSON_OBJECT_SIZE(numKeys)+40+20*numKeys);
    JsonVariant result;
    if(getValuesAux(numKeys, keys, doc, result)) {
       for(int i=0;i<numKeys;i++) {
          JsonVariant val=result[keys[i]]["1"][0]["val"];
          if(val.is<int>()) {
              values[i]=val.as<int>();
              DEBUG_SMAREADER("Int value: %d, key: %s, value: %d\n", i, keys[i].c_str(), values[i]);
          } else {
              values[i]=-1;
              DEBUG_SMAREADER("Value not found or not integer: %d, key: %s\n", i, keys[i].c_str());
          }
       }
       return true;
    }
    return false;
}

bool SMAReader::getValues(int numKeys, const String* keys, String* values) {
    /* Stream is considered read-only input, so the strings have to be stored too, the minimum size is then 
    // numKeys*JSON_ARRAY_SIZE(1)+(2*numKeys+2)*JSON_OBJECT_SIZE(1)+JSON_OBJECT_SIZE(numKeys)+21+20*numkeys 
    // add extra for 20*numKeys for string results */
    DynamicJsonDocument doc(numKeys*JSON_ARRAY_SIZE(1)+(2*numKeys+2)*JSON_OBJECT_SIZE(1)+JSON_OBJECT_SIZE(numKeys)+40+40*numKeys);
    JsonVariant result;
    if(getValuesAux(numKeys, keys, doc, result)) {
       for(int i=0;i<numKeys;i++) {
          JsonVariant val=result[keys[i]]["1"][0]["val"];
          if(val.is<const char*>()) {
              values[i]=val.as<const char*>();
              DEBUG_SMAREADER("String value: %d, key: %s, value: %s\n", i, keys[i].c_str(), values[i].c_str());
          } else if(val.is<int>()) {
              values[i]=val.as<int>();
              DEBUG_SMAREADER("Int value: %d, key: %s, value: %s\n", i, keys[i].c_str(), values[i].c_str());
          } else {
              values[i]="";
              DEBUG_SMAREADER("Value not found: %d, key: %s\n", i, keys[i].c_str());
          }
       }
       return true;
    }
    return false;
}

int SMAReader::getLog(uint32_t startTime, uint32_t endTime, uint32_t* values, uint32_t* timestamps) {
    char scriptURL[128];
    char postText[70];
    sprintf(postText, "{\"key\":28672, \"destDev\":[],\"tStart\":%lu,\"tEnd\":%lu}", startTime, endTime);
    int numValues=max((int)(endTime-startTime)/300+1,1);
    DynamicJsonDocument doc(JSON_ARRAY_SIZE(numValues) + 2*JSON_OBJECT_SIZE(1) + numValues*JSON_OBJECT_SIZE(2)+45+4*numValues);
    for(byte i=0;i<_numTries;i++) {
      DEBUG_SMAREADER("Try: %d\n", i);
      if(authorize()) {
        sprintf(scriptURL, "http://%d.%d.%d.%d/dyn/getLogger.json?sid=%s", _inverterAddress[0], _inverterAddress[1], _inverterAddress[2], _inverterAddress[3], _sid.c_str());
        bool postSuccess=postSMA(scriptURL, postText, doc);
        logout();
        if(postSuccess) {
           JsonObject result0=doc["result"];
           for(JsonPair p: result0) {
              int j=0;
              for(JsonObject obj: p.value().as<JsonArray>()) {
                 JsonVariant timestamp=obj["t"];
                 JsonVariant val=obj["v"];
                 if(val.is<int>()) values[j]=val.as<uint32_t>(); else values[j]=-1;
                 if(timestamps!=nullptr) {
                  if(timestamp.is<int>()) timestamps[j]=timestamp.as<uint32_t>(); else timestamps[j]=-1;
                 }
                 j++;
              }
              DEBUG_SMAREADER("Parsed JSON successfully, try: %d\n", i+1);
              return j;
           }
           DEBUG_SMAREADER("Unexpected JSON format, try: %d\n", i+1);
        }
      }
    }
    return -1;
}

bool SMAReader::getAllValues() {
    if(!authorize()) return false;
    char scriptURL[65];
    sprintf(scriptURL, "http://%d.%d.%d.%d/dyn/getAllOnlValues.json?sid=%s", _inverterAddress[0], _inverterAddress[1], _inverterAddress[2], _inverterAddress[3], _sid.c_str());
    char postText[]="{\"destDev\":[]}";
    DynamicJsonDocument jsonResult(10000);
    bool isSuccess=postSMA(scriptURL, postText, jsonResult);
    return isSuccess;
}

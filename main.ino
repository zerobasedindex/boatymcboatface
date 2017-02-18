
#include "cellular_hal.h"

#include "OneWire.h"
#include "DallasTemperature.h"
#include "devices.h"
#include "TempMgr.h"

STARTUP(cellular_credentials_set("wholesale", "", "", NULL));

int sampleRounds = 0;

OneWire ds(D0);
DallasTemperature temps(&ds);
TempMgr tempMgr(numAddresses);

TCPClient client;

void setup() {
   Serial.begin(9600);
   temps.begin();
}

void loop(void) {

  unsigned long start = millis();

  for (int i = 0; i < numAddresses; i++) {
    if (temps.requestTemperaturesByAddress(addresses[i])) {
      float temp = temps.getTempF(addresses[i]);
      if (validTemp(temp)) {
        tempMgr.addTempSample(i, temp);
      }
    }
  }
  unsigned long end = millis();

  sampleRounds++;
  if (sampleRounds == 20) {
    sendData();
    sampleRounds = 0;
  }

  unsigned long delayMS = 3000;
  if (end > start) {
    delayMS = max(1000, delayMS - (end - start));
  }
  delay(delayMS);
}

bool validTemp(float temp) {
  return (DEVICE_DISCONNECTED_F != temp && temp > 0 && temp < 130);
}

void sendData() {
  int tries = 0;

  String data = "";

  for (int i = 0; i < numAddresses; i++) {
    float temp = tempMgr.getAverageTemp(i);
    if (temp != NO_DATA) {
      if (data.length() > 0) {
        data += "&";
      }
      data += "field" + String(i) + "=" + String(temp);
    }
  }

  while (tries < 3) {
    tries++;

    if (client.connect("api.thingspeak.com", 80)) {
      Serial.println("connected");
      client.println("POST /update HTTP/1.0");
      client.println("Host: api.thingspeak.com");
      client.println("THINGSPEAKAPIKEY: 246JIE9VHBESE217");
      client.print("Content-Length: ");
      client.println(data.length());
      client.println();
      client.println(data);
      client.println();
      client.flush();
      Serial.println("sending data: " + data);
      while (client.read() != -1 && client.connected()) {}
      client.stop();
      Serial.println("data sent");
      break;
    } else {
      Serial.println("connection failed");
      delay(2000);
    }
  }
}

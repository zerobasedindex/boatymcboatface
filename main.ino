
#include "cellular_hal.h"

#include "OneWire.h"
#include "DallasTemperature.h"
#include "devices.h"
#include "TempMgr.h"
#include "LiquidCrystal.h"

STARTUP(cellular_credentials_set("wholesale", "", "", NULL));

int sampleRounds = 0;

OneWire ds(B0);
DallasTemperature temps(&ds);
TempMgr tempMgr(numAddresses);

const int rs = C0, en = C1, d4 = D4, d5 = D5, d6 = D6, d7 = D7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

TCPClient client;

void setup() {
   Serial.begin(9600);
   Serial.println("Starting...");
   temps.begin();

   // set up the LCD's number of columns and rows:
   lcd.begin(16, 2);
   // Print a message to the LCD.
   lcd.print("Starting...");
}

void loop(void) {
  printStatusMsg("Sample Round: " + String(sampleRounds));

  unsigned long start = millis();

  for (int i = 0; i < numAddresses; i++) {
    if (temps.requestTemperaturesByAddress(addresses[i])) {
      float temp = temps.getTempF(addresses[i]);
      if (validTemp(temp)) {
        Serial.print("Addr "); Serial.print(i); Serial.print(" "); Serial.println(temp);
        tempMgr.addTempSample(i, temp);
        lcd.setCursor(0, 1);
        lcd.print("Temp[" + String(i) + "]: " + String(temp));
      } else {
        Serial.print("Addr "); Serial.print(i); Serial.print(" "); Serial.println("Invalid Temp");
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

  printStatusMsg("Prearing data...");
  for (int i = 0; i < numAddresses; i++) {
    float temp = tempMgr.getAverageTemp(i);
    if (temp != NO_DATA) {
      if (data.length() > 0) {
        data += "&";
      }
      data += "field" + String(i) + "=" + String(temp);
    }
  }

  lcd.setCursor(0, 0);
  Serial.print("Sending data: "); Serial.println(data);
  while (tries < 3) {
    tries++;

    printStatusMsg("Connecting...");
    if (client.connect("api.thingspeak.com", 80)) {
      Serial.println("connected");
      printStatusMsg("Connected!");
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
      lcd.setCursor(0, 0);
      lcd.print("Data sent");
      break;
    } else {
      printStatusMsg("Connect failed!");
      Serial.println("connection failed");
      delay(2000);
    }
  }
}

void printStatusMsg(String msg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg);
}

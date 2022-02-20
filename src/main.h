#include "Arduino.h"
#include "config.h"
#include <Wire.h>
#include <WiFi.h>
// #include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <NTPClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSansBoldOblique18pt7b.h>
#include <Fonts/FreeSansBoldOblique24pt7b.h>

#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSansOblique12pt7b.h>
#include <Fonts/FreeSansOblique18pt7b.h>
#include <Fonts/FreeSansOblique24pt7b.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>



#include <PubSubClient.h>
#include <Thread.h>
#include <StaticThreadController.h>

unsigned long counts[2] = {0, 0}, logs[ENTRIES];
// float dose = 0.148;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiServer server(1883);
WiFiClient wificlient;
PubSubClient MQTTclient(wificlient);

Thread threadCurrentLog = Thread();
Thread threadUpdateRadmon = Thread();
StaticThreadController<2> threadController (&threadCurrentLog, &threadUpdateRadmon);
// threadController.add(updateRadmon);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, time_server, TIMEZONE * 3600, 60000);

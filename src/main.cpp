/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/
#include "main.h"

// String partTopic;
String clientId = "RadBoy";
unsigned long lastReconnectAttempt = 0;
float readings[3];

String byteToString(byte *_byte, unsigned _len)
{
  String _ret = "";

  for (unsigned int i = 0; i < _len; i++)
  {
    _ret += (char)_byte[i];
  }

  return _ret;
}

void callback(char* topic, byte* payload, unsigned int length)
{

}
bool reconnect() // Called when client is disconnected from the MQTT server
{
  if (MQTTclient.connect(clientId.c_str()))
  {
    timeClient.update(); // Is this the right time to update the time, when we lost connection?
    Serial.print("reconnect: " + String(millis() / 1000));
    Serial.println("\ttime: " + String(timeClient.getEpochTime()));
    MQTTclient.publish(String("sensors/" + clientId + "/debug/connected").c_str(), String(timeClient.getEpochTime()).c_str());
    // subscribeMQTT();
    // threadPublishCallback(); // Publish after reconnecting, probably not necesary with small LOGPERIOD
  }

  return MQTTclient.connected();
}

String generateClientIdFromMac() // Convert the WiFi MAC address to String
{
  byte _mac[6];
  String _output = "";

  WiFi.macAddress(_mac);

  for (int _i = 5; _i > 0; _i--)
  {
    _output += String(_mac[_i], HEX);
  }
  Serial.println(_output);
  return _output;
}
// send to radmon.org
void threadUpdateRadmonCallback() {
  uint8_t seconds = millis() / 1000;
  if (config.debugRadmon) {
    Serial.println("\r\nUptime: " + String(seconds) + " Seconds");
  }
  if (config.debugRadmon) {
    Serial.println("\r\nUpdating Radmon...");
  }

  WiFiClient clientGet;
  const int httpGetPort = 80;

  //the path and file to send the data to:
  String urlGet = "/radmon.php";
  urlGet += "?function=submit&user=";
  urlGet += RadmonUser;
  urlGet += "&password=";
  urlGet += RadmonPass;
  urlGet += "&value=";
  urlGet += int(readings[1]);
  urlGet += "&unit=CPM";

  if (config.debugRadmon) {
    Serial.print(">>> Connecting to host: ");
    Serial.println(RadmonHost);
  }

  if (!clientGet.connect(RadmonHost, httpGetPort)) {
    if (config.debugRadmon) {
      Serial.print("Connection failed: ");
      Serial.println(RadmonHost);
    }

  } else {
    if (config.debugRadmon) {
      Serial.println("Sending data....");
    }
    // if (config.debugRadmon)
    // {
      Serial.print("sent: ");
      Serial.print(readings[1]);
      Serial.println(" CPM");
    // }
    clientGet.println("GET " + urlGet + " HTTP/1.1");
    if (config.debugRadmon) {
      Serial.println("GET " + urlGet + " HTTP/1.1");
    }
    clientGet.print("Host: ");
    clientGet.println(RadmonHost);
    if (config.debugRadmon) {
      Serial.println("Host: " + String(RadmonHost));
    }
    clientGet.println("User-Agent: ESP8266/1.0");
    if (config.debugRadmon) {
      Serial.println("User-Agent: ESP8266/1.0");
    }
    clientGet.println("Connection: close\r\n\r\n");
    if (config.debugRadmon) {
      Serial.println("Connection: close\r\n\r\n");
    }

    unsigned long timeoutP = millis();
    while (clientGet.available() == 0) {
      if (millis() - timeoutP > 10000) {
        if (config.debugRadmon) {
          Serial.print(">>> Client Timeout: ");
          Serial.println(RadmonHost);
        }
        clientGet.stop();
        return;
      }
    }

    if (config.debugRadmon) {
      Serial.println("End of sending data....\r\n\r\nResponse:");
    }

    //just checks the 1st line of the server response. Could be expanded if needed.
    while (clientGet.available()) {
      String retLine = clientGet.readStringUntil('\r');
      if (config.debugRadmon) {
        Serial.println(retLine);
      }
      break;
    }
  } //end client connection if else

  if (config.debugRadmon) {
    Serial.print(">>> Closing host: ");
    Serial.println(RadmonHost);
  }
  clientGet.stop();
}

void threadCurrentLogCallback()
{
  // Serial.println("threadCurrentLogCallback");
  for (int _i = ENTRIES - 1; _i > 0; _i--)
    logs[_i] = logs[_i - 1];

  logs[0] = counts[0];
  counts[1] += counts[0];
  counts[0] = 0;

  float _cpmMinuteAverage = 0; // dit is het average van de afgelopen minuut
  for (int _i = ENTRIES - 1; _i > 0; _i--)
    _cpmMinuteAverage += logs[_i];
  // PayloadConstructor();
  float _cpmTotal = (float)counts[1] / float (millis() / 1000) * 60; // dit is een inter/extrapolatie van het totaal
  // Serial.print("_cpmTotal: ");Serial.println(_cpmTotal);

  // Serial.print("_cpmMinuteAverage: ");Serial.println(_cpmMinuteAverage);
  float _dose = _cpmTotal / tubeIndex; // dit is de tube index CPM / 151 = uSv/h
// Serial.print("_dose: ");Serial.println(_dose);
  readings[0] = _cpmTotal;
  readings[1] = _cpmMinuteAverage;
  readings[2] = _dose;



 MQTTclient.publish(String("sensors/" + clientId + "/cpmTotal").c_str(), String(_cpmTotal).c_str());
 MQTTclient.publish(String("sensors/" + clientId + "/cpmMinuteAverage").c_str(), String(_cpmMinuteAverage).c_str());
 MQTTclient.publish(String("sensors/" + clientId + "/dose").c_str(), String(_dose).c_str());

//   String _payload = "{\"CPMTotal\":";
//          _payload += (String)_cpmTotal;
//          _payload += ",\"cpmMinuteAverage\":";
//          _payload += (String)_cpmMinuteAverage;
//          _payload += ",\"dose\":";
//          _payload += (String)_dose;
//          _payload += "}";
// //         Serial.println(_dose);
// //         Serial.println(payload);
//          if(MQTTclient.publish(topic, (char*) _payload.c_str())) {
//           Serial.println(_payload);
//           Serial.println("Publish OK!");
//          } else {
//           Serial.println("Publish FAIL!");
//           // InitMQTT();
//          }
}

String macToStr(const uint8_t* mac)
{
  String result;

  for (int i = 0; i < 6; ++i)
  {
    result += String(mac[i], 16);

    if (i < 5) result += ':';
  }

  return result;
}

void countPulse()
{
  counts[0]++;
}


void setup() {
  pinMode(PulsePin, INPUT);
  for (int _i = 0; _i < ENTRIES; _i++) {
    logs[_i] = 0;
  }

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, WiFipass);

  while(WiFi.status() != WL_CONNECTED)  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  MQTTclient.setServer(mqtt_server, mqtt_port);
  MQTTclient.setCallback(callback);

  Wire.begin(SDA, SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    // for(;;);
  }

  // display.setTextSize(4); // Draw 3X-scale text
  // display.print("Setup...");
  delay(2000); // Pause for 2 seconds

  // Clear the buffer.
  display.clearDisplay();
  clientId += "-" + generateClientIdFromMac();
  // partTopic = String("se/" + clientId + "/Ch");

  threadCurrentLog.enabled = true;
  threadCurrentLog.setInterval(LOG_PERIOD * 1000);
  threadCurrentLog.onRun(threadCurrentLogCallback);

  threadUpdateRadmon.enabled = true;
  threadUpdateRadmon.setInterval(LOG_PERIOD * 1000);
  threadUpdateRadmon.onRun(threadUpdateRadmonCallback);

  attachInterrupt(digitalPinToInterrupt(PulsePin), countPulse, FALLING);

  display.clearDisplay();
  display.setTextSize(1); // Draw 1X-scale text
  display.setTextColor(SSD1306_WHITE);
}

void loop() {

  if (!MQTTclient.connected())
  {
    if (millis() - lastReconnectAttempt >= 5000) // try to reconnect every 5000 milliseconds
    {
      lastReconnectAttempt = millis();

      if (reconnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    MQTTclient.loop();
  }
  // Run threads, this makes it all work on time!
  threadController.run();
  // delay(1000);
  display.clearDisplay();

  // HOSTNAME
  display.setCursor(0, 0);
  display.setFont();

  // display.setTextSize(1);
  display.println(clientId);
  display.setCursor(56, 9);
  display.print(" @ ");

  // IP ADDRESS
  display.println(WiFi.localIP());

  // LABEL
  display.setFont(&FreeSansOblique9pt7b);
  display.setTextSize(1);
  display.setCursor(0, 28);
  display.print(F("Dose:"));

  // VALUE
  display.setCursor(0, 62);
  display.setFont(&FreeSans24pt7b);
  display.setTextSize(1); // Draw 3X-scale text
  display.print(readings[2]);

  // UNIT
  display.setCursor(100, 60);
  display.setFont();
  display.setTextSize(1); // Draw 3X-scale text
  display.print(F("mS/h"));
  display.display();      // Show initial text
}

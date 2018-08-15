#include "images.h"
// Wifi package
#include <ESP8266WiFi.h>
// IMportant for the MQTT Libary
#include <Ethernet.h>
// include library, include base class, make path known
#include <GxEPD.h>
#include <GxFont_GFX.h>
#include <PubSubClient.h>
// Some GFX libs
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
// Time Libary
#include <time.h>
// Temp sensor libary
#include <DHTesp.h>

// select the display class to use, only one
//#include <GxGDEW075Z09/GxGDEW075Z09.cpp> // 7.5" b/w/r
#include <GxGDEW075T8/GxGDEW075T8.cpp>

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
// Global var, getting from IO Broker
double outerTempValue;
// Setting up the display
GxIO_Class io(SPI, /*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
GxEPD_Class display(io /*RST=D4*/ /*BUSY=D2*/);              // default selection of D4(=2), D2(=4)
byte server[] = {192, 168, 2, 122};
byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED};

//void callback(char *topic, byte *payload, unsigned int length);
// from https://github.com/computergeek1507/arduino/blob/master/PinControlMQTT/PinControlMQTT.ino
// WIFI
WiFiClient espClient;
// MQTT Client
PubSubClient client(espClient);
DHTesp dhtSensor;
int dhtPin = 1;

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  InitializeTemperatureSensor();
  SetupWifi();
  Serial.println("Setting up timeserver");
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  SetUpMQTT();
  display.init(115200); // enable diagnostic output on Serial
  Serial.println("setup done");
}

void SetupWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin("Fritzrepeater2", "tumalonga2411");
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
}
void InitializeTemperatureSensor()
{
  Serial.println("Initialize Temperature Sensor");
  dhtSensor.setup(dhtPin, DHTesp::DHT11);
}
void SetUpMQTT()
{
  Serial.println("Setup MQTT");
  client.setServer(server, 1886);
  client.setCallback(callback);
  Ethernet.begin(mac, WiFi.localIP());
  Serial.println("Setup MQTT completed");

  Serial.println("MQTT trying to connect");
  while (!client.connected())
  {
    Serial.print(".");
    if (client.connect("DisplayMarijke", "sascha", "tumalonga2411"))
    {
      Serial.println(client.connected());
      Serial.println("Sending testmessages");
      // Abonieren von Nachrichten mit dem angegebenen Topic
      client.subscribe("clients/nodemcu/MDisplay/#");
      Serial.println("MQTT connected");
    }
    else
    {
      Serial.println("MQTT not connected");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void loop()
{

  delay(5000); // Pause 5 Sekunden
  //SetUpMQTT();
  SubscribeMqtt();
  ShowDashboard();
  delay(60000); // Pause 60 Sekunden
}

void SubscribeMqtt()
{
  Serial.print("MQTT Connected:");
  Serial.println(client.connected());
  client.loop();
}
void ShowDashboard()
{
  double boxWidth = display.width() / 4;
  double boxHeight = display.height() / 4;
  double topMiddleLine = ((display.height() / 4) / 2) + 20;
  Serial.println(boxWidth);
  Serial.println(boxHeight);
  Serial.println(topMiddleLine);

  // h: 640  / v: 384 px
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
#if defined(HAS_RED_COLOR)
  display.setTextColor(GxEPD_RED);
#endif
  Serial.println("Drawing total Rectangle");
  display.setRotation(4);
  display.drawRect(0, 0, display.width(), display.height(), GxEPD_BLACK);
  Serial.println("Drawing rectangles");

  // First box
  display.drawRect(0, 0, display.width() / 4, display.height() / 4, GxEPD_BLACK);
  // second Box
  display.drawRect(display.width() / 4, 0, ((display.width() / 4) * 2), display.height() / 4, GxEPD_BLACK);
  // third box
  display.drawRect(display.width() - display.width() / 4, 0, display.width() / 4, display.height() / 4, GxEPD_BLACK);

  // Writing Text Left Box
  display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);
  display.setCursor(2, 10);
  display.println("Aussen Temp.");

  UpdateOuterTempValue(10, topMiddleLine, outerTempValue);

  UpdateTime(boxWidth * 1.4, topMiddleLine);

  // Writing Text right box
  display.setTextSize(1);
  display.setFont(&FreeMono9pt7b);
  display.setCursor((display.width() - boxWidth), 10);
  display.println("Innen Temp.");
  UpdateInnerTempValue(((display.width() - boxWidth)) + 10, topMiddleLine);
  int imageX = (display.width() / 2) - (256 / 2);
  int imageY = (display.height() / 4) + 25;
  display.drawBitmap(icon256_01d, imageX, imageY, 256, 256, GxEPD_WHITE);
  display.update();
}

void UpdateTime(int x, int y)
{
  // Writing Text Center box
  char *buf = getTime();
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(x, y);
  display.setTextSize(2);
  display.println(buf);
}

void UpdateOuterTempValue(int x, int y, double value)
{
  Serial.print("Drawing Outertemp: ");
  Serial.println(outerTempValue);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextSize(2);
  display.setCursor(x, y);
  display.println(outerTempValue);
}

char *getTime()
{
  char buf[9];
  time_t now;
  struct tm *timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  sprintf(buf, "%i:%i", timeinfo->tm_hour, timeinfo->tm_min);
  Serial.println(buf);
  return buf;
}
String getIndoorTemperature()
{
  Serial.println("Try to fetch Temp from Sensor");
  TempAndHumidity lastValues = dhtSensor.getTempAndHumidity();
  return String(lastValues.temperature, 0);
}
void UpdateInnerTempValue(int x, int y)
{
  String innerTemp = getIndoorTemperature();
  Serial.print("Drawing inner temp: ");
  Serial.println(innerTemp);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextSize(2);
  display.setCursor(x, y);
  display.println(innerTemp);
}
String weatherIcon;
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Callback fired");
  char buff[256], echostring[512];
  int n, on;
  for (n = 0; (n < length) && (n < sizeof(buff) - 1); n++)
  {
    buff[n] = payload[n];
  }
  buff[n] = 0;
  sprintf(echostring, "%s", buff);

  if (strcmp(topic, "clients/nodemcu/MDisplay/temp") == 0)
  {
    Serial.println("Setting new outdoor Temperature value from Topic");
    outerTempValue = atof(echostring);
  }

  if (strcmp(topic, "clients/nodemcu/MDisplay/icon") == 0)
  {
    Serial.println("Setting new weathericon value from Topic");
    weatherIcon = String(echostring);
  }
}

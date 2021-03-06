// Contains the weathersymbols
#include "images.h"
// Wifi package
#include <ESP8266WiFi.h>
// IMportant for the MQTT Libary
#include <Ethernet.h>
#include <GxFont_GFX.h>
#include <PubSubClient.h>
// Time Libary
#include <time.h>

// IO Port, important for the DHT Sensor
#include <GxIO/GxIO.cpp>
// Temp sensor libary
#include <DHTesp.h>

// include library, include base class, make path known
#include <GxEPD.h>
// SPI libary for the communication between EINK and ESP
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
// select the display class to use, only one
//#include <GxGDEW075Z09/GxGDEW075Z09.cpp> // 7.5" b/w/r
#include <GxGDEW075T8/GxGDEW075T8.cpp>

// Some GFX libs, required for the Waveshare and fonts libary
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
// FreeFonts from Adafruit_GFX
// Font for the Title in the temperature display
#include <Fonts/FreeMono9pt7b.h>
// Font for the time display
#include <Fonts/FreeMonoBold18pt7b.h>
// Font for the Temperatur values
#include <Fonts/FreeMonoBold12pt7b.h>
const char *WIFIPASS = "";
const char *SID = "";

// Connecting to IOBROKER
const char *IOBROKER_USER = "";
const char *IOBROKER_PASS = "";
const char *mqttServer = "";

// The MAC Adress for the ESP Device
byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED};

// Global var, getting from IO Broker
double outerTempValue;
double innerTempValue;
// Gloabl var, getting from IO Broker.
String weatherIcon;

// Setting up the display
GxIO_Class io(SPI, /*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
GxEPD_Class display(io /*RST=D4*/ /*BUSY=D2*/);              // default selection of D4(=2), D2(=4)

// from https://github.com/computergeek1507/arduino/blob/master/PinControlMQTT/PinControlMQTT.ino

// WIFI
WiFiClient espClient;
// MQTT Client
PubSubClient client(espClient);
// The DHT Sensor instance
DHTesp dhtSensor;
// The connected pin where the dht is attached
int dhtPin = 5;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  InitializeTemperatureSensor();
  SetupWifi();
  SetUpMQTT();
  Serial.println("Setting up timeserver");
  // Set Timezone to +1 (1* 3600)
  configTime(1 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  display.init(115200); // enable diagnostic output on Serial
  Serial.println("setup done");
}

void SetupWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SID, WIFIPASS);
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
  dhtSensor.setup(dhtPin, DHTesp::DHT22);
}
void SetUpMQTT()
{
  Serial.println("Setup MQTT");
  //  client.setServer(server, 1886);

  client.setServer(mqttServer, 1886);
  client.setCallback(callback);
  Ethernet.begin(mac, WiFi.localIP());
  Serial.println("Setup MQTT completed");

  Serial.print("MQTT trying to connect");
  while (!client.connected())
  {
    Serial.print(".");
    client.connect("DisplayMarijke", IOBROKER_USER, IOBROKER_PASS);
    if (client.connected())
    {
      Serial.println("MQTT connected");
      // Abonieren von Nachrichten mit dem angegebenen Topic
      Serial.println("Subsscripe to Topics");
      client.subscribe("clients/nodemcu/#");
      client.subscribe("icon");
    }
    else
    {
      delay(2000);
    }
  }
  client.loop();
  client.loop();
}

void loop()
{

  SetUpMQTT();
  delay(5000); // Pause 5 Sekunden
  SubscribeMqtt();
  SubscribeMqtt();
  ShowDashboard();
  delay(60000); // Pause 60 Sekunden
  SetUpMQTT();
}

void SubscribeMqtt()
{
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
  UpdateInnerTempValue(((display.width() - boxWidth)) + 10, topMiddleLine, innerTempValue);
  int imageX = (display.width() / 2) - (256 / 2);
  int imageY = (display.height() / 4) + 25;

  UpdateImage(imageX, imageY);
  display.update();
}

void UpdateTime(int x, int y)
{
  Serial.println("Drawing Time");
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

void UpdateImage(int x, int y)
{

  if (!weatherIcon)
  {
    Serial.println("No Weathericon set, print default one");
    display.drawBitmap(icon256_01d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_01d")
  {
    Serial.println("Drawing icon256_01d");
    display.drawBitmap(icon256_01d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_01n")
  {
    Serial.println("Drawing icon256_01n");
    display.drawBitmap(icon256_01n, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_02d")
  {
    Serial.println("Drawing icon256_02d");
    display.drawBitmap(icon256_02d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_02n")
  {
    Serial.println("Drawing icon256_02n");
    display.drawBitmap(icon256_02n, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_03d")
  {
    Serial.println("Drawing icon256_03d");
    display.drawBitmap(icon256_03d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_04d")
  {
    Serial.println("Drawing icon256_04d");
    display.drawBitmap(icon256_04d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_09d")
  {
    Serial.println("Drawing icon256_09d");
    display.drawBitmap(icon256_09d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_10d")
  {
    Serial.println("Drawing icon256_10d");
    display.drawBitmap(icon256_10d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_11d")
  {
    Serial.println("Drawing icon256_11d");
    display.drawBitmap(icon256_11d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_13d")
  {
    Serial.println("Drawing icon256_13d");
    display.drawBitmap(icon256_13d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_50d")
  {
    Serial.println("Drawing icon256_50d");
    display.drawBitmap(icon256_50d, x, y, 256, 256, GxEPD_WHITE);
  }
  else if (weatherIcon == "icon256_50n")
  {
    Serial.println("Drawing icon256_50n");
    display.drawBitmap(icon256_50n, x, y, 256, 256, GxEPD_WHITE);
  }
}

char *getTime()
{
  char buf[9];
  time_t now;
  struct tm *timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  sprintf(buf, "%i:%02i", (timeinfo->tm_hour), timeinfo->tm_min);
  Serial.println(buf);
  return buf;
}

void UpdateInnerTempValue(int x, int y, double value)
{
  Serial.println("Try to get inner Temp value");
  Serial.print("Drawing inner temp: ");
  Serial.println(value);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextSize(2);
  display.setCursor(x, y);
  display.println(value);
}
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Callback fired");
  // Set som vars.
  char buff[256], echostring[512];
  int n;
  for (n = 0; (n < length) && (n < sizeof(buff) - 1); n++)
  {
    buff[n] = payload[n];
  }
  // Clear the buffer to prevent memoy leaks
  buff[n] = 0;
  sprintf(echostring, "%s %s", topic, buff);
  Serial.println(echostring);

  sprintf(echostring, "%s", buff);
  if (strcmp(topic, "clients/nodemcu/MDisplay/temp") == 0)
  {
    Serial.println("Setting new outdoor Temperature value from Topic");
    outerTempValue = atof(echostring);
  }

  if (strcmp(topic, "clients/nodemcu/MDisplay/intemp") == 0)
  {
    Serial.println("Setting new indoor Temperature value from Topic");
    innerTempValue = atof(echostring);
  }

  if (strcmp(topic, "clients/nodemcu/MDisplay/icon") == 0 || strcmp(topic, "icon") == 0)
  {
    if (strlen(buff) > 4)
    {
      Serial.print("STRLEN ");
      Serial.println(strlen(buff));
      Serial.println("Setting new weathericon value from Topic");
      weatherIcon = String(buff);
    }
  }
}

#include <ESP8266WiFi.h>
#include <GxEPD.h>
#include <GxFont_GFX.h>
#include <PubSubClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <time.h>

// include library, include base class, make path known
#include <GxEPD.h>

// select the display class to use, only one
//#include <GxGDEW075Z09/GxGDEW075Z09.cpp> // 7.5" b/w/r
#include <GxGDEW075T8/GxGDEW075T8.cpp>
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold18pt7b.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

#if defined(ESP8266)
GxIO_Class io(SPI, /*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
GxEPD_Class display(io /*RST=D4*/ /*BUSY=D2*/);              // default selection of D4(=2), D2(=4)
// Heltec E-Paper 1.54" b/w without BUSY
//GxEPD_Class display(io, /*RST=D4*/ 2, /*BUSY=D2*/ -1); // default selection of D4(=2), no BUSY

#elif defined(ESP32)

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/17, /*RST=*/16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/16, /*BUSY=*/4);        // arbitrary selection of (16), 4

#elif defined(ARDUINO_ARCH_SAMD)

GxIO_Class io(SPI, /*CS=*/4, /*DC=*/7, /*RST=*/6);
GxEPD_Class display(io, /*RST=*/6, /*BUSY=*/5);

#elif defined(ARDUINO_GENERIC_STM32F103C) && defined(MCU_STM32F103C8)

GxIO_Class io(SPI, /*CS=*/SS, /*DC=*/3, /*RST=*/2);
GxEPD_Class display(io, /*RST=*/2, /*BUSY=*/1);

#elif defined(ARDUINO_GENERIC_STM32F103V) && defined(MCU_STM32F103VB)

GxIO_Class io(SPI, /*CS=*/SS, /*DC=*/PE15, /*RST=*/PE14); // DC, RST as wired by DESPI-M01
GxEPD_Class display(io, /*RST=*/PE14, /*BUSY=*/PE13);     // RST, BUSY as wired by DESPI-M01

#else

// pins_arduino.h, e.g. AVR
//#define PIN_SPI_SS    (10)
//#define PIN_SPI_MOSI  (11)
//#define PIN_SPI_MISO  (12)
//#define PIN_SPI_SCK   (13)

GxIO_Class io(SPI, /*CS=*/SS, /*DC=*/8, /*RST=*/9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
GxEPD_Class display(io /*RST=9*/ /*BUSY=7*/);       // default selection of (9), 7

#endif

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  WiFi.mode(WIFI_STA);
  WiFi.begin("Fritzrepeater2", "tumalonga2411");
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Setting up timeserver");
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");

  display.init(115200); // enable diagnostic output on Serial
  Serial.println("setup done");
}

void loop()
{
  delay(5000); // Pause 5 Sekunden
#if !defined(__AVR)
  showFont();
#endif
  delay(60000); // Pause 60 Sekunden
}

void GetDisplayTime()
{
  // DateTime.strftime(displayedTime, 20, "%Y-%m-%d %H:%M", localtime(&now));
  // Serial.println(displayedTime);
}

// #if defined(_GxGDEW075Z09_H_)
// #define HAS_RED_COLOR
// void showBitmapExample()
// {

// #if defined(__AVR)
//   display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
// #elif defined(ARDUINO_GENERIC_STM32F103C)
//   display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
// #elif defined(ARDUINO_GENERIC_STM32F103V)
//   display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
//   delay(2000);
//   display.drawExamplePicture_3C(BitmapPicture_3C, sizeof(BitmapPicture_3C));
// #else
//   display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
//   delay(2000);
//   display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
//   delay(5000);
//   display.drawExamplePicture_3C(BitmapPicture_3C, sizeof(BitmapPicture_3C));
// #endif
// }
// #endif

void ShowDashboard()
{
}

void showFont()
{
  double boxWidth = display.width() / 4;
  double boxHeight = display.height() / 4;
  double topMiddleLine = (display.height() / 4) / 2;

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
  display.setCursor((display.width() / 4) / 2.5, topMiddleLine);
  display.setFont(&FreeMonoBold18pt7b);
  display.println("24");

  // Writing Text Center box
  char buf[9];
  time_t now;
  struct tm *timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  sprintf(buf, "%i:%i", timeinfo->tm_hour, timeinfo->tm_min);
  Serial.println(buf);
  display.setCursor(boxWidth * 1.4, topMiddleLine - 20);
  display.setTextSize(2);
  display.println(buf);
  display.setTextSize(1);

  // Writing Text right box
  display.setCursor(((display.width() - boxWidth) + boxWidth / 2.5), topMiddleLine);
  display.println("18");

  display.update();
}

void drawCornerTest()
{
  display.drawCornerTest();
  delay(5000);
  uint8_t rotation = display.getRotation();
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
    display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
    display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
    display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
    display.update();
    delay(5000);
  }
  display.setRotation(rotation); // restore
}

#if defined(_GxGDEP015OC1_H_) || defined(_GxGDE0213B1_H_) || defined(_GxGDEH029A1_H_)
#include "IMG_0001.h"
void showBoat()
{
  // thanks to bytecrusher: http://forum.arduino.cc/index.php?topic=487007.msg3367378#msg3367378
  uint16_t x = (display.width() - 64) / 2;
  uint16_t y = 5;
  display.fillScreen(GxEPD_WHITE);
  display.drawExampleBitmap(gImage_IMG_0001, x, y, 64, 180, GxEPD_BLACK);
  display.update();
  delay(500);
  uint16_t forward = GxEPD::bm_invert | GxEPD::bm_flip_x;
  uint16_t reverse = GxEPD::bm_invert | GxEPD::bm_flip_x | GxEPD::bm_flip_y;
  for (; y + 180 + 5 <= display.height(); y += 5)
  {
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(gImage_IMG_0001, x, y, 64, 180, GxEPD_BLACK, forward);
    display.updateWindow(0, 0, display.width(), display.height());
    delay(500);
  }
  delay(1000);
  for (; y >= 5; y -= 5)
  {
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(gImage_IMG_0001, x, y, 64, 180, GxEPD_BLACK, reverse);
    display.updateWindow(0, 0, display.width(), display.height());
    delay(1000);
  }
  display.update();
  delay(1000);
}
#endif

// Morphing Clock by Hari Wiguna, July 2018
//
// Thanks to:
// Dominic Buchstaller for PxMatrix
// Tzapu for WifiManager
// Stephen Denne aka Datacute for DoubleResetDetector
// Brian Lough aka WitnessMeNow for tutorials on the matrix and WifiManager

//#define double_buffer
#include <PxMatrix.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <Fonts/TomThumb.h>

#ifdef ESP32

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#endif

#ifdef ESP8266

#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

#endif
// Pins for LED MATRIX

//PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
//PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);

#ifdef ESP8266
// ISR for display refresh
void display_updater()
{
  display.display(display_draw_time);
}
#endif

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time=70; //10-50 is usually fine

#ifdef ESP32
void IRAM_ATTR display_updater(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif

//=== SEGMENTS ===
#include "Digit.h"
Digit digit0(&display, 0, 63 - 1 - 9*1, 8, display.color565(0, 0, 255));
Digit digit1(&display, 0, 63 - 1 - 9*2, 8, display.color565(0, 0, 255));
Digit digit2(&display, 0, 63 - 4 - 9*3, 8, display.color565(0, 0, 255));
Digit digit3(&display, 0, 63 - 4 - 9*4, 8, display.color565(0, 0, 255));
Digit digit4(&display, 0, 63 - 7 - 9*5, 8, display.color565(0, 0, 255));
Digit digit5(&display, 0, 63 - 7 - 9*6, 8, display.color565(0, 0, 255));

//=== CLOCK ===

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, 3600);

unsigned long prevEpoch;
byte prevhh;
byte prevmm;
byte prevss;

const char* dayLut[] = {
  "",
  "So",
  "Mo",
  "Di",
  "Mi",
  "Do",
  "Fr",
  "Sa"
};

const char* monthLut[] {
  "",
  "Jan",
  "Feb",
  "Mrz",
  "Apr",
  "Mai",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Okt",
  "Nov",
  "Dez"
};

void display_update_enable(bool is_enable)
{

#ifdef ESP8266
  if (is_enable)
    display_ticker.attach(0.002, display_updater);
  else
    display_ticker.detach();
#endif

#ifdef ESP32
  if (is_enable)
  {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 2000, true);
    timerAlarmEnable(timer);
  }
  else
  {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
#endif
}


void setup() {
  Serial.begin(9600);
  display.begin(16);
  //display.setFastUpdate(false);

  display.fillScreen(display.color565(0, 0, 0));
  digit1.DrawColon(display.color565(0, 0, 255));
  digit3.DrawColon(display.color565(0, 0, 255));
  WiFi.setAutoConnect(true);
  WiFi.begin("reichholf", "HansAnnemarieStephanDoreen");
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  ntpClient.begin();
  display_update_enable(true);
}

void updateDate()  {
  setTime(ntpClient.getEpochTime());
  String datestring = "";
  datestring = datestring + dayLut[weekday()] + " " + String(day()) + ". " + monthLut[month()];
  Serial.println(datestring);
  int x = 2; 
  int y = 0;
  // display.fillRect(x, y, 64, 6, display.color565(0,0,0));
  // display.flushDisplay();
  display.setCursor(x, y);
  display.setFont(&TomThumb);
  display.setTextColor(display.color565 (0, 255, 0), display.color565(0,0,0));
  display.print(datestring);
}


void loop() {
  ntpClient.update();
  unsigned long epoch = ntpClient.getEpochTime();
  if (epoch != prevEpoch) {
    updateDate();
    int hh = ntpClient.getHours();
    int mm = ntpClient.getMinutes();
    int ss = ntpClient.getSeconds();
    if (prevEpoch == 0) { // If we didn't have a previous time. Just draw it without morphing.
      digit0.Draw(ss % 10);
      digit1.Draw(ss / 10);
      digit2.Draw(mm % 10);
      digit3.Draw(mm / 10);
      digit4.Draw(hh % 10);
      digit5.Draw(hh / 10);
    }
    else
    {
      // epoch changes every miliseconds, we only want to draw when digits actually change.
      if (ss!=prevss) { 
        int s0 = ss % 10;
        int s1 = ss / 10;
        digit0.SetValue(s0);
        digit1.SetValue(s1);
        prevss = ss;
      }

      if (mm!=prevmm) {
        int m0 = mm % 10;
        int m1 = mm / 10;
        digit2.SetValue(m0);
        digit3.SetValue(m1);
        prevmm = mm;
      }
      
      if (hh!=prevhh) {
        int h0 = hh % 10;
        int h1 = hh / 10;
        digit4.SetValue(h0);
        digit5.SetValue(h1);
        prevhh = hh;
      }
    }
    prevEpoch = epoch;
    Serial.println(ntpClient.getFormattedTime());
  }
  digit0.Morph();
  digit1.Morph();
  digit2.Morph();
  digit3.Morph();
  digit4.Morph();
  digit5.Morph();
  delay(30); // animation speed
}

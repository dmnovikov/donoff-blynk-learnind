

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <Bounce2.h>
//#include <EEPROM.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Ticker.h>
#include "dsettings.h"


#define BLYNK_HEARTBEAT 300


// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "xxxxx";
//char pass[] = "xxxxxxxxxxxxxxxxxxxxxxxxxx";

char ssid[] = "lab240";
char pass[] = "xxxxxxxx";

// Your Blynk auth
char auth[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";




/*
 * 
 *  Blynk virtual pins

Buttons

V2 - switch button
V5 - on button
V6 - off button

Temperatures

V8 - tempurature of external ds1820
V11 - tempurature of ds1820 on board

Informers

V19 - uptime
V12 - "on" time
V6 - virtual LED
V18 - tic tac LED
V20 - current time
V21 - current date

*/



static bool BLYNK_ENABLED = true;


WidgetRTC rtc;
Bounce debouncer = Bounce();

OneWire oneWireIn(IN_WIRE_BUS);

OneWire oneWireOut(OUT_WIRE_BUS);

DallasTemperature sensorsIn(&oneWireIn);

DallasTemperature sensorsOut(&oneWireOut);



float temp1 = -127;
float temp2 = -127;
int wrn_level = 30;
ulong start_ms = 0;
ulong stop_ms = 0;
int  sensor_ask = 0;
static char str[12];
boolean isFirstConnect = true;

ulong start_bt = 0;
long last_connect_attempt;
int hours_working = 0;
int minutes_working = 0;
int sec_working = 0;
const int request_circle = 750;
const int reconnect_period = 30;

ulong ms_ask_sensor_in = 0;
ulong ms_ask_sensor_out = 0;

uint8_t  blink_loop = 0;
uint8_t  blink_mode = 0;
int blink_type = 0;

Ticker ticker;
int hronometer;

void tick()
{
  if (  modes[blink_type] & 1 << (blink_loop & 0x07) ) digitalWrite(SONOFF_LED, HIGH);
  else  digitalWrite(SONOFF_LED, LOW);
  blink_loop++;
}

void publish_blynk_time_date()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();

  // Send time to the App
  Blynk.virtualWrite(V20, currentTime);
  // Send date to the App
  Blynk.virtualWrite(V21, currentDate);
}

void set_blink(uint _current_blink_type ) {
  blink_type = _current_blink_type;
}

int is_connected() {
  if (!WL_CONNECTED) return 0;
  if (!Blynk.connected()) return 0;
  return 1;
}

bool r1_is_on() {
  if (digitalRead(RELAY1_PIN) == HIGH) {
    //Serial.println("check: relay is on");
    if (start_ms == 0) start_ms = millis();
    return 1;
  }

  if (start_ms != 0) start_ms = 0;
  //Serial.println("check: relay is off");
  return 0;
}

/*
  int r1_is_off(){
  if(start_ms!=0) return 0;
  return 1;
  }
*/

void sync_blink_mode() {

  if (is_connected() && !r1_is_on()) {
    //Serial.println("--->connected,OFF");
    set_blink(BL_CONNECTED_OFF);
    return;
  }
  if (is_connected() && r1_is_on()) {
    //Serial.println("--->connected,ON");
    set_blink(BL_CONNECTED_ON);
    return;
  }
  if (!is_connected() && !r1_is_on()) {
    //Serial.println("--->not connected,OFF");
    set_blink(BL_OFFLINE_OFF);
    return;
  }
  if (!is_connected() && r1_is_on()) {
    //Serial.println("--->not connected,ON");
    set_blink(BL_OFFLINE_ON);
    return;
  }
}


void TturnOn() {
  Serial.println("Try true ON");
  start_ms = millis();
  stop_ms = 0;
  int s = LOW;
  if (REVERSE_RELAY) {
    digitalWrite(RELAY1_PIN, s);
  } else {
    digitalWrite(RELAY1_PIN, !s);
  }
  sync_blink_mode();
}

void TturnOff() {
  Serial.println("Try true OFF");
  start_ms = 0;
  stop_ms = millis();
  int s = HIGH;
  if (REVERSE_RELAY) {
    digitalWrite(RELAY1_PIN, s);
  } else {
    digitalWrite(RELAY1_PIN, !s);
  }
  sync_blink_mode();
}

void toggle2() {
  if (start_ms == 0) {
    TturnOn();
    //Serial.println("Try turn on");
  }
  else if (start_ms != 0) {
    TturnOff();
    //Serial.println("Try turn off");
  }

}

void sync_Blynk_led() {
  if (r1_is_on()) {
    publish_blynk_on();
  }
  else publish_blynk_off();
}

void publish_blynk_on() {
  Blynk.virtualWrite(6, 220);
  Blynk.virtualWrite(12, get_time_string_str(millis() - start_ms));
  Blynk.virtualWrite(13, "OFF");
}

void publish_blynk_off() {
  Blynk.virtualWrite(6, 0);
  Blynk.virtualWrite(12, "OFF");
  Blynk.virtualWrite(13, get_time_string_str(millis() - stop_ms));
}

//toggle - button
BLYNK_WRITE(2) {
  int a = param.asInt();
  Serial.println("Blynk Button pressed:" + String(a));
  //Serial.println(a);
  if (a == 1) {
    if (r1_is_on()) {
      TturnOff();
      publish_blynk_off();
    }
    else {
      TturnOn();
      publish_blynk_on();
    }
  }
}


//switch - button
BLYNK_WRITE(1) {
  int a = param.asInt();
  Serial.print("SW pressed:");
  Serial.println(a);
  if (a == 1) {
    if (start_ms == 0) {
      Serial.println("Try SW turn on");
      TturnOn();
      publish_blynk_on();
    }
  }
  if (a == 0) {
    if (start_ms != 0) {
      Serial.println("Try SW turn off");
      TturnOff();
      publish_blynk_off();
    }
  }

}

int in_working_interval = 0;
int temp_ask_on = 0;

//SW with temp and time check
BLYNK_WRITE(3) {
  int a = param.asInt();
  Serial.print("SW pressed:");
  Serial.println(a);
  if (a == 1) {
    if (start_ms == 0) {
      Serial.println("Try SW turn on");
      if (in_working_interval) TturnOn();
      temp_ask_on = 1;

    }
  }
  if (a == 0) {
    if (start_ms != 0 || in_working_interval == 0) {
      Serial.println("Try SW turn off");
      TturnOff();
      publish_blynk_off();
      temp_ask_on = 0;
    }
  }


}

//check working_time from Blynk

BLYNK_WRITE(4) {
  int a = param.asInt();
  if (a == 1) {
    in_working_interval = 1;
    if (temp_ask_on == 1) TturnOn();

  }
  if (a == 0) {
    in_working_interval = 0;
    if (start_ms != 0) TturnOff();
  }

}

BLYNK_WRITE(5) {
  int a = param.asInt();
  Serial.println("Press V5:" + String(a));
  if (a == 1) {
    TturnOn();
    publish_blynk_on();
  }
}

BLYNK_WRITE(7) {
  int a = param.asInt();
  Serial.println("Press V7:" + String(a));
  if (a == 1) {
    TturnOff();
    publish_blynk_off();
  }
}

BLYNK_WRITE(0) {
  String payloadStr = param.asString();
  Serial.println("TERMINAL:" + payloadStr);
}

BLYNK_CONNECTED() {
  if (isFirstConnect) {
    // Request Blynk server to re-send latest values for all pins
    //Console.println(F("Sync"));
    Blynk.syncVirtual(V10);
    Blynk.syncVirtual(V15);
    Blynk.syncVirtual(V14);
    Serial.println("Start RTC");
    rtc.begin();
    isFirstConnect = false;
  }
}

// ********************************** SETUP ***************************************************

void setup()
{
  Serial.begin(9600);

  start_ms = 0;
  stop_ms = millis();

  pinMode(SONOFF_LED, OUTPUT);
  ticker.attach(0.25, tick);

  // start ticker with 0.5 because we start in AP mode and try to connect
  blink_type = BL_CONNECTING;


  Serial.println("SSID=" + String(ssid));
  Serial.println("pass=" + String(pass));
  Serial.println("blynk auth=" + String(auth));

  //Blynk.begin(auth, ssid, pass);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println("Connecting Wifi:" + String(ssid));
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Blynk.config(auth, "blynk-cloud.com", 80);
  Blynk.connect();

  last_connect_attempt = millis();
  if (Blynk.connected()) Serial.println("*** wifi - ok, Blynk - connected");

  if (WL_CONNECTED && Blynk.connected() ) {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    //finish
    blink_mode = 1;


  }

  //setup relay
  pinMode(RELAY1_PIN, OUTPUT);

  //TturnOff();

  Serial.println("done setup");
  //  timer.setInterval(5000L, sendUptime);

  sensorsIn.begin();
  DeviceAddress tempDeviceAddress1;
  sensorsIn.getAddress(tempDeviceAddress1, 0);
  sensorsIn.setResolution(tempDeviceAddress1, DALLAS_RES_IN);
  sensorsIn.setWaitForConversion(false);

  sensorsOut.begin();
  DeviceAddress tempDeviceAddress2;
  sensorsOut.getAddress(tempDeviceAddress2, 0);
  sensorsOut.setResolution(tempDeviceAddress2, DALLAS_RES_OUT);
  sensorsOut.setWaitForConversion(false);


  hronometer = 1;
  //  sensor_timer = millis();
  //sensor_ask = 0;

  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);


}

//****************************************** sendUPtime ***************************

void sendUptime()
{
  int connected = Blynk.connected();

  //request temp from dallas
  Serial.println("*************************** timer circle ******************************************");

  Serial.print("online=" + String(connected));
  if (r1_is_on()) Serial.print(", status=ON"); else Serial.print(", status=OFF");
  Serial.println(" ***");

  if (temp1 != -127 && temp1 > 75) {
    TturnOff();
  }

  sync_blink_mode();

  Serial.println("uptime:" + get_time_string_str(millis()));
  if (start_ms != 0) {
    Serial.println("time ON->" + get_time_string_str(millis() - start_ms));
    Serial.println("time OFF->0:0:0");
  } else {
    Serial.println("time OFF->" + get_time_string_str(millis() - stop_ms));
    Serial.println("time ON->0:0:0");
  }

  Serial.println("********************************************");

  if (!Blynk.connected()) {
    return;
  }

  // online publish to blynk

  Blynk.virtualWrite(V11, String(temp1));
  Blynk.virtualWrite(V8, String(temp2));

  sync_Blynk_led();

  //publish chronometer
  if (hronometer == 1)Blynk.virtualWrite(V18, "0");
  else Blynk.virtualWrite(V18, "255");
  hronometer = !hronometer;

  //publish uptime
  Blynk.virtualWrite(V19,  get_time_string_str(millis()));
  publish_blynk_time_date();

}

void get_time_working() {
  ulong uptime = millis() - start_ms;
  if (start_ms != 0) {
    long wt = uptime / 1000;
    //static char str[12];
    long h = wt / 3600;
    wt = wt % 3600;
    int m = wt / 60;
    int s = wt % 60;
    hours_working = h;
    minutes_working = m;
    sec_working = s;
  } else {
    hours_working = 0;
    minutes_working = 0;
    sec_working = 0;
  }
}

String get_time_string_str(unsigned long val) {
  long wt = val / 1000;
  long h = wt / 3600;
  wt = wt % 3600;
  int m = wt / 60;
  int s = wt % 60;
  return String(m) + ":" + String(h) + ":" + String(s);
}

int ask_ds1820(){
  if (ms_ask_sensor_in == 0) {
    sensorsIn.requestTemperatures();
    ms_ask_sensor_in = millis();
    Serial.println("Ask T1");
  }

  if (ms_ask_sensor_out == 0) {
    sensorsOut.requestTemperatures();
    ms_ask_sensor_out = millis();
    Serial.println("Ask T2");
  }
}

int get_ds1820(){
   if (millis() - ms_ask_sensor_in > request_circle) {
    //ready, get temp
    temp1 = sensorsIn.getTempCByIndex(0);
    ms_ask_sensor_in = 0;
    //publish
    Serial.println("T1->" + String(temp1));

  }

  if (millis() - ms_ask_sensor_out > request_circle) {
    //ready, get temp
    temp2 = sensorsOut.getTempCByIndex(0);
    ms_ask_sensor_out = 0;
    //publish
    Serial.println("T2->" + String(temp2));

  }
}

unsigned long mytimer = millis();
int  mycounter = 0;

void loop()
{

  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    Blynk.run();
  }

  //timer.run();

  if ((millis() - mytimer) >= 300 ) {

    if (mycounter == 0) {
      //reconnect
      if (!Blynk.connected() && millis() - last_connect_attempt > reconnect_period * 1000) {
        Serial.println("*** Try to connect again");
        Blynk.connect();
        last_connect_attempt = millis();
      }
    }

    if(mycounter ==1){
      ask_ds1820();
    }

    if(mycounter==2){
      get_ds1820();
    }

    if (mycounter >= 25 && start_bt == 0) {
      sendUptime();
      mycounter = 0;
    } else {
      mycounter++;
    }

    mytimer = millis();
  }

  debouncer.update();
  if ( debouncer.fell()  ) {
    Serial.println( "down");
    start_bt = millis();

  }

  if ( debouncer.rose()  ) {
    ;
    long duration = millis() - start_bt;
    start_bt = 0;
    Serial.println( "up" + String(duration));
    if (duration < BUTTON_DELAY) {
      Serial.println("FAIL PRESS, DO NOTHING");
    } else if (duration >= BUTTON_DELAY && duration < 2000) {
      Serial.println("--->short press - toggle relay");
      toggle2();
      //} else if (duration >=2000 && duration < 5000) {
      //  Serial.print(duration);
      //  Serial.println("-->medium press - restart");
      //  restart();
    } else if (duration >= 3000) {
      //something when long press
    }


  }

}

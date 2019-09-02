 
#ifndef dsettings_h
#define dsettings_h

#include "Arduino.h"

#define PINS_SET_V1

#ifdef PINS_SET_V1
    #define BUTTON_PIN   0      //d3
    #define RELAY1_PIN    5   //d1  
    #define SONOFF_LED      2  //d4
    #define IN_WIRE_BUS    14  //d5
    #define OUT_WIRE_BUS 4 //D2
    #define ANALOG_A0_PIN A0
    #define RELAY2_PIN D6 //new d6 //old d8
#endif
    
#ifdef PINS_SET_V2
    #define BUTTON_PIN   0      //d3
    #define RELAY1_PIN    12   //d6  
    #define SONOFF_LED      2  //d4
    #define IN_WIRE_BUS    14  //d5
    #define OUT_WIRE_BUS 13 //D7
    #define ANALOG_A0_PIN A0
    #define RELAY2_PIN D6 //new d6 //old d8
#endif

//************************** PINS **********************************************


// global settings

#define DALLAS_RES_IN 9
#define DALLAS_RES_OUT 10

#define AUTO_TEMP_ONOFF 1
#define BUTTON_DELAY 50
#define REVERSE_RELAY 0


/********** blink ****************/


#define BL_CONNECTED_ON 0
#define BL_CONNECTED_OFF 1
#define BL_CONNECTING 2
#define BL_CONFIG 2
#define BL_OFFLINE_ON 5
#define BL_OFFLINE_OFF 4



const uint8_t modes[] = {
  0B00000000, //Светодиод выключен
  0B11111111, //Горит постоянно
  0B00001111, //Мигание по 0.5 сек
  0B00000001, //Короткая вспышка раз в секунду
  0B00000101, //Две короткие вспышки раз в секунду
  0B00010101, //Три короткие вспышки раз в секунду
  0B01010101,  //Частые короткие вспышки (4 раза в секунду)
  0B11111110
};

#endif

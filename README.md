# donoff-blynk-learning

Simple sketch for donoff 

Sketch needs some libraries:

- OneWire: https://github.com/PaulStoffregen/OneWire

- DallasTemperature: https://github.com/milesburton/Arduino-Temperature-Control-Library

- Blynk https://github.com/blynkkk/blynk-library/releases

- Bounce2 https://github.com/thomasfredericks/Bounce2


Platform donoff uses pins from dsettings.h (there are teo preset templates) for varios  platform versions

This sketch uses blynk virtual pins: 

Buttons

V2 - switch button
v5 - on button
V6 - off button

Temperatures

V8 - tempurature of external  ds1820
V11 - tempurature of ds1820 on board

Informers

V19 - uptime
V12 - "on" time
V6 - virtual LED
V18 - tic tac LED
V20 - current time 
V21 - current date 

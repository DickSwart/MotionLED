#pragma once
#ifndef SwartNinjaLed_H
#define SwartNinjaLed_H

#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino

class SwartNinjaLed
{
public:
  SwartNinjaLed(int pin);
  void init(void);

  bool getState(void);
  bool setState(bool state);

  uint8_t getBrightness(void);
  bool setBrightness(uint8_t brightness);

  uint16_t getColorTemperature(void);
  bool setColorTemperature(uint16_t colorTemperature);

private:
  int _pin;
  bool _state;
  uint8_t _brightness;
  void _updateLED(void);
};

#endif
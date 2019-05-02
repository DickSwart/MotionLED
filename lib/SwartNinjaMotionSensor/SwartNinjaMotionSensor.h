#ifndef SwartNinjaMotionSensor_H
#define SwartNinjaMotionSensor_H

#include "Arduino.h"

class SwartNinjaMotionSensor
{
public:
  SwartNinjaMotionSensor(int pin, void (*callback)(void));
  void init(void);
  void loop(void);
  bool getState(void);

private:
  int _pin;
  bool _currentState = false;
  void (*_callback)(void);
  void _motionSensorISR(void);
  void _handleEvent(void);
  bool _readMotionState(void);
};
#endif

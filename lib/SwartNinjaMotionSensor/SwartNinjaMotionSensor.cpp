//include the declaration for this class
#include "SwartNinjaMotionSensor.h"

bool SwartNinjaMotionSensorChanged = false;

//  ISRs
void motionSensorISR(void)
{
  SwartNinjaMotionSensorChanged = true;
}

///////////////////////////////////////////////////////////////////////////
//  Constructor, init() & loop()
///////////////////////////////////////////////////////////////////////////
SwartNinjaMotionSensor::SwartNinjaMotionSensor(int pin, void (*callback)(void))
{
  this->_pin = pin;
  this->_callback = callback;
}

void SwartNinjaMotionSensor::init(void)
{
  pinMode(this->_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(this->_pin), motionSensorISR, CHANGE);
  this->_currentState = this->_readMotionState();
}

void SwartNinjaMotionSensor::loop(void)
{
  this->_handleEvent();
}

///////////////////////////////////////////////////////////////////////////
//  Getters
///////////////////////////////////////////////////////////////////////////

bool SwartNinjaMotionSensor::getState(void)
{
  return this->_currentState;
}

///////////////////////////////////////////////////////////////////////////
//  PRIVATE METHODS
///////////////////////////////////////////////////////////////////////////



// read the pin
bool SwartNinjaMotionSensor::_readMotionState(void)
{
  return digitalRead(this->_pin);
}

// handle event
void SwartNinjaMotionSensor::_handleEvent(void)
{
  if (SwartNinjaMotionSensorChanged)
  {
    if (_readMotionState() != this->_currentState)
    {
      this->_currentState = !this->_currentState;
      this->_callback();
    }
    SwartNinjaMotionSensorChanged = false;
  }
}
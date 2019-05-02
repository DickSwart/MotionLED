#include "SwartNinjaLed.h"

///////////////////////////////////////////////////////////////////////////
//   CONSTRUCTOR, INIT()
///////////////////////////////////////////////////////////////////////////
SwartNinjaLed::SwartNinjaLed(int pin)
{
  this->_pin = pin;
}

void SwartNinjaLed::init(void)
{

  analogWriteRange(255);

  pinMode(this->_pin, OUTPUT);

  // set initial state
  this->_state = false;
  this->_brightness = 255;
}

///////////////////////////////////////////////////////////////////////////
//   STATE
///////////////////////////////////////////////////////////////////////////
bool SwartNinjaLed::getState(void)
{
  return this->_state;
}

bool SwartNinjaLed::setState(bool state)
{
  // checks if the given state is different from the actual state
  if (state == this->_state)
    return false;

  // saves the new state value
  this->_state = state;

  // update the LED
  this->_updateLED();

  return true;
}

///////////////////////////////////////////////////////////////////////////
//   BRIGHTNESS
///////////////////////////////////////////////////////////////////////////
uint8_t SwartNinjaLed::getBrightness(void)
{
  return this->_brightness;
}

bool SwartNinjaLed::setBrightness(uint8_t brightness)
{
  // checks if the value is smaller, bigger or equal to the actual brightness value
  if (brightness < 0 || brightness > 255 || brightness == _brightness)
    return false;

  // saves the new brightness value
  this->_brightness = brightness;

  // update the LED
  this->_updateLED();

  return true;
}

///////////////////////////////////////////////////////////////////////////
//   PRIVATE METHODS
///////////////////////////////////////////////////////////////////////////

void SwartNinjaLed::_updateLED()
{
  if (this->_state)
  {
    analogWrite(this->_pin, this->_brightness);
  }
  else
  {
    analogWrite(this->_pin, 0);
  }
}

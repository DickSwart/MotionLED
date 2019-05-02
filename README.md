# MotionLED

A NodeMCU ESP8266 project for controlling 12V LED strip. I added a motion sensor to the mix as I had one. This project uses MQTT to communicat with Home Assistant, I used this project for my kitchen cupboards.

**Implemented Features:**

- MQTT to turn on/off LED strip
- MQTT to set the brightness of the LED strip
- PIR sensor that publish MQTT motion events.
- Over-the-Air (OTA) Upload from the ArduinoIDE!

## Parts

I'm using the following parts for this project

- NodeMCU ESP8266
- IRF540N: N-Channel Mosfet
- AM312 PIR motion sensor
- 12V Power Supply
- DC-DC Buck Converter
- Logic Level Shifter
- 12V White LED Strip
- Some 2.1mm DC Jacks and sockets


## Wiring Diagram
Download [fritzing file](https://github.com/DickSwart/MotionLED/blob/master/assets/MotionLED.fzz)
![alt text](https://github.com/DickSwart/MotionLED/blob/master/assets/MotionLED-bb.jpg?raw=true "Wiring Diagram")

## Home Assistant YAML

Example of Home Assistan "configuration.yaml" file.

```yaml
light:
  - platform: mqtt
    name: 'kitchen_cupboards_leds'
    state_topic: '12345/light/state'
    command_topic: '12345/light/set'
    brightness_state_topic: '4C86CF/light/brightness'
    brightness_command_topic: '4C86CF/light/brightness/set'
    optimistic: false
    qos: 1
    retain: false

sensor:
  - platform: mqtt
    name: 'kitchen_cupboards_wifi'
    state_topic: '12345/sensor/wifi'
    unit_of_measurement: '%'
    availability_topic: '12345/status'
    payload_available: 'online'
    payload_not_available: 'offline'

binary_sensor:
  - platform: mqtt
    name: 'kitchen_cupboards_motion'
    state_topic: '12345/sensor/motion'
    availability_topic: '12345/status'
    device_class: motion
```

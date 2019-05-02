#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#include <SwartNinjaLed.h>
#include <SwartNinjaMotionSensor.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"
#include "user_config.h" // Fixed user configurable options
#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h" // Configuration overrides for my_user_config.h
#endif

/* -------------------------------------------------
 *  WiFi
 * ------------------------------------------------- */
// variables declaration
int previousWiFiSignalStrength = -1;
unsigned long previousMillis = 0;
int reqConnect = 0;
int isConnected = 0;
const long interval = 500;
const long reqConnectNum = 15; // number of intervals to wait for connection
WiFiEventHandler mConnectHandler;
WiFiEventHandler mDisConnectHandler;
WiFiEventHandler mGotIpHandler;

// function declaration
void setupWiFi(void);
void connectWiFi(void);
void onConnected(const WiFiEventStationModeConnected &event);
void onDisconnect(const WiFiEventStationModeDisconnected &event);
void onGotIP(const WiFiEventStationModeGotIP &event);
void loopWiFiSensor(void);
int getWiFiSignalStrength(void);

// Initialize the Ethernet client object
WiFiClient wifiClient;

/* -------------------------------------------------
 *  OTA
 * ------------------------------------------------- */

// function declaration
void setupOTA();
void handleOTA();

/* -------------------------------------------------
 *  MQTT
 * ------------------------------------------------- */
// function declaration
void setupMQTT();
void connectToMQTT();
void callback(char *topic, byte *payload, unsigned int length);
void subscribeToMQTT(char *p_topic);
void publishToMQTT(char *p_topic, char *p_payload);

// variables declaration
char charPayload[50];
volatile unsigned long lastMQTTConnection = 0;
char MQTT_CLIENT_ID[7] = {0};
char MQTT_PAYLOAD[8] = {0};
char MQTT_AVAILABILITY_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_AVAILABILITY_TOPIC_TEMPLATE) - 2] = {0};
char MQTT_WIFI_QUALITY_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(WIFI_QUALITY_SENSOR_NAME) - 4] = {0};
char MQTT_LIGHT_STATE_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_LIGHT_STATE_TOPIC_TEMPLATE) - 2] = {0};
char MQTT_LIGHT_COMMAND_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_LIGHT_COMMAND_TOPIC_TEMPLATE) - 2] = {0};
char MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_LIGHT_BRIGHTNESS_STATE_TEMPLATE) - 2] = {0};
char MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_LIGHT_BRIGHTNESS_COMMAND_TEMPLATE) - 2] = {0};
char MQTT_MOTION_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(MOTION_SENSOR_NAME) - 4] = {0};

// Initialize the mqtt client object
PubSubClient mqttClient(wifiClient);

/* -------------------------------------------------
 *  LED STRIP
 * ------------------------------------------------- */
// function declaration
void publishLightBrightness(void);
void publishLightState(void);

// variables declaration
bool stripStateChanged = false;

// Initialize the LED object
SwartNinjaLed strip(LED_PIN);

/* -------------------------------------------------
 *  MOTION SENSOR
 * ------------------------------------------------- */
// function declaration
void publishMotionSensorState(void);

// Initialize the motion sensor object
SwartNinjaMotionSensor motionSensor(MOTION_SENSOR_PIN, publishMotionSensorState);

//=======================================================================
//                    Power on setup
//=======================================================================
void setup()
{
  Serial.begin(115200);

  // WIFI
  Serial.println("[SETUP]: WIFI");
  setupWiFi();

  // MQTT
  Serial.println("[SETUP]: MQTT");
  setupMQTT();

  // OVER-THE-AIR
  Serial.println("[SETUP]: OVER-THE-AIR");
  setupOTA();

  // LED STRIP
  Serial.println("[SETUP]: LED");
  strip.init();

  // MOTION SENSOR
  Serial.println("[SETUP]: MOTION SENSOR");
  motionSensor.init();
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop()
{
  // WIFI
  connectWiFi();

  // Code will only run if connected to WiFi
  if (isConnected == 2)
  {
    // MQTT
    if (!mqttClient.connected())
    {
      connectToMQTT();
    }
    mqttClient.loop();

    // ATO
    handleOTA();

    // Check WiFi signal
    loopWiFiSensor();

    // MOTION SENSOR
    motionSensor.loop();
  }
}

///////////////////////////////////////////////////////////////////////////
//   WiFi
///////////////////////////////////////////////////////////////////////////

/*
 * Function called to setup WiFi module
 */
void setupWiFi(void)
{
  WiFi.disconnect();
  WiFi.persistent(false);
  mConnectHandler = WiFi.onStationModeConnected(onConnected);
  mDisConnectHandler = WiFi.onStationModeDisconnected(onDisconnect);
  mGotIpHandler = WiFi.onStationModeGotIP(onGotIP);
}

/*
 * Function called to connect to WiFi
 */
void connectWiFi(void)
{
  if (WiFi.status() != WL_CONNECTED && reqConnect > reqConnectNum && isConnected < 2)
  {
    reqConnect = 0;
    isConnected = 0;
    WiFi.disconnect();

    Serial.println();
    Serial.print("[WIFI]: Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("[WIFI]: Connecting...");
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    reqConnect++;
  }
}

/*
 * Function called to handle WiFi events
 */
void onConnected(const WiFiEventStationModeConnected &event)
{
  char macAdddress[20];
  sprintf(macAdddress, "%02X:%02X:%02X:%02X:%02X:%02X", event.bssid[5], event.bssid[4], event.bssid[3], event.bssid[2], event.bssid[1], event.bssid[0]);
  Serial.print(F("[WIFI]: You're connected to the AP. (MAC - "));
  Serial.print(macAdddress);
  Serial.println(")");
  isConnected = 1;
}

void onDisconnect(const WiFiEventStationModeDisconnected &event)
{
  Serial.println("[WIFI]: Disconnected");
  Serial.print("[WIFI]: Reason: ");
  Serial.println(event.reason);
  isConnected = 0;
}

void onGotIP(const WiFiEventStationModeGotIP &event)
{
  Serial.print("[WIFI]: IP Address : ");
  Serial.println(event.ip);
  Serial.print("[WIFI]: Subnet     : ");
  Serial.println(event.mask);
  Serial.print("[WIFI]: Gateway    : ");
  Serial.println(event.gw);

  isConnected = 2;
}

/*
 * Function to check WiFi signal strength
 */
void loopWiFiSensor(void)
{
  static unsigned long lastWiFiQualityMeasure = 0;
  if (lastWiFiQualityMeasure + WIFI_QUALITY_INTERVAL <= millis() || previousWiFiSignalStrength == -1)
  {
    lastWiFiQualityMeasure = millis();
    int currentWiFiSignalStrength = getWiFiSignalStrength();
    if (isnan(previousWiFiSignalStrength) || currentWiFiSignalStrength <= previousWiFiSignalStrength - WIFI_QUALITY_OFFSET_VALUE || currentWiFiSignalStrength >= previousWiFiSignalStrength + WIFI_QUALITY_OFFSET_VALUE)
    {
      previousWiFiSignalStrength = currentWiFiSignalStrength;
      dtostrf(currentWiFiSignalStrength, 2, 2, MQTT_PAYLOAD);
      publishToMQTT(MQTT_WIFI_QUALITY_TOPIC, MQTT_PAYLOAD);
    }
  }
}

/*
 * Helper function to get the current WiFi signal strength
 */
int getWiFiSignalStrength(void)
{
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

///////////////////////////////////////////////////////////////////////////
//   OTA
///////////////////////////////////////////////////////////////////////////

/*
   Function called to setup OTA updates
*/
void setupOTA()
{
  ArduinoOTA.setPort(OTA_PORT);
  Serial.print(F("[AOT]: OTA port sets to: "));
  Serial.println(OTA_PORT);

  ArduinoOTA.onStart([]() {
    Serial.println(F("[AOT]: OTA starts"));
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("[AOT]: OTA ends"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.print(F("[AOT]: OTA progresses: "));
    Serial.print(progress / (total / 100));
    Serial.println(F("%"));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.print(F("[AOT]: ERROR"));
    Serial.println(error);
    if (error == OTA_AUTH_ERROR)
      Serial.println(F("[AOT]: ERROR - Auth failed"));
    else if (error == OTA_BEGIN_ERROR)
      Serial.println(F("[AOT]: ERROR - Begin failed"));
    else if (error == OTA_CONNECT_ERROR)
      Serial.println(F("[AOT]: ERROR - Connect failed"));
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println(F("[AOT]: ERROR - Receive failed"));
    else if (error == OTA_END_ERROR)
      Serial.println(F("[AOT]: ERROR - End failed"));
  });
  ArduinoOTA.begin();
}

/*
   Function called to handle OTA updates
*/
void handleOTA()
{
  ArduinoOTA.handle();
}

///////////////////////////////////////////////////////////////////////////
//   MQTT
///////////////////////////////////////////////////////////////////////////

/*
 * Function called to setup MQTT topics
 */
void setupMQTT()
{
  sprintf(MQTT_CLIENT_ID, "%06X", ESP.getChipId());
  sprintf(MQTT_AVAILABILITY_TOPIC, MQTT_AVAILABILITY_TOPIC_TEMPLATE, MQTT_CLIENT_ID);

  Serial.print(F("[MQTT]: Availability topic: "));
  Serial.println(MQTT_AVAILABILITY_TOPIC);

  sprintf(MQTT_WIFI_QUALITY_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, WIFI_QUALITY_SENSOR_NAME);
  Serial.print(F("[MQTT]: WiFi Quality topic: "));
  Serial.println(MQTT_WIFI_QUALITY_TOPIC);

  sprintf(MQTT_LIGHT_STATE_TOPIC, MQTT_LIGHT_STATE_TOPIC_TEMPLATE, MQTT_CLIENT_ID);
  Serial.print(F("[MQTT]: Light state topic: "));
  Serial.println(MQTT_LIGHT_STATE_TOPIC);

  sprintf(MQTT_LIGHT_COMMAND_TOPIC, MQTT_LIGHT_COMMAND_TOPIC_TEMPLATE, MQTT_CLIENT_ID);
  Serial.print(F("[MQTT]: Light command topic: "));
  Serial.println(MQTT_LIGHT_COMMAND_TOPIC);

  sprintf(MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC, MQTT_LIGHT_BRIGHTNESS_STATE_TEMPLATE, MQTT_CLIENT_ID);
  Serial.print(F("[MQTT]: Light brightness state topic: "));
  Serial.println(MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC);

  sprintf(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC, MQTT_LIGHT_BRIGHTNESS_COMMAND_TEMPLATE, MQTT_CLIENT_ID);
  Serial.print(F("[MQTT]: Light brightness command topic: "));
  Serial.println(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC);

  sprintf(MQTT_MOTION_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, MOTION_SENSOR_NAME);
  Serial.print(F("[MQTT]: Motion sensor topic: "));
  Serial.println(MQTT_MOTION_SENSOR_TOPIC);

  mqttClient.setCallback(callback);
  mqttClient.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
}

/*
  Function called to connect/reconnect to the MQTT broker
*/
void connectToMQTT()
{
  // Loop until we're connected / reconnected
  while (!mqttClient.connected())
  {
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_AVAILABILITY_TOPIC, 0, 1, "offline"))
    {
      Serial.println(F("[MQTT]: The client is successfully connected to the MQTT broker"));
      publishToMQTT(MQTT_AVAILABILITY_TOPIC, "online");

      // Once connected, publish an announcement...
      // publish the initial values
      publishLightState();
      publishLightBrightness();
      publishMotionSensorState();

      // ... and resubscribe
      subscribeToMQTT(MQTT_LIGHT_COMMAND_TOPIC);
      subscribeToMQTT(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC);
    }
    else
    {
      Serial.println(F("[MQTT]: ERROR - The connection to the MQTT broker failed"));
      Serial.print(F("[MQTT]: MQTT username: "));
      Serial.println(MQTT_USERNAME);
      Serial.print(F("[MQTT]: MQTT password: "));
      Serial.println(MQTT_PASSWORD);
      Serial.print(F("[MQTT]: MQTT broker: "));
      Serial.println(MQTT_SERVER);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
  Function called to subscribe to a MQTT topic
*/
void subscribeToMQTT(char *p_topic)
{
  if (mqttClient.subscribe(p_topic))
  {
    Serial.print(F("[MQTT]: subscribeToMQTT - Sending the MQTT subscribe succeeded for topic: "));
    Serial.println(p_topic);
  }
  else
  {
    Serial.print(F("[MQTT]: subscribeToMQTT - ERROR, Sending the MQTT subscribe failed for topic: "));
    Serial.println(p_topic);
  }
}

/*
  Function called to publish to a MQTT topic with the given payload
*/
void publishToMQTT(char *p_topic, char *p_payload)
{
  if (mqttClient.publish(p_topic, p_payload, true))
  {
    Serial.print(F("[MQTT]: publishToMQTT - MQTT message published successfully, topic: "));
    Serial.print(p_topic);
    Serial.print(F(", payload: "));
    Serial.println(p_payload);
  }
  else
  {
    Serial.println(F("[MQTT]: publishToMQTT - ERROR, MQTT message not published, either connection lost, or message too large. Topic: "));
    Serial.print(p_topic);
    Serial.print(F(" , payload: "));
    Serial.println(p_payload);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("[MQTT]: Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // concat the payload into a string
  String strPayload;
  for (uint8_t i = 0; i < length; i++)
  {
    strPayload.concat((char)payload[i]);
  }
  // handle message topic
  if (String(MQTT_LIGHT_COMMAND_TOPIC).equals(topic))
  {
    // test if the payload is equal to "ON" or "OFF"
    if (strip.setState(strPayload.equals(String(MQTT_PAYLOAD_ON))))
    {
      publishLightState();
    }
  }
  else if (String(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC).equals(topic))
  {
    // test if the payload is equal to "ON" or "OFF"
    if (strip.setBrightness(strPayload.toInt()))
    {
      publishLightBrightness();
    }
  }
}

/* -------------------------------------------------
 *  LED STRIP
 * ------------------------------------------------- */
void publishLightState(void)
{
  if (strip.getState())
  {
    publishToMQTT(MQTT_LIGHT_STATE_TOPIC, MQTT_PAYLOAD_ON);
  }
  else
  {
    publishToMQTT(MQTT_LIGHT_STATE_TOPIC, MQTT_PAYLOAD_OFF);
  }
}

void publishLightBrightness(void)
{
  char brightnessValue[20];
  snprintf(brightnessValue, 20, "%d", strip.getBrightness());
  publishToMQTT(MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC, brightnessValue);
}

/* -------------------------------------------------
 *  MOTION SENSOR
 * ------------------------------------------------- */
void publishMotionSensorState(void)
{
  if (motionSensor.getState())
  {
    publishToMQTT(MQTT_MOTION_SENSOR_TOPIC, MQTT_PAYLOAD_ON);
  }
  else
  {
    publishToMQTT(MQTT_MOTION_SENSOR_TOPIC, MQTT_PAYLOAD_OFF);
  }
}
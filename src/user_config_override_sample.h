#ifndef _USER_CONFIG_OVERRIDE_H_
#define _USER_CONFIG_OVERRIDE_H_

// Make a copy of this file and rename it to "user_config_override.h"
// Any values in the "user_config.h" file can be overridden here, this file will not be checked in.

// Force the compiler to show a warning to confirm that this file is inlcuded
#warning **** user_config_override.h: Using Settings from this File ****

///////////////////////////////////////////////////////////////////////////
//   WIFI
///////////////////////////////////////////////////////////////////////////
#undef  WIFI_SSID
#define WIFI_SSID "MY_WIFI_SSID"

#undef  WIFI_PASSWORD
#define WIFI_PASSWORD "MY_WIFI_PASSWORD"

///////////////////////////////////////////////////////////////////////////
//   MQTT
///////////////////////////////////////////////////////////////////////////
#undef  MQTT_SERVER
#define MQTT_SERVER "XXX.XXX.XXX.XXX" // MQTT server IP address

#undef  MQTT_USERNAME
#define MQTT_USERNAME "mqtt_user"     // MQTT username

#undef  MQTT_PASSWORD
#define MQTT_PASSWORD "bdAXq66X*m8UV" // MQTT password


#endif  // _USER_CONFIG_OVERRIDE_H_
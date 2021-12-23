#pragma once
#ifndef MQTT_OPENTHERM_H
#define MQTT_OPENTHERM_H

//#include <ESP8266WiFi.h>
//#include <Ticker.h>
#include <AsyncMqttClient.h>
//#include "SuperSecretSettings.h"

void mqtt_setup();
//void onMqttDisconnect();
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
extern AsyncMqttClient mqttClient;
void connectToWifi();
void _resubscribe();

#define HELLO_TOPIC BASE_TOPIC "online"
#define IP_TOPIC BASE_TOPIC "ip"
#define SPEED_SETTING_TOPIC BASE_TOPIC "fan_setpoint_percent"
#define ERROR_MSG_TOPIC BASE_TOPIC "error/message"
#define ERROR_VAL_TOPIC BASE_TOPIC "error/value"

#endif

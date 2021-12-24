#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include "mqtt.h"
#include "SuperSecretSettings.h"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

// Connect to the WiFi network
void connectToWifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID_2, PASSWORD_2);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
}

// Connect to the MQTT broker - expand function to do anything 'on connect'
void connectToMqtt()
{
    mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
    connectToMqtt();
    mqttClient.publish(IP_TOPIC, 2, true, event.ip.toString().c_str());
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
    //   Serial.println("Disconnected from Wi-Fi.");
    mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent)
{
    _resubscribe();
    mqttClient.publish(HELLO_TOPIC, 0, true, "Fan controller online");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{

    if (WiFi.isConnected())
    {
        mqttReconnectTimer.once(2, connectToMqtt);
    }
}

void mqtt_setup()
{

    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    //   mqttClient.onSubscribe(onMqttSubscribe);
    //   mqttClient.onUnsubscribe(onMqttUnsubscribe);
    //   mqttClient.onMessage(onMqttMessage);
    //   mqttClient.onPublish(onMqttPublish);
    mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void _resubscribe()
{
    mqttClient.subscribe(SPEED_SETTING_TOPIC, 2);
}

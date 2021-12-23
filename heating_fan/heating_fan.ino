#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiClient.h>

#include "SuperSecretSettings.h" // File containing const char*'s for SSID & PASSWORD & KEY
#include "mqtt.h"

#define TX 1
#define RX 3

#define GPIO2 2
#define RELAY_ENABLE TX
#define GPIO0 0

#define TESTMODE 0

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

#define MODE_LISTEN_MASTER 0
#define MODE_LISTEN_SLAVE 1
uint_least8_t mode = MODE_LISTEN_MASTER;

bool openthermActive = true;

byte CH_SETPOINT_HB_OVERRIDE = 0;
// const char *CH_SETPOINT_OVERRIDE_TOPIC = "iot/boiler/overrides/CH_SETPOINT";

#if TESTMODE
WiFiServer telnet_server(999);
WiFiClient client;
#endif TESTMODE

void setup()
{
    delay(200);
    setup_pins();

    // from mqtt_opentherm.h
    mqtt_setup();
    connectToWifi();

    //Updater
    httpUpdater.setup(&server, OTAUSER, OTAPASSWORD);
    server.on("/", redirectToUpdate);
    server.on("/set", fanSetSpeedViaWeb);
    server.begin();

#if TESTMODE
    telnet_server.begin();
#endif TESTMODE
}

void loop()
{
    server.handleClient();
#if TESTMODE == 1
// ...
#else

    // just waiting for a mqtt message...
    // Maybe do something with light sleep for 30 seconds?

#endif TESTMODE
}

void setup_pins()
{
    pinMode(RELAY_ENABLE, FUNCTION_3);
    pinMode(RELAY_ENABLE, OUTPUT);

    pinMode(GPIO0, OUTPUT);
    analogWriteFreq(25000); // 25 kHz

    fanOff();
}

// Switch off the fan
void fanOff()
{
    digitalWrite(RELAY_ENABLE, LOW);
    analogWrite(GPIO0, 0);
}

void fanSetSpeedViaWeb()
{
    if (!server.hasArg("fanSpeed"))
    { //Parameter not found

        mqttClient.publish(ERROR_MSG_TOPIC, 2, false, "argument Fanspeed not found");
        mqttClient.publish(ERROR_VAL_TOPIC, 2, true, "-1");
    }
    else
    { //Parameter found
        int speed = atoi(server.arg("fanSpeed").c_str());
    }
}

// Switch on the fan at a certain percentage
void fanSetSpeed(int speed, bool isPercentSpeed = true)
{
    if (speed > 255 || (speed > 100 && isPercentSpeed) || speed < 0)
    {
        mqttClient.publish(ERROR_MSG_TOPIC, 2, true, "Error, not a legal fan speed");
        char out[8];
        itoa(speed, out, 10);
        mqttClient.publish(ERROR_VAL_TOPIC, 2, true, out);
        return;
    }
    if (speed == 0)
    {
        fanOff();
        return;
    }
    if (isPercentSpeed)
    {
        speed = percentToAnalogWriteRange(speed);
    }
    digitalWrite(RELAY_ENABLE, HIGH);
    analogWrite(GPIO0, speed);
}

int percentToAnalogWriteRange(int percentInput)
{
    return percentInput * 255 / 100;
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    // mqttClient.publish("iot/boiler/overrideTest_2", 0, true, payload);
    if (strcmp(SPEED_SETTING_TOPIC, topic) == 0)
    {
        int fanSetpointPercent = atoi(payload);
        fanSetSpeed(fanSetpointPercent);
    }
}

void redirectToUpdate()
{
    //   TRACE("Redirect...");
    String url = "/update";

    server.sendHeader("Location", url, true);
    server.send(302);
}

#if TESTMODE
void test_loop()
{
}

#endif

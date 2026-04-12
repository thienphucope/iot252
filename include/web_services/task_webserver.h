
#ifndef __TASK_WEBSERVER_H__
#define __TASK_WEBSERVER_H__

#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include "web_services/task_handler.h"

extern AsyncWebServer server;
extern AsyncWebSocket ws;

void Webserver_stop();
void Webserver_reconnect();
void Webserver_sendata(String data);

#endif
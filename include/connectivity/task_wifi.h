#ifndef __TASK_WIFI_H__
#define __TASK_WIFI_H__

#include <WiFi.h>
#include "connectivity/task_check_info.h"
#include "web_services/task_webserver.h"

extern bool Wifi_reconnect();
extern void startAP();

#endif
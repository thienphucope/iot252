#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// Threshold constants
#define TEMP_NORMAL_LOW       25.0f
#define TEMP_NORMAL_HIGH      30.0f
#define HUMI_NORMAL_LOW       40.0f
#define HUMI_NORMAL_HIGH      60.0f

// Task 3: Định nghĩa cấu trúc dữ liệu
struct SensorData {
    float temperature;
    float humidity;
};

// Queue để truyền dữ liệu sensor giữa các Task
extern QueueHandle_t xSensorQueue;

// Temperature levels: Low (<25), Normal (25-35), High (>=35)
extern SemaphoreHandle_t xTempLowSemaphore;
extern SemaphoreHandle_t xTempNormalSemaphore;
extern SemaphoreHandle_t xTempHighSemaphore;

// Humidity levels: Low (<40), Normal (40-70), High (>70)
extern SemaphoreHandle_t xHumiLowSemaphore;
extern SemaphoreHandle_t xHumiNormalSemaphore;
extern SemaphoreHandle_t xHumiHighSemaphore;

// LCD Display Status (combined temp + humidity): Normal, Warning, Critical
extern SemaphoreHandle_t xStatusNormalSemaphore;
extern SemaphoreHandle_t xStatusWarningSemaphore;
extern SemaphoreHandle_t xStatusCriticalSemaphore;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern String ssid;
extern String password;
extern String wifi_ssid;
extern String wifi_password;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;
#endif
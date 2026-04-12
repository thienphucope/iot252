#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// Task 3: Định nghĩa cấu trúc dữ liệu
struct SensorData {
    float temperature;
    float humidity;
};

// Task 3: Cấu trúc dữ liệu và Mutex để chia sẻ giữa nhiều Task
extern struct SensorData sharedSensorData;
extern SemaphoreHandle_t xSensorMutex;

// Task 3: Khai báo Queue (vẫn giữ để truyền sự kiện nếu cần)
extern QueueHandle_t xSensorQueue;

// Task 3: Khai báo các Semaphore cho trạng thái LCD
extern SemaphoreHandle_t xNormalSemaphore;
extern SemaphoreHandle_t xWarningSemaphore;
extern SemaphoreHandle_t xCriticalSemaphore;

// Task 2: Semaphore cho NeoPixel theo độ ẩm
extern SemaphoreHandle_t xHumiHighSemaphore;
extern SemaphoreHandle_t xHumiLowSemaphore;

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
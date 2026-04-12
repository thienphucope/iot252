#include "global.h"

// Task 3: Biến cho Queue, Mutex và Shared Data
QueueHandle_t xSensorQueue = NULL;
SemaphoreHandle_t xSensorMutex = NULL;
struct SensorData sharedSensorData = {0.0, 0.0};

// Các Semaphore trạng thái LCD
SemaphoreHandle_t xNormalSemaphore = NULL;
SemaphoreHandle_t xWarningSemaphore = NULL;
SemaphoreHandle_t xCriticalSemaphore = NULL;

// Các Semaphore độ ẩm (NeoPixel)
SemaphoreHandle_t xHumiHighSemaphore = NULL;
SemaphoreHandle_t xHumiLowSemaphore = NULL;

// Các biến cấu hình khác
String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid = "abcde";
String wifi_password = "123456789";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

// Hàm khởi tạo các đối tượng RTOS
void init_globals() {
    if (xSensorQueue == NULL) {
        xSensorQueue = xQueueCreate(5, sizeof(struct SensorData));
    }
    
    // Khởi tạo Mutex (Task 3: Resource protection)
    if (xSensorMutex == NULL) {
        xSensorMutex = xSemaphoreCreateMutex();
    }
    
    // Khởi tạo các Semaphore trạng thái
    if (xNormalSemaphore == NULL) xNormalSemaphore = xSemaphoreCreateBinary();
    if (xWarningSemaphore == NULL) xWarningSemaphore = xSemaphoreCreateBinary();
    if (xCriticalSemaphore == NULL) xCriticalSemaphore = xSemaphoreCreateBinary();
    
    if (xHumiHighSemaphore == NULL) xHumiHighSemaphore = xSemaphoreCreateBinary();
    if (xHumiLowSemaphore == NULL) xHumiLowSemaphore = xSemaphoreCreateBinary();

    // Mặc định cho trạng thái Normal
    xSemaphoreGive(xNormalSemaphore);
}
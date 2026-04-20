#include "global.h"

// Queue để truyền dữ liệu sensor
QueueHandle_t xSensorQueue = NULL;

// Các Semaphore trạng thái LCD
SemaphoreHandle_t xTempLowSemaphore = NULL;
SemaphoreHandle_t xTempNormalSemaphore = NULL;
SemaphoreHandle_t xTempHighSemaphore = NULL;

SemaphoreHandle_t xHumiLowSemaphore = NULL;
SemaphoreHandle_t xHumiNormalSemaphore = NULL;
SemaphoreHandle_t xHumiHighSemaphore = NULL;

SemaphoreHandle_t xStatusNormalSemaphore = NULL;
SemaphoreHandle_t xStatusWarningSemaphore = NULL;
SemaphoreHandle_t xStatusCriticalSemaphore = NULL;

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
    
    // Khởi tạo các Semaphore trạng thái
    if (xTempLowSemaphore == NULL) xTempLowSemaphore = xSemaphoreCreateBinary();
    if (xTempNormalSemaphore == NULL) xTempNormalSemaphore = xSemaphoreCreateBinary();
    if (xTempHighSemaphore == NULL) xTempHighSemaphore = xSemaphoreCreateBinary();

    if (xHumiLowSemaphore == NULL) xHumiLowSemaphore = xSemaphoreCreateBinary();
    if (xHumiNormalSemaphore == NULL) xHumiNormalSemaphore = xSemaphoreCreateBinary();
    if (xHumiHighSemaphore == NULL) xHumiHighSemaphore = xSemaphoreCreateBinary();

    if (xStatusNormalSemaphore == NULL) xStatusNormalSemaphore = xSemaphoreCreateBinary();
    if (xStatusWarningSemaphore == NULL) xStatusWarningSemaphore = xSemaphoreCreateBinary();
    if (xStatusCriticalSemaphore == NULL) xStatusCriticalSemaphore = xSemaphoreCreateBinary();

    // Mặc định: Normal status
    xSemaphoreGive(xStatusNormalSemaphore);
}
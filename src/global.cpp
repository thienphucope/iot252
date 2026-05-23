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

// Các biến cấu hình khác (Load_info_File() sẽ ghi đè nếu có file đã lưu)
String WIFI_SSID       = DEFAULT_WIFI_SSID;
String WIFI_PASS       = DEFAULT_WIFI_PASS;
String CORE_IOT_TOKEN  = DEFAULT_COREIOT_TOKEN;
String CORE_IOT_SERVER = DEFAULT_COREIOT_SERVER;
String CORE_IOT_PORT   = DEFAULT_COREIOT_PORT;

String ssid     = SSID_AP;
String password = String(PASS_AP);
String wifi_ssid;
String wifi_password;
String core_iot_token;
String core_iot_server;
String core_iot_port;
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

bool led_blinky_enabled = true;
bool neo_blinky_enabled = true;

// Hàm khởi tạo các đối tượng RTOS
void init_globals() {
    // WIFI_SSID/WIFI_PASS phải bắt đầu rỗng để check_info_File() có thể kích hoạt AP mode
    // khi chưa có credentials. Giá trị sẽ được nạp từ LittleFS bởi Load_info_File().

    if (xSensorQueue == NULL) {
        xSensorQueue = xQueueCreate(1, sizeof(struct SensorData)); // Mailbox: chỉ giữ giá trị mới nhất
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
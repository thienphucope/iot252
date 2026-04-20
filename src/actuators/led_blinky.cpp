#include "actuators/led_blinky.h"
#include "global.h"

void led_blinky(void *pvParameters) {
    pinMode(LED_GPIO, OUTPUT);
    uint32_t blinkInterval = 1000; // Mặc định 1s (NORMAL)

    while (1) {
        // Task 1: Dùng semaphore để xác định tốc độ chớp theo mức nhiệt độ
        if (xSemaphoreTake(xTempHighSemaphore, 0) == pdTRUE) {
            blinkInterval = 200;  // Chớp nhanh: Nóng (> 30°C)
        } else if (xSemaphoreTake(xTempNormalSemaphore, 0) == pdTRUE) {
            blinkInterval = 1000; // Chớp vừa: Bình thường (25-30°C)
        } else if (xSemaphoreTake(xTempLowSemaphore, 0) == pdTRUE) {
            blinkInterval = 2000; // Chớp chậm: Lạnh (< 25°C)
        }

        // Thực hiện chớp LED
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(blinkInterval / portTICK_PERIOD_MS);
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(blinkInterval / portTICK_PERIOD_MS);
    }
}
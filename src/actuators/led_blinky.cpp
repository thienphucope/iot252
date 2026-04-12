#include "actuators/led_blinky.h"
#include "global.h"

void led_blinky(void *pvParameters) {
    pinMode(LED_GPIO, OUTPUT);
    
    struct SensorData receivedData;
    uint32_t blinkInterval = 1000; // Mặc định 1s

    while (1) {
        // Task 3: Nhận dữ liệu từ Queue (không dùng biến toàn cục)
        // Đợi tối đa 100ms để nhận dữ liệu mới, nếu không có thì dùng dữ liệu cũ
        if (xSensorQueue != NULL) {
            if (xQueueReceive(xSensorQueue, &receivedData, (TickType_t)100 / portTICK_PERIOD_MS) == pdTRUE) {
                // Task 1: Thay đổi tốc độ chớp theo 3 mức nhiệt độ
                if (receivedData.temperature < 25.0) {
                    blinkInterval = 2000; // Mức 1: Chớp chậm (Mát)
                } else if (receivedData.temperature >= 25.0 && receivedData.temperature <= 35.0) {
                    blinkInterval = 1000; // Mức 2: Chớp vừa (Bình thường)
                } else {
                    blinkInterval = 200;  // Mức 3: Chớp rất nhanh (Cảnh báo nóng)
                }
            }
        }

        // Thực hiện chớp LED
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(blinkInterval / portTICK_PERIOD_MS);
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(blinkInterval / portTICK_PERIOD_MS);
    }
}
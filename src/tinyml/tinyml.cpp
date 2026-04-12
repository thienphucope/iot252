#include "tinyml/tinyml.h"
#include "global.h"

// Example TinyML logic placeholder
void tiny_ml_task(void *pvParameters) {
    while (1) {
        float temp = 0;
        float humi = 0;

        // Task 3: Đọc dữ liệu an toàn dùng Mutex
        if (xSemaphoreTake(xSensorMutex, (TickType_t)100 / portTICK_PERIOD_MS) == pdTRUE) {
            temp = sharedSensorData.temperature;
            humi = sharedSensorData.humidity;
            xSemaphoreGive(xSensorMutex);
        }

        // Thực hiện logic inference (giả lập)
        // input->data.f[0] = temp;
        // input->data.f[1] = humi;

        Serial.printf("[TinyML] Analyzing: Temp=%.1f, Humi=%.1f\n", temp, humi);
        
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
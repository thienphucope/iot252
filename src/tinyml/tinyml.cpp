#include "tinyml/tinyml.h"
#include "global.h"

// Example TinyML logic placeholder
void tiny_ml_task(void *pvParameters) {
    while (1) {
        struct SensorData sensorData = {0.0, 0.0};

        // Nhận dữ liệu từ Queue (thread-safe)
        if (xSensorQueue != NULL) {
            xQueueReceive(xSensorQueue, &sensorData, 0);
        }

        // Thực hiện logic inference (giả lập)
        // input->data.f[0] = sensorData.temperature;
        // input->data.f[1] = sensorData.humidity;

        Serial.printf("[TinyML] Analyzing: Temp=%.1f, Humi=%.1f\n", sensorData.temperature, sensorData.humidity);
        
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
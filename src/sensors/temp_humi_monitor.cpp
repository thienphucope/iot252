#include "sensors/temp_humi_monitor.h"
#include "global.h"

DHT20 dht20;

void temp_humi_monitor(void *pvParameters) {
    Wire.begin(11, 12);
    dht20.begin();

    float last_temp = -100;
    float last_humi = -100;

    while (1) {
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

        if (!isnan(temperature) && !isnan(humidity)) {
            // Task 3: Cập nhật shared data qua Mutex (An toàn cho nhiều Task đọc)
            if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE) {
                sharedSensorData.temperature = temperature;
                sharedSensorData.humidity = humidity;
                xSemaphoreGive(xSensorMutex);
            }

            // Task 3: Vẫn đẩy vào Queue để kích hoạt các Task chờ (LED, LCD)
            struct SensorData data;
            data.temperature = temperature;
            data.humidity = humidity;
            if (xSensorQueue != NULL) {
                xQueueSend(xSensorQueue, &data, 0);
            }

            // Task 3: Phát tín hiệu Semaphore khi trạng thái thay đổi
            if (temperature < 25.0 && last_temp >= 25.0) {
                xSemaphoreGive(xNormalSemaphore);
            } else if (temperature >= 25.0 && temperature < 35.0 && (last_temp < 25.0 || last_temp >= 35.0)) {
                xSemaphoreGive(xWarningSemaphore);
            } else if (temperature >= 35.0 && last_temp < 35.0) {
                xSemaphoreGive(xCriticalSemaphore);
            }

            // Task 2: Phát tín hiệu Semaphore cho độ ẩm
            if (humidity > 70.0 && last_humi <= 70.0) {
                xSemaphoreGive(xHumiHighSemaphore);
            } else if (humidity < 40.0 && last_humi >= 40.0) {
                xSemaphoreGive(xHumiLowSemaphore);
            }

            last_temp = temperature;
            last_humi = humidity;
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
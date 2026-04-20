#include "sensors/temp_humi_monitor.h"
#include "global.h"

DHT20 dht20;

void temp_humi_init() {
    Wire.begin(11, 12);
    delay(200); // DHT20 cần thời gian khởi động
    bool ok = dht20.begin();
    Serial.printf("[DHT20] begin() = %s, isConnected=%d\n",
                  ok ? "OK" : "FAIL", dht20.isConnected());
}

void temp_humi_monitor(void *pvParameters) {

    float last_temp = -100;
    float last_humi = -100;

    // DHT20 cần tối thiểu 1s giữa begin() và read() đầu tiên
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    while (1) {
        int status = dht20.read();
        // Retry 1 lần nếu bị lỗi LASTREAD (đọc quá nhanh)
        if (status == DHT20_ERROR_LASTREAD) {
            vTaskDelay(1100 / portTICK_PERIOD_MS);
            status = dht20.read();
        }
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();
        Serial.printf("[DHT20] read=%d  T=%.2f  H=%.2f\n", status, temperature, humidity);

        if (status == DHT20_OK && !isnan(temperature) && !isnan(humidity)) {
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
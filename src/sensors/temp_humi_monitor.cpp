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
    int last_status = -1; // -1=init, 0=NORMAL, 1=WARNING, 2=CRITICAL

    vTaskDelay(1500 / portTICK_PERIOD_MS);

    while (1) {
        int status = dht20.read();
        if (status == DHT20_ERROR_LASTREAD) {
            vTaskDelay(1100 / portTICK_PERIOD_MS);
            status = dht20.read();
        }
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();
        Serial.printf("[DHT20] read=%d  T=%.2f  H=%.2f\n", status, temperature, humidity);

        if (status == DHT20_OK && !isnan(temperature) && !isnan(humidity)) {
            // Gửi dữ liệu qua Queue
            struct SensorData data;
            data.temperature = temperature;
            data.humidity = humidity;
            if (xSensorQueue != NULL) {
                xQueueSend(xSensorQueue, &data, 0);
            }

            // Compute combined status (temp + humidity)
            int current_status;
            if (temperature > TEMP_NORMAL_HIGH) {
                current_status = 2; // CRITICAL
            } else if (temperature >= TEMP_NORMAL_LOW || humidity < HUMI_NORMAL_LOW || humidity > HUMI_NORMAL_HIGH) {
                current_status = 1; // WARNING
            } else {
                current_status = 0; // NORMAL
            }

            // Give status semaphore khi status thay đổi
            if (current_status != last_status) {
                if (current_status == 2)      xSemaphoreGive(xStatusCriticalSemaphore);
                else if (current_status == 1) xSemaphoreGive(xStatusWarningSemaphore);
                else                          xSemaphoreGive(xStatusNormalSemaphore);
                last_status = current_status;
            }

            // Tín hiệu semaphore cho LED khi temp thay đổi mức
            if (temperature < TEMP_NORMAL_LOW && last_temp >= TEMP_NORMAL_LOW) {
                xSemaphoreGive(xTempLowSemaphore);
            } else if (temperature >= TEMP_NORMAL_LOW && temperature <= TEMP_NORMAL_HIGH && (last_temp < TEMP_NORMAL_LOW || last_temp > TEMP_NORMAL_HIGH)) {
                xSemaphoreGive(xTempNormalSemaphore);
            } else if (temperature > TEMP_NORMAL_HIGH && last_temp <= TEMP_NORMAL_HIGH) {
                xSemaphoreGive(xTempHighSemaphore);
            }

            // Tín hiệu semaphore cho NeoPixel khi humi thay đổi mức
            if (humidity > HUMI_NORMAL_HIGH && last_humi <= HUMI_NORMAL_HIGH) {
                xSemaphoreGive(xHumiHighSemaphore);
            } else if (humidity >= HUMI_NORMAL_LOW && humidity <= HUMI_NORMAL_HIGH && (last_humi < HUMI_NORMAL_LOW || last_humi > HUMI_NORMAL_HIGH)) {
                xSemaphoreGive(xHumiNormalSemaphore);
            } else if (humidity < HUMI_NORMAL_LOW && last_humi >= HUMI_NORMAL_LOW) {
                xSemaphoreGive(xHumiLowSemaphore);
            }

            last_temp = temperature;
            last_humi = humidity;
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
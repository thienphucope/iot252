#include "cloud/task_core_iot.h"

#define MQTT_TOPIC  "esp/telemetry"

static WiFiClient   wifiClient;
static PubSubClient mqttClient(wifiClient);

void task_core_iot(void *pvParameters)
{
    Serial.println("[CoreIoT] Task started, waiting for internet...");

    while (1) {
        if (xSemaphoreTake(xBinarySemaphoreInternet, pdMS_TO_TICKS(5000)) == pdTRUE) {
            xSemaphoreGive(xBinarySemaphoreInternet);
            Serial.println("[CoreIoT] ✅ Internet ready.");
            break;
        }
        Serial.printf("[CoreIoT] Waiting... WiFi status=%d\n", (int)WiFi.status());
    }

    Serial.printf("[CoreIoT] Server=%s  Port=%s\n", CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.c_str());
    Serial.printf("[CoreIoT] Token=%s\n", CORE_IOT_TOKEN.c_str());

    mqttClient.setServer(CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.toInt());

    TickType_t lastSendTick = xTaskGetTickCount();

    while (1)
    {
        if (!mqttClient.connected())
        {
            Serial.printf("[CoreIoT] Connecting to %s:%d ...\n",
                CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.toInt());

            String clientId = "ESP32_" + WiFi.macAddress();
            // CoreIoT: username = access token, password = empty
            if (mqttClient.connect(clientId.c_str(), CORE_IOT_TOKEN.c_str(), ""))
            {
                Serial.println("[CoreIoT] ✅ Connected!");
            }
            else
            {
                Serial.printf("[CoreIoT] ❌ Failed rc=%d, retry in 5s...\n", mqttClient.state());
                vTaskDelay(5000 / portTICK_PERIOD_MS);
                continue;
            }
        }

        // Gửi telemetry mỗi 10 giây
        if ((xTaskGetTickCount() - lastSendTick) >= pdMS_TO_TICKS(10000))
        {
            lastSendTick = xTaskGetTickCount();

            SensorData sd = {0.0f, 0.0f, 0};
            if (xSensorQueue != NULL) xQueuePeek(xSensorQueue, &sd, 0);

            StaticJsonDocument<128> doc;
            doc["temperature"] = sd.temperature;
            doc["humidity"]    = sd.humidity;
            char payload[128];
            serializeJson(doc, payload, sizeof(payload));

            Serial.printf("[CoreIoT] Publishing → %s : %s\n", MQTT_TOPIC, payload);
            bool ok = mqttClient.publish(MQTT_TOPIC, payload);
            Serial.printf("[CoreIoT] Result: %s\n", ok ? "OK" : "FAIL");
        }

        mqttClient.loop();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

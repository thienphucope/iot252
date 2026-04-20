#include "cloud/task_core_iot.h"
#include "global.h" // Thêm global.h

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

constexpr char LED_STATE_ATTR[] = "ledState";

void processSharedAttributes(const Shared_Attribute_Data &data)
{
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        // Xử lý các thuộc tính chia sẻ từ server ở đây
    }
}

RPC_Response setLedSwitchValue(const RPC_Data &data)
{
    Serial.println("Received Switch state from Cloud");
    bool newState = data;
    Serial.print("Cloud LED command: ");
    Serial.println(newState ? "ON" : "OFF");
    return RPC_Response("setLedSwitchValue", newState);
}

const std::array<RPC_Callback, 1U> callbacks = {
    RPC_Callback{"setLedSwitchValue", setLedSwitchValue}};

// Sử dụng đúng kiểu dữ liệu và thứ tự tham số cho ThingsBoard Callback
const Shared_Attribute_Callback attributes_callback(processSharedAttributes, std::vector<const char*>{LED_STATE_ATTR});
const Attribute_Request_Callback attribute_shared_request_callback(processSharedAttributes, std::vector<const char*>{LED_STATE_ATTR});

void CORE_IOT_sendata(String mode, String feed, String data)
{
    if (mode == "attribute")
    {
        tb.sendAttributeData(feed.c_str(), data.c_str());
    }
    else if (mode == "telemetry")
    {
        float value = data.toFloat();
        tb.sendTelemetryData(feed.c_str(), value);
    }
}

void task_core_iot(void *pvParameters)
{
    // Đợi có kết nối Internet qua Semaphore
    Serial.println("[Cloud] Waiting for Internet connection...");
    while(1) {
        if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
            xSemaphoreGive(xBinarySemaphoreInternet); // Trả lại để các task khác cũng có thể lấy
            break;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    Serial.println("[Cloud] Internet connected, starting MQTT...");

    while (1)
    {
        if (!tb.connected())
        {
            Serial.print("[Cloud] Connecting to server: ");
            Serial.println(CORE_IOT_SERVER);
            
            if (!tb.connect(CORE_IOT_SERVER.c_str(), CORE_IOT_TOKEN.c_str(), CORE_IOT_PORT.toInt()))
            {
                vTaskDelay(5000 / portTICK_PERIOD_MS);
                continue;
            }

            tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
            tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
            
            tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend());
            tb.Shared_Attributes_Subscribe(attributes_callback);
            tb.Shared_Attributes_Request(attribute_shared_request_callback);
            
            Serial.println("[Cloud] Connected and Subscribed!");
        }

        // Nhận dữ liệu từ Queue (thread-safe)
        struct SensorData sensorData = {0.0, 0.0};
        if (xSensorQueue != NULL) {
            xQueueReceive(xSensorQueue, &sensorData, 0);
        }

        // Gửi dữ liệu cảm biến định kỳ
        tb.sendTelemetryData("temperature", sensorData.temperature);
        tb.sendTelemetryData("humidity", sensorData.humidity);

        tb.loop();
        vTaskDelay(10000 / portTICK_PERIOD_MS); // Gửi mỗi 10 giây
    }
}

#include "web_services/task_webserver.h"
#include <WiFi.h>
#include "global.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool webserver_isrunning = false;
static bool s_wifiScanPending = false;

static void sendDeviceInfo(AsyncWebSocketClient *client = nullptr) {
    StaticJsonDocument<1024> doc;
    doc["type"] = "info";
    bool isAP = (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA);
    doc["ip"]   = isAP ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
    doc["mode"] = isAP ? "AP" : "STA";
    doc["ssid"] = isAP ? WiFi.softAPSSID() : WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = (uint32_t)(millis() / 1000);
    
    // Các biến từ global.cpp
    doc["ssid"]            = ssid;
    doc["password"]        = password;
    doc["WIFI_SSID"]       = WIFI_SSID;
    doc["WIFI_PASS"]       = WIFI_PASS;
    doc["CORE_IOT_TOKEN"]  = CORE_IOT_TOKEN;
    doc["CORE_IOT_SERVER"] = CORE_IOT_SERVER;
    doc["CORE_IOT_PORT"]   = CORE_IOT_PORT;
    doc["core_iot_token"]  = core_iot_token;
    doc["core_iot_server"] = core_iot_server;
    doc["core_iot_port"]   = core_iot_port;

    String msg;
    serializeJson(doc, msg);
    if (client) client->text(msg);
    else        ws.textAll(msg);
}

void Webserver_sendata(String data) {
    if (ws.count() > 0) {
        ws.textAll(data);
        Serial.println("📤 Đã gửi dữ liệu qua WebSocket: " + data);
    } else {
        Serial.println("⚠️ Không có client WebSocket nào đang kết nối!");
    }
}

void Webserver_startWifiScan() {
    s_wifiScanPending = true;
    WiFi.scanNetworks(true); // async, non-blocking
    ws.textAll("{\"type\":\"scanning\"}");
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket client #%u connected from %s\n",
                      client->id(), client->remoteIP().toString().c_str());
        sendDeviceInfo(client);
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_TEXT) {
            String message = String((char *)data).substring(0, len);
            handleWebSocketMessage(message);
        }
    }
}

void connnectWSV() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });
    server.begin();
    ElegantOTA.begin(&server);
    webserver_isrunning = true;
}

void Webserver_stop() {
    ws.closeAll();
    server.end();
    webserver_isrunning = false;
}

void Webserver_reconnect() {
    if (!webserver_isrunning) {
        connnectWSV();
    }
}

void task_webserver_run(void *pvParameters) {
    while (WiFi.getMode() == WIFI_MODE_NULL) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);

    Serial.println("[Web] Starting web server...");
    Webserver_reconnect();
    Serial.println("[Web] Web server ready");

    TickType_t lastSensorTick = xTaskGetTickCount();
    TickType_t lastInfoTick   = xTaskGetTickCount();

    while (1) {
        ElegantOTA.loop();
        ws.cleanupClients();

        TickType_t now = xTaskGetTickCount();

        // Broadcast sensor data mỗi 5 giây
        if ((now - lastSensorTick) >= pdMS_TO_TICKS(5000) && ws.count() > 0) {
            lastSensorTick = now;
            SensorData data;
            if (xSensorQueue != NULL && xQueuePeek(xSensorQueue, &data, 0) == pdTRUE) {
                char json[96];
                snprintf(json, sizeof(json),
                         "{\"type\":\"sensor\",\"temp\":%.1f,\"humi\":%.1f}",
                         data.temperature, data.humidity);
                ws.textAll(String(json));
            }
        }

        // Broadcast device info mỗi 30 giây
        if ((now - lastInfoTick) >= pdMS_TO_TICKS(30000) && ws.count() > 0) {
            lastInfoTick = now;
            sendDeviceInfo();
        }

        // Kiểm tra kết quả WiFi scan async
        if (s_wifiScanPending) {
            int n = WiFi.scanComplete();
            if (n != WIFI_SCAN_RUNNING) {
                s_wifiScanPending = false;
                StaticJsonDocument<2048> doc;
                doc["type"] = "wifi_list";
                JsonArray nets = doc.createNestedArray("networks");
                if (n > 0) {
                    for (int i = 0; i < n; i++) {
                        JsonObject net = nets.createNestedObject();
                        net["ssid"]   = WiFi.SSID(i);
                        net["rssi"]   = WiFi.RSSI(i);
                        net["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
                    }
                }
                String result;
                serializeJson(doc, result);
                ws.textAll(result);
                WiFi.scanDelete();
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

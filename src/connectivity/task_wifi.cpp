#include "connectivity/task_wifi.h"
#include <lwip/dns.h>

void startAP()
{
    // Bắt sự kiện WiFi để debug và give semaphore
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        switch (event) {
            case ARDUINO_EVENT_WIFI_STA_CONNECTED:
                Serial.println("[WiFi] STA connected to AP.");
                break;
            case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
                // Set Google DNS vì AP_STA mode có thể dùng nhầm DNS của AP
                const ip_addr_t dns1 = IPADDR4_INIT_BYTES(8, 8, 8, 8);
                const ip_addr_t dns2 = IPADDR4_INIT_BYTES(8, 8, 4, 4);
                dns_setserver(0, &dns1);
                dns_setserver(1, &dns2);
                Serial.printf("[WiFi] STA got IP: %s  DNS: 8.8.8.8\n", WiFi.localIP().toString().c_str());
                isWifiConnected = true;
                xSemaphoreGive(xBinarySemaphoreInternet);
                break;
            }
            case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
                Serial.println("[WiFi] STA disconnected.");
                isWifiConnected = false;
                break;
            default:
                break;
        }
    });

    WiFi.mode(WIFI_AP_STA); // AP luôn visible, STA có thể connect sau
    WiFi.softAP(ssid.c_str(), password.c_str());
    Serial.print("[WiFi] AP IP: ");
    Serial.println(WiFi.softAPIP());
}

bool Wifi_reconnect()
{
    if (WIFI_SSID.isEmpty()) return false;
    if (WiFi.status() == WL_CONNECTED) return true;

    // Giữ AP_STA — không dùng WIFI_STA vì sẽ tắt AP
    if (WiFi.getMode() != WIFI_AP_STA) {
        WiFi.mode(WIFI_AP_STA);
    }

    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    return false;
}

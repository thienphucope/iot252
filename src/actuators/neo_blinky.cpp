#include "actuators/neo_blinky.h"
#include "global.h"
#include <Adafruit_NeoPixel.h>

// Khởi tạo NeoPixel (Chân GPIO 45 theo tài liệu YOLO UNO)
Adafruit_NeoPixel pixels(1, 45, NEO_GRB + NEO_KHZ800);

void neo_blinky(void *pvParameters) {
    pixels.begin();
    pixels.setBrightness(50);
    uint32_t current_color = pixels.Color(0, 0, 0); // Mặc định tắt cho đến khi nhận được dữ liệu
    bool current_state = true;

    while (1) {
        bool color_changed = false;

        // Luôn luôn bắt semaphore để cập nhật màu mong muốn (kể cả khi disabled)
        if (xSemaphoreTake(xHumiHighSemaphore, 0) == pdTRUE) {
            current_color = pixels.Color(0, 0, 255);
            color_changed = true;
        }
        else if (xSemaphoreTake(xHumiNormalSemaphore, 0) == pdTRUE) {
            current_color = pixels.Color(0, 255, 0);
            color_changed = true;
        }
        else if (xSemaphoreTake(xHumiLowSemaphore, 0) == pdTRUE) {
            current_color = pixels.Color(255, 0, 0);
            color_changed = true;
        }

        // Kiểm tra xem trạng thái toggle có thay đổi không
        bool state_changed = (current_state != neo_blinky_enabled);
        current_state = neo_blinky_enabled;

        // Cập nhật lại màu thực tế ra đèn khi:
        // 1. Màu bị thay đổi (do sensor) và đèn đang ON
        // 2. Trạng thái đèn vừa chuyển từ OFF sang ON hoặc từ ON sang OFF
        if ((color_changed && current_state) || state_changed) {
            if (current_state) {
                // Đèn đang bật -> hiện màu hiện tại
                pixels.setPixelColor(0, current_color);
            } else {
                // Đèn đang tắt -> xoá màu
                pixels.clear();
            }
            pixels.show();
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
#include "actuators/neo_blinky.h"
#include "global.h"
#include <Adafruit_NeoPixel.h>

// Khởi tạo NeoPixel (Chân GPIO 45 theo tài liệu YOLO UNO)
Adafruit_NeoPixel pixels(1, 45, NEO_GRB + NEO_KHZ800);

void neo_blinky(void *pvParameters) {
    pixels.begin();
    pixels.setBrightness(50);

    while (1) {
        // Task 2: Dùng semaphore độ ẩm để xác định màu NeoPixel
        if (xSemaphoreTake(xHumiHighSemaphore, 0) == pdTRUE) {
            // Độ ẩm cao (> 60%) -> Màu Xanh Dương (Blue)
            pixels.setPixelColor(0, pixels.Color(0, 0, 255));
            pixels.show();
        }
        else if (xSemaphoreTake(xHumiNormalSemaphore, 0) == pdTRUE) {
            // Độ ẩm bình thường (40-60%) -> Màu Xanh Lá (Green)
            pixels.setPixelColor(0, pixels.Color(0, 255, 0));
            pixels.show();
        }
        else if (xSemaphoreTake(xHumiLowSemaphore, 0) == pdTRUE) {
            // Độ ẩm thấp (< 40%) -> Màu Đỏ (Red)
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            pixels.show();
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
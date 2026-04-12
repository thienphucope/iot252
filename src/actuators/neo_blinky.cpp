#include "actuators/neo_blinky.h"
#include "global.h"
#include <Adafruit_NeoPixel.h>

// Khởi tạo NeoPixel (Chân GPIO 45 theo tài liệu YOLO UNO)
Adafruit_NeoPixel pixels(1, 45, NEO_GRB + NEO_KHZ800);

void neo_blinky(void *pvParameters) {
    pixels.begin();
    pixels.setBrightness(50);

    while (1) {
        // Task 2: Đợi tín hiệu Semaphore độ ẩm (không dùng biến toàn cục)
        // Dùng 3 màu cho 3 mức độ ẩm (Yêu cầu Task 2)
        
        if (xSemaphoreTake(xHumiHighSemaphore, 0) == pdTRUE) {
            // Độ ẩm cao (>70%) -> Màu Xanh Dương (Blue)
            pixels.setPixelColor(0, pixels.Color(0, 0, 255));
            pixels.show();
        } 
        else if (xSemaphoreTake(xHumiLowSemaphore, 0) == pdTRUE) {
            // Độ ẩm thấp (<40%) -> Màu Đỏ (Red)
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            pixels.show();
        }
        else {
            // Độ ẩm bình thường -> Màu Xanh Lá (Green)
            // (Thực tế bạn có thể thêm Semaphore Normal Humi nếu muốn)
            pixels.setPixelColor(0, pixels.Color(0, 255, 0));
            pixels.show();
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
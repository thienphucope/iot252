#include "sensors/lcd_task.h"
#include <Wire.h> // Thêm thư viện Wire cho I2C

// Khởi tạo LCD trên địa chỉ 0x27, 16 cột, 2 dòng
LiquidCrystal_I2C lcd(0x27, 16, 2);

void lcd_task(void *pvParameters) {
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM STARTING");
    lcd.clear();
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    struct SensorData currentData = {0.0, 0.0, 0};
    const char* statuses[] = {"NORMAL  ", "WARNING ", "CRITICAL"};

    while (1) {
        // Đọc data + lcd_status từ Queue (cùng nguồn với webserver → luôn đồng bộ)
        if (xSensorQueue != NULL) {
            xQueuePeek(xSensorQueue, &currentData, (TickType_t)100 / portTICK_PERIOD_MS);
        }

        lcd.setCursor(0, 1);
        lcd.printf("T:%.1fC H:%.1f%%  ", currentData.temperature, currentData.humidity);

        lcd.setCursor(0, 0);
        lcd.printf("STAT: %s", statuses[currentData.lcd_status]);

        // Drain semaphore status (để task khác không bị nghẽn nếu vẫn give)
        xSemaphoreTake(xStatusCriticalSemaphore, 0);
        xSemaphoreTake(xStatusWarningSemaphore, 0);
        xSemaphoreTake(xStatusNormalSemaphore, 0);

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
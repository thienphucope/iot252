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

    struct SensorData currentData = {0.0, 0.0};
    String displayStatus = "NORMAL";

    while (1) {
        // Nhận dữ liệu từ Queue (thread-safe)
        if (xSensorQueue != NULL) {
            xQueueReceive(xSensorQueue, &currentData, (TickType_t)100 / portTICK_PERIOD_MS);
        }
        
        lcd.setCursor(0, 1);
        lcd.printf("T:%.1fC H:%.1f%%  ", currentData.temperature, currentData.humidity);

        // Triggered by status semaphores (combined temp + humidity)
        if (xSemaphoreTake(xStatusCriticalSemaphore, 0) == pdTRUE) {
            displayStatus = "CRITICAL";
        } else if (xSemaphoreTake(xStatusWarningSemaphore, 0) == pdTRUE) {
            displayStatus = "WARNING ";
        } else if (xSemaphoreTake(xStatusNormalSemaphore, 0) == pdTRUE) {
            displayStatus = "NORMAL  ";
        }

        lcd.setCursor(0, 0);
        lcd.printf("STAT: %s", displayStatus.c_str());

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
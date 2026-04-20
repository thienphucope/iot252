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
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    struct SensorData currentData = {0.0, 0.0};
    String currentStatus = "NORMAL";

    while (1) {
        // Task 3: Cập nhật giá trị Temp/Humi từ Queue
        if (xSensorQueue != NULL) {
            xQueueReceive(xSensorQueue, &currentData, 0);
        }
        lcd.setCursor(0, 1);
        lcd.printf("T:%.1fC H:%.1f%%  ", currentData.temperature, currentData.humidity);

        // Task 3: Kiểm tra Semaphore trạng thái
        if (xSemaphoreTake(xNormalSemaphore, 0) == pdTRUE) {
            currentStatus = "NORMAL  ";
        } 
        else if (xSemaphoreTake(xWarningSemaphore, 0) == pdTRUE) {
            currentStatus = "WARNING ";
        } 
        else if (xSemaphoreTake(xCriticalSemaphore, 0) == pdTRUE) {
            currentStatus = "CRITICAL";
        }

        lcd.setCursor(0, 0);
        lcd.print("STAT: ");
        lcd.print(currentStatus);

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
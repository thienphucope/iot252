#ifndef __LCD_TASK_H__
#define __LCD_TASK_H__

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "global.h"

// Hàm thực thi Task hiển thị LCD
void lcd_task(void *pvParameters);

#endif
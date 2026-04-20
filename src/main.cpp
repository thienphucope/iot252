#include "global.h"

#include "actuators/led_blinky.h"
#include "actuators/neo_blinky.h"
#include "sensors/temp_humi_monitor.h"
#include "sensors/lcd_task.h" // Thêm header LCD
#include "tinyml/tinyml.h"

// include task
#include "connectivity/task_check_info.h"
#include "connectivity/task_toogle_boot.h"
#include "connectivity/task_wifi.h"
#include "web_services/task_webserver.h"
#include "cloud/task_core_iot.h"

void init_globals(); // Khai báo hàm khởi tạo

void setup()
{
  Serial.begin(115200);
  init_globals(); // Khởi tạo Queue/Semaphore trước khi chạy Task
  temp_humi_init(); // Khởi tạo I2C + DHT20 một lần duy nhất
  check_info_File(0);

  // Nếu có saved credentials: init STA mode ngay để WiFi stack sẵn sàng
  // (startAP đã được gọi trong check_info_File nếu không có credentials)
  // Không block ở đây - việc chờ WL_CONNECTED sẽ do loop() xử lý
  if (!WIFI_SSID.isEmpty()) {
    WiFi.mode(WIFI_STA);
    if (WIFI_PASS.isEmpty()) {
      WiFi.begin(WIFI_SSID.c_str());
    } else {
      WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }
  }

  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 4096, NULL, 2, NULL);
  xTaskCreate(lcd_task, "Task LCD Display", 4096, NULL, 2, NULL); // Chạy Task LCD
  xTaskCreate(tiny_ml_task, "Tiny ML Task" ,4096  ,NULL  ,2 , NULL);
  xTaskCreate(task_core_iot, "Task CoreIOT", 4096, NULL, 2, NULL);
  xTaskCreate(task_webserver_run, "Task WebServer", 4096, NULL, 2, NULL);
  xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
}

void loop()
{
  // Chỉ quản lý kết nối WiFi định kỳ
  if (check_info_File(1))
  {
    Wifi_reconnect();
  }
  vTaskDelay(5000 / portTICK_PERIOD_MS); // Kiểm tra mỗi 5 giây
}
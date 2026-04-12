#include "global.h"

#include "actuators/led_blinky.h"
#include "actuators/neo_blinky.h"
#include "sensors/temp_humi_monitor.h"
#include "tinyml/tinyml.h"

// include task
#include "connectivity/task_check_info.h"
#include "connectivity/task_toogle_boot.h"
#include "connectivity/task_wifi.h"
#include "web_services/task_webserver.h"
#include "cloud/task_core_iot.h"

void setup()
{
  Serial.begin(115200);
  check_info_File(0);

  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, NULL, 2, NULL);
  xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
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
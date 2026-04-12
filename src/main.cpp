#include "global.h"

#include "actuators/led_blinky.h"
#include "actuators/neo_blinky.h"
#include "sensors/temp_humi_monitor.h"
// #include "web_services/mainserver.h"
// #include "tinyml/tinyml.h"
#include "cloud/coreiot.h"

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
  // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  // xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
}

void loop()
{
  if (check_info_File(1))
  {
    if (!Wifi_reconnect())
    {
      Webserver_stop();
    }
    else
    {
      //CORE_IOT_reconnect();
    }
  }
  Webserver_reconnect();
}
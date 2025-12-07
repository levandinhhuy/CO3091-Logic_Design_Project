#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"

void setup()
{
  Serial.begin(115200);
  check_info_File(0);

  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task Temperature and Humidity Monitor", 8192, NULL, 2, NULL);  // Increased from 2048 to 8192 (8KB)
  xTaskCreate(tiny_ml_task, "Tiny ML Task", 16384, NULL, 2, NULL);  // Increased from 2048 to 16384 (16KB)
  xTaskCreate(coreiot_task, "CoreIOT Task", 4096, NULL, 2, NULL);
  xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
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
      // CORE_IOT_reconnect();
      // Serial.println("WiFi connected");
      // Serial.print("IP address: ");
      // Serial.println(WiFi.localIP());
    }
  }
  Webserver_reconnect();
}
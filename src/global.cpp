#include "global.h"
String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

QueueHandle_t xQueueSensorData = xQueueCreate(10, sizeof(&xData));
QueueHandle_t xQueueAnomalyResult = xQueueCreate(10, sizeof(&xData));
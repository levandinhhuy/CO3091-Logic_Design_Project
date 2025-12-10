#include "global.h"
bool anomaly_detected = false;

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

sensorData xData;

QueueHandle_t xQueueSensorData = xQueueCreate(1, sizeof(sensorData*));
QueueHandle_t xQueueAnomalyResult = xQueueCreate(1, sizeof(sensorData*));
SemaphoreHandle_t xBinarySemaphorePowerOptimize = xSemaphoreCreateBinary();
SemaphoreHandle_t xBinarySemaphoreNormalMode = xSemaphoreCreateBinary();

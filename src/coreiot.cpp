#include "coreiot.h"

// ----------- CONFIGURE THESE! -----------
#define FAN_GPIO 25
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    Serial.println(CORE_IOT_TOKEN.c_str());
    Serial.println(CORE_IOT_SERVER.c_str());
    Serial.println(mqttPort);
    if (client.connect("ESP32Client", CORE_IOT_TOKEN.c_str(), NULL)) {    //fix hard code
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* method = doc["method"];
  if (strcmp(method, "turnOnFan") == 0) {
    int params = doc["params"]["state"];

    if (params == 1) {
      Serial.println("Fan turned ON.");
      digitalWrite(FAN_GPIO, HIGH);

    } else {   
      Serial.println("Fan turned OFF.");
      digitalWrite(FAN_GPIO, LOW);
    }
  } else {
    Serial.print("Unknown method: ");
    Serial.println(method);
  }
}


void setup_coreiot(){

  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY) == pdTRUE) {
      break;
    } else {
      Serial.println("[COREIOT] Failed WiFi connection.");
    }
    delay(500);
    Serial.print(".");
  }


  Serial.println(" -------------------------Connected!");

  client.setServer(CORE_IOT_SERVER.c_str(), mqttPort);    //fix hard code
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters){
    Serial.println("[COREIOT] Starting CoreIOT task...");
    setup_coreiot();

    while(1){

        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        sensorData* pxdata;
        if (xQueueReceive(xQueueAnomalyResult, &pxdata, portMAX_DELAY) == pdPASS) {
          String payload = "{\"temperature\":" + String(pxdata->temperature) +  ",\"humidity\":" + String(pxdata->humidity) + ",\"anomaly\":" + String (pxdata ->anomaly) + "}";
        
          client.publish("v1/devices/me/telemetry", payload.c_str());

            Serial.println("Published payload: " + payload);
        } else {
            Serial.println("Failed to receive from anomaly result queue.");
            vTaskDelay(1000);
            continue;
        }
        vTaskDelay(5000);  // Publish every 5 seconds
    }
}
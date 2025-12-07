#include "led_blinky.h"
#include "tinyml.h"

void led_blinky(void *pvParameters){
    pinMode(LED_GPIO, OUTPUT);
  
  while(1) {       
    if (anomaly_detected) {
      // Anomaly detected - fast blink
      digitalWrite(LED_GPIO, HIGH);  // turn the LED ON
      vTaskDelay(200);
      digitalWrite(LED_GPIO, LOW);  // turn the LED OFF
      vTaskDelay(200);
    } else {
      // Normal - slow blink
      digitalWrite(LED_GPIO, HIGH);  // turn the LED ON
      vTaskDelay(2000);
      digitalWrite(LED_GPIO, LOW);  // turn the LED OFF
      vTaskDelay(2000);
    }
  }
}
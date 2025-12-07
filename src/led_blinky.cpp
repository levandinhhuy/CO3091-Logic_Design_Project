#include "led_blinky.h"
#include "tinyml.h"
#include "global.h"

void led_blinky(void *pvParameters){
    pinMode(LED_GPIO, OUTPUT);
  
  while(1) {
    uint16_t delay_ms;
    
    // Check WiFi connection status
    if (!isWifiConnected) {
      delay_ms = 200;  // Fast blink (300ms) when WiFi NOT connected
    } else {
      delay_ms = 2000; // Normal blink (1000ms) when WiFi connected
    }
    
    digitalWrite(LED_GPIO, HIGH);  // turn the LED ON
    vTaskDelay(delay_ms);
    digitalWrite(LED_GPIO, LOW);   // turn the LED OFF
    vTaskDelay(delay_ms);
  }
}
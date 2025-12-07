#include "neo_blinky.h"
#include "global.h"


void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    while(1) {
        uint16_t delay_ms;
        
        if (anomaly_detected) {
            delay_ms = 200;  // Fast blink (200ms) when ANOMALY
            strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red for anomaly
        } else {
            delay_ms = 2000; // Slow blink (2000ms) when NORMAL
            strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green for normal
        }
        
        strip.show(); // Update the strip
        vTaskDelay(delay_ms);

        // Set the pixel to off
        strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn pixel off
        strip.show(); // Update the strip

        // Wait for the same duration before blinking again
        vTaskDelay(delay_ms);
    }
}
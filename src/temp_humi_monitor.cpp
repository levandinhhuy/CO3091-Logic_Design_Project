#include "temp_humi_monitor.h"
DHT20 dht20;
// Set the LCD I2C address to 0x27 for a 16 column and 2 row.
LiquidCrystal_I2C lcd(0x27,16,2);

void temp_humi_monitor(void *pvParameters){

    Wire.begin(11, 12);
    // Serial.begin(115200);
    dht20.begin();
    lcd.begin();
    lcd.backlight();
    
    while (1){
        /* code */
        
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            //return;
        }

        //Update global variables for temperature and humidity and light level
        glob_temperature = temperature;
        glob_humidity = humidity;

        // LCD display        
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");

        // Hàng 1: Temp + Humi
        lcd.setCursor(0, 0);
        char line1[17];
        sprintf(line1, "T:%.1f H:%.1f", temperature, humidity);
        lcd.print(line1);

        // Hàng 2: Status
        lcd.setCursor(0, 1);
        lcd.print("Status: ");
        if (anomaly_detected) {
            lcd.print("ANOMALY ");
        } else {
            lcd.print("NORMAL  ");
        }



        vTaskDelay(1000);
        
        // ==================================================
        StaticJsonDocument<128> doc;
        doc["type"] = "sensor_data";
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;

        String jsonString;
        serializeJson(doc, jsonString);
        
        Webserver_sendata(jsonString);
        // ==================================================
        vTaskDelay(3000);
    }
}
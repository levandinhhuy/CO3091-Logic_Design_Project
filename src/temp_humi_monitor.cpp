#include "temp_humi_monitor.h"
DHT20 dht20;
// Set the LCD I2C address to 0x27 for a 16 column and 2 row.
LiquidCrystal_I2C lcd(0x27,16,2);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#define LIGHT_ANALOG_PIN 1 //A0

void temp_humi_monitor(void *pvParameters){

    Wire.begin(11, 12);
    dht20.begin();
    lcd.begin();
    lcd.backlight();
    while (1){
        /* code */
        sensorData* pxdata = &xData;
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
        }

        pxdata->temperature = temperature;
        pxdata->humidity = humidity;
        xQueueSend( xQueueSensorData, (void *) &pxdata, (TickType_t) 0 );
        // LCD display
        
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");

        lcd.setCursor(0,0);
        lcd.print("Temp: ");
        lcd.setCursor(6,0);
        lcd.print(temperature);

        lcd.setCursor(0,1);
        lcd.print("Humi: ");
        lcd.setCursor(6,1);
        lcd.print(humidity);

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
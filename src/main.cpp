/*
 * ESP32 System Monitor
 * Version 2.0 - WiFi Connection
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>

// OLED display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Wi-Fi settings (Change on yours!)
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

void setup() {
    Serial.begin(115200);
    
    // Init Display
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "ESP32 Monitor");
    u8g2.drawStr(0, 25, "Connecting WiFi...");
    u8g2.sendBuffer();
    
    // Подключение к Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 30) {
        delay(500);
        Serial.print(".");
        timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi Connected!");
        u8g2.setCursor(0, 25);
        u8g2.print(WiFi.localIP().toString().c_str());
        u8g2.sendBuffer();
    } else {
        Serial.println("\nConnection failed!");
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi Failed!");
        u8g2.drawStr(0, 25, "Check credentials");
        u8g2.sendBuffer();
    }
}

void loop() {
    delay(1000);
}
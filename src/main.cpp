/*
 * ESP32 System Monitor
 * Version 3.0 - WiFiManager (No Serial conflicts)
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Wire.h>

// OLED display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// WiFiManager
WiFiManager wm;

void setup() {
    delay(2000);
    
    // Initialize display first
    Wire.begin(8, 9);
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "ESP32 Monitor");
    u8g2.drawStr(0, 25, "v3.0 WiFi");
    u8g2.drawStr(0, 40, "Starting...");
    u8g2.sendBuffer();
    
    // Configure WiFiManager
    wm.setConfigPortalTimeout(120);
    
    // Connect to WiFi
    if (wm.autoConnect("ESP32_Monitor_Setup")) {
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi OK!");
        u8g2.setCursor(0, 25);
        u8g2.print(WiFi.localIP().toString().c_str());
        u8g2.drawStr(0, 40, "Ready!");
        u8g2.sendBuffer();
    } else {
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi Failed!");
        u8g2.drawStr(0, 25, "Timeout");
        u8g2.drawStr(0, 40, "Restarting...");
        u8g2.sendBuffer();
        
        delay(3000);
        ESP.restart();
    }
}

void loop() {
    delay(1000);
}
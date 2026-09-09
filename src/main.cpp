/*
 * ESP32 System Monitor
 * Version 7.0 - UDP Server with JSON
 * 
 * Receives system stats from PC via UDP in JSON format.
 * Expected format: {"cpu_temp":45.0,"gpu_temp":62.0,"cpu_usage":50,"gpu_usage":70,"ram_usage":60}
 */

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// OLED display (I2C: SDA=GPIO8, SCL=GPIO9)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// WiFiManager
WiFiManager wm;

// UDP server
WiFiUDP udp;
const int UDP_PORT = 4210;

// System data
float cpuTemp = 0.0;
float gpuTemp = 0.0;
int cpuUsage = 0;
int gpuUsage = 0;
int ramUsage = 0;

// Status tracking
String status = "No";
unsigned long lastUpdate = 0;
bool wifiConnected = false;

// Draw a progress bar
void drawProgressBar(int x, int y, int width, int height, int percent) {
    u8g2.drawFrame(x, y, width, height);
    int fillWidth = (width - 2) * constrain(percent, 0, 100) / 100;
    if (fillWidth > 0) {
        u8g2.drawBox(x + 1, y + 1, fillWidth, height - 2);
    }
}

// Draw right-aligned text
void drawRightAligned(int y, const char* text) {
    int width = u8g2.getStrWidth(text);
    u8g2.setCursor(128 - width, y);
    u8g2.print(text);
}

void setup() {
    delay(2000);
    
    // Initialize I2C and display
    Wire.begin(8, 9);
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    
    // Boot screen
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "ESP32 Monitor");
    u8g2.drawStr(0, 25, "v7.0 UDP+JSON");
    u8g2.drawStr(0, 40, "Starting...");
    u8g2.sendBuffer();
    
    // Configure WiFiManager
    wm.setConfigPortalTimeout(120);
    
    // Connect to WiFi
    if (wm.autoConnect("ESP32_Monitor_Setup")) {
        wifiConnected = true;
        
        // Start UDP server
        udp.begin(UDP_PORT);
        
        // Show success
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi OK!");
        u8g2.setCursor(0, 25);
        u8g2.print(WiFi.localIP().toString().c_str());
        u8g2.drawStr(0, 35, "UDP:");
        u8g2.setCursor(25, 35);
        u8g2.print(UDP_PORT);
        u8g2.sendBuffer();
        
        delay(2000);
    } else {
        // Connection failed
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi Failed!");
        u8g2.drawStr(0, 25, "Restarting...");
        u8g2.sendBuffer();
        delay(3000);
        ESP.restart();
    }
}

void loop() {
    // Check for incoming UDP packets
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        char buf[256];
        int len = udp.read(buf, sizeof(buf) - 1);
        buf[len] = '\0';
        
        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, buf);
        
        if (!error) {
            // Extract values with defaults
            cpuTemp = doc["cpu_temp"] | 0.0;
            gpuTemp = doc["gpu_temp"] | 0.0;
            cpuUsage = doc["cpu_usage"] | 0;
            gpuUsage = doc["gpu_usage"] | 0;
            ramUsage = doc["ram_usage"] | 0;
            
            status = "OK";
            lastUpdate = millis();
        } else {
            status = "No";
        }
    }
    
    // Check timeout (no data for 10 seconds)
    if (millis() - lastUpdate > 10000 && lastUpdate > 0) {
        status = "No";
    }
    
    // Update display
    u8g2.clearBuffer();
    
    // Header with status (right-aligned with spacing)
    u8g2.setFont(u8g2_font_7x13_tr);
    u8g2.drawStr(0, 12, "System Monitor");
    
    // Status indicator (right-aligned with gap)
    u8g2.setFont(u8g2_font_6x10_tr);
    int statusWidth = u8g2.getStrWidth(status.c_str());
    u8g2.setCursor(128 - statusWidth - 5, 12);  // 5px gap from right edge
    u8g2.print(status.c_str());
    
    // CPU with progress bar
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 25, "CPU:");
    u8g2.setCursor(30, 25);
    u8g2.print(cpuUsage);
    u8g2.print("% ");
    
    // Temperature (right-aligned)
    String cpuTempStr = String(cpuTemp, 1) + "C";
    drawRightAligned(25, cpuTempStr.c_str());
    
    drawProgressBar(0, 28, 128, 4, cpuUsage);
    
    // GPU with progress bar
    u8g2.drawStr(0, 40, "GPU:");
    u8g2.setCursor(30, 40);
    u8g2.print(gpuUsage);
    u8g2.print("% ");
    
    // Temperature (right-aligned)
    String gpuTempStr = String(gpuTemp, 1) + "C";
    drawRightAligned(40, gpuTempStr.c_str());
    
    drawProgressBar(0, 43, 128, 4, gpuUsage);
    
    // RAM with progress bar
    u8g2.drawStr(0, 55, "RAM:");
    u8g2.setCursor(30, 55);
    u8g2.print(ramUsage);
    u8g2.print("%");
    drawProgressBar(0, 58, 128, 4, ramUsage);
    
    u8g2.sendBuffer();
    
    delay(500);
}
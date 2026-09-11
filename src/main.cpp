/*
 * ESP32 System Monitor
 * Version 9.0 - Final with mDNS and Auto-Reconnect
 * 
 * Features:
 * - mDNS hostname (esp32monitor.local) instead of IP
 * - Wi-Fi auto-reconnect
 * - Password protected AP mode
 */

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>  // ← ДОБАВИТЬ ЭТУ БИБЛИОТЕКУ!

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
    
    Wire.begin(8, 9);
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "ESP32 Monitor");
    u8g2.drawStr(0, 25, "v9.1 Auto-AP");
    u8g2.drawStr(0, 40, "Starting...");
    u8g2.sendBuffer();
    
    // Config WiFiManager
    wm.setConfigPortalTimeout(120);  // Портал активен 2 минуты
    
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Connecting...");
    u8g2.drawStr(0, 25, "to saved WiFi");
    u8g2.drawStr(0, 45, "Wait 30s...");
    u8g2.sendBuffer();
    
    WiFi.mode(WIFI_STA);
    WiFi.begin();  // Use saved credentials
    
    int waitCount = 0;
    while (WiFi.status() != WL_CONNECTED && waitCount < 15) {
        delay(1000);
        waitCount++;
        
        // Timer
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Connecting...");
        u8g2.setCursor(0, 25);
        u8g2.print("Attempt: ");
        u8g2.print(waitCount);
        u8g2.print("/15");
        u8g2.sendBuffer();
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        // Success
        wifiConnected = true;
        udp.begin(UDP_PORT);
        
        if (MDNS.begin("esp32monitor")) {
            MDNS.addService("http", "tcp", 80);
        }
        
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi OK!");
        u8g2.setCursor(0, 25);
        u8g2.print(WiFi.localIP().toString().c_str());
        u8g2.drawStr(0, 35, "mDNS:");
        u8g2.drawStr(0, 45, "esp32monitor.local");
        u8g2.sendBuffer();
        
        delay(2000);
    } else {
        // Не смогли подключиться — запускаем портал конфигурации
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi Failed!");
        u8g2.drawStr(0, 25, "Starting AP...");
        u8g2.drawStr(0, 40, "Connect to:");
        u8g2.drawStr(0, 50, "System_monitor");
        u8g2.sendBuffer();
        
        delay(2000);
        
        // Wi-Fi config
        WiFi.mode(WIFI_AP);
        wm.startConfigPortal("System_monitor", "12345678");
        
        // Restart when configured
        ESP.restart();
    }
}

void loop() {
    // 1. Wi-Fi Resilience: Auto-reconnect if dropped
    if (WiFi.status() != WL_CONNECTED) {
        status = "No";
        wifiConnected = false;
        WiFi.reconnect();
        delay(1000);
        return;
    } else {
        wifiConnected = true;
    }

    // 2. Check for incoming UDP packets
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        char buf[256];
        int len = udp.read(buf, sizeof(buf) - 1);
        buf[len] = '\0';
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, buf);
        
        if (!error) {
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
    
    // 3. Check timeout (no data for 10 seconds)
    if (millis() - lastUpdate > 10000 && lastUpdate > 0) {
        status = "No";
    }
    
    // 4. Update display
    u8g2.clearBuffer();
    
    // Header
    u8g2.setFont(u8g2_font_7x13_tr);
    u8g2.drawStr(0, 12, "System Monitor");
    
    // Status + Wi-Fi Signal (RSSI) on the right
    u8g2.setFont(u8g2_font_5x8_tr);
    String statusStr;
    if(status =="No"){
        statusStr = status;
    }
    else{
        statusStr = " [" + String(WiFi.RSSI()) + "]";
    }
    int statusWidth = u8g2.getStrWidth(statusStr.c_str());
    u8g2.setCursor(128 - statusWidth - 2, 11);
    u8g2.print(statusStr.c_str());
    
    // CPU
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 25, "CPU:");
    u8g2.setCursor(30, 25);
    u8g2.print(cpuUsage);
    u8g2.print("% ");
    String cpuTempStr = String(cpuTemp, 1) + "C";
    drawRightAligned(25, cpuTempStr.c_str());
    drawProgressBar(0, 28, 128, 4, cpuUsage);
    
    // GPU
    u8g2.drawStr(0, 40, "GPU:");
    u8g2.setCursor(30, 40);
    u8g2.print(gpuUsage);
    u8g2.print("% ");
    String gpuTempStr = String(gpuTemp, 1) + "C";
    drawRightAligned(40, gpuTempStr.c_str());
    drawProgressBar(0, 43, 128, 4, gpuUsage);
    
    // RAM
    u8g2.drawStr(0, 55, "RAM:");
    u8g2.setCursor(30, 55);
    u8g2.print(ramUsage);
    u8g2.print("%");
    drawProgressBar(0, 58, 128, 4, ramUsage);
    
    u8g2.sendBuffer();
    
    delay(500);
}
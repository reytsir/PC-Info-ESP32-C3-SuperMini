#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
WiFiManager wm;
WiFiUDP udp;
const int UDP_PORT = 4210;

float cpuTemp = 0.0, gpuTemp = 0.0;
int cpuUsage = 0, gpuUsage = 0, ramUsage = 0;
String status = "No";
unsigned long lastUpdate = 0;

void drawProgressBar(int x, int y, int width, int height, int percent) {
    u8g2.drawFrame(x, y, width, height);
    int fillWidth = (width - 2) * constrain(percent, 0, 100) / 100;
    if (fillWidth > 0) u8g2.drawBox(x + 1, y + 1, fillWidth, height - 2);
}

void drawRightAligned(int y, const char* text) {
    u8g2.setCursor(128 - u8g2.getStrWidth(text), y);
    u8g2.print(text);
}

void setup() { 
    delay(1000);
    Wire.begin(8, 9);
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "ESP32 Monitor");
    u8g2.drawStr(0, 25, "Release 1.0");
    u8g2.drawStr(0, 40, "Starting...");
    u8g2.sendBuffer();
    delay(1000);

    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Connecting...");
    u8g2.drawStr(0, 25, "to saved WiFi");
    u8g2.drawStr(0, 45, "Wait 15s...");
    u8g2.sendBuffer();

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin();

    int waitCount = 0;
    while (WiFi.status() != WL_CONNECTED && waitCount < 15 && wm.getWiFiIsSaved()) {
        delay(1000);
        waitCount++;
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Connecting...");
        u8g2.setCursor(0, 25);
        u8g2.print("Attempt: ");
        u8g2.print(waitCount);
        u8g2.print("/15");
        u8g2.sendBuffer();
    }

        if (WiFi.status() == WL_CONNECTED) {
        udp.begin(UDP_PORT);
        delay(1000);
        const char* mdnsName = "esp32";
        MDNS.begin(mdnsName);
        MDNS.addService("_http", "_tcp", 80);    
        MDNS.addService("esp32", "udp", 4210); 
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "WiFi OK!");
        u8g2.setCursor(0, 25);
        u8g2.print(WiFi.localIP().toString().c_str());
        u8g2.drawStr(0, 35, "mDNS:");
        u8g2.drawStr(0, 45, "esp32.local"); 
        u8g2.sendBuffer();
        
        delay(2000);
    } else {
        u8g2.clearBuffer();
        if (wm.getWiFiIsSaved()) {
            u8g2.drawStr(0, 10, "WiFi Failed!");
            u8g2.drawStr(0, 25, "Reconfigure network");
        } else {
            u8g2.drawStr(0, 10, "WiFi not configured!");
            u8g2.drawStr(0, 25, "Configure network");
        }
        u8g2.drawStr(0, 35, "SSID: System_monitor");
        u8g2.drawStr(0, 45, "Pass: 12345678");
        u8g2.drawStr(0, 55, "cfg: 192.168.4.1");
        u8g2.sendBuffer();
        delay(2000);

        WiFi.mode(WIFI_AP_STA);
        wm.startConfigPortal("System_monitor", "12345678");
        ESP.restart();
    }
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        delay(1000);
        return;
    }
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        char buf[256];
        int len = udp.read(buf, sizeof(buf) - 1);
        buf[len] = '\0';

        JsonDocument doc;
        if (!deserializeJson(doc, buf)) {
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

    if (millis() - lastUpdate > 10000 && lastUpdate > 0) status = "No";

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x13_tr);
    u8g2.drawStr(0, 12, "System Monitor");

    u8g2.setFont(u8g2_font_5x8_tr);
    String statusStr = (status == "No") ? "No" : ("OK");
    u8g2.setCursor(128 - u8g2.getStrWidth(statusStr.c_str()) - 2, 11);
    u8g2.print(statusStr.c_str());

    u8g2.setFont(u8g2_font_6x10_tr);

    u8g2.drawStr(0, 25, "CPU:");
    u8g2.setCursor(30, 25);
    u8g2.print(cpuUsage);
    u8g2.print("% ");
    drawRightAligned(25, (String(cpuTemp, 1) + "C").c_str());
    drawProgressBar(0, 28, 128, 4, cpuUsage);

    u8g2.drawStr(0, 40, "GPU:");
    u8g2.setCursor(30, 40);
    u8g2.print(gpuUsage);
    u8g2.print("% ");
    drawRightAligned(40, (String(gpuTemp, 1) + "C").c_str());
    drawProgressBar(0, 43, 128, 4, gpuUsage);

    u8g2.drawStr(0, 55, "RAM:");
    u8g2.setCursor(30, 55);
    u8g2.print(ramUsage);
    u8g2.print("%");
    drawProgressBar(0, 58, 128, 4, ramUsage);

    u8g2.sendBuffer();
    delay(500);
}
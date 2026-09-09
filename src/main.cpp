/*
 * ESP32 System Monitor
 * Version 1.0 - Hello World
 */

#include <Arduino.h>
#include <U8g2lib.h>

// Инициализация OLED экрана (I2C)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
    // Инициализация Serial
    Serial.begin(115200);
    Serial.println("ESP32 Monitor starting...");
    
    // Инициализация дисплея
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    
    // Тестовое сообщение
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "ESP32 Monitor");
    u8g2.drawStr(0, 25, "Version 1.0");
    u8g2.drawStr(0, 40, "Hello World!");
    u8g2.sendBuffer();
    
    Serial.println("Display initialized!");
}

void loop() {
    // Пока ничего не делаем
    delay(1000);
}
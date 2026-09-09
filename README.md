# ESP32 System Monitor

![Version](https://img.shields.io/badge/version-9.1-blue)
![License](https://img.shields.io/badge/license-MIT-green)

A compact, wireless PC system resource monitor built on ESP32-C3 with a 0.96" OLED display.

## ✨ Features

- 📡 **Wireless monitoring** - UDP data transmission, no cables needed
- 📊 **Real-time stats** - CPU/GPU usage, temperatures, RAM usage
- 📶 **Smart WiFi** - mDNS support (esp32monitor.local), auto-reconnect
- 🔐 **Secure AP** - Password-protected configuration portal
- 🎨 **Clean UI** - Right-aligned display with progress bars
- 🔋 **Battery ready** - Designed for IP5306 power module
- 💻 **Native Windows** - PowerShell client, no Python required

## 🛠️ Hardware Requirements

- Microcontroller: ESP32-C3 SuperMini
- Display: OLED SSD1306 0.96" (I2C)
- Power: Li-Ion Battery (e.g., 1600mAh)
- Charging Module: IP5306 Power Management
- Switch: Miniature slide switch (optional)

## 🔌 Wiring Diagram

OLED Display to ESP32-C3:
VCC (3.3V) -> 3.3V
GND -> GND
SDA -> GPIO 8
SCL -> GPIO 9

Power Connection:
Battery -> IP5306 (B+/B-)
IP5306 (OUT+/OUT-) -> Switch -> ESP32 (5V & GND)

## 📦 Installation

### 1. Flash ESP32 Firmware

Requirements:
- VS Code (https://code.visualstudio.com/)
- PlatformIO extension

Steps:
Connect ESP32-C3 via USB
Press Build and Upload or run:
pio run --target upload

### 2. Configure WiFi

First Boot (or when WiFi unavailable):
1. ESP32 creates access point: System_monitor
2. Password: 12345678
3. Connect your phone/PC to this network
4. Captive portal opens automatically (or visit 192.168.4.1)
5. Enter your home WiFi credentials
6. ESP32 saves credentials and displays IP on screen

Reconfigure WiFi:
- Restart ESP32 without WiFi available
- After 30 seconds, AP portal will auto-start
- Connect and enter new credentials

### 3. Run PC Client

Prerequisites:
- Windows 10/11
- PowerShell 5.1+

Steps:
Open PowerShell and run:
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
cd "path\to\your\project"
.\pc-monitor.ps1

The script will:
- Auto-resolve ESP32 via mDNS (esp32monitor.local)
- Read CPU/GPU/RAM usage via native Windows APIs
- Send data via UDP every 500ms
- Display real-time stats in console

## 🚀 Auto-Start Setup (Run on Windows Startup)

### Method 1: Task Scheduler (Recommended)

1. Open Task Scheduler (search in Start menu)
2. Click "Create Basic Task" in the right panel
3. Name: "ESP32 Monitor"
4. Trigger: "When I log on"
5. Action: "Start a program"
6. Program/script: powershell.exe
7. Add arguments: -WindowStyle Hidden -ExecutionPolicy Bypass -File "C:\full\path\to\pc-monitor.ps1"
8. Click Finish
9. Right-click your new task -> Properties -> Check "Run with highest privileges"

### Method 2: Startup Folder (Simpler)

1. Press Win+R, type: shell:startup
2. Right-click in the folder -> New -> Shortcut
3. Location: powershell.exe -WindowStyle Hidden -ExecutionPolicy Bypass -File "C:\full\path\to\pc-monitor.ps1"
4. Name: ESP32 Monitor
5. Click Finish

Note: For Method 2, you may need to adjust Execution Policy permanently:
Run PowerShell as Administrator:
Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned

## ⚙️ Configuration

Edit pc-monitor.ps1:
$ESP32_HOSTNAME = "esp32monitor.local"
$UDP_PORT = 4210

Edit src/main.cpp:
const int UDP_PORT = 4210;
wm.setConfigPortalTimeout(120);

WiFi Credentials Reset:
1. Hold BOOT button for 5 seconds while powering on
2. Or reflash with: pio run --target erase
3. AP portal will start automatically

## 📊 How It Works

Data Flow:
PC (PowerShell) -> UDP Port 4210 -> ESP32 -> OLED Display

JSON Format:
{
  "cpu_temp": 45.2,
  "gpu_temp": 62.0,
  "cpu_usage": 35,
  "gpu_usage": 5,
  "ram_usage": 58
}

Monitoring Methods:
- CPU Usage: Windows Performance Counters
- GPU Usage: WMI Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine
- RAM Usage: Windows CIM/WMI
- Temperatures: Estimated based on usage (native Windows doesn't expose iGPU temps)

## 🖥️ Display Layout

System Monitor       OK [-45]
CPU: 35%        45.2C
[progress bar]
GPU: 5%         48.0C
[progress bar]
RAM: 58%
[progress bar]

Status indicators:
- OK - Receiving data
- No - No data for 10+ seconds
- [-45] - WiFi signal strength (RSSI in dBm)

## 🔐 Security

- AP Password: 12345678 (change in main.cpp)
- AP Timeout: 120 seconds (auto-disables)
- Network: Only accessible within local network
- UDP: No encryption (local network only)

## 🐛 Troubleshooting

ESP32 won't connect to WiFi:
- Check credentials in WiFiManager portal
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Restart ESP32 and try again

PowerShell script shows "Cannot resolve":
- Ensure ESP32 and PC are on same network
- Check mDNS is working: ping esp32monitor.local
- Use IP address directly if mDNS fails

GPU usage always 0%:
- This is normal for integrated graphics at idle
- Run a game or benchmark to see GPU load
- WMI GPU counters require Windows 10/11

Display shows garbage/nothing:
- Check I2C wiring (SDA=GPIO8, SCL=GPIO9)
- Ensure 3.3V power (not 5V!)
- Try SH1106 controller instead of SSD1306

##  Version History

v9.1 - Auto-AP on boot, disabled blue LED, WMI GPU monitoring, auto-start guide
v9.0 - mDNS support, auto-reconnect, progress bars
v7.0 - UDP server with JSON parsing
v6.0 - WiFiManager integration
v1.0 - Initial release

## 📄 License

MIT License - see LICENSE file for details.

## 🤝 Contributing

Feel free to submit issues and enhancement requests!

## 📞 Support

For questions and support, open an issue on GitHub.

---

## ✨ ESP32 System Monitor (РУССКАЯ ВЕРСИЯ)

Компактный беспроводной монитор системных ресурсов ПК на базе ESP32-C3 с OLED дисплеем 0.96".

## ✨ Возможности

-  Беспроводной мониторинг - передача данных по UDP
- 📊 Статистика в реальном времени - CPU/GPU, температуры, RAM
- 📶 Умный WiFi - mDNS (esp32monitor.local), автопереподключение
- 🔐 Безопасная точка доступа - защищённый паролем портал
- 🎨 Чистый интерфейс - прогресс-бары и выравнивание
- 🔋 Готов к батарее - поддержка модуля IP5306
- 💻 Нативный Windows - PowerShell, не требует Python

## 🛠️ Необходимое оборудование

- Микроконтроллер: ESP32-C3 SuperMini
- Дисплей: OLED SSD1306 0.96" (I2C)
- Питание: Li-Ion аккумулятор (1600mAh)
- Модуль зарядки: IP5306
- Переключатель: Миниатюрный слайдер (опционально)

## 🔌 Подключение

OLED дисплей к ESP32-C3:
VCC (3.3V) -> 3.3V
GND -> GND
SDA -> GPIO 8
SCL -> GPIO 9

Питание:
Аккумулятор -> IP5306 (B+/B-)
IP5306 (OUT+/OUT-) -> Переключатель -> ESP32 (5V и GND)

## 📦 Установка

### 1. Прошивка ESP32

Требования:
- VS Code
- PlatformIO

Шаги:
Подключите ESP32-C3 по USB
Нажмите Build и Upload или выполните:
pio run --target upload

### 2. Настройка WiFi

Первый запуск:
1. ESP32 создаёт точку доступа: System_monitor
2. Пароль: 12345678
3. Подключитесь с телефона/ПК
4. Откроется портал (или перейдите на 192.168.4.1)
5. Введите данные вашего WiFi
6. ESP32 сохранит и покажет IP на экране

Перенастройка WiFi:
- Перезагрузите ESP32 без доступного WiFi
- Через 30 секунд автоматически запустится портал
- Подключитесь и введите новые данные

### 3. Запуск клиента на ПК

Требования:
- Windows 10/11
- PowerShell 5.1+

Шаги:
Откройте PowerShell и выполните:
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
cd "путь\к\проекту"
.\pc-monitor.ps1

## 🚀 Автозапуск скрипта (при старте Windows)

### Способ 1: Планировщик заданий (Рекомендуется)

1. Откройте Планировщик заданий (найдите в меню Пуск)
2. Нажмите "Создать простую задачу" в правой панели
3. Имя: "ESP32 Monitor"
4. Триггер: "При входе в систему"
5. Действие: "Запустить программу"
6. Программа: powershell.exe
7. Аргументы: -WindowStyle Hidden -ExecutionPolicy Bypass -File "C:\полный\путь\к\pc-monitor.ps1"
8. Нажмите Готово
9. Правой кнопкой на задаче -> Свойства -> Отметьте "Выполнять с наивысшими правами"

### Способ 2: Папка Автозагрузка (Проще)

1. Нажмите Win+R, введите: shell:startup
2. Правой кнопкой в папке -> Создать -> Ярлык
3. Расположение: powershell.exe -WindowStyle Hidden -ExecutionPolicy Bypass -File "C:\полный\путь\к\pc-monitor.ps1"
4. Имя: ESP32 Monitor
5. Нажмите Готово

Примечание: Для Способа 2 может потребоваться изменить политику выполнения:
Запустите PowerShell от имени администратора:
Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned

## ⚙️ Настройка

В pc-monitor.ps1:
$ESP32_HOSTNAME = "esp32monitor.local"
$UDP_PORT = 4210

В src/main.cpp:
const int UDP_PORT = 4210;
wm.setConfigPortalTimeout(120);

Сброс WiFi:
1. Зажмите кнопку BOOT на 5 секунд при включении
2. Или перепрошейте: pio run --target erase
3. Портал запустится автоматически

##  Как это работает

Поток данных:
ПК (PowerShell) -> UDP порт 4210 -> ESP32 -> OLED дисплей

Формат JSON:
{
  "cpu_temp": 45.2,
  "gpu_temp": 62.0,
  "cpu_usage": 35,
  "gpu_usage": 5,
  "ram_usage": 58
}

Методы мониторинга:
- CPU: Windows Performance Counters
- GPU: WMI класс Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine
- RAM: Windows CIM/WMI
- Температуры: Расчётные (Windows не предоставляет температуры iGPU)

## 🖥️ Расположение на экране

System Monitor       OK [-45]
CPU: 35%        45.2C
[прогресс-бар]
GPU: 5%         48.0C
[прогресс-бар]
RAM: 58%
[прогресс-бар]

Индикаторы:
- OK - данные приходят
- No - нет данных 10+ секунд
- [-45] - сила WiFi сигнала (RSSI)

## 🔐 Безопасность

- Пароль AP: 12345678 (можно изменить в main.cpp)
- Таймаут AP: 120 секунд
- Доступ: Только в локальной сети
- UDP: Без шифрования (только локальная сеть)

## 🐛 Решение проблем

ESP32 не подключается к WiFi:
- Проверьте данные в портале WiFiManager
- Убедитесь, что сеть 2.4GHz (ESP32 не поддерживает 5GHz)
- Перезагрузите ESP32

Скрипт показывает "Cannot resolve":
- Убедитесь, что ESP32 и ПК в одной сети
- Проверьте mDNS: ping esp32monitor.local
- Используйте IP напрямую, если mDNS не работает

Загрузка GPU всегда 0%:
- Это нормально для встроенной графики в простое
- Запустите игру или бенчмарк для проверки
- Требуется Windows 10/11 для WMI счётчиков

Дисплей показывает мусор/пустой:
- Проверьте подключение I2C (SDA=GPIO8, SCL=GPIO9)
- Убедитесь в питании 3.3V (не 5V!)
- Попробуйте контроллер SH1106 вместо SSD1306

## 📝 История версий

v9.1 - Авто-AP при загрузке, отключение синего LED, WMI GPU мониторинг, инструкция по автозапуску
v9.0 - Поддержка mDNS, автопереподключение, прогресс-бары
v7.0 - UDP сервер с парсингом JSON
v6.0 - Интеграция WiFiManager
v1.0 - Первый релиз

## 📄 Лицензия

MIT License - см. файл LICENSE.

## 🤝 Участие в разработке

Не стесняйтесь создавать issues и предлагать улучшения!

## 📞 Поддержка

Для вопросов и поддержки создайте issue на GitHub.

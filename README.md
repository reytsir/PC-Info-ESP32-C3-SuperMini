# ESP32 System Monitor

> 🖥️ A compact, wireless PC system resource monitor built on ESP32-C3 with a 0.96" OLED display. Monitors CPU/GPU usage, temperatures, and RAM in real-time.

---

## ✨ Features

- 📡 Wireless monitoring — UDP data transmission, no cables needed
- 📊 Real-time stats — CPU/GPU usage, temperatures, RAM usage
- 📶 Smart WiFi — mDNS support (esp32.local), auto-reconnect
- 🔐 Secure AP — Password-protected configuration portal
- 🎨 Clean UI — Right-aligned display with progress bars
- 🔋 Battery ready — Custom IP5306 + MT3608 boost converter power scheme
- 💻 Native Windows — PowerShell client, runs silently in background

---

## ⚠️ Important Prerequisite for Windows

For the mDNS name (esp32.local) to work on Windows, you must install Apple's Bonjour service (it adds native mDNS support to Windows).
- Download: Bonjour Print Services for Windows
- Install it and restart your PC.
- (Alternative: If you don't install Bonjour, you must use the direct IP address in the script instead of esp32.local).

---

## 🛠️ Hardware Requirements

| Component | Specification |
|-----------|---------------|
| Microcontroller | ESP32-C3 SuperMini |
| Display | OLED SSD1306 0.96" (I2C, 3.3V) |
| Battery | Li-Ion (e.g., 1600mAh) |
| Charging Module | IP5306 Power Management |
| Boost Converter | MT3608 Step-Up Module (set to ~5V) |
| Switch | Miniature slide switch |
| Capacitor | 470µF 10V (highly recommended across MT3608 VOUT+/VOUT- for stability) |

---

## 🔌 Wiring Diagram

### Power Path (Critical for stability)
> [Battery +] ──> IP5306 (B+) [Battery -] ──> IP5306 (B-) AND MT3608 (VIN-)  IP5306 (OUT+) ──> [Switch] ──> MT3608 (VIN+) IP5306 (OUT-) ──> MT3608 (VIN-) [Shared Ground]  MT3608 (VOUT+) ──> ESP32 (5V) MT3608 (VOUT-) ──> ESP32 (GND) AND OLED (GND) [Shared Ground Point] 
💡 Tip: Solder a 470µF capacitor between MT3608 VOUT+ and VOUT- to prevent voltage drops when ESP32 WiFi spikes.

### Data & OLED Power
> ESP32 3.3V ──> OLED VCC ESP32 GND ──> OLED GND (Connect to the shared MT3608 VOUT- ground) ESP32 GPIO 8 ──> OLED SDA ESP32 GPIO 9 ──> OLED SCL 

---

## 📦 Installation

### 1. Flash ESP32 Firmware
Requirements: VS Code with PlatformIO extension.

Steps:
1. Connect ESP32-C3 via USB.
2. Press Build and Upload (→) or run in terminal:
 bash  pio run --target upload 

### 2. Configure WiFi
First Boot (or when saved WiFi is unavailable):
1. ESP32 creates an access point: System_monitor
2. Password: 12345678
3. Connect your phone or PC to this network.
4. A captive portal will open automatically (or visit 192.168.4.1).
5. Enter your home/office WiFi credentials.
6. ESP32 saves them, connects, and displays its IP and esp32.local on the screen.

To Reconfigure: Restart the ESP32 when your saved WiFi is unavailable. After 15 seconds, the AP portal will auto-start.

### 3. Run PC Client
Prerequisites: Windows 10/11, PowerShell 5.1+, Bonjour installed.

Steps:
1. Open PowerShell.
2. Allow script execution (one-time only):
 powershell  Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass 
3. Navigate to the project folder and run:
 powershell  .\pc-monitor.ps1 

---

## 🚀 Auto-Start Setup (Run Silently on Windows Startup)

To make the script start hidden in the background when you log in, use Task Scheduler (Recommended).

1. Open Task Scheduler (search in Start menu).
2. Click Create Basic Task in the right panel.
3. Name: ESP32 Monitor. Click Next.
4. Trigger: When I log on. Click Next.
5. Action: Start a program. Click Next.
6. Program/script: powershell.exe
7. Add arguments (copy exactly, replace path with yours):
 >  -WindowStyle Hidden -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "C:\full\path\to\your\pc-monitor.ps1" 
8. Click Finish.
9. Find your new task in the library, right-click it → Properties.
10. Check "Run with highest privileges" and click OK.

(The script is designed to wait patiently if the ESP32 is turned on after the PC, retrying connection every 5 seconds).

---

## ⚙️ Configuration

In pc-monitor.ps1:
powershell $HOSTNAME = "esp32.local" # mDNS hostname (requires Bonjour) $PORT = 4210 # UDP port 

In src/main.cpp:
cpp const int UDP_PORT = 4210; // mDNS name is set to "esp32" -> resolves to esp32.local 

Reset WiFi Credentials:
1. Hold the BOOT button on ESP32 for 5 seconds while powering on.
2. Or erase flash via PlatformIO: pio run --target erase.
3. The AP portal will start automatically on next boot.

---

## 📊 How It Works

Data Flow:
PC (PowerShell) → UDP Port 4210 → ESP32 → OLED Display

JSON Payload:
json {  "cpu_temp": 45.2,  "gpu_temp": 62.0,  "cpu_usage": 35,  "gpu_usage": 5,  "ram_usage": 58 } 

Monitoring Methods:
- CPU: Windows Performance Counters
- GPU: WMI Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine
- RAM: Windows CIM/WMI
- Temperatures: Estimated based on load (native Windows does not expose integrated GPU temperatures).

---

## 🖥️ Display Layout

> System Monitor OK CPU: 35% 45.2C [███████░░░░░░░░░░░] GPU: 5% 48.0C [█░░░░░░░░░░░░░░░░░] RAM: 58% [████████░░░░░░░░░░] 
Status indicators: OK (Receiving data) | No (No data for 10+ seconds)

---

## 🐛 Troubleshooting

- ESP32 won't connect to WiFi: Ensure it's a 2.4GHz network (ESP32-C3 does not support 5GHz). Check password in the portal.
- PowerShell shows "Waiting for ESP32":
 1. Ensure ESP32 and PC are on the same network.
 2. Verify mDNS: open CMD and type ping esp32.local.
 3. If it fails, ensure Bonjour is installed, or change $HOSTNAME to the direct IP (e.g., "192.168.0.221") in the script.
- GPU usage always 0%: Normal for integrated graphics at desktop idle. Launch a game or benchmark to see it rise. Requires Windows 10/11.
- Display shows garbage/nothing: Check I2C wiring (SDA=GPIO8, SCL=GPIO9). Ensure OLED is powered by 3.3V, not 5V. Check for cold solder joints.
- ESP32 restarts when WiFi turns on: Add a 470µF capacitor across the MT3608 VOUT+ and VOUT- to handle the 500mA WiFi power spike.

---

## 📝 Version History

- v1.0.0 - Final Release: MT3608 power scheme, mDNS fix (esp32.local), silent auto-start, optimized code, bilingual docs.
- v0.9.x - Development iterations: WMI GPU monitoring, auto-AP fallback, progress bars.

---

## 📄 License

MIT License - see LICENSE file for details.

---

## 🤝 Contributing & Support

Feel free to submit issues and enhancement requests on GitHub!

---
---

# ✨ ESP32 System Monitor (РУССКАЯ ВЕРСИЯ)

Компактный беспроводной монитор системных ресурсов ПК на базе ESP32-C3 с OLED дисплеем 0.96". Отслеживает загрузку CPU/GPU, температуры и использование RAM в реальном времени.

---

## ⚠️ Важное требование для Windows

Для работы mDNS-имени (esp32.local) в Windows необходимо установить службу Bonjour (она добавляет нативную поддержку mDNS).
- Скачать: Bonjour Print Services for Windows
- Установите и перезагрузите ПК.
- (Альтернатива: если не устанавливать Bonjour, в скрипте придется использовать прямой IP-адрес вместо esp32.local).

---

## 🛠️ Необходимое оборудование

| Компонент | Характеристики |
|-----------|----------------|
| Микроконтроллер | ESP32-C3 SuperMini |
| Дисплей | OLED SSD1306 0.96" (I2C, 3.3V) |
| Питание | Li-Ion аккумулятор (например, 1600mAh) |
| Модуль зарядки | IP5306 |
| Повышающий преобразователь | MT3608 (настроен на ~5V) |
| Переключатель | Миниатюрный слайдер |
| Конденсатор | 470 мкФ 10В (настоятельно рекомендуется между VOUT+/VOUT- модуля MT3608 для стабильности) |

---

## 🔌 Схема подключения

### Цепь питания (Критично для стабильности)
> [Аккумулятор +] ──> IP5306 (B+) [Аккумулятор -] ──> IP5306 (B-) И MT3608 (VIN-)  IP5306 (OUT+) ──> [Выключатель] ──> MT3608 (VIN+) IP5306 (OUT-) ──> MT3608 (VIN-) [Общая земля]  MT3608 (VOUT+) ──> ESP32 (5V) MT3608 (VOUT-) ──> ESP32 (GND) И OLED (GND) [Точка объединения земли] 
💡 Совет: Припаяйте конденсатор 470 мкФ между VOUT+ и VOUT- модуля MT3608, чтобы предотвратить просадки напряжения при пиковом потреблении WiFi модуля ESP32.

### Данные и питание OLED
> ESP32 3.3V ──> OLED VCC ESP32 GND ──> OLED GND (Подключить к общей земле MT3608 VOUT-) ESP32 GPIO 8 ──> OLED SDA ESP32 GPIO 9 ──> OLED SCL 

---

## 📦 Установка

### 1. Прошивка ESP32
Требования: VS Code с расширением PlatformIO.

Шаги:
1. Подключите ESP32-C3 по USB.
2. Нажмите Build and Upload (→) или выполните в терминале:
 bash  pio run --target upload 

### 2. Настройка WiFi
Первый запуск (или если сохраненная сеть недоступна):
1. ESP32 создает точку доступа: System_monitor
2. Пароль: 12345678
3. Подключите телефон или ПК к этой сети.
4. Автоматически откроется портал настройки (или перейдите по адресу 192.168.4.1).
5. Введите данные вашего домашнего/рабочего WiFi.
6. ESP32 сохранит их, подключится и покажет свой IP и esp32.local на экране.

Для перенастройки: Перезагрузите ESP32, когда сохраненная сеть WiFi недоступна. Через 15 секунд портал настройки запустится автоматически.

### 3. Запуск клиента на ПК
Требования: Windows 10/11, PowerShell 5.1+, установленный Bonjour.

Шаги:
1. Откройте PowerShell.
2. Разрешите выполнение скриптов (один раз):
 powershell  Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass 
3. Перейдите в папку проекта и запустите:
 powershell  .\pc-monitor.ps1 

---

## 🚀 Автозапуск (Скрытый режим при старте Windows)

Чтобы скрипт запускался скрыто в фоне при входе в систему, используйте Планировщик заданий.

1. Откройте Планировщик заданий (найдите в меню Пуск).
2. Нажмите Создать простую задачу в правой панели.
3. Имя: ESP32 Monitor. Нажмите "Далее".
4. Триггер: При входе в систему. Нажмите "Далее".
5. Действие: Запустить программу. Нажмите "Далее".
6. Программа или сценарий: powershell.exe
7. Добавить аргументы (скопируйте точно, заменив путь на свой):
 >  -WindowStyle Hidden -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "C:\полный\путь\к\вашему\pc-monitor.ps1" 
8. Нажмите Готово.
9. Найдите созданную задачу в библиотеке, кликните правой кнопкой → Свойства.
10. Поставьте галочку "Выполнять с наивысшими правами" и нажмите ОК.

(Скрипт настроен так, что если ПК включился раньше ESP32, он будет терпеливо ждать появления платы в сети, проверяя связь каждые 5 секунд).

---

## ⚙️ Настройка

В pc-monitor.ps1:
powershell $HOSTNAME = "esp32.local" # mDNS имя (требует Bonjour) $PORT = 4210 # UDP порт 

В src/main.cpp:
cpp const int UDP_PORT = 4210; // Имя mDNS установлено как "esp32" -> резолвится как esp32.local 

Сброс настроек WiFi:
1. Зажмите кнопку BOOT на ESP32 на 5 секунд при включении.
2. Или очистите память через PlatformIO: pio run --target erase.
3. При следующем включении портал настройки запустится автоматически.

---

## 📊 Как это работает

Поток данных:
ПК (PowerShell) → UDP порт 4210 → ESP32 → OLED дисплей

Методы мониторинга:
- CPU: Счетчики производительности Windows
- GPU: WMI класс Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine
- RAM: Windows CIM/WMI
- Температуры: Расчетные на основе нагрузки (Windows не предоставляет нативный доступ к датчикам температуры встроенной графики).

---

## 🖥️ Расположение на экране

> System Monitor OK CPU: 35% 45.2C [прогресс-бар] GPU: 5% 48.0C [прогресс-бар] RAM: 58% [прогресс-бар] 
Индикаторы статуса: OK (данные приходят) | No (нет данных более 10 секунд)

---

## 🐛 Решение проблем

- ESP32 не подключается к WiFi: Убедитесь, что это сеть 2.4 ГГц (ESP32-C3 не поддерживает 5 ГГц). Проверьте пароль в портале.
- Скрипт пишет "Waiting for ESP32":
 1. Убедитесь, что ESP32 и ПК находятся в одной сети.
 2. Проверьте mDNS: откройте CMD и введите ping esp32.local.
 3. Если не пингуется, убедитесь, что Bonjour установлен, или измените $HOSTNAME на прямой IP (например, "192.168.0.221") в скрипте.
- Загрузка GPU всегда 0%: Нормально для встроенной графики в режиме простоя. Запустите игру или бенчмарк, чтобы увидеть рост. Требуется Windows 10/11.
- Дисплей показывает мусор или не горит: Проверьте провода I2C (SDA=GPIO8, SCL=GPIO9). Убедитесь, что OLED запитан от 3.3V, а не от 5V. Проверьте качество пайки.
- ESP32 перезагружается при включении WiFi: Припаяйте конденсатор 470 мкФ между VOUT+ и VOUT- модуля MT3608, чтобы сгладить пиковое потребление тока модулем WiFi.

---

## 📝 История версий

- v1.0.0 - Финальный релиз: схема питания с MT3608, исправление mDNS (esp32.local), скрытый автозапуск, оптимизация кода, двуязычная документация.
- v0.9.x - Этапы разработки: WMI мониторинг GPU, авто-AP, прогресс-бары.

---

## 📄 Лицензия

MIT License - см. файл LICENSE.

---

## 🤝 Участие в разработке и Поддержка

Не стесняйтесь создавать issues и предлагать улучшения на GitHub!
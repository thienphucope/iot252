# Báo Cáo Dự Án: YoloUNO PlatformIO-RTOS

**Môn học:** Internet of Things  
**Nền tảng:** ESP32-S3 (YOLO UNO board) — PlatformIO + FreeRTOS  
**Ngày báo cáo:** 22/05/2026  

---

## 1. Giới Thiệu & Mục Tiêu

### 1.1 Tổng quan dự án

Dự án này xây dựng một hệ thống IoT nhúng toàn diện trên bo mạch **YOLO UNO** (chip ESP32-S3 Dual Core 240 MHz, 4MB Flash, 8MB PSRAM) sử dụng framework Arduino trên PlatformIO kết hợp với **FreeRTOS** để quản lý đa nhiệm (multi-tasking) theo thời gian thực.

Hệ thống đọc dữ liệu từ cảm biến nhiệt độ/độ ẩm DHT20, xử lý tín hiệu qua các cơ chế RTOS (Queue và Semaphore), điều khiển các thiết bị ngoại vi (LED, NeoPixel, LCD), cung cấp giao diện web, chạy mô hình học máy nhỏ (TinyML) và đẩy dữ liệu lên nền tảng đám mây CoreIOT.

### 1.2 Mục tiêu cụ thể

| Task | Mục tiêu | Trạng thái |
|------|----------|------------|
| Task 1 | LED đơn chớp theo 3 mức nhiệt độ, đồng bộ qua Semaphore | Hoàn thành |
| Task 2 | NeoPixel RGB đổi màu theo 3 mức độ ẩm, dùng Semaphore | Hoàn thành |
| Task 3 | LCD hiển thị 3 trạng thái, loại bỏ biến toàn cục, dùng Queue + Semaphore | Hoàn thành |
| Task 4 | Web Server AP Mode, điều khiển relay qua WebSocket | Hoàn thành (cần cải tiến UI) |
| Task 5 | TinyML phát hiện bất thường nhiệt độ/độ ẩm bằng ANN | Đã triển khai |
| Task 6 | Đẩy telemetry lên CoreIOT qua MQTT (chế độ STA) | Đang chờ token |

### 1.3 Phần cứng sử dụng

- **Vi điều khiển:** YOLO UNO (ESP32-S3, OhSTEM)
- **Cảm biến:** DHT20 (nhiệt độ/độ ẩm, giao tiếp I2C trên SDA=GPIO11, SCL=GPIO12)
- **Hiển thị:** LCD 16x2 I2C địa chỉ 0x27
- **Chấp hành:** LED đơn (GPIO 48), NeoPixel RGB (GPIO 45, thư viện Adafruit)
- **Nút nhấn:** BOOT button (GPIO 0) để xóa cấu hình
- **Kết nối:** WiFi 2.4GHz (802.11b/g/n)

---

## 2. Thiết Bị Sử Dụng

### 2.1 Thiết bị đầu vào

| Thiết bị | Loại | Giao tiếp | GPIO | Vai trò |
|----------|------|-----------|------|---------|
| **DHT20** | Cảm biến nhiệt độ & độ ẩm | I2C | SDA=11, SCL=12 | Cung cấp dữ liệu T/H cho toàn bộ hệ thống |
| **BOOT Button** | Nút nhấn tích hợp | GPIO | 0 | Xóa cấu hình WiFi/CoreIOT khi giữ > 2 giây |

**DHT20** sử dụng thư viện `AHTxx` để đọc giá trị qua I2C. Cảm biến được đọc mỗi 5 giây trong task `temp_humi_monitor`. Địa chỉ I2C mặc định của DHT20 là `0x38`.

**BOOT Button** được giám sát bởi task `Task_Toogle_BOOT` với cơ chế debounce: task đếm thời gian giữ nút, nếu vượt 2 giây thì gọi `LittleFS.format()` để xóa toàn bộ cấu hình đã lưu và khởi động lại thiết bị.

### 2.2 Thiết bị đầu ra

| Thiết bị | Loại | Giao tiếp | GPIO | Vai trò |
|----------|------|-----------|------|---------|
| **LED đơn** | LED tích hợp trên board | Digital output | 48 | Chớp theo 3 mức nhiệt độ (Task 1) |
| **NeoPixel RGB** | LED RGB addressable (WS2812B) | Digital (1-wire) | 45 | Đổi màu theo 3 mức độ ẩm (Task 2) |
| **LCD 16×2 I2C** | Màn hình ký tự | I2C | Địa chỉ 0x27 | Hiển thị T, H và trạng thái hệ thống (Task 3) |
| **Relay (GPIO bất kỳ)** | Relay module | Digital output | Cấu hình qua Web | Điều khiển thiết bị điện qua Web dashboard (Task 4) |

**LED đơn** được điều khiển bằng `digitalWrite(48, HIGH/LOW)` với chu kỳ thay đổi theo semaphore nhiệt độ: 200 ms (nóng), 1000 ms (bình thường), 2000 ms (lạnh).

**NeoPixel** sử dụng thư viện `Adafruit_NeoPixel` với độ sáng 50%. Màu được cập nhật ngay lập tức khi semaphore độ ẩm được give: Đỏ (khô), Xanh lá (bình thường), Xanh dương (ẩm).

**LCD 16×2** được điều khiển qua thư viện `LiquidCrystal_I2C`. Task `lcd_task` cập nhật 2 dòng mỗi 500 ms:
- Dòng 0: `STAT: NORMAL  ` / `STAT: WARNING ` / `STAT: CRITICAL`
- Dòng 1: `T:28.5C H:55.3%`

**Relay động**: Người dùng thêm relay qua Web dashboard bằng cách nhập tên và số GPIO. Firmware thực hiện `digitalWrite(gpio, state)` khi nhận lệnh qua WebSocket.

---

## 2. Kiến Trúc Hệ Thống

### 2.1 Cấu trúc thư mục

```
YoloUNO_PlatformIO-RTOS_Project/
├── platformio.ini              # Cấu hình build, thư viện, board
├── src/
│   ├── main.cpp                # Khởi tạo và đăng ký tất cả RTOS Task
│   ├── global.cpp              # Khởi tạo Queue và Semaphore
│   ├── actuators/
│   │   ├── led_blinky.cpp      # Task 1: LED chớp theo nhiệt độ
│   │   └── neo_blinky.cpp      # Task 2: NeoPixel theo độ ẩm
│   ├── sensors/
│   │   ├── temp_humi_monitor.cpp  # Task 3: Đọc DHT20, đóng gói, gửi Queue
│   │   ├── lcd_task.cpp           # Task 3: Hiển thị LCD từ Queue
│   │   └── task_rs485.cpp         # Modbus RS485 (relay điều khiển)
│   ├── connectivity/
│   │   ├── task_wifi.cpp          # Quản lý WiFi AP+STA
│   │   ├── task_check_info.cpp    # Đọc/ghi LittleFS credentials
│   │   └── task_toogle_boot.cpp   # Nút BOOT xóa cấu hình
│   ├── web_services/
│   │   ├── task_webserver.cpp     # Task 4: AsyncWebServer + WebSocket
│   │   └── task_handler.cpp       # Xử lý message WebSocket
│   ├── cloud/
│   │   └── task_core_iot.cpp      # Task 6: MQTT lên CoreIOT
│   └── tinyml/
│       ├── tinyml.cpp             # Task 5: TFLite Micro inference
│       ├── dht_model_v2.h         # Model TFLite (.tflite as C array)
│       ├── custom_iot_dataset.csv # Dataset 2000 mẫu
│       └── train_anomaly_detector.ipynb  # Notebook huấn luyện
├── include/
│   ├── global.h                # Struct SensorData, khai báo Queue/Semaphore
│   ├── actuators/              # Header cho LED, NeoPixel
│   ├── sensors/                # Header cho DHT20, LCD
│   ├── connectivity/           # Header cho WiFi, LittleFS
│   ├── web_services/           # Header cho WebServer, Handler
│   ├── cloud/                  # Header cho CoreIOT
│   └── tinyml/
│       ├── tinyml.h            # Header TinyML task
│       └── dht_anomaly_model.h # Model v1 (dự phòng)
└── data/                       # File web phục vụ qua LittleFS
    ├── index.html              # Dashboard 4 trang (Home/Device/Info/Settings)
    ├── script.js               # WebSocket client, gauges, relay control
    ├── styles.css              # CSS responsive
    ├── justgage.min.js         # Thư viện gauge đồng hồ
    └── raphael.min.js          # SVG rendering cho gauge
```

### 2.2 Sơ đồ luồng dữ liệu

```
+-----------------------------------------------------------------------------+
|                          YOLO UNO (ESP32-S3)                                |
|                                                                             |
|  +------------------+    xQueueOverwrite     +------------------------+     |
|  |  temp_humi_      | ---------------------->|     xSensorQueue       |     |
|  |  monitor (5s)    |    struct SensorData   |  (Queue depth=1,       |     |
|  |  [DHT20 I2C]     |    {temp,humi,status}  |   "Mailbox" pattern)   |     |
|  +------------------+                        +----------+-------------+     |
|          |                                              |                   |
|          | xSemaphoreGive                    xQueuePeek| (non-destructive) |
|          | (khi mức thay đổi)                          |                   |
|          v                                   +----------+--------------+    |
|  +--------------------------------------+    |  lcd_task (500ms)       |    |
|  |       9 Binary Semaphore             |    |  - Hiển thị T, H, STAT  |    |
|  |  Temp: Low / Normal / High           |    |  - Drain 3 status sema  |    |
|  |  Humi: Low / Normal / High           |    +------------------------+     |
|  |  Status: Normal / Warning / Critical |                                   |
|  +--------+----------+------------------+    +------------------------+     |
|           |          |                       |  task_webserver (100ms)|     |
|           v          v                       |  - Broadcast sensor/5s |     |
|  +--------------+  +--------------+          |  - Nhận GPIO command   |     |
|  | led_blinky   |  | neo_blinky   |          |  - WiFi scan           |     |
|  | (Task 1)     |  | (Task 2)     |          +------------------------+     |
|  | GPIO 48 LED  |  | GPIO 45 NEO  |                                         |
|  | 200/1000/    |  | Blue/Green/  |          +------------------------+     |
|  | 2000ms blink |  | Red color    |          |  tiny_ml_task (5s)     |     |
|  +--------------+  +--------------+          |  - Z-score normalize   |     |
|                                              |  - TFLite inference    |     |
|  +--------------------------------------+    |  - Give status sema    |     |
|  |  task_core_iot (MQTT 10s)            |    +------------------------+     |
|  |  - Chờ xBinarySemaphoreInternet      |                                   |
|  |  - Ket noi app.coreiot.io:1883       |    +------------------------+     |
|  |  - Publish esp/telemetry             |    |  Task_Toogle_BOOT      |     |
|  +--------------------------------------+    |  - Giu >2s -> xoa FS   |     |
|                                              +------------------------+     |
+-----------------------------------------------------------------------------+

   WiFi AP (192.168.4.1)         WiFi STA -> Router -> Internet -> CoreIOT
         ^                                              ^
         | WebSocket ws://                              | MQTT
    Browser / Mobile                          app.coreiot.io:1883
```

### 2.3 Danh sách RTOS Tasks

| Tên Task | Hàm | Stack (bytes) | Ưu tiên | Vai trò |
|----------|-----|---------------|---------|---------|
| Task LED Blink | `led_blinky` | 2048 | 2 | Điều khiển LED GPIO 48 theo Semaphore nhiệt độ |
| Task NEO Blink | `neo_blinky` | 2048 | 2 | Điều khiển NeoPixel GPIO 45 theo Semaphore độ ẩm |
| Task TEMP HUMI Monitor | `temp_humi_monitor` | 4096 | 2 | Đọc DHT20, gửi Queue, give Semaphore trạng thái |
| Task LCD Display | `lcd_task` | 4096 | 2 | Nhận Queue, hiển thị LCD 16x2 I2C |
| Tiny ML Task | `tiny_ml_task` | 4096 | 2 | TFLite inference phát hiện bất thường |
| Task CoreIOT | `task_core_iot` | 8192 | 2 | MQTT telemetry lên CoreIOT |
| Task WebServer | `task_webserver_run` | 8192 | 2 | AsyncWebServer + WebSocket dashboard |
| Task_Toogle_BOOT | `Task_Toogle_BOOT` | 4096 | 2 | Giám sát nút BOOT, xóa LittleFS khi giữ >2s |

> Ngoài 8 task trên, vòng `loop()` của Arduino (chạy trên `loopTask` mặc định của ESP32) thực hiện kiểm tra WiFi reconnect mỗi 5 giây.

### 2.4 Kiến Trúc Phần Mềm Của Hệ Thống

#### 2.4.1 Mô hình phân lớp

```
+---------------------------------------------------------------+
|                    Lớp Ứng Dụng (Application)                 |
|  Task 1 (LED)  Task 2 (NEO)  Task 3 (LCD)  Task 4 (WebServer) |
|  Task 5 (TinyML)             Task 6 (CoreIOT)                  |
+---------------------------------------------------------------+
|                 Lớp Truyền Thông Dữ Liệu RTOS                 |
|       xSensorQueue (Mailbox)  +  9 Binary Semaphore            |
+---------------------------------------------------------------+
|                  Lớp Thu Thập Dữ Liệu (Sensors)               |
|   temp_humi_monitor  →  đọc DHT20, tính trạng thái, phân phối |
+---------------------------------------------------------------+
|              Lớp Kết Nối (Connectivity)                        |
|   task_wifi (AP+STA)   task_check_info (LittleFS credentials)  |
+---------------------------------------------------------------+
|             FreeRTOS Scheduler + Arduino HAL + ESP-IDF         |
+---------------------------------------------------------------+
|                  ESP32-S3 Hardware (240 MHz Dual Core)         |
+---------------------------------------------------------------+
```

#### 2.4.2 Quy trình khởi động (Boot sequence)

```
setup()
  │
  ├─ init_globals()           ← Tạo Queue + 9 Semaphore (TRƯỚC xTaskCreate)
  │
  ├─ xTaskCreate(led_blinky)
  ├─ xTaskCreate(neo_blinky)
  ├─ xTaskCreate(temp_humi_monitor)
  ├─ xTaskCreate(lcd_task)
  ├─ xTaskCreate(tiny_ml_task)
  ├─ xTaskCreate(task_core_iot)
  ├─ xTaskCreate(task_webserver_run)
  └─ xTaskCreate(Task_Toogle_BOOT)
         │
         └─ FreeRTOS Scheduler bắt đầu điều phối
```

**Quy tắc quan trọng:** `init_globals()` bắt buộc phải được gọi trước bất kỳ `xTaskCreate()` nào. Nếu Queue/Semaphore chưa được tạo mà task đã chạy và gọi `xSemaphoreTake()`, firmware sẽ crash (NULL pointer dereference).

#### 2.4.3 Chiến lược module hóa

| Nguyên tắc | Cách áp dụng |
|---|---|
| **Single Responsibility** | Mỗi file `.cpp` chỉ chứa 1 FreeRTOS task với 1 mục đích duy nhất |
| **Không chia sẻ trạng thái qua biến toàn cục** | Dữ liệu sensor truyền qua `xSensorQueue`; trạng thái thiết bị truyền qua Semaphore |
| **Producer-Consumer rõ ràng** | `temp_humi_monitor` là producer duy nhất; các task còn lại là consumer thuần túy |
| **Interface qua header** | Mỗi module có header riêng trong `include/`, không include `.cpp` chéo |
| **Cấu hình tập trung** | Ngưỡng nhiệt độ/độ ẩm định nghĩa tại `include/global.h`, thay đổi một chỗ ảnh hưởng toàn hệ thống |

#### 2.4.4 Quản lý tài nguyên phần cứng

- **Core 0 (Protocol CPU):** ESP-IDF dành riêng cho WiFi/BLE stack
- **Core 1 (Application CPU):** Tất cả FreeRTOS task của ứng dụng chạy trên core này (mặc định của `xTaskCreate`)
- **I2C bus chia sẻ:** DHT20 (0x38) và LCD (0x27) cùng bus I2C. Thư viện `Wire` của Arduino có mutex nội bộ, nhưng cần đảm bảo không có 2 task gọi I2C đồng thời — điều này được đảm bảo nhờ `temp_humi_monitor` và `lcd_task` không overlap về thời điểm gọi do chu kỳ khác nhau (5s vs 500ms).

---

## 3. Các Giải Pháp Triển Khai

### 3.1 Task 1: LED Blink Theo Nhiệt Độ

**Yêu cầu:** LED chớp với tốc độ khác nhau tương ứng ít nhất 3 mức nhiệt độ, sử dụng semaphore để đồng bộ.

**Ngưỡng nhiệt độ** (định nghĩa trong `include/global.h`):
- `TEMP_NORMAL_LOW = 25.0°C`
- `TEMP_NORMAL_HIGH = 30.0°C`

**Hành vi LED** (`src/actuators/led_blinky.cpp`):

| Mức nhiệt độ | Semaphore | Khoảng chớp |
|---|---|---|
| Lạnh: `T < 25°C` | `xTempLowSemaphore` | 2000 ms |
| Bình thường: `25 ≤ T ≤ 30°C` | `xTempNormalSemaphore` | 1000 ms |
| Nóng: `T > 30°C` | `xTempHighSemaphore` | 200 ms |

**Cơ chế hoạt động:**

Task `led_blinky` không đọc cảm biến trực tiếp. Nó liên tục kiểm tra (poll) ba Semaphore bằng `xSemaphoreTake(..., 0)` với timeout = 0 (non-blocking). Khi nhận được semaphore nào, nó cập nhật biến `blinkInterval` và duy trì giá trị đó cho đến khi có semaphore mới:

```cpp
// src/actuators/led_blinky.cpp, dòng 10-16
if (xSemaphoreTake(xTempHighSemaphore, 0) == pdTRUE) {
    blinkInterval = 200;
} else if (xSemaphoreTake(xTempNormalSemaphore, 0) == pdTRUE) {
    blinkInterval = 1000;
} else if (xSemaphoreTake(xTempLowSemaphore, 0) == pdTRUE) {
    blinkInterval = 2000;
}
```

Task `temp_humi_monitor` (producer) chỉ `xSemaphoreGive` khi **phát hiện sự chuyển mức** (edge detection), không give liên tục, nhờ biến `last_temp` theo dõi mức cũ:

```cpp
// src/sensors/temp_humi_monitor.cpp, dòng 64-70
if (temperature < TEMP_NORMAL_LOW && last_temp >= TEMP_NORMAL_LOW) {
    xSemaphoreGive(xTempLowSemaphore);
} else if (temperature >= TEMP_NORMAL_LOW && temperature <= TEMP_NORMAL_HIGH
           && (last_temp < TEMP_NORMAL_LOW || last_temp > TEMP_NORMAL_HIGH)) {
    xSemaphoreGive(xTempNormalSemaphore);
} else if (temperature > TEMP_NORMAL_HIGH && last_temp <= TEMP_NORMAL_HIGH) {
    xSemaphoreGive(xTempHighSemaphore);
}
```

**Ưu điểm thiết kế:** Chỉ give semaphore khi có thay đổi mức, tránh tràn semaphore và giảm lãng phí CPU. LED không cần phụ thuộc vào tần suất đọc cảm biến (5 giây/lần).

---

### 3.2 Task 2: NeoPixel RGB Theo Độ Ẩm

**Yêu cầu:** NeoPixel hiển thị 3 màu khác nhau tương ứng 3 mức độ ẩm.

**Ngưỡng độ ẩm** (định nghĩa trong `include/global.h`):
- `HUMI_NORMAL_LOW = 40.0%`
- `HUMI_NORMAL_HIGH = 60.0%`

**Hành vi NeoPixel** (`src/actuators/neo_blinky.cpp`):

| Mức độ ẩm | Semaphore | Màu hiển thị | RGB |
|---|---|---|---|
| Khô: `H < 40%` | `xHumiLowSemaphore` | Đỏ (Red) | (255, 0, 0) |
| Bình thường: `40 ≤ H ≤ 60%` | `xHumiNormalSemaphore` | Xanh lá (Green) | (0, 255, 0) |
| Ẩm: `H > 60%` | `xHumiHighSemaphore` | Xanh dương (Blue) | (0, 0, 255) |

**Chi tiết triển khai:** NeoPixel được khởi tạo trong hàm task với độ sáng 50% (`pixels.setBrightness(50)`) trên GPIO 45, sử dụng thư viện `Adafruit_NeoPixel`. Tương tự Task 1, task `neo_blinky` poll 3 semaphore độ ẩm với timeout 0. Semaphore được give bởi `temp_humi_monitor` khi phát hiện chuyển mức độ ẩm (`src/sensors/temp_humi_monitor.cpp`, dòng 73-79). Task cập nhật màu NeoPixel và gọi `pixels.show()` để hiển thị ngay.

---

### 3.3 Task 3: LCD + Loại Bỏ Biến Toàn Cục

**Yêu cầu:** Hiển thị 3 trạng thái (NORMAL/WARNING/CRITICAL) trên LCD, **loại bỏ hoàn toàn biến toàn cục** liên quan đến cảm biến.

#### 3.3.1 Struct SensorData thay thế biến toàn cục

Thay vì dùng `float glob_temp; float glob_humi;`, toàn bộ dữ liệu sensor được đóng gói trong một struct duy nhất (`include/global.h`, dòng 17-21):

```cpp
struct SensorData {
    float temperature;
    float humidity;
    int   lcd_status; // 0=NORMAL, 1=WARNING, 2=CRITICAL
};
```

#### 3.3.2 Mailbox Queue — truyền dữ liệu an toàn

Queue được tạo với depth = 1 để hoạt động như **mailbox** (chỉ giữ giá trị mới nhất):

```cpp
// src/global.cpp, dòng 42
xSensorQueue = xQueueCreate(1, sizeof(struct SensorData));
```

Task `temp_humi_monitor` dùng `xQueueOverwrite` (ghi đè không chờ) thay vì `xQueueSend`:

```cpp
// src/sensors/temp_humi_monitor.cpp, dòng 48
xQueueOverwrite(xSensorQueue, &data);
```

Task `lcd_task` và các task consumer khác dùng `xQueuePeek` (đọc không xóa), đảm bảo nhiều task cùng đọc dữ liệu mà không cần task nào phải đợi nhau:

```cpp
// src/sensors/lcd_task.cpp, dòng 22
xQueuePeek(xSensorQueue, &currentData, (TickType_t)100 / portTICK_PERIOD_MS);
```

#### 3.3.3 Logic tính trạng thái tổng hợp

Trạng thái LCD được tính trong `temp_humi_monitor` theo logic ưu tiên (`src/sensors/temp_humi_monitor.cpp`, dòng 34-39):

- `CRITICAL (2)`: Khi `T > 30°C` (nhiệt độ vượt ngưỡng trên)
- `WARNING (1)`: Khi `T >= 25°C` hoặc `H < 40%` hoặc `H > 60%`
- `NORMAL (0)`: Tất cả điều kiện còn lại

#### 3.3.4 Hiển thị LCD

Task `lcd_task` chạy với chu kỳ 500 ms, đọc Queue và hiển thị 2 dòng trên LCD 16x2 (địa chỉ I2C 0x27):
- **Dòng 0:** `STAT: NORMAL  ` / `STAT: WARNING ` / `STAT: CRITICAL`
- **Dòng 1:** `T:28.5C H:55.3%  `

Task LCD còn thực hiện "drain" 3 semaphore trạng thái (take với timeout 0) để tránh tràn bộ đệm semaphore khi producer give liên tục (`src/sensors/lcd_task.cpp`, dòng 32-34).

---

### 3.4 Task 4: Web Server AP Mode

**Yêu cầu:** Web server hoạt động ở chế độ AP, điều khiển ít nhất 2 thiết bị qua giao diện có ít nhất 2 nút.

#### 3.4.1 Kiến trúc WebSocket

Hệ thống sử dụng **ESPAsyncWebServer** kết hợp **AsyncWebSocket** thay vì HTTP polling thông thường, cho phép cập nhật dữ liệu theo thời gian thực mà không cần tải lại trang:

- WebSocket endpoint: `/ws`
- HTTP server: cổng 80, phục vụ file tĩnh từ LittleFS
- OTA update: tích hợp ElegantOTA

#### 3.4.2 Chế độ WiFi AP+STA đồng thời

WiFi được cấu hình chế độ `WIFI_AP_STA` (vừa Access Point vừa Station), đảm bảo dashboard luôn truy cập được qua IP `192.168.4.1` ngay cả khi chưa có kết nối Internet (`src/connectivity/task_wifi.cpp`, dòng 32-33):

```cpp
WiFi.mode(WIFI_AP_STA);
WiFi.softAP(ssid.c_str(), password.c_str()); // SSID: "ESP32 LOCAL", Pass: 12345678
```

Khi STA kết nối thành công, DNS được chỉnh thủ công về Google DNS (8.8.8.8 / 8.8.4.4) vì chế độ AP_STA có thể dùng nhầm DNS của AP nội bộ (`src/connectivity/task_wifi.cpp`, dòng 14-16).

#### 3.4.3 Giao diện Web Dashboard

Dashboard 4 trang phục vụ từ LittleFS (`data/index.html`):

1. **Trang chủ (Home):** Hai đồng hồ gauge (JustGage + Raphael.js) hiển thị nhiệt độ và độ ẩm thời gian thực với gradient màu phản ánh các ngưỡng trong `global.h`. Nhiệt độ: phạm vi -10 đến 50°C; Độ ẩm: 0 đến 100%.

2. **Thiết bị (Device):**
   - Bảng trạng thái 3 thiết bị: LED đơn (tốc độ chớp), NeoPixel (màu hiện tại), LCD (trạng thái NORMAL/WARNING/CRITICAL).
   - Điều khiển relay động: người dùng nhấn nút "+" để thêm relay mới (nhập tên + số GPIO), sau đó bật/tắt từng relay qua WebSocket. Hỗ trợ xóa relay.

3. **Thông tin (Info):** Hiển thị IP, uptime, cấu hình AP/STA, thông tin CoreIOT.

4. **Cài đặt (Settings):** Form cấu hình WiFi (có nút quét mạng WiFi) và CoreIOT. Sau khi lưu, thiết bị tự khởi động lại sau 2 giây.

#### 3.4.4 Xử lý WebSocket Message

Handler `handleWebSocketMessage` (`src/web_services/task_handler.cpp`) phân tích JSON theo trường `page`:

| `page` | Hành động |
|--------|-----------|
| `"device"` | `digitalWrite(gpio, HIGH/LOW)` điều khiển GPIO bất kỳ |
| `"scan_wifi"` | Kích hoạt quét WiFi async, trả về danh sách RSSI |
| `"setting"` | Lưu SSID/Pass/Token vào LittleFS, lên lịch restart sau 2s |

#### 3.4.5 Broadcast dữ liệu sensor

Task `task_webserver_run` broadcast sensor data mỗi 5 giây qua WebSocket cho tất cả client. Quan trọng hơn, dữ liệu được gửi ngay lập tức khi client mới kết nối để không phải chờ đến chu kỳ tiếp theo (`src/web_services/task_webserver.cpp`, dòng 68-77). Dữ liệu được lấy từ `xSensorQueue` qua `xQueuePeek`, đồng nhất với nguồn dữ liệu LCD.

**Điểm cần cải tiến:** Danh sách relay không được lưu vào LittleFS nên mất sau mỗi lần tải lại trang.

---

### 3.5 Task 5: TinyML — Phát Hiện Bất Thường

**Yêu cầu:** Triển khai mô hình TinyML trên vi điều khiển, mô tả dataset, đánh giá độ chính xác.

#### 3.5.1 Bộ dữ liệu (Dataset)

Dataset được sinh tổng hợp (`src/tinyml/custom_iot_dataset.csv`) với **2000 mẫu** từ notebook `train_anomaly_detector.ipynb`:

| Nhãn | Số lượng | Điều kiện |
|------|----------|-----------|
| Normal (0) | 1600 (80%) | `25 ≤ T ≤ 30°C` VÀ `40 ≤ H ≤ 60%` |
| Anomaly (1) | 400 (20%) | Nóng (T>35°C), Lạnh (T<20°C), Ẩm (H>70%), Khô (H<30%) |

Dữ liệu được trộn ngẫu nhiên (seed=42), tỷ lệ train/validation = 90/10.

#### 3.5.2 Kiến trúc mô hình ANN

Mô hình phân loại nhị phân đơn giản được thiết kế để phù hợp với tài nguyên MCU hạn chế:

```
Input(2) -> Dense(8, ReLU) -> Dense(1, Sigmoid)
```

| Layer | Units | Kích hoạt | Tham số |
|-------|-------|-----------|---------|
| Input | 2 | — | 0 |
| Dense | 8 | ReLU | 24 |
| Dense | 1 | Sigmoid | 9 |
| **Tổng** | | | **33** |

Kích thước model sau khi chuyển đổi sang `.tflite`: **1928 bytes**.

#### 3.5.3 Tiền xử lý dữ liệu (Z-score Normalization)

Thông số chuẩn hóa được tính từ tập huấn luyện và nhúng trực tiếp vào firmware (`src/tinyml/tinyml.cpp`, dòng 15-17):

```cpp
const float SCALER_MEAN[]  = {27.4874f, 49.9192f};  // [mean_temp, mean_humi]
const float SCALER_SCALE[] = {4.2157f, 12.0575f};   // [std_temp,  std_humi]
const float ANOMALY_THRESHOLD = 0.5f;
```

Tại runtime, tiền xử lý được thực hiện trước khi nạp vào input tensor:

```cpp
// src/tinyml/tinyml.cpp, dòng 54-55
float temp_scaled = (sensorData.temperature - SCALER_MEAN[0]) / SCALER_SCALE[0];
float humi_scaled = (sensorData.humidity    - SCALER_MEAN[1]) / SCALER_SCALE[1];
```

#### 3.5.4 Luồng inference TFLite Micro

Pipeline inference trong `tiny_ml_task` (`src/tinyml/tinyml.cpp`, dòng 45-86):
1. **Đọc dữ liệu:** `xQueuePeek(xSensorQueue, ...)` — nhận struct SensorData từ Queue
2. **Tiền xử lý:** Chuẩn hóa Z-score theo hằng số đã nhúng
3. **Nạp input tensor:** `input->data.f[0] = temp_scaled`, `input->data.f[1] = humi_scaled`
4. **Chạy inference:** `interpreter->Invoke()`
5. **Đọc kết quả:** `anomaly_score = output->data.f[0]`
6. **Quyết định:** `is_anomaly = (anomaly_score > 0.5f)`
7. **Give semaphore:** `xSemaphoreGive(xStatusWarningSemaphore)` nếu anomaly, `xStatusNormalSemaphore` nếu normal

**Cấu hình TFLite Micro:** Sử dụng `tflite::AllOpsResolver`, tensor arena 10 KB tĩnh (`constexpr int kTensorArenaSize = 10 * 1024`).

#### 3.5.5 Quy trình chuyển đổi model

Model được huấn luyện bằng TensorFlow/Keras trong Jupyter notebook, sau đó:
1. Xuất sang định dạng TFLite: `tf.lite.TFLiteConverter.from_keras_model(model)`
2. Chuyển đổi binary thành C array: ghi thẳng vào `include/tinyml/dht_model_v2.h`
3. Firmware include header này và sử dụng mảng `dht_anomaly_model_tflite[]`

#### 3.5.6 Kết quả đánh giá độ chính xác

Kết quả từ notebook huấn luyện:

| Metric | Giá trị |
|--------|---------|
| Train accuracy (epoch 100) | **99.44%** |
| Train loss | 0.022765 |
| Kích thước model | 1928 bytes |
| Tensor arena cần thiết | 10 KB |

Kiểm tra với các trường hợp biên (notebook Cell 4, chạy qua TFLite interpreter):

| Input | Score | Kết quả | Mong đợi | OK? |
|-------|-------|---------|----------|-----|
| T=27.5, H=50.0 | 0.0001 | Normal | Normal (giữa vùng) | Đúng |
| T=29.9, H=59.9 | 0.0001 | Normal | Normal (sát biên) | Đúng |
| T=35.5, H=50.0 | 0.9620 | ANOMALY | ANOMALY (nóng) | Đúng |
| T=20.0, H=50.0 | 0.9319 | ANOMALY | ANOMALY (lạnh) | Đúng |
| T=27.5, H=70.0 | 0.7363 | ANOMALY | ANOMALY (ẩm) | Đúng |
| T=27.5, H=25.0 | 0.9864 | ANOMALY | ANOMALY (khô) | Đúng |

6/6 test case biên đều cho kết quả chính xác.

**Giới hạn hiện tại:** Dataset là dữ liệu tổng hợp (không thu thập từ cảm biến thực). Đánh giá độ chính xác trên phần cứng thực (so với ground truth thực tế) chưa được thực hiện.

---

### 3.6 Task 6: CoreIOT Cloud (MQTT)

**Yêu cầu:** Đẩy dữ liệu sensor lên CoreIOT, ESP32-S3 ở chế độ STA, dùng đúng token xác thực.

#### 3.6.1 Cấu hình MQTT

Thư viện sử dụng: **PubSubClient**. Cấu hình mặc định trong `platformio.ini`:

```ini
-DDEFAULT_COREIOT_SERVER='"app.coreiot.io"'
-DDEFAULT_COREIOT_PORT='"1883"'
-DDEFAULT_COREIOT_TOKEN='"YKrSFjuZDqIXym2ClhqW"'
```

Topic publish: `esp/telemetry`

#### 3.6.2 Cơ chế đồng bộ với WiFi

Task `task_core_iot` không chạy ngay lập tức. Nó chờ semaphore `xBinarySemaphoreInternet` — semaphore này chỉ được give trong callback sự kiện WiFi `ARDUINO_EVENT_WIFI_STA_GOT_IP` khi STA kết nối thành công và nhận được IP (`src/connectivity/task_wifi.cpp`, dòng 20):

```cpp
// Trong task_core_iot: chờ Internet có sẵn
xSemaphoreTake(xBinarySemaphoreInternet, pdMS_TO_TICKS(5000))
// Trả lại ngay để các task khác cũng có thể check
xSemaphoreGive(xBinarySemaphoreInternet);
```

#### 3.6.3 Cấu trúc payload telemetry

Mỗi 10 giây, task đọc Queue và publish JSON (`src/cloud/task_core_iot.cpp`, dòng 54-66):

```json
{"temperature": 28.5, "humidity": 55.3}
```

Xác thực CoreIOT theo chuẩn ThingsBoard: **username = access token, password = chuỗi rỗng**. ClientID được tạo từ MAC address để đảm bảo tính duy nhất: `"ESP32_" + WiFi.macAddress()`.

#### 3.6.4 Xử lý kết nối lại

Khi MQTT ngắt kết nối, task tự động thử kết nối lại sau 5 giây. `mqttClient.loop()` được gọi mỗi 100 ms trong vòng lặp chính của task để duy trì keep-alive.

**Trạng thái hiện tại:** Code đã hoàn chỉnh. Cần cấu hình token thực tế qua trang Settings của web dashboard để kết nối thành công với thiết bị cụ thể trên CoreIOT.

---

## 4. Cơ Chế Đồng Bộ (Semaphore & Queue)

### 4.1 Tổng quan các đối tượng RTOS

Tất cả Queue và Semaphore được khai báo trong `include/global.h` và khởi tạo trong `src/global.cpp` hàm `init_globals()`, được gọi trước khi bất kỳ task nào được tạo (`src/main.cpp`, dòng 21-22).

#### Queue

| Tên | Loại | Depth | Payload | Mục đích |
|-----|------|-------|---------|---------|
| `xSensorQueue` | Queue | 1 (Mailbox) | `struct SensorData` | Truyền {temp, humi, status} từ monitor sang LCD/WebServer/TinyML |

#### Semaphore nhiệt độ (Binary)

| Tên | Give bởi | Take bởi | Điều kiện give |
|-----|----------|----------|----------------|
| `xTempLowSemaphore` | `temp_humi_monitor` | `led_blinky` | Temp chuyển sang < 25°C |
| `xTempNormalSemaphore` | `temp_humi_monitor` | `led_blinky` | Temp chuyển về 25-30°C |
| `xTempHighSemaphore` | `temp_humi_monitor` | `led_blinky` | Temp chuyển sang > 30°C |

#### Semaphore độ ẩm (Binary)

| Tên | Give bởi | Take bởi | Điều kiện give |
|-----|----------|----------|----------------|
| `xHumiLowSemaphore` | `temp_humi_monitor` | `neo_blinky` | Humi chuyển sang < 40% |
| `xHumiNormalSemaphore` | `temp_humi_monitor` | `neo_blinky` | Humi chuyển về 40-60% |
| `xHumiHighSemaphore` | `temp_humi_monitor` | `neo_blinky` | Humi chuyển sang > 60% |

#### Semaphore trạng thái LCD (Binary)

| Tên | Give bởi | Drain bởi | Mục đích |
|-----|----------|-----------|---------|
| `xStatusNormalSemaphore` | `temp_humi_monitor`, `tiny_ml_task`; khởi tạo = given | `lcd_task` | Báo hiệu chuyển sang NORMAL |
| `xStatusWarningSemaphore` | `temp_humi_monitor`, `tiny_ml_task` | `lcd_task` | Báo hiệu chuyển sang WARNING |
| `xStatusCriticalSemaphore` | `temp_humi_monitor` | `lcd_task` | Báo hiệu chuyển sang CRITICAL |

#### Semaphore kết nối Internet (Binary)

| Tên | Give bởi | Take bởi | Mục đích |
|-----|----------|----------|---------|
| `xBinarySemaphoreInternet` | WiFi event callback `STA_GOT_IP` | `task_core_iot` | Đồng bộ bắt đầu MQTT khi có Internet |

### 4.2 Pattern: Edge Detection cho Semaphore

Dự án áp dụng pattern **edge detection**: producer chỉ give semaphore khi phát hiện thay đổi trạng thái, không give ở mỗi vòng đọc. Consumer dùng `xSemaphoreTake(..., 0)` (non-blocking poll) và duy trì state cuối cùng. Điều này tránh semaphore bị "cộng dồn" và giảm tải cho scheduler FreeRTOS.

### 4.3 Pattern: Mailbox Queue với Overwrite

`xSensorQueue` (depth=1) hoạt động như **mailbox**. Khi producer ghi dữ liệu mới bằng `xQueueOverwrite`, giá trị cũ chưa đọc bị ghi đè. Tất cả consumer dùng `xQueuePeek` (đọc không xóa item), đảm bảo:
- LCD, WebServer và TinyML đều đọc cùng giá trị nhất quán
- Không task nào "ăn mất" dữ liệu của task khác
- Producer không bao giờ bị block khi queue đầy

---

## 5. Kết Quả & Đánh Giá

### 5.1 Task 1 — LED Blink

**Kết quả: Hoàn thành.**
LED GPIO 48 chớp chính xác 3 tốc độ (2s/1s/200ms) tương ứng 3 mức nhiệt độ. Semaphore hoạt động đúng theo cơ chế edge detection. LED không phụ thuộc trực tiếp vào tần suất đọc cảm biến.

### 5.2 Task 2 — NeoPixel

**Kết quả: Hoàn thành.**
NeoPixel GPIO 45 hiển thị 3 màu Đỏ/Xanh lá/Xanh dương tương ứng độ ẩm Khô/Bình thường/Ẩm. Màu được cập nhật ngay khi semaphore được give, phản hồi nhanh với thay đổi môi trường.

### 5.3 Task 3 — LCD + Loại bỏ biến toàn cục

**Kết quả: Hoàn thành.**
Toàn bộ dữ liệu sensor được truyền qua `xSensorQueue` (struct `SensorData`), không có biến toàn cục `float` nào liên quan đến sensor. LCD cập nhật 500 ms/lần, hiển thị chính xác 3 trạng thái. Cơ chế `xQueuePeek` cho phép WebServer và TinyML cùng đọc dữ liệu mà không xung đột.

### 5.4 Task 4 — Web Server

**Kết quả: Hoàn thành chức năng cốt lõi.**
Web dashboard có đầy đủ 4 trang, gauge thời gian thực, điều khiển relay động qua WebSocket, quét WiFi, cấu hình CoreIOT. Giao diện responsive hoạt động tốt trên mobile. Điểm cần cải tiến: danh sách relay không được lưu vào LittleFS.

### 5.5 Task 5 — TinyML

**Kết quả: Đã triển khai, cần đánh giá thực tế.**
Mô hình ANN 33 tham số (1928 bytes) chạy thành công trên ESP32-S3 với tensor arena 10 KB. Độ chính xác 99.44% trên dataset tổng hợp 2000 mẫu. Tất cả 6 test case biên cho kết quả đúng. Chưa có đánh giá trên dữ liệu thực tế từ phần cứng trong điều kiện môi trường thực.

### 5.6 Task 6 — CoreIOT Cloud

**Kết quả: Code hoàn chỉnh, cần cấu hình token.**
Toàn bộ logic MQTT (kết nối, xác thực ThingsBoard, publish JSON, reconnect) đã hoàn chỉnh. Thiết bị tự động kết nối sau khi STA nhận IP. Cần cấu hình access token chính xác qua trang Settings của web dashboard để kết nối thành công.

---

## 6. Kết Luận

### 6.1 Tóm tắt thành tựu

Dự án đã thành công xây dựng một hệ thống IoT nhúng đa nhiệm đầy đủ trên ESP32-S3 với kiến trúc RTOS chuẩn:

- **Task 1, 2, 3 hoàn thành hoàn toàn** — đạt đầy đủ yêu cầu semaphore, queue, loại bỏ biến toàn cục.
- **Task 4 hoàn thành chức năng cốt lõi** — WebSocket dashboard đa trang, điều khiển relay động.
- **Task 5 đã triển khai** — TinyML ANN chạy trên MCU với độ chính xác cao trên dữ liệu tổng hợp.
- **Task 6 code hoàn chỉnh** — chờ cấu hình token thực tế.
- Kiến trúc module hóa rõ ràng theo từng chức năng giúp mã dễ bảo trì và mở rộng.

### 6.2 Điểm cần cải thiện

1. **Task 4:** Lưu cấu hình relay vào LittleFS để không mất sau khi khởi động lại.
2. **Task 5:** Thu thập dataset thực tế và đánh giá độ chính xác trên phần cứng.
3. **Task 6:** Cấu hình token thực tế và kiểm tra truyền dữ liệu end-to-end lên CoreIOT.
4. **TinyML và status semaphore:** Task TinyML hiện chỉ give `xStatusWarningSemaphore` hoặc `xStatusNormalSemaphore`, không give `xStatusCriticalSemaphore` — có thể gây mâu thuẫn với logic của `temp_humi_monitor` khi nhiệt độ cực cao.

### 6.3 Bài học kinh nghiệm

- **Mailbox pattern** (`xQueueCreate(1, ...)` + `xQueueOverwrite` + `xQueuePeek`) là giải pháp tối ưu cho dữ liệu sensor "chỉ cần giá trị mới nhất" — không làm block producer, không cạnh tranh giữa các consumer.
- **Edge detection cho semaphore** (chỉ give khi có thay đổi mức) tốt hơn level detection vì tránh tràn semaphore binary trong FreeRTOS và giảm tải scheduler.
- **WiFi AP+STA đồng thời** (`WIFI_AP_STA`) cho phép vừa truy cập dashboard nội bộ vừa kết nối cloud, giải quyết mâu thuẫn giữa Task 4 (AP) và Task 6 (STA).
- **TFLite Micro** yêu cầu model định dạng `.tflite` flatbuffer — phải dùng TensorFlow/Keras để xuất, không thể dùng PyTorch trực tiếp. Kích thước tensor arena cần được điều chỉnh phù hợp với kiến trúc model.
- **Khởi tạo trước task creation** là bắt buộc: `init_globals()` phải được gọi trong `setup()` trước `xTaskCreate()` để tránh race condition khi task chạy ngay mà Queue/Semaphore chưa được tạo.

---

## 7. Mã Nguồn — Các Đoạn Code Quan Trọng

### 7.1 Khởi tạo hệ thống (`src/main.cpp` & `src/global.cpp`)

Hàm `init_globals()` tạo toàn bộ đối tượng RTOS trước khi bất kỳ task nào được đăng ký:

```cpp
// src/global.cpp
void init_globals() {
    xSensorQueue = xQueueCreate(1, sizeof(struct SensorData));  // Mailbox depth=1

    xTempLowSemaphore     = xSemaphoreCreateBinary();
    xTempNormalSemaphore  = xSemaphoreCreateBinary();
    xTempHighSemaphore    = xSemaphoreCreateBinary();

    xHumiLowSemaphore     = xSemaphoreCreateBinary();
    xHumiNormalSemaphore  = xSemaphoreCreateBinary();
    xHumiHighSemaphore    = xSemaphoreCreateBinary();

    xStatusNormalSemaphore   = xSemaphoreCreateBinary();
    xStatusWarningSemaphore  = xSemaphoreCreateBinary();
    xStatusCriticalSemaphore = xSemaphoreCreateBinary();

    xBinarySemaphoreInternet = xSemaphoreCreateBinary();

    xSemaphoreGive(xStatusNormalSemaphore);  // Khởi động ở trạng thái Normal
}
```

### 7.2 Task 1 — Semaphore edge detection (`src/sensors/temp_humi_monitor.cpp`)

Producer chỉ give semaphore khi phát hiện thay đổi mức — không give ở mỗi chu kỳ đọc:

```cpp
// Theo dõi mức cũ
static float last_temp = -999.0f;
static float last_humi = -999.0f;

// Give semaphore nhiệt độ khi vượt ngưỡng
if (temperature < TEMP_NORMAL_LOW && last_temp >= TEMP_NORMAL_LOW) {
    xSemaphoreGive(xTempLowSemaphore);
} else if (temperature >= TEMP_NORMAL_LOW && temperature <= TEMP_NORMAL_HIGH
           && (last_temp < TEMP_NORMAL_LOW || last_temp > TEMP_NORMAL_HIGH)) {
    xSemaphoreGive(xTempNormalSemaphore);
} else if (temperature > TEMP_NORMAL_HIGH && last_temp <= TEMP_NORMAL_HIGH) {
    xSemaphoreGive(xTempHighSemaphore);
}

last_temp = temperature;
```

Consumer poll non-blocking, duy trì state cuối cùng cho đến khi có semaphore mới:

```cpp
// src/actuators/led_blinky.cpp
void led_blinky(void *pvParameters) {
    int blinkInterval = 1000;
    while (true) {
        if      (xSemaphoreTake(xTempHighSemaphore,   0) == pdTRUE) blinkInterval = 200;
        else if (xSemaphoreTake(xTempNormalSemaphore, 0) == pdTRUE) blinkInterval = 1000;
        else if (xSemaphoreTake(xTempLowSemaphore,    0) == pdTRUE) blinkInterval = 2000;

        digitalWrite(LED_BUILTIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(blinkInterval));
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(blinkInterval));
    }
}
```

### 7.3 Task 3 — Mailbox Queue (`src/sensors/temp_humi_monitor.cpp` & `lcd_task.cpp`)

**Producer** dùng `xQueueOverwrite` — ghi đè không cần chờ queue trống:

```cpp
// src/sensors/temp_humi_monitor.cpp
struct SensorData data;
data.temperature = temperature;
data.humidity    = humidity;
data.lcd_status  = lcd_status;  // 0=NORMAL, 1=WARNING, 2=CRITICAL

xQueueOverwrite(xSensorQueue, &data);  // Luôn thành công, không block
```

**Consumer** dùng `xQueuePeek` — đọc mà không xóa item khỏi queue:

```cpp
// src/sensors/lcd_task.cpp
struct SensorData currentData;
if (xQueuePeek(xSensorQueue, &currentData, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Nhiều task khác (WebServer, TinyML) cũng đọc cùng item này
    lcd.setCursor(0, 1);
    lcd.printf("T:%.1fC H:%.1f%%", currentData.temperature, currentData.humidity);
}
```

### 7.4 Task 4 — WebSocket handler (`src/web_services/task_handler.cpp`)

Phân tích JSON từ browser và điều khiển GPIO:

```cpp
void handleWebSocketMessage(AsyncWebSocket *server, AsyncWebSocketClient *client,
                            AwsEventType type, void *arg, uint8_t *data, size_t len) {
    StaticJsonDocument<512> doc;
    deserializeJson(doc, (char *)data);
    String page = doc["page"].as<String>();

    if (page == "device") {
        int gpio  = doc["gpio"].as<int>();
        int state = doc["state"].as<int>();
        pinMode(gpio, OUTPUT);
        digitalWrite(gpio, state);
    } else if (page == "scan_wifi") {
        WiFi.scanNetworksAsync([&](int n) { /* broadcast RSSI list */ });
    } else if (page == "setting") {
        // Lưu SSID, password, CoreIOT token vào LittleFS
        saveCredentials(doc["ssid"], doc["password"], doc["token"]);
        delay(2000);
        ESP.restart();
    }
}
```

### 7.5 Task 5 — TFLite Micro inference (`src/tinyml/tinyml.cpp`)

Tiền xử lý Z-score và chạy inference:

```cpp
// Hằng số chuẩn hóa nhúng từ tập huấn luyện
const float SCALER_MEAN[]  = {27.4874f, 49.9192f};
const float SCALER_SCALE[] = {4.2157f,  12.0575f};

void tiny_ml_task(void *pvParameters) {
    // ... khởi tạo TFLite Micro interpreter ...
    while (true) {
        struct SensorData sensorData;
        xQueuePeek(xSensorQueue, &sensorData, pdMS_TO_TICKS(100));

        // Z-score normalization
        input->data.f[0] = (sensorData.temperature - SCALER_MEAN[0]) / SCALER_SCALE[0];
        input->data.f[1] = (sensorData.humidity    - SCALER_MEAN[1]) / SCALER_SCALE[1];

        interpreter->Invoke();

        float score = output->data.f[0];
        if (score > 0.5f) {
            xSemaphoreGive(xStatusWarningSemaphore);   // Bất thường
        } else {
            xSemaphoreGive(xStatusNormalSemaphore);    // Bình thường
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
```

### 7.6 Task 6 — MQTT CoreIOT (`src/cloud/task_core_iot.cpp`)

Đồng bộ với WiFi qua semaphore, sau đó publish JSON định kỳ:

```cpp
void task_core_iot(void *pvParameters) {
    PubSubClient mqttClient(espClient);
    mqttClient.setServer(COREIOT_SERVER, 1883);

    while (true) {
        // Chờ WiFi STA nhận IP (semaphore give trong WiFi event callback)
        xSemaphoreTake(xBinarySemaphoreInternet, pdMS_TO_TICKS(5000));
        xSemaphoreGive(xBinarySemaphoreInternet);  // Trả lại cho task khác

        if (!mqttClient.connected()) {
            String clientId = "ESP32_" + WiFi.macAddress();
            mqttClient.connect(clientId.c_str(), ACCESS_TOKEN, "");  // ThingsBoard auth
        }

        // Publish telemetry mỗi 10 giây
        struct SensorData data;
        xQueuePeek(xSensorQueue, &data, pdMS_TO_TICKS(100));
        char payload[64];
        snprintf(payload, sizeof(payload),
                 "{\"temperature\":%.2f,\"humidity\":%.2f}",
                 data.temperature, data.humidity);
        mqttClient.publish("esp/telemetry", payload);

        mqttClient.loop();
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
```

---

*Tài liệu kỹ thuật bổ sung: `docs/semaphore.md`, `docs/YOLO_UNO.md`*
*Notebook huấn luyện TinyML: `src/tinyml/train_anomaly_detector.ipynb`*

# Project Status Report: YoloUNO PlatformIO-RTOS

Tài liệu này mô tả trạng thái hiện tại của mã nguồn, cấu trúc module hóa và vai trò của các thành phần trong hệ thống.

---

## 1. Cấu trúc Cây Thư mục (Project Tree)

```text
.
├── data/                       # Dữ liệu web nạp vào LittleFS
│   ├── images/                 # Chứa logo.png thực tế
│   ├── index.html              # Giao diện chính (Task 4)
│   ├── script.js               # Logic WebSocket phía Client
│   └── styles.css              # Định dạng giao diện
├── include/
│   ├── actuators/              # Header điều khiển đầu ra
│   ├── cloud/                  # Header kết nối ThingsBoard/CoreIOT
│   ├── connectivity/           # Header quản lý WiFi/Hệ thống
│   ├── sensors/                # Header đọc dữ liệu cảm biến
│   ├── tinyml/                 # Header mô hình AI & Inference
│   ├── web_services/           # Header phục vụ giao diện Web
│   └── global.h                # Biến dùng chung & Mutex/Semaphore
├── src/
│   ├── actuators/              # Code thực thi LED, NeoPixel
│   ├── cloud/                  # Code MQTT, Telemetry, RPC
│   ├── connectivity/           # Code kết nối WiFi, LittleFS, Boot logic
│   ├── sensors/                # Code đọc DHT20, RS485
│   ├── tinyml/                 # Code chạy mô hình TensorFlow Lite
│   ├── web_services/           # Code AsyncServer, WebSocket, OTA
│   ├── global.cpp              # Khởi tạo biến toàn cục & Semaphore
│   └── main.cpp                # Điểm khởi đầu & Quản lý Task
├── platformio.ini              # Cấu hình thư viện & Build flags
└── docs/                       # Tài liệu dự án
```

---

## 2. Vai trò của các Module

| Module | Vai trò chính | Tệp tin chủ chốt |
| :--- | :--- | :--- |
| **Connectivity** | Cảnh sát giao thông: Quản lý kết nối WiFi (AP/STA), kiểm tra tệp cấu hình và xử lý nút nhấn BOOT để reset thiết bị. | `task_wifi`, `task_check_info` |
| **Sensors** | Người thu thập: Đọc dữ liệu thô từ cảm biến DHT20 và thiết bị công nghiệp qua RS485/Modbus. | `temp_humi_monitor`, `task_rs485` |
| **Actuators** | Công nhân thực thi: Điều khiển trạng thái LED đơn và hiệu ứng màu sắc trên NeoPixel (RGB). | `led_blinky`, `neo_blinky` |
| **Web Services** | Cổng thông tin: Cung cấp giao diện Web không đồng bộ (Async), WebSocket thời gian thực và cập nhật Firmware từ xa (OTA). | `task_webserver`, `task_handler` |
| **Cloud (IoT)** | Phóng viên báo cáo: Giao tiếp với CoreIOT/ThingsBoard qua MQTT để gửi dữ liệu cảm biến và nhận lệnh điều khiển từ xa (RPC). | `task_core_iot` |
| **TinyML** | Bộ não phân tích: Sử dụng TensorFlow Lite để phân tích dữ liệu cảm biến và phát hiện các dấu hiệu bất thường. | `tinyml`, `dht_anomaly_model.h` |

---

## 3. Trạng thái các Task (Specs Alignment)

- [x] **Kiến trúc Module:** Đã hoàn thành tổ chức mã nguồn theo hướng module hóa chuyên nghiệp.
- [x] **Đa nhiệm (Multi-tasking):** Tất cả các thành phần đã được chuyển đổi thành FreeRTOS Task độc lập.
- [x] **Web & OTA:** Đã có hệ thống Web không đồng bộ phục vụ file từ LittleFS và tích hợp ElegantOTA.
- [!] **Task 3 (Globals):** Cần thực hiện gỡ bỏ biến toàn cục bằng cách sử dụng Mutex/Queue (Đang chuẩn bị).
- [ ] **TinyML Logic:** Tác vụ đã được tạo nhưng cần thêm logic đánh giá độ chính xác (Task 5).

---
*Ngày cập nhật: 12/04/2026*

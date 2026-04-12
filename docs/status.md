# Project Status Report: YoloUNO PlatformIO-RTOS Template

Tài liệu này mô tả trạng thái hiện tại của mã nguồn, các tính năng đã triển khai, cấu trúc hệ thống và các thành phần thư viện đang sử dụng.

---

## 1. Cấu hình Hệ thống (Environment)
- **Platform:** ESP32 (S3) dựa trên nền tảng `espressif32`.
- **Board:** `yolo_uno`.
- **Framework:** Arduino với FreeRTOS tích hợp sẵn.
- **Filesystem:** `LittleFS` được sử dụng để lưu trữ cấu hình và dữ liệu web.
- **Cấu trúc Module:** Đã phân chia mã nguồn thành các module chuyên biệt (`connectivity`, `sensors`, `actuators`, `web_services`, `cloud`, `tinyml`).

---

## 2. Các Tính năng Đã Triển khai (Active Features)

### 2.1. Quản lý Kết nối (Module: `connectivity`)
- **Cơ chế Dual-Mode:** Hệ thống kiểm tra tệp cấu hình `/info.dat`. Nếu không có thông tin WiFi, nó tự động bật chế độ **Access Point (AP)**. Nếu có, nó kết nối vào mạng **Station (STA)**.
- **Đồng bộ hóa:** Sử dụng `xBinarySemaphoreInternet` để thông báo cho các tác vụ khác khi đã có kết nối internet.

### 2.2. Thu thập Dữ liệu Cảm biến (Module: `sensors`)
- **Cảm biến DHT20:** Đọc nhiệt độ và độ ẩm mỗi 5 giây trong tác vụ `temp_humi_monitor`.
- **Biến Toàn cục:** Dữ liệu cảm biến tạm thời lưu vào `glob_temperature` và `glob_humidity`.

### 2.3. Điều khiển Thiết bị (Module: `actuators`)
- **LED Đơn:** Tác vụ `led_blinky` điều khiển LED tại chân GPIO 48.
- **NeoPixel:** Tác vụ `neo_blinky` điều khiển LED RGB tại chân GPIO 45.

### 2.4. Web Server & Giao diện (Module: `web_services`)
- **AsyncWebServer & WebSocket:** Giao tiếp thời gian thực qua kênh `/ws`.
- **Cập nhật OTA:** Tích hợp `ElegantOTA` tại `/update`.

### 2.5. Kết nối Cloud (Module: `cloud`)
- **CoreIOT (ThingsBoard):** Giao thức MQTT để gửi Telemetry và nhận lệnh RPC.

### 2.6. TinyML (Module: `tinyml`)
- **Model:** `dht_anomaly_model.h` đã sẵn sàng cho nhận diện bất thường.

---

## 3. Đánh giá dựa trên Yêu cầu (Specs Match)
- [x] **Module Organization:** Đã hoàn thành phân chia module cho dễ quản lý.
- [x] **Task 1 (LED):** Đã có tác vụ, cần thêm logic semaphore.
- [x] **Task 2 (NeoPixel):** Đã có tác vụ, cần thêm logic ánh xạ độ ẩm.
- [!] **Task 3 (LCD & Globals):** Cần gỡ bỏ biến toàn cục và đồng bộ qua Semaphore/Mutex.
- [x] **Task 4 (Web Server):** Hoàn thiện nền tảng Async + WebSocket.
- [ ] **Task 5 (TinyML):** Cần kích hoạt tác vụ và đánh giá độ chính xác.
- [x] **Task 6 (CoreIOT):** Đã có logic kết nối cơ bản.

---
*Ngày cập nhật: 12/04/2026*

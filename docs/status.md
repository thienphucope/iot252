# Project Status Report: YoloUNO PlatformIO-RTOS

Tài liệu này mô tả trạng thái hiện tại của mã nguồn, cấu trúc module hóa và vai trò của các thành phần trong hệ thống.

---

## 1. Cấu trúc Cây Thư mục (Project Tree)

```text
.
├── data/                       # Dữ liệu web nạp vào LittleFS
├── include/
│   ├── actuators/              # led_blinky.h, neo_blinky.h
│   ├── cloud/                  # task_core_iot.h
│   ├── connectivity/           # task_wifi.h, task_check_info.h
│   ├── sensors/                # temp_humi_monitor.h, lcd_task.h (MỚI)
│   ├── tinyml/                 # tinyml.h
│   └── global.h                # Khai báo Queue & Semaphore (Không còn biến toàn cục sensor)
├── src/
│   ├── actuators/              # Logic LED (Task 1) & NeoPixel (Task 2)
│   ├── sensors/                # Logic DHT20 & LCD Task (Task 3)
│   ├── global.cpp              # Khởi tạo Queue & Semaphore
│   └── main.cpp                # Khởi chạy 8+ RTOS Tasks
└── docs/                       
    ├── semaphore.md            # Tài liệu hướng dẫn về Semaphore (MỚI)
    └── status.md               
```

---

## 2. Vai trò của các Module (Cập nhật)

| Module | Vai trò chính | Trạng thái |
| :--- | :--- | :--- |
| **Sensors** | Đọc DHT20, đóng gói dữ liệu vào **Queue** và bắn **Semaphore** trạng thái. | Hoàn thành |
| **LCD Task** | Nhận dữ liệu từ Queue và chuyển 3 trạng thái hiển thị qua Semaphore. | Hoàn thành |
| **Actuators** | LED chớp theo Temp (Task 1), NeoPixel đổi màu theo Humi (Task 2). | Hoàn thành |
| **TinyML** | Phân tích dữ liệu để phát hiện bất thường. | Đang phát triển |

---

## 3. Trạng thái các Task (Specs Alignment)

- [x] **Kiến trúc Module:** Đã tổ chức mã nguồn theo hướng module hóa chuyên nghiệp.
- [x] **Task 1 (Single LED):** LED chớp theo 3 mức nhiệt độ (<25, 25-35, >35), đồng bộ qua Queue/Semaphore.
- [x] **Task 2 (NeoPixel):** NeoPixel đổi màu (Đỏ/Xanh lá/Xanh dương) theo độ ẩm dùng Semaphore.
- [x] **Task 3 (Globals & LCD):** 
    - Đã gỡ bỏ **100%** biến toàn cục liên quan đến cảm biến. 
    - LCD có 3 trạng thái (Normal/Warning/Critical) kích hoạt bằng Semaphore.
- [ ] **Task 4 (Web Server):** Cần thiết kế lại UI điều khiển 2 thiết bị.
- [ ] **Task 5 (TinyML):** Cần thêm logic thu thập dataset và đánh giá độ chính xác.
- [ ] **Task 6 (CoreIOT):** Đang chờ cấu hình Token để đẩy dữ liệu lên Cloud.

---
*Ngày cập nhật: 12/04/2026*

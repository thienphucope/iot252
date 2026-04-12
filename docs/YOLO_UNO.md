# YOLO UNO Board Information

Board YOLO UNO là một sản phẩm của OhSTEM, được thiết kế mạnh mẽ dựa trên chip vi điều khiển ESP32-S3, kết hợp hình dáng quen thuộc của Arduino UNO với các tính năng hiện đại cho IoT và AI.

---

## 1. Thông số kỹ thuật (Technical Specifications)

| Thành phần | Chi tiết |
| :--- | :--- |
| **Chip xử lý** | ESP32-S3 Dual Core 240MHz Tensilica |
| **Bộ nhớ** | 4MB Flash, 8MB PSRAM |
| **WiFi** | 2.4GHz (802.11b/g/n) |
| **Bluetooth** | Bluetooth 5 (BLE + Mesh) |
| **Cổng cấp nguồn** | USB Type-C hoặc Jack DC 12V |
| **LED Onboard** | LED Nguồn, LED D13, LED RGB NeoPixel (GPIO 45) |
| **Nút nhấn** | Reset, Boot (GPIO 0) |
| **Kích thước** | 73mm x 55mm |

---

## 2. Tính năng nổi bật

- **Tương thích Arduino UNO:** Thiết kế theo chuẩn chân cắm (form factor) của Arduino UNO, cho phép sử dụng hầu hết các Shield mở rộng có sẵn.
- **Hỗ trợ TinyML:** Chip ESP32-S3 tích hợp các tập lệnh tăng tốc cho xử lý trí tuệ nhân tạo (AI), phù hợp cho nhận diện giọng nói, hình ảnh và phân tích dữ liệu.
- **Hệ sinh thái kết nối nhanh Grove:**
    - Tích hợp sẵn 12 cổng Grove (4x Analog, 4x Digital, 4x I2C).
    - Cổng STEMMA QT/QWIIC cho các cảm biến I2C hiện đại.
- **Đa dạng môi trường lập trình:**
    - Lập trình kéo thả (Blockly).
    - Lập trình Python (MicroPython).
    - Lập trình C++ chuyên sâu (Arduino IDE, PlatformIO, ESP-IDF).

---

## 3. Sơ đồ chân và Kết nối (Pinout)

YOLO UNO duy trì sơ đồ chân tương đương Arduino UNO nhưng ánh xạ vào GPIO của ESP32-S3:

- **LED RGB NeoPixel:** Được kết nối vào chân **GPIO 45**.
- **Nút nhấn BOOT:** Kết nối vào **GPIO 0**.
- **Các cổng Grove:**
    - Cổng Digital thường nối vào các chân như GPIO 1, 2, 3, 4...
    - Cổng Analog hỗ trợ đọc ADC từ ESP32-S3.
    - Cổng I2C (SDA/SCL) hỗ trợ kết nối màn hình LCD, cảm biến DHT20.

---

## 4. Tài liệu tham khảo
- [Trang sản phẩm OhSTEM](https://ohstem.vn/product/yolo-uno/)
- [Tài liệu hướng dẫn](https://docs.ohstem.vn/en/latest/esp32/yolo-uno.html)

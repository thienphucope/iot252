# Tìm hiểu về Semaphore trong FreeRTOS (ESP32)

Trong hệ điều hành thời gian thực (RTOS), **Semaphore** (tạm dịch: Cờ hiệu) là một cơ chế đồng bộ hóa cực kỳ quan trọng giúp các Task (tác vụ) "nói chuyện" và phối hợp với nhau một cách an toàn.

---

## 1. Tại sao cần Semaphore? (Vấn đề của Biến toàn cục)

Thông thường, bạn dùng biến toàn cục (Global Variable) để truyền dữ liệu:
```cpp
// Cách làm thông thường (Không an toàn trong RTOS)
float glob_temp; // Biến dùng chung

void task_sensor() {
    glob_temp = read_dht20(); // Task A ghi vào
}

void task_led() {
    if (glob_temp > 30) { // Task B đọc ra
        blink_fast();
    }
}
```
**Vấn đề:** Trong RTOS, các Task chạy song song. Có khả năng Task B đang đọc dở dang thì Task A nhảy vào ghi đè lên, dẫn đến dữ liệu bị sai lệch hoặc xung đột (Race Condition). **Task 3** yêu cầu xóa bỏ biến toàn cục để tránh lỗi này.

---

## 2. Các loại Semaphore phổ biến

### A. Binary Semaphore (Cờ hiệu nhị phân)
Giống như một chiếc vé (Token). Chỉ có 2 trạng thái: Có vé (1) hoặc Không có vé (0).
- **Công dụng:** Dùng để **đánh thức** một Task khác (Signal/Trigger).
- **Ví dụ:** Task cảm biến thấy nhiệt độ cao -> "Gửi" cờ -> Task LED "Nhận" cờ và bắt đầu chớp nhanh.

### B. Mutex (Mutual Exclusion - Loại trừ tương hỗ)
Giống như chìa khóa nhà vệ sinh duy nhất. Ai cầm chìa khóa mới được vào, dùng xong phải trả chìa khóa.
- **Công dụng:** Bảo vệ tài nguyên dùng chung (biến, màn hình LCD, cảm biến).
- **Đặc điểm:** Có tính năng "Priority Inheritance" giúp tránh lỗi nghẽn ưu tiên (Priority Inversion).

---

## 3. Cách sử dụng trong Code (Cú pháp cơ bản)

### Khai báo và Khởi tạo
```cpp
SemaphoreHandle_t xMySemaphore; // Khai báo tay cầm (handle)

void setup() {
    // Khởi tạo Binary Semaphore
    xMySemaphore = xSemaphoreCreateBinary();
    
    // Hoặc khởi tạo Mutex
    // xMySemaphore = xSemaphoreCreateMutex();
}
```

### Sử dụng (Gửi và Nhận)
```cpp
// Task A: Gửi tín hiệu (Give)
xSemaphoreGive(xMySemaphore);

// Task B: Chờ tín hiệu (Take)
// portMAX_DELAY nghĩa là chờ mãi mãi cho đến khi có tín hiệu
if (xSemaphoreTake(xMySemaphore, portMAX_DELAY) == pdTRUE) {
    // Thực hiện hành động sau khi nhận được tín hiệu
}
```

---

## 4. Giải pháp cho Task 3: "Xóa bỏ biến toàn cục"

Để xóa biến toàn cục mà vẫn truyền được dữ liệu (như nhiệt độ, độ ẩm), chúng ta có 2 cách chuẩn RTOS:

### Cách 1: Dùng Queue (Hàng đợi) - Khuyên dùng cho Task 3
Thay vì để biến `glob_temperature` ở ngoài, bạn tạo một "ống dẫn" dữ liệu. Task Cảm biến đẩy dữ liệu vào ống, Task LED/LCD rút dữ liệu ra. Dữ liệu được **copy** qua ống nên cực kỳ an toàn.

### Cách 2: Dùng Mutex bảo vệ Struct
Tạo một `struct` chứa toàn bộ dữ liệu, và mỗi khi truy cập (đọc hoặc ghi) đều phải mượn "chìa khóa" Mutex.

---

## 5. Ví dụ áp dụng cho Task 1 & 3 (Logic demo)

```cpp
// Định nghĩa cấu trúc dữ liệu thay cho biến toàn cục lẻ tẻ
struct SensorData {
    float temp;
    float humi;
};

QueueHandle_t sensorQueue; // Dùng Queue để truyền struct này

void sensorTask(void *pv) {
    SensorData data;
    while(1) {
        data.temp = readTemp();
        data.humi = readHumi();
        
        // Gửi dữ liệu vào Queue (Không dùng biến toàn cục)
        xQueueSend(sensorQueue, &data, 0);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void ledTask(void *pv) {
    SensorData receivedData;
    while(1) {
        // Chờ nhận dữ liệu từ Queue
        if (xQueueReceive(sensorQueue, &receivedData, portMAX_DELAY)) {
            if (receivedData.temp > 35) {
                // Trạng thái Cảnh báo (Critical) -> Chớp nhanh
                blink(100); 
            } else {
                // Trạng thái Bình thường -> Chớp chậm
                blink(1000);
            }
        }
    }
}
```

**Kết luận:** Semaphore/Queue giúp code của bạn chuyên nghiệp hơn, tránh lỗi xung đột bộ nhớ và đáp ứng đúng yêu cầu khắt khe của hệ thống nhúng thực tế.

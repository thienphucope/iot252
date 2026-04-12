Dưới đây là bảng so sánh trạng thái hiện tại của project so
  với yêu cầu của đề bài:

  Bảng Trạng thái Task & Kế hoạch "Khác biệt 30%"

  | Task | Yêu cầu từ Đề bài (Specs) | Trạng thái Hiện tại (Template) | Những gì CẦN LÀM để đạt 30% và đúng yêu cầu |
  |------|---------------------------|-------------------------------|---------------------------------------------|
  | Task 1 | LED chớp theo 3 ngưỡng nhiệt độ (dùng Semaphore). | Chớp tắt 1s cố định, chưa có điều kiện. | Thêm logic: < 25°C chớp chậm, 25-30°C chớp nhanh, > 30°C sáng đứng. Dùng Semaphore để kích hoạt task. |
  | Task 2 | NeoPixel đổi màu theo 3 ngưỡng độ ẩm (dùng Semaphore). | Chỉ chớp màu đỏ cố định. | Thêm logic ánh xạ độ ẩm: Thấp (Xanh), TB (Vàng), Cao (Đỏ). Dùng Semaphore để đồng bộ việc đổi màu. |
  | Task 3 | LCD hiển thị 3 trạng thái (Normal/Warning/Critical). Bỏ hoàn toàn biến toàn cục. | Hiển thị thông số thô, chưa có trạng thái. Vẫn dùng biến toàn cục. | Xây dựng 3 giao diện hiển thị khác nhau trên LCD. Chuyển sang dùng Mutex/Queue để truyền dữ liệu cảm biến. |
  | Task 4 | Thiết kế lại giao diện Web, điều khiển 2 thiết bị (LED1, LED2). | Giao diện cũ (nháp) hoặc file tĩnh đơn giản trong data/. | Task trọng tâm: Chỉnh sửa data/index.html và script.js để tạo UI hiện đại hơn. Thêm nút bấm điều khiển LED thứ 2 qua WebSocket. |
  | Task 5 | Triển khai TinyML & Đánh giá độ chính xác. | Chỉ có khung code (Boilerplate) và model thô. | Viết code để lấy kết quả từ mô hình AI, so sánh với giá trị cảm biến thực tế để tính độ chính xác (%) và ghi vào báo cáo. |
  | Task 6 | Gửi dữ liệu lên CoreIOT ở chế độ STA. | Đã có code kết nối ThingsBoard cơ bản. | Tối ưu việc gửi dữ liệu qua MQTT, đảm bảo cơ chế reconnect ổn định và sử dụng đúng Solution Template của CoreIOT. |

  ---

  Đánh giá mức độ "Khác biệt 30%"

  Hiện tại, mã nguồn của bạn đang ở mức 15-20% khác biệt (chủ
  yếu nhờ việc tôi đã giúp bạn module hóa lại cấu trúc cây thư
  mục, dọn dẹp mainserver và chuyển sang AsyncWebServer).

  Để đạt > 30% và hoàn thành dự án, bạn nên tập trung vào 3
  hướng này:

   1. Xóa sạch biến toàn cục (Task 3): Đây là thay đổi lớn
      nhất về mặt kiến trúc code RTOS. Khi bạn thay đổi toàn
      bộ cách truyền dữ liệu từ biến sang
      xQueueSend/xQueueReceive, mã nguồn sẽ khác biệt hoàn
      toàn so với bản gốc.
   2. Logic điều kiện (Task 1, 2, 3): Bản gốc chỉ chớp tắt vô
      hồn. Bạn thêm logic "thông minh" dựa trên môi trường sẽ
      tạo ra sự khác biệt về tính năng.
   3. Redesign Web UI (Task 4): OhSTEM cung cấp giao diện rất
      cơ bản. Bạn chỉ cần thay đổi CSS/HTML trong thư mục
      data/ là đã tạo ra một bộ mặt hoàn toàn mới cho dự án.

  ---
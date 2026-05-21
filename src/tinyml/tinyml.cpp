#include "tinyml/tinyml.h"
#include "global.h"

// TFLite globals
namespace {
    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;
    constexpr int kTensorArenaSize = 10 * 1024; 
    uint8_t tensor_arena[kTensorArenaSize];

    // Cập nhật từ kết quả notebook sau mỗi lần train lại
    const float SCALER_MEAN[]  = {27.4874f, 49.9192f};
    const float SCALER_SCALE[] = {4.2157f, 12.0575f};
    const float ANOMALY_THRESHOLD = 0.5f;  // Sigmoid output threshold
}

void setupTinyML() {
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(dht_anomaly_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter, "Model version mismatch!");
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);
    Serial.println("[TinyML] Model initialized successfully");
}

void tiny_ml_task(void *pvParameters) {
    setupTinyML();
    
    while (1) {
        struct SensorData sensorData;
        // Chờ nhận dữ liệu từ Queue (do task_rs485 hoặc temp_humi gửi vào)
        if (xQueuePeek(xSensorQueue, &sensorData, portMAX_DELAY)) {
            
            // 1. Pre-processing: Chuẩn hóa dữ liệu (Z-score Scaling)
            float temp_scaled = (sensorData.temperature - SCALER_MEAN[0]) / SCALER_SCALE[0];
            float humi_scaled = (sensorData.humidity - SCALER_MEAN[1]) / SCALER_SCALE[1];

            // 2. Đưa vào Input Tensor
            input->data.f[0] = temp_scaled;
            input->data.f[1] = humi_scaled;

            // 3. Chạy Inference
            if (interpreter->Invoke() != kTfLiteOk) {
                Serial.println("[TinyML] Invoke failed");
                continue;
            }

            // 4. Đọc kết quả phân loại (ANN Classifier, sigmoid output)
            float anomaly_score = output->data.f[0];

            // 5. Kết luận
            bool is_anomaly = (anomaly_score > ANOMALY_THRESHOLD);

            const char* pred = is_anomaly ? "abnormal" : "normal";
            Serial.printf("[TinyML] Input: %.1f,%.1f | Score: %.4f | pred: %s\n",
                          sensorData.temperature, sensorData.humidity, anomaly_score, pred);

            // Kích hoạt Semaphore trạng thái để LCD/LED hiển thị
            if (is_anomaly) {
                xSemaphoreGive(xStatusWarningSemaphore);
            } else {
                xSemaphoreGive(xStatusNormalSemaphore);
            }
            vTaskDelay(5000 / portTICK_PERIOD_MS); // Khớp chu kỳ với sensor (5s)
        }
    }
}

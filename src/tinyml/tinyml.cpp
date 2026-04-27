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

    // Cấu hình dựa trên kết quả Train: Temp [25-30], Humi [40-60]
    const float SCALER_MEAN[] = {27.5f, 50.0f}; 
    const float SCALER_SCALE[] = {1.443f, 5.773f}; 
    const float ANOMALY_THRESHOLD = 0.000589f;  // Ngưỡng MSE lấy từ Notebook
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
        if (xQueueReceive(xSensorQueue, &sensorData, portMAX_DELAY)) {
            
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

            // 4. Tính toán sai số tái tạo (MSE)
            float recon_temp = output->data.f[0];
            float recon_humi = output->data.f[1];
            
            float mse = (pow(temp_scaled - recon_temp, 2) + pow(humi_scaled - recon_humi, 2)) / 2.0f;

            // 5. Kết luận
            bool is_anomaly = (mse > ANOMALY_THRESHOLD);
            
            Serial.printf("[TinyML] Input: %.1f,%.1f | MSE: %.4f | Status: %s\n", 
                          sensorData.temperature, sensorData.humidity, mse, 
                          is_anomaly ? "ANOMALY!" : "Normal");

            // Kích hoạt Semaphore trạng thái để LCD/LED hiển thị
            if (is_anomaly) {
                xSemaphoreGive(xStatusWarningSemaphore);
            } else {
                xSemaphoreGive(xStatusNormalSemaphore);
            }
        }
    }
}

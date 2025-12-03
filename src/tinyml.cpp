#include "tinyml.h"
#include "person_detect_model.h" 
#include <esp_heap_caps.h>
#include <JPEGDEC.h> 

namespace {
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;

    constexpr int kTensorArenaSize = 9 * 1024 * 1024; 
    uint8_t *tensor_arena = nullptr;
}

JPEGDEC jpeg;

// Giải mã JPEG -> Chuyển đổi sang INT8
int JPEGDraw(JPEGDRAW *pDraw) {
    if (input == nullptr) return 0;

    int8_t *input_data_ptr = input->data.int8;
    int width = 240; 

    for (int i = 0; i < pDraw->iWidth * pDraw->iHeight; i++) {
        int x = pDraw->x + (i % pDraw->iWidth);
        int y = pDraw->y + (i / pDraw->iWidth);

        if (x < width && y < width) {
            uint16_t pixel = pDraw->pPixels[i];
            
            // Tách RGB565
            uint8_t r = (pixel & 0xF800) >> 8;
            uint8_t g = (pixel & 0x07E0) >> 3;
            uint8_t b = (pixel & 0x001F) << 3;
            r = (r * 255) / 31; g = (g * 255) / 63; b = (b * 255) / 31;

            int index = ((y * width) + x) * 3; 

            // Chuyển đổi sang INT8: [0, 255] -> [-128, 127]
            input_data_ptr[index]     = (int8_t)((int)r - 128);
            input_data_ptr[index + 1] = (int8_t)((int)g - 128);
            input_data_ptr[index + 2] = (int8_t)((int)b - 128);
        }
    }
    return 1;
}

void setupTinyML() {
    Serial.println("[TinyML] Khởi tạo...");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // 1. Cấp phát PSRAM
    tensor_arena = (uint8_t *)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);
    if (tensor_arena == nullptr) {
        Serial.println("LỖI: Không thể cấp phát PSRAM!");
        return;
    }
    Serial.printf("[TinyML] Arena 6MB OK. Free PSRAM: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    // 2. Load Model
    model = tflite::GetModel(person_detect_model); 
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report("Lỗi Version Model!");
        return;
    }

    // 3. Resolver (Chỉ cần khai báo 1 lần ở đây thôi)
    static tflite::MicroMutableOpResolver<20> resolver;
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddAdd();
    resolver.AddRelu6();
    resolver.AddAveragePool2D(); 
    resolver.AddMaxPool2D();
    resolver.AddFullyConnected();
    resolver.AddSoftmax();
    resolver.AddReshape();
    resolver.AddQuantize(); 
    resolver.AddDequantize();
    resolver.AddMul();
    resolver.AddSub();
    resolver.AddPad(); 
    resolver.AddMean();

    // 4. Interpreter
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("AllocateTensors thất bại!");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);
    
    // Debug
    if (input->type == kTfLiteInt8) Serial.println("[TinyML] Input Type: INT8 (OK)");
    else Serial.printf("[TinyML] Cảnh báo: Input Type là %d (Cần check lại)\n", input->type);

    Serial.println("[TinyML] Sẵn sàng!");
}

void run_inference(uint8_t *image_data, size_t image_len) {
    // Tự động setup nếu chưa chạy
    if (interpreter == nullptr) {
        printf("[TinyML] Chưa khởi tạo, đang khởi tạo...\n");
        setupTinyML();
    }
    if (input == nullptr) {
        Serial.println("[TinyML] Lỗi: Chưa khởi tạo TinyML!");        
        return;
    }

    // 1. Giải mã JPEG -> INT8 Tensor
    if (jpeg.openRAM(image_data, image_len, JPEGDraw)) {
        jpeg.decode(0, 0, 0); 
        jpeg.close();
    } else {
        Serial.println("[TinyML] Lỗi: Ảnh JPEG hỏng!");
        return;
    }

    // 2. Chạy AI
    long start = millis();
    TfLiteStatus invoke_status = interpreter->Invoke();
    long end = millis();

    if (invoke_status != kTfLiteOk) {
        Serial.println("Invoke failed!");
        return;
    }

    // 3. Xử lý kết quả
    float scale = output->params.scale;
    int zero_point = output->params.zero_point;
    int8_t raw_score = output->data.int8[1]; 
    float person_score = (raw_score - zero_point) * scale;

    Serial.printf("AI Time: %ld ms | Score: %.2f%%\n", (end - start), person_score * 100);

    if (person_score > 0.65) {
        Serial.println("=> 🚨 CÓ NGƯỜI!");
    } else {
        Serial.println("=> Không có người.");
    }
    Serial.println("-----------------------------");
}
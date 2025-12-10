#include "tinyml.h"

namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 32 * 1024; 
    uint8_t tensor_arena[kTensorArenaSize];
    
    // StandardScaler parameters from Python training
    // Extract from: scaler.mean_ and scaler.scale_ (which is 1/std)
    const float TEMP_MEAN = 29.95f;
    const float TEMP_STD = 10.02f;
    const float HUMID_MEAN = 0.6234f;
    const float HUMID_STD = 0.1447f;
}

void setupTinyML()
{
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(dht_anomaly_model_tflite); 
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                               model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    // Debug tensor info - DETAILED
    Serial.println("\n=== TFLite Model Info ===");
    Serial.printf("Input dims: %d, type: %d (0=float32, 1=int8, 3=int32)\n", input->dims->size, input->type);
    Serial.printf("Output dims: %d, type: %d\n", output->dims->size, output->type);
    
    Serial.print("Input shape: [");
    for (int i = 0; i < input->dims->size; i++) {
        Serial.print(input->dims->data[i]);
        if (i < input->dims->size - 1) Serial.print(" x ");
    }
    Serial.println("]");
    
    Serial.print("Output shape: [");
    for (int i = 0; i < output->dims->size; i++) {
        Serial.print(output->dims->data[i]);
        if (i < output->dims->size - 1) Serial.print(" x ");
    }
    Serial.println("]");
    
    Serial.printf("Input bytes: %d, Output bytes: %d\n", input->bytes, output->bytes);
    Serial.println("=========================\n");
}

void tiny_ml_task(void *pvParameters)
{

    setupTinyML();

    while (1)
    {
        // Convert humidity from 0-100% to 0-1 range
        sensorData* pxdata;
        xQueueReceive( xQueueSensorData, &pxdata, portMAX_DELAY );

        float humidity_normalized_range = pxdata->humidity / 100.0f;
        
        // Normalize input using StandardScaler parameters
        float temp_normalized = (pxdata->temperature - TEMP_MEAN) / TEMP_STD;
        float humid_normalized = (humidity_normalized_range - HUMID_MEAN) / HUMID_STD;
        
        // Set normalized values to model input
        input->data.f[0] = temp_normalized;
        input->data.f[1] = humid_normalized;

        Serial.printf("\nInference\n");
        Serial.printf("Temperature=%.2f, Humidity=%.2f\n", pxdata->temperature, pxdata->humidity);

        // Run inference
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk)
        {
            Serial.printf("Invoke failed with status: %d\n", invoke_status);
            error_reporter->Report("Invoke failed");
            vTaskDelay(3000);
            continue;
        }

        // Get output
        float result = output->data.f[0];
        Serial.printf("Output[0] (score): %.6f\n", result);
        
        // Check for NaN/Inf
        if (isnan(result)) {
            Serial.println("Output is NaN!");
        } else if (isinf(result)) {
            Serial.println("Output is Infinity!");
        } else {
            if (result > 0.5)
            {
                Serial.println("Result => ANOMALY\n");
            }
            else
            {
                Serial.println("Result => NORMAL\n");
            }
        }
        
        pxdata->anomaly = (result > 0.5) ? 1 : 0;
        xQueueSend( xQueueAnomalyResult, (void *) &pxdata, (TickType_t) 0 );
        vTaskDelay(3000);
    }
}
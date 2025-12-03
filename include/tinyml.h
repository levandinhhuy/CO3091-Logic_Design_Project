#ifndef __TINY_ML__
#define __TINY_ML__

#include <Arduino.h>

#include "dht_anomaly_model.h"
#include "global.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

void setupTinyML();
void run_inference(uint8_t *image_data, size_t image_len);
#endif
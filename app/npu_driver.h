/**
 * @file npu_driver.h
 * @brief Ethos-U55 NPU Driver for Alif E8
 */

#ifndef NPU_DRIVER_H
#define NPU_DRIVER_H

#include <stdint.h>
#include <stddef.h>

#define NPU_BASE_ADDR   0x50004000UL
#define NPU_ARENA_SIZE  (128 * 1024)

#define NPU_OK              0
#define NPU_ERROR_INIT      -1
#define NPU_ERROR_INFERENCE -2
#define NPU_ERROR_TIMEOUT   -3

int npu_init(void);
int npu_run_inference(const uint8_t* model_data, size_t model_size,
                      const int8_t* input, size_t input_size,
                      int8_t* output, size_t output_size);
uint32_t npu_get_cycles(void);
int argmax_int8(const int8_t* data, size_t size);
int calculate_confidence(const int8_t* scores, size_t size, int predicted_idx);

#endif /* NPU_DRIVER_H */

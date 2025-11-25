/**
 * @file main.c
 * @brief MNIST Inference Demo for Alif E8 with Ethos-U55
 *        Using SEGGER RTT for output (works on macOS!)
 */

#include <stdint.h>
#include <string.h>
#include "SEGGER_RTT.h"
#include "npu_driver.h"
#include "mnist_model_data.h"
#include "test_data.h"
#include "model_config.h"

#define SYSTICK_BASE    0xE000E010UL
#define CYCLES_PER_US   (160)  /* 160 MHz */

typedef struct {
    volatile uint32_t CTRL, LOAD, VAL, CALIB;
} SysTick_TypeDef;

static SysTick_TypeDef* const SysTick = (SysTick_TypeDef*)SYSTICK_BASE;
static int8_t output_scores[MODEL_OUTPUT_SIZE];

/* ASCII art digits */
static const char* digit_art[10][5] = {
    {" ### ", "#   #", "#   #", "#   #", " ### "},  /* 0 */
    {"  #  ", " ##  ", "  #  ", "  #  ", " ### "},  /* 1 */
    {" ### ", "#   #", "  ## ", " #   ", "#####"},  /* 2 */
    {"#### ", "    #", " ### ", "    #", "#### "},  /* 3 */
    {"#   #", "#   #", "#####", "    #", "    #"},  /* 4 */
    {"#####", "#    ", "#### ", "    #", "#### "},  /* 5 */
    {" ### ", "#    ", "#### ", "#   #", " ### "},  /* 6 */
    {"#####", "    #", "   # ", "  #  ", " #   "},  /* 7 */
    {" ### ", "#   #", " ### ", "#   #", " ### "},  /* 8 */
    {" ### ", "#   #", " ####", "    #", " ### "}   /* 9 */
};

static void systick_init(void) {
    SysTick->CTRL = 0;
    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x05;
}

static uint32_t systick_get(void) { 
    return SysTick->VAL; 
}

static uint32_t cycles_to_us(uint32_t c) { 
    return c / CYCLES_PER_US; 
}

static void print_banner(void) {
    SEGGER_RTT_WriteString(0, "\r\n");
    SEGGER_RTT_WriteString(0, "========================================\r\n");
    SEGGER_RTT_WriteString(0, "     ALIF E8 MNIST NPU DEMO\r\n");
    SEGGER_RTT_WriteString(0, "     Ethos-U55 Accelerated\r\n");
    SEGGER_RTT_WriteString(0, "     (RTT Output)\r\n");
    SEGGER_RTT_WriteString(0, "========================================\r\n");
    SEGGER_RTT_WriteString(0, "\r\n");
}

static void print_digit_art(int digit) {
    if (digit < 0 || digit > 9) {
        SEGGER_RTT_WriteString(0, "  ???\r\n");
        return;
    }
    SEGGER_RTT_WriteString(0, "\r\n");
    for (int r = 0; r < 5; r++) {
        SEGGER_RTT_printf(0, "        %s\r\n", digit_art[digit][r]);
    }
    SEGGER_RTT_WriteString(0, "\r\n");
}

static void print_confidence_bar(int confidence) {
    SEGGER_RTT_WriteString(0, "  Confidence: [");
    int bar = (confidence * 20) / 100;
    for (int i = 0; i < 20; i++) {
        SEGGER_RTT_WriteString(0, (i < bar) ? "#" : "-");
    }
    SEGGER_RTT_printf(0, "] %d%%\r\n", confidence);
}

static void print_result(int digit, int confidence, uint32_t inference_us) {
    SEGGER_RTT_WriteString(0, "\r\n");
    SEGGER_RTT_WriteString(0, "+--------------------------------------+\r\n");
    SEGGER_RTT_WriteString(0, "|       DIGIT RECOGNITION RESULT       |\r\n");
    SEGGER_RTT_WriteString(0, "+--------------------------------------+\r\n");
    SEGGER_RTT_printf(0, "  Predicted Digit: %d\r\n", digit);
    
    print_digit_art(digit);
    print_confidence_bar(confidence);
    
    SEGGER_RTT_printf(0, "  Inference Time: %u us\r\n", inference_us);
    
    uint32_t fps = inference_us > 0 ? 1000000 / inference_us : 0;
    SEGGER_RTT_printf(0, "  Throughput: %u FPS\r\n", fps);
    
    SEGGER_RTT_WriteString(0, "+--------------------------------------+\r\n");
    SEGGER_RTT_WriteString(0, "\r\n");
}

static void run_demo_inference(void) {
    SEGGER_RTT_WriteString(0, "Running inference on test image...\r\n");
    SEGGER_RTT_printf(0, "Expected digit: %d\r\n", EXPECTED_DIGIT);
    
    uint32_t start = systick_get();
    int result = npu_run_inference(mnist_model_data, MNIST_MODEL_SIZE,
                                   test_input_data, TEST_IMAGE_SIZE,
                                   output_scores, MODEL_OUTPUT_SIZE);
    uint32_t end = systick_get();
    
    uint32_t elapsed = (start >= end) ? (start - end) : ((0xFFFFFF - end) + start);
    uint32_t us = cycles_to_us(elapsed);
    
    if (result != NPU_OK) {
        SEGGER_RTT_printf(0, "ERROR: Inference failed (%d)\r\n", result);
        return;
    }
    
    int predicted = argmax_int8(output_scores, MODEL_OUTPUT_SIZE);
    int confidence = calculate_confidence(output_scores, MODEL_OUTPUT_SIZE, predicted);
    
    print_result(predicted, confidence, us);
    
    if (predicted == EXPECTED_DIGIT) {
        SEGGER_RTT_WriteString(0, ">>> CORRECT! <<<\r\n");
    } else {
        SEGGER_RTT_WriteString(0, ">>> INCORRECT <<<\r\n");
    }
    SEGGER_RTT_WriteString(0, "\r\n");
}

static void run_benchmark(int iterations) {
    SEGGER_RTT_printf(0, "Running benchmark: %d iterations...\r\n", iterations);
    
    uint32_t total_start = systick_get();
    for (int i = 0; i < iterations; i++) {
        npu_run_inference(mnist_model_data, MNIST_MODEL_SIZE,
                         test_input_data, TEST_IMAGE_SIZE,
                         output_scores, MODEL_OUTPUT_SIZE);
        if ((i + 1) % 100 == 0) {
            SEGGER_RTT_printf(0, "  Completed: %d\r\n", i + 1);
        }
    }
    uint32_t total_end = systick_get();
    
    uint32_t elapsed = (total_start >= total_end) ? 
                       (total_start - total_end) : ((0xFFFFFF - total_end) + total_start);
    uint32_t us = cycles_to_us(elapsed);
    
    SEGGER_RTT_WriteString(0, "\r\n");
    SEGGER_RTT_WriteString(0, "========================================\r\n");
    SEGGER_RTT_WriteString(0, "BENCHMARK RESULTS\r\n");
    SEGGER_RTT_WriteString(0, "========================================\r\n");
    SEGGER_RTT_printf(0, "  Iterations: %d\r\n", iterations);
    SEGGER_RTT_printf(0, "  Total time: %u us\r\n", us);
    SEGGER_RTT_printf(0, "  Avg/inference: %u us\r\n", us / iterations);
    SEGGER_RTT_printf(0, "  Throughput: %u FPS\r\n", (iterations * 1000000UL) / us);
    SEGGER_RTT_WriteString(0, "========================================\r\n");
    SEGGER_RTT_WriteString(0, "\r\n");
}

static void print_menu(void) {
    SEGGER_RTT_WriteString(0, "Commands (type in RTT Viewer):\r\n");
    SEGGER_RTT_WriteString(0, "  1 - Run single inference\r\n");
    SEGGER_RTT_WriteString(0, "  2 - Run benchmark (100 iterations)\r\n");
    SEGGER_RTT_WriteString(0, "  3 - Run benchmark (1000 iterations)\r\n");
    SEGGER_RTT_WriteString(0, "  4 - Show model info\r\n");
    SEGGER_RTT_WriteString(0, "  5 - Show output scores\r\n");
    SEGGER_RTT_WriteString(0, "  h - Show this menu\r\n");
    SEGGER_RTT_WriteString(0, "\r\n> ");
}

static void show_model_info(void) {
    SEGGER_RTT_WriteString(0, "\r\n");
    SEGGER_RTT_WriteString(0, "========================================\r\n");
    SEGGER_RTT_WriteString(0, "MODEL INFORMATION\r\n");
    SEGGER_RTT_WriteString(0, "========================================\r\n");
    SEGGER_RTT_printf(0, "  Model size: %u bytes\r\n", MNIST_MODEL_SIZE);
    SEGGER_RTT_printf(0, "  Input size: %d (28x28x1)\r\n", MODEL_INPUT_SIZE);
    SEGGER_RTT_printf(0, "  Output: %d classes\r\n", MODEL_OUTPUT_SIZE);
    SEGGER_RTT_printf(0, "  Arena: %u bytes\r\n", TENSOR_ARENA_SIZE);
    SEGGER_RTT_WriteString(0, "========================================\r\n");
    SEGGER_RTT_WriteString(0, "\r\n");
}

static void show_scores(void) {
    SEGGER_RTT_WriteString(0, "\r\nOutput Scores:\r\n");
    for (int i = 0; i < MODEL_OUTPUT_SIZE; i++) {
        SEGGER_RTT_printf(0, "  [%d]: %d  ", i, output_scores[i]);
        int bar = (output_scores[i] + 128) / 10;
        for (int j = 0; j < bar; j++) {
            SEGGER_RTT_WriteString(0, "#");
        }
        SEGGER_RTT_WriteString(0, "\r\n");
    }
    SEGGER_RTT_WriteString(0, "\r\n");
}

static void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 16000; i++) {
        __asm__("nop");
    }
}

int main(void) {
    systick_init();
    SEGGER_RTT_Init();
    
    /* Small delay to let RTT connect */
    delay_ms(100);
    
    print_banner();
    
    SEGGER_RTT_WriteString(0, "Initializing NPU... ");
    int r = npu_init();
    SEGGER_RTT_WriteString(0, (r == NPU_OK) ? "OK\r\n" : "FAILED\r\n");
    SEGGER_RTT_WriteString(0, "\r\n");
    
    /* Run initial inference */
    run_demo_inference();
    
    /* Print menu */
    print_menu();
    
    /* Main loop - check for RTT input */
    while (1) {
        if (SEGGER_RTT_HasKey()) {
            int cmd = SEGGER_RTT_GetKey();
            SEGGER_RTT_printf(0, "%c\r\n", cmd);
            
            switch (cmd) {
                case '1': run_demo_inference(); break;
                case '2': run_benchmark(100); break;
                case '3': run_benchmark(1000); break;
                case '4': show_model_info(); break;
                case '5': show_scores(); break;
                case 'h': case 'H': case '?': print_menu(); break;
                default: 
                    SEGGER_RTT_WriteString(0, "Unknown command. Press 'h' for help.\r\n"); 
                    break;
            }
            SEGGER_RTT_WriteString(0, "> ");
        }
        
        /* Small delay to avoid busy-waiting */
        for (volatile int i = 0; i < 10000; i++);
    }
    
    return 0;
}

/* Startup Code */
extern uint32_t _estack;

void Reset_Handler(void) __attribute__((noreturn));
void Reset_Handler(void) { main(); while (1); }

void Default_Handler(void) { while (1); }
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))(&_estack), Reset_Handler, NMI_Handler, HardFault_Handler,
    MemManage_Handler, BusFault_Handler, UsageFault_Handler, 0, 0, 0, 0,
    SVC_Handler, 0, 0, PendSV_Handler, SysTick_Handler
};

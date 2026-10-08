#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "bms.h"
#include "safety_interlock.h"

int main(int argc, char **argv) {
    assert(argc == 2);
    uint16_t values[BMS_NUM_CELLS];
    for (unsigned i = 0; i < BMS_NUM_CELLS; ++i) {
        values[i] = 3500;
        g_bms_data.cell_voltages[i] = 3.5f;
    }
    g_bms_data.state = BMS_STATE_NORMAL;
    GPIO_Init_Relays();
    I2C_Sensor_Init();
    UART_Init(115200);
    if (!strcmp(argv[1], "uart")) {
        char c;
        assert(RingBuf_Read(NULL) == -1);
        assert(RingBuf_Read(&c) == -1);
        for (unsigned i = 0; i < RING_BUF_SIZE - 1; ++i) assert(RingBuf_Write((char)i) == 0);
        assert(RingBuf_Write('x') == -1);
        assert(RingBuf_GetCount() == RING_BUF_SIZE - 1);
        for (unsigned i = 0; i < RING_BUF_SIZE - 1; ++i) {
            assert(RingBuf_Read(&c) == 0 && (unsigned char)c == (unsigned char)i);
        }
        assert(RingBuf_Write('z') == 0 && RingBuf_Read(&c) == 0 && c == 'z');
        puts("PASS: UART ring overflow, empty and wrap");
        return 0;
    }
    if (!strcmp(argv[1], "dma")) {
        ADC_DMA_Init(g_adc_dma_buffer, NUM_CELLS);
        assert(DMA2->Stream[0].CR & (1U << 10));
        assert(DMA2->Stream[0].M0AR == (uintptr_t)g_adc_dma_buffer);
        DMA2->LISR = 1U << 5;
        DMA2_Stream0_IRQHandler();
        assert(!(DMA2->LISR & (1U << 5)));
        puts("PASS: DMA mock configuration and interrupt clearing");
        return 0;
    }
    if (!strcmp(argv[1], "bounds")) {
        assert(SafetyInterlock_Evaluate(NULL, BMS_NUM_CELLS) == RELAY_STATE_OPEN);
        assert(SafetyInterlock_Evaluate(values, BMS_NUM_CELLS - 1) == RELAY_STATE_OPEN);
        return 0;
    }
    if (!strcmp(argv[1], "latch")) {
        values[11] = 4300;
        assert(SafetyInterlock_Evaluate(values, BMS_NUM_CELLS) == RELAY_STATE_OPEN);
        values[11] = 3500;
        assert(SafetyInterlock_Evaluate(values, BMS_NUM_CELLS) == RELAY_STATE_OPEN);
        return 0;
    }
    if (!strcmp(argv[1], "over")) g_bms_data.cell_voltages[11] = 4.3f;
    if (!strcmp(argv[1], "under")) g_bms_data.cell_voltages[11] = 2.7f;
    if (!strcmp(argv[1], "nan")) g_bms_data.cell_voltages[11] = NAN;
    if (!strcmp(argv[1], "thermal")) g_sim_overtemp = 1;
    TIM2->SR |= TIM_SR_UIF;
    TIM2_IRQHandler();
    if (!strcmp(argv[1], "nominal")) {
        assert(g_bms_data.state == BMS_STATE_NORMAL);
        assert(GPIOA->ODR & (1U << 5));
    } else {
        assert(g_bms_data.state == BMS_STATE_FAULT);
        assert(!(GPIOA->ODR & (1U << 5)));
        assert(g_bms_data.pack_current == 0.0f);
    }
    printf("PASS: %s\n", argv[1]);
    return 0;
}

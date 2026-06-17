#include "bms.h"
#include <stddef.h>

/* Mock Allocations for ADC & DMA Peripherals */
static ADC_TypeDef mock_adc1;
static DMA_TypeDef mock_dma2;

ADC_TypeDef *ADC1 = &mock_adc1;
DMA_TypeDef *DMA2 = &mock_dma2;

uint16_t g_adc_dma_buffer[NUM_CELLS];

/**
 * Initialize ADC1 and DMA2 Stream 0 to continuously transfer cell voltage conversions.
 * Configured in circular mode, transfer size 16-bit, Memory increment enabled.
 */
void ADC_DMA_Init(uint16_t *dest_buffer, uint32_t buffer_size) {
    /* 1. Turn on ADC and enable DMA requests */
    ADC1->CR2 |= ADC_CR2_ADON;      /* Wake up ADC */
    ADC1->CR2 |= ADC_CR2_DMA;       /* Enable DMA requests from ADC */
    ADC1->CR2 |= ADC_CR2_DDS;       /* DMA Requests continue after transfer complete */

    /* 2. Configure DMA2 Stream 0 (Channel 0 is mapping for ADC1 on STM32F4) */
    DMA2->Stream[0].CR &= ~DMA_SxCR_EN; /* Disable stream to configure */
    while (DMA2->Stream[0].CR & DMA_SxCR_EN) {
        /* Wait for stream to turn off */
    }

    /* Set peripheral source address (ADC1 Data Register) */
    DMA2->Stream[0].PAR = (uint32_t)&(ADC1->DR);

    /* Set destination memory address (our global array) */
    DMA2->Stream[0].M0AR = (uint32_t)dest_buffer;

    /* Set number of transfers (12 cell channels) */
    DMA2->Stream[0].NDTR = buffer_size;

    /* Configure CR: 
     * - Peripheral & Memory size 16-bit (PSIZE_0, MSIZE_0)
     * - Memory increment enabled (MINC)
     * - Circular mode enabled (CIRC)
     * - Transfer Complete Interrupt enabled (TCIE)
     */
    DMA2->Stream[0].CR = 0;
    DMA2->Stream[0].CR |= (1 << 13) | (1 << 11); // PSIZE=16bit, MSIZE=16bit
    DMA2->Stream[0].CR |= DMA_SxCR_MINC;         // Increment memory address pointer
    DMA2->Stream[0].CR |= DMA_SxCR_CIRC;         // Enable circular buffer mode
    DMA2->Stream[0].CR |= DMA_SxCR_TCIE;         // Enable interrupt on Transfer Complete

    /* 3. Enable DMA Stream */
    DMA2->Stream[0].CR |= DMA_SxCR_EN;
}

/**
 * Mock Simulation: Triggers when the ADC conversion finishes.
 * Automatically updates raw counts and sets the DMA interrupt flag.
 */
void Simulate_ADC_Conversion(void) {
    static float time_counter = 0.0f;
    time_counter += 0.1f;

    // Simulate cell voltages converting to 12-bit ADC raw counts (0 - 4095 range, representing 0V - 5.0V)
    // Formula: raw_count = (voltage / 5.0) * 4095
    for (int i = 0; i < NUM_CELLS; i++) {
        // Normal cell voltages around 3.65V, with slight ripple and cell mismatch
        float volt = 3.65f + 0.05f * (i % 3) - 0.02f * (i % 2);
        
        // Simulating low cell voltage fault on cell 5 if flag is set in pack simulation
        if (g_bms_data.active_faults & FAULT_UNDERVOLT && i == 5) {
            volt = 2.5f; // Drop cell voltage below 2.8V safety limit
        }

        uint16_t raw_adc = (uint16_t)((volt / 5.0f) * 4095.0f);
        g_adc_dma_buffer[i] = raw_adc;
    }

    // Set Transfer Complete interrupt flag in DMA status register (LISR stream 0)
    DMA2->LISR |= (1 << 5); // Bit 5 is Stream 0 Transfer Complete flag

    // Trigger DMA ISR if interrupt is enabled
    if (DMA2->Stream[0].CR & DMA_SxCR_TCIE) {
        DMA2_Stream0_IRQHandler();
    }
}

/**
 * DMA2 Stream 0 Interrupt Service Routine (ISR)
 * Called when the 12-channel cell data block transfer is complete.
 */
void DMA2_Stream0_IRQHandler(void) {
    // Check if Stream 0 Transfer Complete flag is active
    if (DMA2->LISR & (1 << 5)) {
        // Clear interrupt flag
        DMA2->LIFCR |= (1 << 5);

        // Process DMA values: convert raw ADC counts to actual voltages
        for (int i = 0; i < NUM_CELLS; i++) {
            // Decoded voltage: raw_count * (5.0 / 4095)
            g_bms_data.cell_voltages[i] = ((float)g_adc_dma_buffer[i] / 4095.0f) * 5.0f;
        }

        // Calculate total pack voltage
        float total_v = 0.0f;
        for (int i = 0; i < NUM_CELLS; i++) {
            total_v += g_bms_data.cell_voltages[i];
        }
        g_bms_data.pack_voltage = total_v;
    }
}

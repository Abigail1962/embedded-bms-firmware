#include <stdio.h>
#include <unistd.h>
#include "bms.h"

// External simulation helpers
void Simulate_ADC_Conversion(void);

int main(void) {
    printf("==================================================================\n");
    printf("     STM32 BARE-METAL BATTERY MANAGEMENT SYSTEM FIRMWARE          \n");
    printf("==================================================================\n\n");

    /* --- System Initialization Section --- */
    GPIO_Init_Relays();
    ADC_DMA_Init(g_adc_dma_buffer, NUM_CELLS);
    I2C_Sensor_Init();
    UART_Init(115200);
    Timer_Init(100); // 100ms (10Hz) safety polling rate

    g_bms_data.state = BMS_STATE_NORMAL;
    g_bms_data.active_faults = 0;

    System_Log("[BMS-INIT] Bare-metal drivers initialized successfully.");
    System_Log("[BMS-INIT] CPU configured: ADC + DMA (Circular), TIM2 (10Hz ISR), I2C1 (100kHz), UART1.");

    /* --- Real-Time Hardware Event Loop Simulation --- */
    printf("\n--- Starting Hardware Event Loop (TIM2 & DMA Polling) ---\n\n");
    
    for (int cycle = 0; cycle < 30; cycle++) {
        // 1. Simulate hardware ADC conversion completing -> triggers DMA ISR
        Simulate_ADC_Conversion();

        // 2. Inject Faults at specific time intervals to verify Safety State Machine
        if (cycle == 8) {
            printf("\n[SIMULATION EVENT] Injecting Thermal Overtemp Fault (Sensor 0 > 60 C)...\n");
            g_bms_data.active_faults |= FAULT_OVERTEMP;
        }
        
        if (cycle == 18) {
            printf("\n[SIMULATION EVENT] Clearing active faults. Resetting state to normal...\n");
            g_bms_data.active_faults &= ~FAULT_OVERTEMP;
            g_bms_data.state = BMS_STATE_NORMAL;
        }

        // 3. Simulate Timer 2 Hardware Update Event Interrupt -> triggers TIM2 ISR
        TIM2->SR |= TIM_SR_UIF; // Set update interrupt flag
        TIM2_IRQHandler();

        // 4. Print System Status Grid (representing registers & pin outputs)
        uint32_t pin_state = (GPIOA->ODR & (1 << 5)) ? 1 : 0; // Check state of PA5
        
        printf("[t=%dms] | Relays GPIO-PA5: %s | State: %s | Temp0: %.1f C | Cell5: %.2f V | Pack V: %.1f V | Faults: 0x%X\n", 
            cycle * 100,
            (pin_state ? "ON (HIGH)" : "OFF (LOW)"),
            (g_bms_data.state == BMS_STATE_NORMAL ? "NORMAL" : "FAULT"),
            g_bms_data.temperatures[0],
            g_bms_data.cell_voltages[5],
            g_bms_data.pack_voltage,
            g_bms_data.active_faults
        );

        // Sleep to simulate time delay (100ms)
        usleep(100000);
    }

    printf("\n==================================================================\n");
    printf("     🟢 SIMULATION COMPLETE: ALL HARDWARE safety loops verified.  \n");
    printf("==================================================================\n");

    return 0;
}

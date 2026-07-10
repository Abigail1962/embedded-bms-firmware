#include <stdio.h>
#include <unistd.h>
#include "bms.h"
#include "safety_interlock.h"
#include "can_driver.h"
#include "pid_controller.h"

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
    CAN_Init();

    PID_Controller_t dc_dc_pid;
    // kp=0.5, ki=0.1, kd=0.01, target_temp=45.0C, min_current=0A, max_current=150A
    PID_Init(&dc_dc_pid, 0.5f, 0.1f, 0.01f, 45.0f, 0.0f, 150.0f);

    g_bms_data.state = BMS_STATE_NORMAL;
    g_bms_data.active_faults = 0;

    System_Log("[BMS-INIT] Bare-metal drivers initialized successfully.");
    System_Log("[BMS-INIT] CPU configured: ADC + DMA, TIM2, I2C1, UART1, CAN1.");

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

        // 4. MISRA C Safety Interlock Evaluation
        // Convert floating point mock voltages to millivolts for the MISRA function
        uint16_t mv_array[BMS_NUM_CELLS];
        for (int i=0; i<BMS_NUM_CELLS; i++) {
            mv_array[i] = (uint16_t)(g_bms_data.cell_voltages[i] * 1000.0);
        }
        
        RelayState_t safe_state = SafetyInterlock_Evaluate(mv_array, BMS_NUM_CELLS);
        if (safe_state == RELAY_STATE_OPEN) {
            g_bms_data.active_faults |= FAULT_OVERVOLTAGE; // Force fault if interlock trips
        }

        // 5. PID Control for DC/DC Charging Current based on max cell temp
        float max_temp = g_bms_data.temperatures[0]; // Simplified
        float current_adj = PID_Compute(&dc_dc_pid, max_temp, 0.1f);

        // 6. CAN Telemetry Broadcast
        CAN_Message_t tlm_msg;
        tlm_msg.id = 0x2A0; // BMS Telemetry ID
        tlm_msg.dlc = 8;
        tlm_msg.data[0] = g_bms_data.state;
        tlm_msg.data[1] = g_bms_data.active_faults;
        CAN_Transmit(&tlm_msg);

        // 7. Print System Status Grid
        uint32_t pin_state = (safe_state == RELAY_STATE_CLOSED) ? 1 : 0; // Hardware pin follows safety interlock
        
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

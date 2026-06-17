#include "bms.h"
#include <stddef.h>

/* Mock Allocations for TIM2 & GPIO Peripherals */
static TIM_TypeDef mock_tim2;
static GPIO_TypeDef mock_gpioa;
static GPIO_TypeDef mock_gpiob;

TIM_TypeDef *TIM2 = &mock_tim2;
GPIO_TypeDef *GPIOA = &mock_gpioa;
GPIO_TypeDef *GPIOB = &mock_gpiob;

BMS_Data_t g_bms_data;

/**
 * Configure TIM2 (general-purpose timer) for periodic safety interrupts.
 */
void Timer_Init(uint16_t period_ms) {
    /* 1. Set Prescaler and Auto-reload register values */
    TIM2->PSC = 16000 - 1;         // Prescale clock down to 1kHz (assuming 16MHz clock)
    TIM2->ARR = period_ms - 1;     // Set reload period to parameter (e.g. 100ms -> TIM2 interrupts at 10Hz)
    
    /* 2. Enable Update Interrupt */
    TIM2->DIER |= TIM_DIER_UIE;

    /* 3. Enable Timer Counter */
    TIM2->CR1 |= TIM_CR1_CEN;
}

/**
 * Initialize GPIO pins for Safety Relay Control.
 * GPIOA Pin 5: High Voltage Contactor Coil Relay (Active High output).
 */
void GPIO_Init_Relays(void) {
    /* Set GPIOA Pin 5 mode as Output (01 in MODER register) */
    GPIOA->MODER &= ~(3 << (5 * 2)); // Clear mode bits
    GPIOA->MODER |= (1 << (5 * 2));  // Set as general purpose output mode
    
    /* Set GPIOA Pin 5 speed as High Speed (11 in OSPEEDR) */
    GPIOA->OSPEEDR |= (3 << (5 * 2));
}

/**
 * Toggle the contactor coil relay state via GPIO.
 */
void Contactor_Control(uint8_t close_contactors) {
    if (close_contactors) {
        /* Set Pin 5 high (using Bit Set/Reset Register BSRR) */
        GPIOA->BSRR = (1 << 5); 
        GPIOA->ODR |= (1 << 5);          // Simulating hardware register side-effect
        g_bms_data.pack_current = 15.0f; // Active load current
    } else {
        /* Reset Pin 5 low (using write BSRR upper half) */
        GPIOA->BSRR = (1 << (5 + 16)); 
        GPIOA->ODR &= ~(1 << 5);         // Simulating hardware register side-effect
        g_bms_data.pack_current = 0.0f;  // Cut off load current
    }
}

/**
 * Timer 2 Interrupt Service Routine (TIM2_IRQHandler)
 * Execution rate: 10Hz (triggered every 100ms by hardware timer).
 * Performs critical safety state monitoring: checks ADC voltage array and I2C temperature sensors.
 */
void TIM2_IRQHandler(void) {
    // Check if Update Interrupt flag is set
    if (TIM2->SR & TIM_SR_UIF) {
        // Clear Update Interrupt flag
        TIM2->SR &= ~TIM_SR_UIF;

        uint8_t safety_fault_detected = 0;

        /* 1. Query I2C temperature sensors */
        for (int i = 0; i < 4; i++) {
            g_bms_data.temperatures[i] = I2C_Read_Temperature(i);
            
            // Check over-temperature threshold
            if (g_bms_data.temperatures[i] > CRITICAL_TEMP_HIGH) {
                g_bms_data.active_faults |= FAULT_OVERTEMP;
                safety_fault_detected = 1;
            }
        }

        /* 2. Analyze cell voltages (previously converted and populated by DMA) */
        for (int i = 0; i < NUM_CELLS; i++) {
            // Check critical cell undervoltage limit
            if (g_bms_data.cell_voltages[i] < CRITICAL_VOLT_LOW) {
                g_bms_data.active_faults |= FAULT_UNDERVOLT;
                safety_fault_detected = 1;
            }
        }

        /* 3. Safety State Machine Transitions & Interlock Relays Control */
        if (safety_fault_detected) {
            g_bms_data.state = BMS_STATE_FAULT;
            
            // EMERGENCY SAFETY TRIP: Instantly open contactors
            Contactor_Control(0);
            System_Log("[BMS-ISR] SAFETY FAULT TRIP! High Voltage contactors opened via GPIO.");
        } else {
            // Normal operation - maintain closed contactors
            if (g_bms_data.state == BMS_STATE_NORMAL) {
                Contactor_Control(1);
            }
        }
    }
}

#ifndef BMS_H
#define BMS_H

#include <stdint.h>
#include "stm32f4xx.h"

#define NUM_CELLS 12
#define RING_BUF_SIZE 256

/* Safety Thresholds */
#define CRITICAL_TEMP_HIGH  60.0f  /* Celsius */
#define CRITICAL_VOLT_LOW   2.8f   /* Volts */

typedef enum {
    BMS_STATE_NORMAL = 0,
    BMS_STATE_FAULT
} BMS_State_t;

typedef struct {
    uint16_t cell_voltages_raw[NUM_CELLS]; /* Raw ADC counts */
    float cell_voltages[NUM_CELLS];         /* Decoded voltages */
    float temperatures[4];                  /* Cell temperatures (C) */
    float pack_voltage;
    float pack_current;
    BMS_State_t state;
    uint32_t active_faults;                 /* Bitmask of faults */
} BMS_Data_t;

/* Active Fault Bitmasks */
#define FAULT_OVERTEMP      (1 << 0)
#define FAULT_UNDERVOLT     (1 << 1)
#define FAULT_SENSOR_ERR    (1 << 2)

/* Global variables */
extern BMS_Data_t g_bms_data;
extern uint16_t g_adc_dma_buffer[NUM_CELLS];

/* API Declarations */

/* Driver initializations */
void GPIO_Init_Relays(void);
void ADC_DMA_Init(uint16_t *dest_buffer, uint32_t buffer_size);
void I2C_Sensor_Init(void);
void UART_Init(uint32_t baudrate);
void Timer_Init(uint16_t period_ms);

/* Operation functions */
float I2C_Read_Temperature(uint8_t sensor_addr);
void Contactor_Control(uint8_t close_contactors);
void System_Log(const char *msg);

/* Ring Buffer / Circular Queue API */
int RingBuf_Write(char c);
int RingBuf_Read(char *c);
uint32_t RingBuf_GetCount(void);

/* Interrupt Service Routines (ISRs) */
void TIM2_IRQHandler(void);
void USART1_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);

#endif /* BMS_H */

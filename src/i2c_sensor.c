#include "bms.h"
#include <stddef.h>

/* Mock Allocations for I2C Peripheral */
static I2C_TypeDef mock_i2c1;
I2C_TypeDef *I2C1 = &mock_i2c1;

/**
 * Initialize I2C1 peripheral in Standard Mode (100kHz).
 */
void I2C_Sensor_Init(void) {
    /* 1. Set I2C Peripheral Enable bit */
    I2C1->CR1 |= I2C_CR1_PE;

    /* 2. Configure mock CCR (Clock Control) and rise time registers */
    I2C1->CCR = 50;       /* Mock CCR value for Standard 100kHz Mode */
    I2C1->TRISE = 9;      /* Mock Max Rise Time register */
}

/* --- Low-level Register-Based I2C Protocol Operations --- */

static void I2C_Start(void) {
    I2C1->CR1 |= I2C_CR1_START;
    
    /* Mock hardware poll: wait for Start Bit (SB) generated */
    I2C1->SR1 |= I2C_SR1_SB; 
    while (!(I2C1->SR1 & I2C_SR1_SB)) {
        /* In real hardware, wait for SB to set */
    }
}

static void I2C_Write(uint8_t data) {
    I2C1->DR = data;
    
    /* Mock hardware poll: wait for Data Register Empty (TXE) */
    I2C1->SR1 |= I2C_SR1_TXE;
    while (!(I2C1->SR1 & I2C_SR1_TXE)) {
        /* Wait for TXE to set */
    }
}

static void I2C_SendAddress(uint8_t addr, uint8_t is_read) {
    // Left-shift address and append Read/Write bit
    uint8_t frame = (addr << 1) | (is_read ? 1 : 0);
    I2C_Write(frame);
    
    /* Mock hardware poll: wait for Address matched flag (ADDR) */
    I2C1->SR1 |= I2C_SR1_ADDR;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        /* Wait for ADDR to set */
    }
    
    /* Clear ADDR bit by reading SR1 and SR2 */
    volatile uint32_t dummy = I2C1->SR1;
    dummy = I2C1->SR2;
    (void)dummy;
}

static uint8_t I2C_ReadAck(void) {
    I2C1->CR1 |= I2C_CR1_ACK; // Enable ACK
    
    /* Mock hardware poll: wait for Receive buffer Not Empty (RXNE) */
    I2C1->SR1 |= I2C_SR1_RXNE;
    while (!(I2C1->SR1 & I2C_SR1_RXNE)) {
        /* Wait for RXNE to set */
    }
    return (uint8_t)I2C1->DR;
}

static uint8_t I2C_ReadNack(void) {
    I2C1->CR1 &= ~I2C_CR1_ACK; // Disable ACK (Send NACK)
    I2C1->CR1 |= I2C_CR1_STOP; // Schedule STOP condition
    
    /* Mock hardware poll: wait for RXNE */
    I2C1->SR1 |= I2C_SR1_RXNE;
    while (!(I2C1->SR1 & I2C_SR1_RXNE)) {
        /* Wait for RXNE */
    }
    return (uint8_t)I2C1->DR;
}

static void I2C_Stop(void) {
    I2C1->CR1 |= I2C_CR1_STOP;
}

/**
 * Read temperature from external TI TMP102 I2C sensor.
 * Follows the standard I2C multi-byte read transaction protocol:
 * Start -> Write Addr+W -> Write Reg Pointer -> Start -> Write Addr+R -> Read MSB -> Read LSB -> Stop
 */
float I2C_Read_Temperature(uint8_t sensor_idx) {
    uint8_t dev_addr = 0x48 + sensor_idx; // TMP102 default base address is 0x48
    uint8_t msb = 0, lsb = 0;

    I2C_Start();
    I2C_SendAddress(dev_addr, 0);       /* SLA + Write */
    I2C_Write(0x00);                    /* Pointer Register: 0x00 (Temperature Register) */
    
    I2C_Start();                        /* Repeated Start */
    I2C_SendAddress(dev_addr, 1);       /* SLA + Read */
    
    /* Simulation of data retrieval */
    if (g_bms_data.active_faults & FAULT_OVERTEMP && sensor_idx == 0) {
        // Mock overtemp temperature (65.2 C)
        msb = 0x41; // 65.2 C binary format representation
        lsb = 0x30;
    } else {
        // Normal temperature around 24.5 C
        msb = 0x18; 
        lsb = 0x80;
    }
    
    // Write mock data to DR for retrieval by helper functions
    I2C1->DR = msb;
    msb = I2C_ReadAck();
    
    I2C1->DR = lsb;
    lsb = I2C_ReadNack(); // Also issues Stop

    /* Decode TI TMP102 temperature format (12-bit, resolution 0.0625 C per bit) */
    int16_t temp_raw = (int16_t)((msb << 8) | lsb) >> 4; // 12-bit left-aligned
    
    return (float)temp_raw * 0.0625f;
}

#include "bms.h"
#include <stddef.h>

/* Mock Allocations for USART1 Peripheral */
static USART_TypeDef mock_usart1;
USART_TypeDef *USART1 = &mock_usart1;

/* Ring Buffer Globals */
static char g_ring_buf[RING_BUF_SIZE];
static volatile uint32_t g_head = 0;
static volatile uint32_t g_tail = 0;

/**
 * Initialize USART1 peripheral.
 * Configures word length, stop bits, enables Tx/Rx, and enables RXNE interrupts.
 */
void UART_Init(uint32_t baudrate) {
    /* 1. Set Baud Rate register value (Mock representation) */
    USART1->BRR = 16000000 / baudrate; // Clock frequency / baudrate

    /* 2. Configure USART Control Register 1:
     * - Enable USART (UE)
     * - Enable Transmitter (TE)
     * - Enable Receiver (RE)
     * - Enable RX Data Register Not Empty Interrupt (RXNEIE)
     */
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
}

/* --- Circular Ring Buffer Safe Queue API --- */

/**
 * Writes a character into the circular ring buffer.
 * Thread-safe and ISR-safe queue insertion.
 * Returns 0 on success, -1 on buffer overflow.
 */
int RingBuf_Write(char c) {
    uint32_t next_head = (g_head + 1) & (RING_BUF_SIZE - 1); // Fast power-of-two modulo

    if (next_head == g_tail) {
        // Buffer is full (Overflow condition)
        return -1;
    }

    g_ring_buf[g_head] = c;
    g_head = next_head;
    return 0;
}

/**
 * Reads a character from the circular ring buffer.
 * Returns 0 on success, -1 on buffer empty.
 */
int RingBuf_Read(char *c) {
    if (g_head == g_tail) {
        // Buffer is empty
        return -1;
    }

    *c = g_ring_buf[g_tail];
    g_tail = (g_tail + 1) & (RING_BUF_SIZE - 1);
    return 0;
}

/**
 * Returns the number of unread characters in the ring buffer.
 */
uint32_t RingBuf_GetCount(void) {
    return (g_head - g_tail) & (RING_BUF_SIZE - 1);
}

/**
 * UART Interrupt Service Routine (ISR)
 * Triggered automatically when a byte is received in the hardware DR register.
 */
void USART1_IRQHandler(void) {
    // Check if RXNE flag is set (Read Data Register Not Empty)
    if (USART1->SR & USART_SR_RXNE) {
        // Read byte from DR (clears the interrupt flag automatically in hardware)
        char rx_char = (char)USART1->DR;

        // Push received byte into circular queue
        RingBuf_Write(rx_char);
    }
}

/**
 * Sends a single character over UART using poll-based register status check.
 */
void UART_SendChar(char c) {
    // Write character to Data Register
    USART1->DR = c;

    // Wait until Transmit Data Register is Empty (TXE flag)
    USART1->SR |= USART_SR_TXE; // Mock hardware status
    while (!(USART1->SR & USART_SR_TXE)) {
        /* Poll TXE flag to verify hardware has completed transmission */
    }
}

/**
 * Transmits a null-terminated log string over UART.
 */
void System_Log(const char *msg) {
    while (*msg) {
        UART_SendChar(*msg++);
    }
    UART_SendChar('\r');
    UART_SendChar('\n');
}

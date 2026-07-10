#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Represents a standard CAN frame (Telemetry)
 */
typedef struct {
    uint32_t id;         /**< Standard CAN ID (11-bit) */
    uint8_t data[8];     /**< CAN Payload up to 8 bytes */
    uint8_t dlc;         /**< Data Length Code */
} CAN_Message_t;

/**
 * @brief Initialize the CAN peripheral (mock hardware registers)
 * 
 * Configures the CAN filter banks for incoming messages and 
 * sets up the transmission mailboxes.
 */
void CAN_Init(void);

/**
 * @brief Broadcasts a CAN message using a hardware mailbox
 * 
 * @param msg The CAN message to transmit.
 * @return true if mailbox was available and message sent, false otherwise.
 */
bool CAN_Transmit(const CAN_Message_t *msg);

#endif // CAN_DRIVER_H

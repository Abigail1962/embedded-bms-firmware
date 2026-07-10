/**
 * @file safety_interlock.h
 * @brief MISRA C:2012 Compliant Safety Interlock State Machine
 */

#ifndef SAFETY_INTERLOCK_H
#define SAFETY_INTERLOCK_H

#include <stdint.h>
#include <stdbool.h>

/* Constants representing Safe Operating Area (SOA) limits in millivolts */
#define BMS_CELL_V_MAX_MV (4200U)
#define BMS_CELL_V_MIN_MV (2500U)

/* Number of cells to monitor */
#define BMS_NUM_CELLS     (4U)

/**
 * @brief Enum for High-Voltage Contactor Relay State
 */
typedef enum {
    RELAY_STATE_OPEN = 0U,
    RELAY_STATE_CLOSED = 1U
} RelayState_t;

/**
 * @brief Evaluates the safety limits and determines the required relay state.
 * 
 * @param cell_voltages_mv Array of cell voltages in millivolts.
 * @param num_cells The number of elements in the array (must match BMS_NUM_CELLS).
 * @return RelayState_t Returns RELAY_STATE_OPEN on fault, RELAY_STATE_CLOSED if safe.
 */
RelayState_t SafetyInterlock_Evaluate(const uint16_t cell_voltages_mv[], uint16_t num_cells);

#endif /* SAFETY_INTERLOCK_H */

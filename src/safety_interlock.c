/**
 * @file safety_interlock.c
 * @brief Implementation of host-simulation safety interlock
 */

#include "safety_interlock.h"

static bool System_Is_Latched_Fault = false;

RelayState_t SafetyInterlock_Evaluate(const uint16_t cell_voltages_mv[], uint16_t num_cells)
{
    RelayState_t desired_state = RELAY_STATE_CLOSED;
    uint16_t i;

    /* Defensive check: Check for null pointers defensively */
    if (cell_voltages_mv == (const uint16_t*)0)
    {
        desired_state = RELAY_STATE_OPEN;
    }
    /* Bounds checking */
    else if (num_cells != BMS_NUM_CELLS)
    {
        desired_state = RELAY_STATE_OPEN;
    }
    /* If a fault was previously latched, system must remain open until reset (not implemented in this scope) */
    else if (System_Is_Latched_Fault == true)
    {
        desired_state = RELAY_STATE_OPEN;
    }
    else
    {
        for (i = 0U; i < num_cells; ++i)
        {
            uint16_t voltage = cell_voltages_mv[i];

            if ((voltage > BMS_CELL_V_MAX_MV) || (voltage < BMS_CELL_V_MIN_MV))
            {
                desired_state = RELAY_STATE_OPEN;
                System_Is_Latched_Fault = true; /* Latch the fault */
                break; /* Stop at the first voltage fault */
            }
        }
    }

    return desired_state;
}

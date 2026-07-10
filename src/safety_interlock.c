/**
 * @file safety_interlock.c
 * @brief Implementation of MISRA C compliant safety interlock
 */

#include "safety_interlock.h"

static bool System_Is_Latched_Fault = false;

RelayState_t SafetyInterlock_Evaluate(const uint16_t cell_voltages_mv[], uint16_t num_cells)
{
    RelayState_t desired_state = RELAY_STATE_CLOSED;
    uint16_t i;

    /* MISRA Rule: Check for null pointers defensively */
    if (cell_voltages_mv == (const uint16_t*)0)
    {
        desired_state = RELAY_STATE_OPEN;
    }
    /* MCDC / Bounds checking */
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
                break; /* MISRA allows single break for early exit in fault conditions */
            }
        }
    }

    return desired_state;
}

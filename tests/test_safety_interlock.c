#include <stdio.h>
#include "minunit.h"
#include "../inc/safety_interlock.h"

int tests_run = 0;

static char * test_nominal_voltage() {
    uint16_t voltages[4] = {3500, 3600, 3550, 3500};
    mu_assert("error, nominal voltages should return CLOSED", 
              SafetyInterlock_Evaluate(voltages, 4) == RELAY_STATE_CLOSED);
    return 0;
}

static char * test_over_voltage() {
    uint16_t voltages[4] = {3500, 4300, 3550, 3500}; // Cell 1 is 4.3V (Over BMS_CELL_V_MAX_MV)
    mu_assert("error, over-voltage should return OPEN", 
              SafetyInterlock_Evaluate(voltages, 4) == RELAY_STATE_OPEN);
    return 0;
}

static char * test_under_voltage() {
    uint16_t voltages[4] = {3500, 3600, 2400, 3500}; // Cell 2 is 2.4V (Under BMS_CELL_V_MIN_MV)
    mu_assert("error, under-voltage should return OPEN", 
              SafetyInterlock_Evaluate(voltages, 4) == RELAY_STATE_OPEN);
    return 0;
}

static char * test_mcdc_bounds_checking() {
    uint16_t voltages[4] = {3500, 3600, 3550, 3500};
    mu_assert("error, invalid num_cells should return OPEN", 
              SafetyInterlock_Evaluate(voltages, 3) == RELAY_STATE_OPEN);
    
    mu_assert("error, NULL pointer should return OPEN", 
              SafetyInterlock_Evaluate(0, 4) == RELAY_STATE_OPEN);
    return 0;
}

static char * test_latched_fault() {
    uint16_t voltages_safe[4] = {3500, 3600, 3550, 3500};
    // Because over_voltage already latched the fault in the static variable, this should STILL return OPEN
    mu_assert("error, latched fault did not persist", 
              SafetyInterlock_Evaluate(voltages_safe, 4) == RELAY_STATE_OPEN);
    return 0;
}

static char * all_tests() {
    mu_run_test(test_nominal_voltage);
    mu_run_test(test_mcdc_bounds_checking);
    // Order matters because test_over_voltage latches the fault!
    mu_run_test(test_over_voltage); 
    mu_run_test(test_under_voltage);
    mu_run_test(test_latched_fault);
    return 0;
}

int main(void) {
    printf("=========================================\n");
    printf("  RUNNING MCDC SAFETY INTERLOCK TESTS    \n");
    printf("=========================================\n");
    
    char *result = all_tests();
    if (result != 0) {
        printf("❌ TEST FAILED: %s\n", result);
    }
    else {
        printf("✅ ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    
    return result != 0;
}

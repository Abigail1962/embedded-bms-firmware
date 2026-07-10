#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>

/**
 * @brief PID Controller State and Gains
 */
typedef struct {
    float kp;             /**< Proportional gain */
    float ki;             /**< Integral gain */
    float kd;             /**< Derivative gain */
    
    float setpoint;       /**< Target temperature */
    
    float integral_sum;   /**< Accumulated integral error */
    float integral_max;   /**< Anti-windup max limit */
    float integral_min;   /**< Anti-windup min limit */
    
    float prev_error;     /**< Previous error for derivative */
    
    float output_max;     /**< Max output (e.g., max DC/DC current) */
    float output_min;     /**< Min output */
} PID_Controller_t;

/**
 * @brief Initialize the PID controller with gains and limits
 */
void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, float target, float out_min, float out_max);

/**
 * @brief Computes the next control output (DC/DC current adjustment)
 * 
 * @param pid Pointer to the PID controller instance
 * @param current_value The current thermal feedback reading
 * @param dt Delta time in seconds since last update
 * @return Control output value
 */
float PID_Compute(PID_Controller_t *pid, float current_value, float dt);

#endif // PID_CONTROLLER_H

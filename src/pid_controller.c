#include "pid_controller.h"

void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, float target, float out_min, float out_max) {
    if (!pid) return;
    
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    
    pid->setpoint = target;
    
    pid->integral_sum = 0.0f;
    pid->integral_max = out_max / ki; // Simple anti-windup heuristic
    pid->integral_min = out_min / ki;
    
    pid->prev_error = 0.0f;
    
    pid->output_max = out_max;
    pid->output_min = out_min;
}

float PID_Compute(PID_Controller_t *pid, float current_value, float dt) {
    if (!pid || dt <= 0.0f) return 0.0f;
    
    float error = pid->setpoint - current_value;
    
    // Proportional
    float p_out = pid->kp * error;
    
    // Integral with anti-windup
    pid->integral_sum += error * dt;
    if (pid->integral_sum > pid->integral_max) pid->integral_sum = pid->integral_max;
    else if (pid->integral_sum < pid->integral_min) pid->integral_sum = pid->integral_min;
    
    float i_out = pid->ki * pid->integral_sum;
    
    // Derivative (on error to prevent derivative kick)
    float derivative = (error - pid->prev_error) / dt;
    float d_out = pid->kd * derivative;
    
    // Total output
    float output = p_out + i_out + d_out;
    
    // Saturate output
    if (output > pid->output_max) output = pid->output_max;
    else if (output < pid->output_min) output = pid->output_min;
    
    pid->prev_error = error;
    
    return output;
}

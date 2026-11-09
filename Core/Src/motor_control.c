/*
 * motor_control.c
 *
 *  Created on: Sep 22, 2026
 *      Author: hamer
 */

#include "motor_control.h"

volatile uint8_t motor_stall_fault = 0;

static uint8_t stall_timer_active = 0;
static uint32_t stall_start_ms = 0;

static float integral_percent = 0.0f;
static float requested_percent = 0.0f;
static float output_percent = 0.0f;

static uint32_t last_control_ms = 0;
static uint32_t last_ramp_ms = 0;

void MotorControl_Init(void)
{
	motor_stall_fault = 0;
	stall_timer_active = 0;
	stall_start_ms = 0;

    integral_percent = 0.0f;
    requested_percent = 0.0f;
    output_percent = 0.0f;
    last_control_ms = 0;
    last_ramp_ms = 0;
}

uint32_t MotorControl_Update(float target_rpm,
                            float measured_rpm,
                            uint32_t now_ms)
{
    // Use speed magnitude for our current single direction.
    float speed = measured_rpm;
    if (speed < 0.0f)
        speed = -speed;

    // Zero command stops immediately and clears the controller.
    if (target_rpm <= 0.0f)
    {
        integral_percent = 0.0f;
        requested_percent = 0.0f;
        output_percent = 0.0f;
        last_control_ms = now_ms;
        last_ramp_ms = now_ms;
        motor_stall_fault = 0;
        stall_timer_active = 0;
        stall_start_ms = now_ms;
        return 0;
    }

    // A fault stays latched until the knob returns to zero.
    if (motor_stall_fault)
    {
        integral_percent = 0.0f;
        requested_percent = 0.0f;
        output_percent = 0.0f;
        last_control_ms = now_ms;
        last_ramp_ms = now_ms;
        return 0;
    }

    // Detect sustained lack of movement while applying PWM.
    if ((output_percent >= 10.0f) && (speed < 1.0f))
    {
        if (!stall_timer_active)
        {
            stall_timer_active = 1;
            stall_start_ms = now_ms;
        }
        else if ((now_ms - stall_start_ms) >= 1000U)
        {
            motor_stall_fault = 1;

            integral_percent = 0.0f;
            requested_percent = 0.0f;
            output_percent = 0.0f;

            return 0;
        }
    }
    else
    {
        stall_timer_active = 0;
    }

    uint32_t elapsed_ms = now_ms - last_control_ms;

    if (elapsed_ms >= 100U)
    {
        last_control_ms = now_ms;

        // Limit the integration interval after a debugger pause.
        float dt = (float)elapsed_ms / 1000.0f;
        if (dt > 0.2f)
            dt = 0.2f;

        float error = target_rpm - speed;

        // Initial tuning values; we will tune these on the bench.
        const float kp = 0.5f;
        const float ki = 0.6f; // Previously 0.3f

        // Pause integration when the output cannot follow.
        if (((error > 0.0f) &&
             (requested_percent <= output_percent)) ||
            ((error < 0.0f) &&
             (requested_percent >= output_percent)))
        {
            integral_percent += ki * error * dt;
        }

        if (integral_percent > 100.0f)
            integral_percent = 100.0f;
        if (integral_percent < 0.0f)
            integral_percent = 0.0f;

        requested_percent = kp * error + integral_percent;

        if (requested_percent > 100.0f)
            requested_percent = 100.0f;
        if (requested_percent < 0.0f)
            requested_percent = 0.0f;
    }

    // Preserve the gradual PWM ramp.
    if ((now_ms - last_ramp_ms) >= 20U)
    {
        last_ramp_ms = now_ms;

        if (output_percent < requested_percent)
        {
            output_percent += 1.0f;
            if (output_percent > requested_percent)
                output_percent = requested_percent;
        }
        else if (output_percent > requested_percent)
        {
            output_percent -= 1.0f;
            if (output_percent < requested_percent)
                output_percent = requested_percent;
        }
    }

    return (uint32_t)(output_percent * 8500.0f / 100.0f);
}

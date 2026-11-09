/*
 * motor_control.h
 *
 *  Created on: Sep 22, 2026
 *      Author: hamer
 */

#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#include <stdint.h>

void MotorControl_Init(void);
uint32_t MotorControl_Update(float target_rpm,
        float measured_rpm,
        uint32_t now_ms);

#endif /* MOTOR_CONTROL_H_ */

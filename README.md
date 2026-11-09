# STM32 DC Motor Control

A closed-loop brushed DC gearmotor controller in C for the STM32 NUCLEO-G474RE. This bench project uses a Pololu 4755 12 V gearmotor with a quadrature encoder and a Pololu G2 18v17 motor driver as the hardware platform for roller-shade development.

## How it works

- ADC1 reads a potentiometer. A low-pass filter and a deadband produce a speed command from 0 to approximately 30 RPM.
- TIM4 counts quadrature edges in hardware. The main loop estimates gearbox output speed from signed count changes approximately every 100 ms, using about 6,533 counts per output-shaft revolution.
- A proportional-integral (PI) controller updates the duty-cycle request approximately every 100 ms. A separate ramp limits normal duty-cycle changes to one percentage point per 20 ms. TIM3 channel 1 generates 20 kHz PWM in a fixed direction.
- When the applied PWM duty cycle is at least 10% and measured speed stays below 1 RPM for one second, the controller latches a stall fault and sets PWM to zero. Returning the potentiometer to zero clears the fault.

| Firmware signal | MCU pin | Peripheral |
| --- | --- | --- |
| Potentiometer input | PA0 | ADC1_IN1 |
| Motor PWM | PA6 | TIM3_CH1 |
| Motor direction | PA8 | GPIO output |
| Encoder A | PB6 | TIM4_CH1 |
| Encoder B | PB7 | TIM4_CH2 |

## Build

1. In STM32CubeIDE, choose **File > Import > General > Existing Projects into Workspace** and select this repository. The IDE project is named `G474_PWM_Motor`.
2. Select the **Debug** build configuration and build the project. Connect the board via its ST-LINK USB port to program the MCU.
3. See [main.c](Core/Src/main.c) for ADC, encoder, and PWM setup and [motor_control.c](Core/Src/motor_control.c) for the PI loop, PWM ramp, and stall detection. The CubeMX configuration is `G474_Blink.ioc`.

Motor-current monitoring and shade travel control are future work. The stall check uses encoder speed and PWM duty cycle.

## Related project

[STM32 Traffic-Light Controller](https://github.com/Jeffhamer/STM32-Industrial-Controls)

## Hardware references

- [Pololu 4755 gearmotor](https://www.pololu.com/product/4755)
- [Pololu G2 18v17 driver](https://www.pololu.com/product/2991)

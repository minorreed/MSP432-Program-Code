MSP432 Menu Program: RGB Control, Button Input, and Temperature Readout

Overview
This project is a bare-metal C program for the TI MSP432 that provides a UART-driven text menu. From a serial terminal, the user can select between RGB LED blinking control, reading two onboard pushbuttons, and reading the internal temperature sensor using the ADC14 with factory calibration constants.

What it does
1. RGB Control
User selects an RGB combination from 1 to 7, a toggle time in seconds, and the number of blinks. The program blinks the LED color on P2 using Timer32 delays.

2. Digital Input
Reads two buttons on P1 and reports whether Button 1, Button 2, both, or none are pressed.

3. Temperature Reading
User selects 1 to 5 readings. The program uses ADC14 to read the internal temperature sensor, converts the result to Celsius using TLV calibration constants, then converts to Fahrenheit. Each reading is printed over UART with a one-second delay between readings.

Hardware and peripherals used
Microcontroller: TI MSP432
UART: EUSCI_A0 for serial menu and printing
GPIO:
P2 outputs for RGB LED control
P1 inputs for two pushbuttons with pull-up resistors enabled
ADC: ADC14 reading internal temperature sensor on channel 22
Reference: REF_A internal reference configured for ADC temperature measurement
Timers:
TIMER32_1 used for RGB blinking delays in seconds
SysTick used for one-second delays between temperature readings

Pin mapping
RGB LED outputs on Port 2
P2.0 red component
P2.1 green component
P2.2 blue component

Buttons on Port 1
P1.1 button 1 input
P1.4 button 2 input

UART pins
EUSCI_A0 mapped to P1.2 and P1.3

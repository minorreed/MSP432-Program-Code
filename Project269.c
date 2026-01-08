#include "msp.h"
#include <stdio.h>
#include <stdlib.h>

// Function Prototypes
void UARTInit(void);
void TX(char text[]);
int RX(void);
void Timer32(int n);
void ADCInit(void);
float tempRead(void);
void SystickDelay(int milliseconds);

//SysTick Delay Function
void SysTick_Delay(uint32_t delay) 
{
    SysTick -> LOAD = 3000000 - 1; //1 second delay at 3MHz
    SysTick -> VAL = 0; 
    SysTick -> CTRL = 5; 

    while((SysTick -> CTRL & 0x10000) == 0);
    SysTick -> CTRL = 0; // Stop SysTick
}

// Timer32 Functions
void Timer32(int n) {
    // Set up Timer32
    TIMER32_1->LOAD = 3000000 * n - 1; // Set the timer to n seconds
    TIMER32_1->CONTROL |= 0xC2; // Set up 32-bit, periodic, enabled, with interrupt
    while ((TIMER32_1->RIS & 1) != 1); // Wait until timer flag is set
    TIMER32_1->INTCLR = 0; // Clear the flag
}

// ADC Initilization
void ADCInit(void) {
    //Ref_A settings
    REF_A->CTL0 &= ~0x8; //enable temp sensor
    REF_A->CTL0 |= 0x30; //set ref voltage
    REF_A->CTL0 &= ~0x01; //enable ref voltage
    //do ADC stuff
    ADC14->CTL0 |= 0x10; //turn on the ADC
    ADC14->CTL0 &= ~0x02; //disable ADC
    ADC14->CTL0 |=0x4180700; //no prescale, mclk, 192 SHT
    ADC14->CTL1 &= ~0x1F0000; //configure memory register 0
    ADC14->CTL1 |= 0x800000; //route temp sense
    ADC14->MCTL[0] |= 0x100; //vref pos int buffer
    ADC14->MCTL[0] |= 22; //channel 22
    ADC14->CTL0 |=0x02; //enable adc
}

// Temperature Reading
float tempRead(void) {
    float temp; // temperature variable
    uint32_t cal30 = TLV->ADC14_REF2P5V_TS30C; // calibration constant
    uint32_t cal85 = TLV->ADC14_REF2P5V_TS85C; // calibration constant
    float calDiff = cal85 - cal30; // calibration difference
    ADC14->CTL0 |= 0x01; // start conversion
    while ((ADC14->IFGR0) == 0) {
        // wait for conversion
    }
    temp = ADC14->MEM[0]; // assign ADC value
    temp = (temp - cal30) * 55; // math for temperature per manual
    temp = (temp / calDiff) + 30; // math for temperature per manual
    return temp; // return temperature in degrees C
}

int main(void) {
    // Set up of pins
    // LED lights
    P2->SEL1 &= ~7;
    P2->SEL0 &= ~7;
    // Inputs buttons
    P1->SEL1 &= ~18;
    P1->SEL0 &= ~18;
    // Outputs
    P2->DIR |= 7;
    P2->OUT &= ~7;
    // Buttons
    P1->DIR &= ~18;
    P1->REN |= 18;
    P1->OUT |= 18;

    ADCInit();
    UARTInit();

    while (1) {
        // Menu selection
        TX("\n\r MSP432 Menu\n");
        TX("\n\r 1. RGB Control ");
        TX("\n\r 2. Digital Input ");
        TX("\n\r 3. Temperature Reading \n\r");

        int menuchoice = RX();

        switch (menuchoice) {
            case 1:
                TX("\r Enter Combination of RGB (1 - 7):");
                int combRGB = RX();
                if (combRGB > 7)
                    combRGB = 7;

                TX("\r\n Enter Toggle Time:");
                int toggle = RX();

                TX("\r\n Enter Number of Blinks:");
                int blinks = RX();

                TX("\r\n Blinking LED...");

                for (int i = 0; i < blinks; i++) {
                    P2->OUT = (combRGB); // This outputs the color specified
                    Timer32(toggle); // toggle time in seconds
                    P2->OUT = 0; // and this turns off for the same time and restarts
                    Timer32(toggle);
                }

                TX("\r\n Done\n ");
                break;

            case 2:
                if ((P1->IN & 18) == 0) {
                    TX("\n\r Both buttons pressed.\n ");
                } else if ((P1->IN & 2) == 0) {
                    TX("\n\r Button 1 pressed.\n ");
                } else if ((P1->IN & 16) == 0) {
                    TX("\n\r Button 2 pressed.\n ");
                } else {
                    TX("\n\r No button pressed.\n ");
                }
                break;

            case 3:
                TX("\n\r Enter Number of Temperature Reading (1-5): \n\r");
                int number = RX(); // This indicates the number of readings
                if (number > 5)
                    number = 5;
                for (int i = 0; i < number; i++) {
                    float tempC = tempRead();
                    float tempF = (tempC * (9.0 / 5.0) + 32);
                    char reading[50];
                    sprintf(reading, "\n\r Reading %d: %.2f C & %.2f F \n\r", i + 1, tempC, tempF);
                    TX(reading);
                    SysTick_Delay(1000); // Delay for 1 second
                }
                break;
        }
    }
    return 0;
}

// UART initialization
void UARTInit(void) {
    EUSCI_A0->CTLW0 |= 1;
    EUSCI_A0->MCTLW = 0;
    EUSCI_A0->CTLW0 |= 0x80;
    EUSCI_A0->BRW = 0x34;
    EUSCI_A0->CTLW0 &= ~0x01;
    P1->SEL0 |= 0x0C;
    P1->SEL1 &= ~0x0C;
}

// UART transmission
void TX(char text[]) {
    int i = 0;
    while (text[i] != '\0') {
        EUSCI_A0->TXBUF = text[i];
        while ((EUSCI_A0->IFG & 0x02) == 0) { }
        i++;
    }
}

// UART reception
int RX(void) {
    int i = 0;
    char command[2];
    char x;
    while (1) {
        if ((EUSCI_A0->IFG & 0x01) != 0) // data in RX buffer
        {
            command[i] = EUSCI_A0->RXBUF;
            EUSCI_A0->TXBUF = command[i]; // echo
            while ((EUSCI_A0->IFG & 0x02) == 0); // wait
            if (command[i] == '\r') {
                command[i] = '\0';
                break;
            } else {
                i++;
            }
        }
    }
    x = atoi(command);
    TX("\n\r");
    return x;
}
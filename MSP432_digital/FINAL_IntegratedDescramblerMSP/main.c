/*
 * main.c
 *
 *  Created on: 18 Jan 2022
 *      Author: Iason Chaimalas
 *     Purpose: To integrate fully the ADC, Bandstop Filter and Sinewave Descrambling subsystems into final code product.
 */

/* Header files: */

/* DriverLib Includes */
#include "driverlib.h"
    // includes wdt_a.h watchdog timer for the WDT function,
    // includes rom_map.h to use MAP prefix (MAP = macro that decides to use ROM or Flash version of each func)
    // includes bit definition for the bits BIT0, BIT1, etc of the PSEL register

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>

/* Maths Includes */
#include <math.h>  // This library is required to use the "sin" & "cos" functions

// 2nd Order Type II Chebyshev Filter - Bandstop Infinite Impulse Response digital filter
#include "bsdef.h"

// Sinewave definitions of parameters
#define PTS 50
#define PI 3.14159265359
volatile float sin_value[PTS];

int main(void)
{
    /* Halting the Watchdog Timer */
    MAP_WDT_A_holdTimer();  // very common command to prevent MSP from stopping automatically.
        // if timer wdt_a enabled -> potential infinite boot loops in initialisation, unknown initial position of time register etc

    // SINEWAVE:

    int t;
    for( t = 0; t < PTS; t++)
    {
        sin_value[t] = 0.5 * ( ( sin(2*PI*2*(t/(float)PTS))*cos(2*PI*5*(t/(float)PTS)) +
                sin(2*PI*5*(t/(float)PTS))*cos(2*PI*2*(t/(float)PTS)) ) +1);
        // this is essentially sin_value[t] = 255*(sin(2*PI*7000*(t/(float)50000))+1) -- 7kHz sinusoid at 50kHz sample rate
        // but 50000/7000 not perfectly divisible, so using the trig rule:
        //     sin(a+b) = sin(a)cos(b) + sin(b)cos(a)
        // this is broken into 2kHz and 5kHz sine & cosine waves, since 2k and 5k divide into 50k
        // add 255/2 to sinewave & scale it by 255/2 so it is scaled at 0-5V for 8bit output (2^8 = 256) w mean at 2.5V
    }

    // PIN DEFINITIONS:

    /* P6.0 set as output. This is used for timing the duration of the systick_isr. P6.0 is set high on entering the systick_isr and
     * off on exiting.  An oscilloscope can be used to monitor how much time the interrupt requires to complete all the operations.
     * It can also measure the frequency at which the isr is called. This should be 50 kHz*/
        //P6DIR |= BIT0;
        MAP_GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0);
        // initialise P6.0 as output by writing a 1 bit to the direction register of BIT0 in P6 - ie. P6.0.

    /* P2.0 to P2.7 configured as outputs for the filtered signal
     * Port P2 and selected pins 0-7 w mask value logical OR = | of the pins */
        MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);

    /* Configure P5.0 for ADC14 input */
        //P5SEL1 |= BIT0;
        //P5SEL0 |= BIT0;

    /* Configure P2 for 8-bit output after ADC input, filtering and sinewave multiplication */
        //P2DIR = 0xFF; // 0xFF = 0b11111111 so writing output to direction register of all pins in Port 2
        //P2OUT = 0x00; // 0x00 = 0b00000000 so all outputs in Port 2 are 0 at the moment - P2 will be changing once ADC inputs start coming into the MSP432

    /* Configure P5.6 to its analog function to output VREF.  VREF is set to 1.2V below and so the ADC input voltage should be between 0 V and 1.2 V */
        P5SEL0 |= BIT6 | BIT7;
        P5SEL1 |= BIT6 | BIT7;
            // Set pins P5.6 and P5.7 as external reference voltage. See section 10.2.6 in slau356a.pdf, tables 4.1 and 6.45 of msp432p401r.pdf, slau596.pdf.
            // sets the bits 0/1 of the Function Select Register (SEL) of the pins

        REFCTL0 |= REFON;               // Turn on reference module. Enable reference in static mode.
        REFCTL0 |= REFOUT;              // Output reference voltage to a pin. Reference output available externally.
            // reference control register 0 (REFCTL0) altered here to output reference voltage externally. Section 19.3.1 in slau365a.pdf, Section 21.3.1 in slau356h.pdf.

     /* Output VREF = 1.2V */
        REFCTL0 &= ~(REFVSEL_3);        // Clear existing VREF voltage level setting. Table 19-2 in slau365a.pdf.
        REFCTL0 |= REFVSEL_0;           // Set VREF = 1.2V by setting REFCTL0 bits 4 & 5 to 00 corresponding to 1.2V. Table 19-2 in slau365a.pdf.
        while (REFCTL0 & REFGENBUSY);   // Wait until the reference generation is settled.  Table 19-2 in slau365a.pdf.


    /* Configuring pins for high frequency crystal (HFXT) crystal for 48 MHz clock */
        MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_PJ,
            GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION); //  Section 10.4.2.13 in MSP432_DriverLib_Users_Guide.  In Figure 4.1 in msp432p401r.pdf, you can see that the HFXT is conneccted to pins 2 and 3 of Port J.


    /* Set P4.3 to output the internal 48 MHz master clock as (MCLK) its primary function so you can check with a scope on this pin to make sure the clock is indeed 48 MHz */
        MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);


    /* There is a 32 kHz low frequency crystal (LFXT) and a 48 MHz high frequency crystal (HFXT) on the Launchpad. This command tells the MSP432 what frequencies these two external crystals are*/
    /* Section 6.6.2.4 in MSP432_DriverLib_Users_Guide*/
    CS_setExternalClockSourceFrequency(32000,48000000);


    /* Starting HFXT in non-bypass mode without a timeout.
     * Before increasing MCLK to a higher speed, it is necessary for software to ensure that the CPU voltage or core voltage (VCORE level) is
     * sufficiently high for the chosen frequency.  This is done through the Power Control Manager (PCM).  See Chapter 7 in slau356a.pdf.
     * To run the CPU at the maximum frequency of 48 MHz, the core voltage must be set to VCORE1.  See section 7.4.1 in slau356a.pdf.
     * It is much easier to use the library commands below to set the core voltage to VCORE1.  */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);  // Section 14.7.2.16 in MSP432_DriverLib_Users_Guide.
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false);  // Initialise the HFXT.  "false" means we are not using the bypass mode but are using the crystal.  Section 6.6.2.27 in MSP432_DriverLib_Users_Guide.


    /* Initializing the master clock (MCLK) to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);  // The divider is set to 1 and so the MCLK is the same as the HFXT at 48 MHz.  Section 6.6.2.18 in in MSP432_DriverLib_Users_Guide.


    /* Configuring the timer SysTick to trigger at 50 kHz which will be the sampling frequency for sampling and processing the audio signal.
     * See Chapter 22 in MSP432_DriverLib_Users_Guide.
     * The other two timers "Timer32" and "Timer_A" can also be used instead */
    MAP_SysTick_enableModule();  //Enable the timer SysTick
    MAP_SysTick_setPeriod(960);  // SysTick will count 960 MCLK cycle before generating and calling an interrupt.  Therefore the sampling frequency is 48 MHz / 960 = 50 kHz
    MAP_Interrupt_enableSleepOnIsrExit();  //Enables the processor to sleep when exiting an ISR. For low power operation, this is ideal as power cycles are not wasted with the processing required for waking up from an ISR and going back to sleep.
    MAP_SysTick_enableInterrupt();  // Enable timer interrupt.

    /* Configure ADC14: */
        MAP_ADC14_enableModule(); /* Initialising ADC (MCLK/1/1) */
        MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);

        MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
            // Set P5.0 (A5) as the ADC input pin

        ADC14_setResolution(ADC_10BIT); // make ADC14 into 10bit ADC, i.e. analog input to P5.0 converted to 10-bit binary [0, 2^10 - 1] value

        MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
            // continuously samples & stores ADC result into MEM0 register (can be read later)
        MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A5, false);
            // configures the ADC MEM0 memory location at internal reference voltage 1.2,1.45,2.5 (VSS) to GND for ADC input stored at P5.0 = A5.
            // VSS is 1.2V here due to the setting of REFVSEL to 1.2V
            // differentialMode = "false" means ADC output is the ADC of A5. If "true", ADC output is ADC of difference between A4 and A5 adjacent pair

        /* Enabling sample timer */
        MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

        /* Enabling MASTER interrupts */
        MAP_Interrupt_enableMaster();

    while (1)
    {
        //MAP_PCM_gotoLPM0();
    }

}

int j = 0;

void SysTick_Handler(void)
{
    P6OUT |= BIT0; // set P6.0 high on entering this interrupt service routine (isr). Include your codes below

    static long ADC_value = 0; // initially in case ADC is not read properly

    // ADC
    ADC_value = ADC14_getResult(ADC_MEM0); // read ADC output at MEM0 register
    ADC_value = ADC_value / 4; // div by 4 as ADC14 here is configured for 10-bit and this program uses 8-bit as P2OUT is 8-bit wide

    //ADC_value -= 512.0f; // to get it from 0-1023 to -512 to 511 +-

    // BS
    float bs_output = bandstop(bandstop_num, bandstop_denom, bandstop_gain, ADC_value, CircularBuffer);

    CircularBuffer[0] = CircularBuffer[1];
    CircularBuffer[1] = ADC_value;
    CircularBuffer[3] = CircularBuffer[2];
    CircularBuffer[2] = bs_output;

    // SINE
    //static

    float final_out = bs_output * sin_value[j];
    j++;
    if (j == PTS) {j = 0;} // increment and loop back to start of sinewave cycle

    //final_out += 512.0f; // add back the 512 out of 1023 to get output back to positive range for output on pin

    P2OUT = (final_out);

    /* Enabling/Toggling Conversion */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();

    P6OUT &= ~BIT0; // set P6.0 low on exiting this interrupt service routine (isr). Include yours codes above
}

/* ADC Interrupt Handler. This handler is called whenever there is a conversion
* that is finished for ADC_MEM0.
*/
void ADC14_IRQHandler(void)
{

    uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);
    /* This simply clears the ADC14 interrupt and exits this adc_isr. */

}

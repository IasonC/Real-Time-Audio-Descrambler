These are live demos of the subsystem testing and integration testing of the full main.c program combining all subsystems. Individual subsystem testing is necessary to identify bugs and errros at each stage of the final product.

The ramp test is the first test, which verifies that the specific MSP432 board can output values correctly, and that the analog DAC at the output Port 2 correctly converts digital outputs to the analog signal. The ramp can be produced from the MSP using the same standard code as the ADCtest.c file but by omitting the ADC configuration declarations in main and the ADC14_IRQHandler, and by declaring the SysTick_Handler as follows:
```
void SysTick_Handler(void)
{
    P6OUT |= BIT0; // set P6 PIN0 High

    static int j = 0; // declare counter

    P2OUT = j;
    // Port 2 pins P2.0 to P2.7 are assigned the values of j (from 0 to 255)
    // 0-255 means 8 bit, since Port 2 is 8 bits (pins) wide

    j++;
        if (j == 256)
        {j = 0;}
        // loop back to 0 to create a sawtooth output - i.e. ramp

    // on each SysTick ISR (called every 20 μs) the function is incremented by 1/255.
    // For a full iteration (full ramp), 255 SysTick cycles are needed = 5100 μs -> frequency of ramp output = 196 Hz

    P6OUT &= ~BIT0; // set P6 PIN0 Low - for monitoring how long SysTick needs to complete the routine (must be <20 μs)


}
```
The demo for the ramp shows an exact frequency of 196 Hz so the MSP and the DAC work correctly.

The ADC test shows that the ADC correctly converts an input voltage to digital form, and that the ADC is correctly interfaced with SysTick to reproduce the same signal at the DAC output. There is minimal noise from the analog components and the imperfect R-2R (1 kΩ and 2.2 kΩ) DAC.

The Bandstop test shows that the signal is strongly attenuated at 7.9 kHz to 8.2 kHz so the bandstop filter effectively filters the 8 kHz tone.

The Sinewave test shows that a smooth 7 kHz sinewave sampled at 50 kHz is effectively produced by the MSP. Thus, this means that in the sinewave-multiplication stage the bandstop-filtered scrambled signal is multiplied by a *smooth* sine. If the sine is not smooth, this introduces noise.

Finally, the integrated analog & digital descrambler is used to show successful real-time descrambling.

<details>
  <summary>Secret message</summary>
  
  "You're only supposed to blow the bloody doors off!" - from The Italian Job
  
</details>

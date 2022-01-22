These are live demos of the subsystem testing and integration testing of the full main.c program combining all subsystems. Individual subsystem testing is necessary to identify bugs and errros at each stage of the final product.

The ramp test is the first test, which verifies that the specific MSP432 board can output values correctly, and that the analog DAC at the output Port 2 correctly converts digital outputs to the analog signal. The ramp can be produced from the MSP using the same standard code as the ADCtest.c file but by omitting the ADC configuration declarations and the IRQ handler, and by declaring the SysTick_Handler as follows:
'''
void SysTick_Handler(void)
{
    P6OUT |= BIT0; // set P6 PIN0 High

    static int j = 0;


    P2OUT = j;  // This is where the 8 output pins of port 2 are assigned the values of j (from 0 to 255)

    j++;
        if (j == 256)
        {j = 0;}


    P6OUT &= ~BIT0; // set P6 PIN0 Low


}
'''

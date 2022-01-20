# Real-Time Audio Descrambler

![20220119_021817](https://user-images.githubusercontent.com/73920832/150238612-9ad2512e-82f9-4f44-951c-f0ceea7d534d.jpg | width = 100)

## Overview
This design uses a Texas Instruments MSP432P401R development board interfaced with a potentiometer biasing circuit, a Digital-to-Analog converter (DAC), an Anti-Aliasing Filter, a voltage amplifier and the speaker. The MSP432 implements an Analog-to-Digital (ADC), Bandstop filter and sinewave modulation.

**This project is able to receive a scrambled audio message played from a computer through an audio jack and play the unscrambled, original audio message from a speaker in *real time*** (i.e. the timing of the audio output matches perfectly the corresponding point in the scrambled input).

## Scenario
A gang of robbers is scrambling their audio communications to communicate securely over telephone lines regarding their upcoming heist. However, the Police apprehend the gang-member who created the audio descramblers that the robbers are using. After searching his home, the Police find an audio descrambler and a recorded scrambled message. They must now descramble this message in time, in order to stop the heist!

## Analysis of Descrambling Process
The "audio descrambler" discovered in the arrested robber's house is - for the purposes of this scenario - a MATLAB file which records the user's voice for 5 seconds and produces a scrambled audio file. By recording my voice and observing both the original audio and the outputted scrambled audio, in the time and frequency domains, it is possible to see that in this particular scenario, the descrambling is achieved by adding an 8 kHz tone to the original audio and reflecting the audio spectrum about 3.5 kHz. Hence, this descrambling project filters out the 8 kHz tone and re-flips the scrambled audio about 3.5 kHz to reverse the scrambling.

The connected circuit receives as input the scrambled audio signal played from a computer through a TRS audio jack. Then, the potentiometer scales this zero-mean voltage into a fully positive signal in the range 0V-1.2V, suitable for input into the MSP432 at Pin 5.0. The MSP ADC then converts this voltage into a 10-bit digital signal (0-1023). A digital IIR Chebyshev Type II filter is used as the bandstop filter that attenuates the scrambled audio at a maximum of approximately -47 dB at 8 kHz. The Passband has no ripple as this is a Type II Chebyshev filter, which avoids the introduction of noise to the signal. Finally, the filtered signal is multiplied inside the MSP432 by a sinewave at 7 kHz, which has the effect of creating two half-power sidebands about 7 kHz: one non-reflected sideband above 7 kHz and one reflected sideband from DC to 7 kHz. As the scrambled audio is now reflected twice about 3.5 kHz, it is fully unscrambled. This unscrambled audio is outputted from the MSP, converted to an analog voltage with an R-2R DAC (1 kΩ and 2.2 kΩ), and smoothed out by the Anti-Aliasing Filter (this is where the upper half-power sideband is filtered out). The final signal is amplified with respect to ground by an INA128 instrumentation amplifier and played by a speaker.

## Design Considerations
First, the ADC is using the built-in ADC14 module configured automatically for 14 bits. However, it is set in my code to represent voltage inputs only using 10 bits. This is to reduce complexity and computation time while maintaining acceptable resolution. The emphasis on computation time is because a sampling frequency of 50 kHz is being used so the SysTick Interrupt Service Routine (ISR) that runs the ADC samples only has 20 μs to complete the ADC acquisition, bandstop filtering, and sinewave multiplication. If the SysTick does not complete its operation in 20 μs, the next SysTick is called so the output will be incorrect.

Also, 2 kHz and 5 kHz sinuoids are used to generate the 7 kHz sine sampled at the sampling frequency of 50 kHz. This is because they are divisible by 50 kHz while 7 kHz is not.

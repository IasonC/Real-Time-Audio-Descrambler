/*
 * bsdef.h
 *
 *  Created on: 15 Jan 2022
 *      Author: Iason Chaimalas
 *     Purpose: Define Bandstop Filter Parameters & Functions here for clear syntax
 */

#ifndef BSDEF_H_
#define BSDEF_H_

// PARAMETERS for 2nd Order Type 2 Chebyshev Bandstop IIR Filter:

static float bandstop_gain = 0.96179879474;
static float bandstop_num[3] = {1, -1.071654, 1};
static float bandstop_denom[3] = {1, -1.0307159, 0.9235976};

float CircularBuffer[4] = {0, 0, 0, 0}; // {x(n-2), x(n-1), y(n-1), y(n-2)}

// PERFORM BANDSTOP FILTER FUNC:
float bandstop(float num[3], float denom[3], float g, float adc_in /* x */, float circbuff[4])
{
    float bsout = -denom[1]*circbuff[2] - denom[2]*circbuff[3] + g*(adc_in + num[1]*circbuff[1] + circbuff[0]);
    // Chebyshev Type II 2nd Order filter w bandstop at 7990Hz-8010Hz

    circbuff[0] = circbuff[1];
    circbuff[1] = adc_in;
    circbuff[3] = circbuff[2];
    circbuff[2] = bsout;

    return bsout;
}

#endif /* BSDEF_H_ */

## Contents
This project is broken into subsystems. The digital subsystems implemented in the MSP432 are the *ADC*, *bandstop filter*, and *sinewave multiplication*.

The folders here contains the code for:
- implementing and testing the three MSP432 subsystems
- integrating into the full main.c program that is used in the full descrambler
- CCS-generated files used in running the code on the MSP432
- MSP432 driverlib header file

## DriverLib
Here I have included the driverlib header file with all MSP432 definitions. This folder is downloaded from Texas Instruments: https://www.ti.com/tool/MSPDRIVERLIB (Accessed 20 Jan 2022)

The folder with the header files from TI comes with a "driverlib.h" file at \msp432_driverlib_3_21_00_05.zip\msp432_driverlib_3_21_00_05\driverlib\MSP432P4xx\driverlib.c.

This file, however, only includes .h files in the full header file, so I edited the file from the TI download to also include all .c files included in the download. This is because the .h header files define void functions, whereas the .c functions include the actual function declarations used in my MSP code. Hence, the folder in this repo includes the edited driverlib.h inside MSP432P4xx.
To run the code in CCS with the
```
#include "driverlib.h"
```
command, the folder in this repo including the function definitions must be included as a path in Project -> Properties -> Include Options.

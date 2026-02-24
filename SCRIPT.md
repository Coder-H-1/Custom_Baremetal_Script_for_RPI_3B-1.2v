# This is the format of the script that is to be provided to get output through GPIO pins and HDMI.

    I only implemented OUTPUT only, It is very stressing to work with very low level stuff T-T 

####    `pin*number` represents select PIN number, where `number` is a value between (1 to 40)

####    `+` represents PIN `ON` 

####    `-` represents PIN `OFF`

####    `(number)` represents delay time, where number is a natural number

## EXAMPLES:
    1. SCRIPT FOR BASIC 

        pin*7 +(1) -(1)

        -> this line selects 
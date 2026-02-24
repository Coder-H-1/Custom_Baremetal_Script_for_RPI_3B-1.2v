# This is the format of the script that is to be provided to get output through GPIO pins and HDMI.

    I only implemented OUTPUT only, It is very stressing to work with very low level stuff T-T 

####    `pin*number` represents select PIN number, where `number` is a value between (1 to 40)

####    `+` represents PIN `ON` 

####    `-` represents PIN `OFF`

####    `(number)` represents delay time, where number is a natural number.

####    `(random=number)` represents a random number between (0 to `number`), where `number` is a natural number.

# EXAMPLES:
    1. SCRIPT FOR BASIC ON and OFF (executes only once)

        pin*7 + (1) - (1)

        
        This line selects pin 7 and ON for 1 second then OFF for 1 second  and program closes.

    

    2. SCRIPT FOR BASIC ON and OFF (executes infinitely)
     
        pin*7 inf > + (1) - (2)


        This line selects pin 7 and ON for 1 second then OFF for 2 second and repeats process after `>`.

    -

    3. SCRIPT FOR BASIC ON and OFF with random delay (executes infinitely)

        pin*7 inf > + (random=10) - (random=5)


        This line selects pin 7 and ON for (a random number between 0 and 10) and OFF for (a random number between 0 and 5) and repeats after `>`.   
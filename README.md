# CP/M Plus date-time test utility

This utility reads date-time from SCB (menu option 1), or from BDOS function 105 (menu option 2).

Tests on Amstrad PCW 8512 shows how an interrupt updates SCB datetime.


Compile with:

zcc +cpm -lm -o CLOCK.COM CLOCK.c

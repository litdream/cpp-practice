# Any other Notes.


## Hello world in Assembly

### Version 2: Kinda working??  but still infinite-loop

```
* 3000: A2 00 BD 0C 30 F0 06 20 ED FD E8 4C 02 30 60 C8 C5 CC CC CF A0 D7 CF D2 CC C4 8D 00

```

V2: 
```
ORG $3000

COUT        EQU $FDED

START       LDX #$00        ; Start index at 0

LOOP        LDA STRING,X    ; Load character
            BEQ DONE        ; IF character is $00, WE ARE DONE! Exit loop.
            JSR COUT        ; Else, print it
            INX             ; Move to next character
            JMP LOOP        ; CRITICAL FIX: Jump back to start the next iteration unconditionally

DONE        RTS             ; Return cleanly to the Monitor

; Your string (12 bytes + 1 carriage return + 1 null terminator = 14 bytes)
STRING      HEX C8 C5 CC CC CF A0 D7 CF D2 CC C4 8D 00

```

### Not working

```
] call -151

* 3000: A2 00 BD 0C 30 F0 06 20 ED FD E8 D0 F3 60 C8 C5 CC CC CF A0 D7 CF D2 CC C4 8D 00

* 3000G

```

V1:  Infinite loop
```
ORG $3000       ; Start our program at memory address $3000

COUT        EQU $FDED       ; Define the Apple II ROM screen output routine

START       LDX #$00        ; Initialize X register to 0 (our string index)

LOOP        LDA STRING,X    ; Load character from STRING + X into Accumulator
            BEQ DONE        ; If character is 0 (our terminator), branch to DONE
            JSR COUT        ; Jump to Subroutine COUT to print the character
            INX             ; Increment X to point to the next character
            BNE LOOP        ; Loop back to get the next character

DONE        RTS             ; Return to Subroutine (ends program, back to prompt)

; Our message string (with Bit 7 set high for standard text)
STRING      HEX C8          ; 'H' (Normal ASCII 48 + 80)
            HEX C5          ; 'E'
            HEX CC          ; 'L'
            HEX CC          ; 'L'
            HEX CF          ; 'O'
            HEX A0          ; ' ' (Space)
            HEX D7          ; 'W'
            HEX CF          ; 'O'
            HEX D2          ; 'R'
            HEX CC          ; 'L'
            HEX C4          ; 'D'
            HEX 8D          ; Carriage Return
            HEX 00          ; Null terminator to end the loop
```			

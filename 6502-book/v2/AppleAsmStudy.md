# Apple Assembly Study

https://retroapplejs.github.io/
  - Assembler -> Help tab
    1. Enter source code pane
    2. Assemble button
    3. Listing pane
    4. To emulator,  or To debugger button

Registers are all 1 byte (8-bit) except PC (16-bit)
  - Which makes sense
  - A, X, Y, P, S, PC
    - S: default $0100-$01FF


PRAGMAS

ADRW	Address bus width
BPE	Bits per Element
CSV	0=single (default),  1=comma separated
TYP	EXP=Expression (default), HEX=HEX number

DIRECTIVE	ARGUMENTS		PARTIAL/FULL	DESCRIPTION
....


OPCODES AND ADDRESSING
OPC		implied
OPC A		Accumulator
OPC #BB		immediate    (BB := byte)
OPC HHLL	absolute
OPC HHLL,X	absolute, X-indexed
OPC HHLL,Y	absolute, Y-indexed
OPC *LL		zeropage
OPC *LL,X	zeropage, X-indexed
OPC *LL,Y	zeropage, Y-indexed
OPC (BB,X)	X-indexed, indirect
OPC (LL),Y	indirect, Y-indexed
OPC (HHLL)	indirect
OPC BB		relative


$[0-9A-Fa-f]	hex
%[01]		binary
0[0-7]		octal
[0-9]		decimal
<		LO-byte portion
>		HI-byte portion


LABELS AND IDENTIFIERS

ORG $C000		Set start address (PC) to $c000
LABEL1  LDA #4
	BNE LABEL2
STORE	EQU $0810	STORE=$0810  (define directly)
HERE	EQU *		HERE is current (PC)
HERE2			HERE2 is also current (PC)
	LDA STORE	Load LO-byte of STORE having value $10

PRAGMA
.BYTE	BB		insert 8bit byte at current address into code
.WORD	HHLL		insert 16bit word at current addr
.AT	/ABC/		Insert ascii string Terminated
.END			End of source code. stop assembly.

; comment







My first hello world:
```
ORG $3000

START LDX #$00
LOOP  LDA STRING,X
      BEQ DONE
      ORA #$80    ; bit-7 is for flashing on (0), normal on (1).  
      JSR $FDED   ; print a char
      INX
      JMP LOOP

DONE  LDA #$8D
      JSR $FDED
      RTS

STRING
MSB   ON
.AT   /HELLO WORLD/
MSB   OFF
.BYTE 00      ; null terminate
                    
```

Object file:
```
3000: a2 00 bd 16 30 f0 09 09
3008: 80 20 ed fd e8 4c 02 30
3010: a9 8d 20 ed fd 60 48 45
3018: 4c 4c 4f 20 57 4f 52 4c
3020: c4 00
```



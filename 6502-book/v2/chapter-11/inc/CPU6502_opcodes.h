#pragma once

#include <cstdint>

// Flag masks
#define   AF_SIGN       0x80
#define   AF_OVERFLOW   0x40
#define   AF_RESERVED   0x20
#define   AF_BREAK      0x10
#define   AF_DECIMAL    0x08
#define   AF_INTERRUPT  0x04
#define   AF_ZERO       0x02
#define   AF_CARRY      0x01

// General Purpose Macros
#define AF_TO_EF { \
    flagc = (S & AF_CARRY); \
    flagn = (S & AF_SIGN); \
    flagv = (S & AF_OVERFLOW); \
    flagz = (S & AF_ZERO); \
}

#define EF_TO_AF { \
    S = (S & ~(AF_CARRY | AF_SIGN | AF_OVERFLOW | AF_ZERO)) \
        | flagc \
        | flagn \
        | (flagv ? AF_OVERFLOW : 0) \
        | (flagz ? AF_ZERO     : 0) \
        | AF_RESERVED | AF_BREAK; \
}

#define CYC(a)   { uint32_t _c = (a)+uExtraCycles; uExecutedCycles += _c; g_nIrqCheckTimeout -= _c; bus->tick(_c); }

#define POP bus->read(((SP >= 0x1FF) ? (SP = 0x100) : ++SP))

#define PUSH(a) { \
    bus->write(SP--, a); \
    if (SP < 0x100) \
        SP = 0x1FF; \
}

#define READ bus->read(addr)

#define SETNZ(a) { \
    flagn = (((a) & 0xFF) & 0x80); \
    flagz = !((a) & 0xFF); \
}

#define SETZ(a)   flagz = !((a) & 0xFF);

#define WRITE(a) bus->write(addr, a)

#define INV { std::cout << "Invalid opcode at 0x" << std::hex << (PC-1) << std::dec << std::endl; jammed = true; }
#define DoIrqProfiling(a) // Dummy

#define BRANCH_TAKEN { \
    base = PC; \
    PC += addr; \
    if ((base ^ PC) & 0xFF00) \
        uExtraCycles = 2; \
    else \
        uExtraCycles = 1; \
}

#define CHECK_PAGE_CHANGE if (bSlowerOnPagecross) { \
    if ((base ^ addr) & 0xFF00) \
        uExtraCycles = 1; \
}

// Addressing Mode Macros

#define ABS { addr = bus->read(PC) | (bus->read(PC + 1) << 8); PC += 2; }

#define IABSX { \
    uint16_t base_addr = bus->read(PC) | (bus->read(PC + 1) << 8); \
    addr = bus->read(base_addr + X) | (bus->read(base_addr + X + 1) << 8); \
    PC += 2; \
}

#define ABSX { \
    base = bus->read(PC) | (bus->read(PC + 1) << 8); \
    addr = base + X; \
    PC += 2; \
    CHECK_PAGE_CHANGE; \
}

#define ABSY { \
    base = bus->read(PC) | (bus->read(PC + 1) << 8); \
    addr = base + Y; \
    PC += 2; \
    CHECK_PAGE_CHANGE; \
}

#define IABSCMOS { \
    base = bus->read(PC) | (bus->read(PC + 1) << 8); \
    addr = bus->read(base) | (bus->read(base + 1) << 8); \
    if ((base & 0xFF) == 0xFF) uExtraCycles = 1; \
    PC += 2; \
}

#define IABSNMOS { \
    base = bus->read(PC) | (bus->read(PC + 1) << 8); \
    if ((base & 0xFF) == 0xFF) \
        addr = bus->read(base) | (bus->read(base & 0xFF00) << 8); \
    else \
        addr = bus->read(base) | (bus->read(base + 1) << 8); \
    PC += 2; \
}

#define IMM addr = PC++;

#define INDX { \
    base = (bus->read(PC++) + X) & 0xFF; \
    if (base == 0xFF) \
        addr = bus->read(0xFF) | (bus->read(0x00) << 8); \
    else \
        addr = bus->read(base) | (bus->read(base + 1) << 8); \
}

#define INDY { \
    uint8_t zp_addr = bus->read(PC); \
    if (zp_addr == 0xFF) \
        base = bus->read(0xFF) | (bus->read(0x00) << 8); \
    else \
        base = bus->read(zp_addr) | (bus->read(zp_addr + 1) << 8); \
    PC++; \
    addr = base + Y; \
    CHECK_PAGE_CHANGE; \
}

#define IZPG { \
    base = bus->read(PC++); \
    if (base == 0xFF) \
        addr = bus->read(0xFF) | (bus->read(0x00) << 8); \
    else \
        addr = bus->read(base) | (bus->read(base + 1) << 8); \
}

#define REL addr = (int8_t)bus->read(PC++);

#define ZPG addr = bus->read(PC++);

#define ZPGX addr = (bus->read(PC++) + X) & 0xFF;

#define ZPGY addr = (bus->read(PC++) + Y) & 0xFF;

// Instruction Macros

#define ADC_NMOS { \
    bSlowerOnPagecross = true; \
    temp = READ; \
    if (S & AF_DECIMAL) { \
        val = (A & 0x0F) + (temp & 0x0F) + flagc; \
        if (val > 0x09) val += 0x06; \
        if (val <= 0x0F) val = (val & 0x0F) + (A & 0xF0) + (temp & 0xF0); \
        else val = (val & 0x0F) + (A & 0xF0) + (temp & 0xF0) + 0x10; \
        flagz = !((A + temp + flagc) & 0xFF); \
        flagn = (val & 0x80); \
        flagv = ((A ^ val) & 0x80) && !((A ^ temp) & 0x80); \
        if ((val & 0x1F0) > 0x90) val += 0x60; \
        flagc = ((val & 0xFF0) > 0xF0); \
        A = val & 0xFF; \
    } else { \
        val = A + temp + flagc; \
        flagc = (val > 0xFF); \
        flagv = (((A & 0x80) == (temp & 0x80)) && ((A & 0x80) != (val & 0x80))); \
        A = val & 0xFF; \
        SETNZ(A); \
    } \
}

#define ADC_CMOS { \
    bSlowerOnPagecross = true; \
    temp = READ; \
    flagv = !((A ^ temp) & 0x80); \
    if (S & AF_DECIMAL) { \
        uExtraCycles++; \
        val = (A & 0x0f) + (temp & 0x0f) + flagc; \
        if (val >= 0x0A) val = 0x10 | ((val + 6) & 0x0f); \
        val += (A & 0xf0) + (temp & 0xf0); \
        if (val >= 0xA0) { \
            flagc = 1; \
            if (val >= 0x180) flagv = 0; \
            val += 0x60; \
        } else { \
            flagc = 0; \
            if (val < 0x80) flagv = 0; \
        } \
    } else { \
        val = A + temp + flagc; \
        if (val >= 0x100) { \
            flagc = 1; \
            if (val >= 0x180) flagv = 0; \
        } else { \
            flagc = 0; \
            if (val < 0x80) flagv = 0; \
        } \
    } \
    A = val & 0xFF; \
    SETNZ(A); \
}

#define ALR { \
    A &= READ; \
    flagc = (A & 1); \
    flagn = 0; \
    A >>= 1; \
    SETZ(A); \
}

#define AND { \
    bSlowerOnPagecross = true; \
    A &= READ; \
    SETNZ(A); \
}

#define ANC { \
    A &= READ; \
    SETNZ(A); \
    flagc = !!flagn; \
}

#define ARR { \
    temp = A & READ; \
    if (S & AF_DECIMAL) { \
        val = temp; \
        val |= (flagc ? 0x100 : 0); \
        val >>= 1; \
        flagn = (flagc ? 0x80 : 0); \
        SETZ(val); \
        flagv = ((val ^ temp) & 0x40); \
        if (((val & 0x0F) + (val & 0x01)) > 0x05) val = (val & 0xF0) | ((val + 0x06) & 0x0F); \
        if (((val & 0xF0) + (val & 0x10)) > 0x50) { \
            val = (val & 0x0F) | ((val + 0x60) & 0xF0); \
            flagc = 1; \
        } else flagc = 0; \
        A = (val & 0xFF); \
    } else { \
        val = temp | (flagc ? 0x100 : 0); \
        val >>= 1; \
        SETNZ(val); \
        flagc = !!(val & 0x40); \
        flagv = ((val & 0x40) ^ ((val & 0x20) << 1)); \
        A = (val & 0xFF); \
    } \
}

#define ASL_NMOS { \
    bSlowerOnPagecross = false; \
    val = READ << 1; \
    flagc = (val > 0xFF); \
    SETNZ(val); \
    WRITE(val); \
}

#define ASL_CMOS { \
    bSlowerOnPagecross = true; \
    val = READ << 1; \
    flagc = (val > 0xFF); \
    SETNZ(val); \
    WRITE(val); \
}

#define ASLA { \
    val = A << 1; \
    flagc = (val > 0xFF); \
    SETNZ(val); \
    A = (uint8_t)val; \
}

#define ASO { \
    bSlowerOnPagecross = false; \
    val = READ << 1; \
    flagc = (val > 0xFF); \
    WRITE(val); \
    A |= val; \
    SETNZ(A); \
}

#define AXA { \
    bSlowerOnPagecross = false; \
    val = A & X & (((base >> 8) + 1) & 0xFF); \
    WRITE(val); \
}

#define AXS { \
    bSlowerOnPagecross = false; \
    WRITE(A & X); \
}

#define BCC if (!flagc) BRANCH_TAKEN;
#define BCS if ( flagc) BRANCH_TAKEN;
#define BEQ if ( flagz) BRANCH_TAKEN;

#define BIT { \
    bSlowerOnPagecross = true; \
    val = READ; \
    flagz = !(A & val); \
    flagn = val & 0x80; \
    flagv = val & 0x40; \
}

#define BITI flagz = !(A & READ);

#define BMI if ( flagn) BRANCH_TAKEN;
#define BNE if (!flagz) BRANCH_TAKEN;
#define BPL if (!flagn) BRANCH_TAKEN;
#define BRA BRANCH_TAKEN;

#define BRK { \
    PC++; \
    PUSH(PC >> 8); \
    PUSH(PC & 0xFF); \
    EF_TO_AF; \
    PUSH(S); \
    S |= AF_INTERRUPT; \
    PC = bus->read(0xFFFE) | (bus->read(0xFFFF) << 8); \
}

#define BVC if (!flagv) BRANCH_TAKEN;
#define BVS if ( flagv) BRANCH_TAKEN;

#define CLC flagc = 0;
#define CLD S &= ~AF_DECIMAL;
#define CLI S &= ~AF_INTERRUPT;
#define CLV flagv = 0;

#define CMP { \
    bSlowerOnPagecross = true; \
    val = READ; \
    flagc = (A >= val); \
    val = A - val; \
    SETNZ(val); \
}

#define CPX { \
    val = READ; \
    flagc = (X >= val); \
    val = X - val; \
    SETNZ(val); \
}

#define CPY { \
    val = READ; \
    flagc = (Y >= val); \
    val = Y - val; \
    SETNZ(val); \
}

#define DCM { \
    bSlowerOnPagecross = false; \
    val = READ - 1; \
    WRITE(val); \
    flagc = (A >= val); \
    val = A - val; \
    SETNZ(val); \
}

#define DEA { \
    --A; \
    SETNZ(A); \
}

#define DEC_NMOS { \
    bSlowerOnPagecross = false; \
    val = READ - 1; \
    SETNZ(val); \
    WRITE(val); \
}

#define DEC_CMOS { \
    bSlowerOnPagecross = true; \
    val = READ - 1; \
    SETNZ(val); \
    WRITE(val); \
}

#define DEX { \
    --X; \
    SETNZ(X); \
}

#define DEY { \
    --Y; \
    SETNZ(Y); \
}

#define EOR { \
    bSlowerOnPagecross = true; \
    A ^= READ; \
    SETNZ(A); \
}

#define HLT { \
    jammed = true; \
    --PC; \
}

#define INA { \
    ++A; \
    SETNZ(A); \
}

#define INC_NMOS { \
    bSlowerOnPagecross = false; \
    val = READ + 1; \
    SETNZ(val); \
    WRITE(val); \
}

#define INC_CMOS { \
    bSlowerOnPagecross = true; \
    val = READ + 1; \
    SETNZ(val); \
    WRITE(val); \
}

#define INS { \
    bSlowerOnPagecross = false; \
    val = READ + 1; \
    WRITE(val); \
    temp = val; \
    temp2 = A - temp - !flagc; \
    if (S & AF_DECIMAL) { \
        val = (A & 0x0F) - (temp & 0x0F) - !flagc; \
        if (val & 0x10) val = ((val - 0x06) & 0x0F) | ((A & 0xF0) - (temp & 0xF0) - 0x10); \
        else val = (val & 0x0F) | ((A & 0xF0) - (temp & 0xF0)); \
        if (val & 0x100) val -= 0x60; \
        flagc = (temp2 < 0x100); \
        SETNZ(temp2 & 0xFF); \
        flagv = ((A ^ temp2) & 0x80) && ((A ^ temp) & 0x80); \
        A = val & 0xFF; \
    } else { \
        val = temp2; \
        flagc = (val < 0x100); \
        flagv = (((A & 0x80) != (temp & 0x80)) && ((A & 0x80) != (val & 0x80))); \
        A = val & 0xFF; \
        SETNZ(A); \
    } \
}

#define INX { \
    ++X; \
    SETNZ(X); \
}

#define INY { \
    ++Y; \
    SETNZ(Y); \
}

#define JMP PC = addr;

#define JSR { \
    --PC; \
    PUSH(PC >> 8); \
    PUSH(PC & 0xFF); \
    PC = addr; \
}

#define LAS { \
    bSlowerOnPagecross = true; \
    val = (uint8_t)(READ & SP); \
    A = X = (uint8_t)val; \
    SP = val | 0x100; \
    SETNZ(val); \
}

#define LAX { \
    bSlowerOnPagecross = true; \
    A = X = READ; \
    SETNZ(A); \
}

#define LDA { \
    bSlowerOnPagecross = true; \
    A = READ; \
    SETNZ(A); \
}

#define LDX { \
    bSlowerOnPagecross = true; \
    X = READ; \
    SETNZ(X); \
}

#define LDY { \
    bSlowerOnPagecross = true; \
    Y = READ; \
    SETNZ(Y); \
}

#define LSE { \
    bSlowerOnPagecross = false; \
    val = READ; \
    flagc = (val & 1); \
    val >>= 1; \
    WRITE(val); \
    A ^= val; \
    SETNZ(A); \
}

#define LSR_NMOS { \
    bSlowerOnPagecross = false; \
    val = READ; \
    flagc = (val & 1); \
    flagn = 0; \
    val >>= 1; \
    SETZ(val); \
    WRITE(val); \
}

#define LSR_CMOS { \
    bSlowerOnPagecross = true; \
    val = READ; \
    flagc = (val & 1); \
    flagn = 0; \
    val >>= 1; \
    SETZ(val); \
    WRITE(val); \
}

#define LSRA { \
    flagc = (A & 1); \
    flagn = 0; \
    A >>= 1; \
    SETZ(A); \
}

#define NOP bSlowerOnPagecross = true;

#define OAL { \
    A |= 0xEE; \
    A &= READ; \
    X = A; \
    SETNZ(A); \
}

#define ORA { \
    bSlowerOnPagecross = true; \
    A |= READ; \
    SETNZ(A); \
}

#define PHA PUSH(A)

#define PHP { \
    EF_TO_AF; \
    PUSH(S); \
}

#define PHX PUSH(X)
#define PHY PUSH(Y)

#define PLA { \
    A = POP; \
    SETNZ(A); \
}

#define PLP { \
    S = POP | AF_RESERVED | AF_BREAK; \
    AF_TO_EF; \
}

#define PLX { \
    X = POP; \
    SETNZ(X); \
}

#define PLY { \
    Y = POP; \
    SETNZ(Y); \
}

#define RLA { \
    bSlowerOnPagecross = false; \
    val = (READ << 1) | flagc; \
    flagc = (val > 0xFF); \
    WRITE(val); \
    A &= val; \
    SETNZ(A); \
}

#define ROL_NMOS { \
    bSlowerOnPagecross = false; \
    val = (READ << 1) | flagc; \
    flagc = (val > 0xFF); \
    WRITE(val); \
    SETNZ(val); \
}

#define ROL_CMOS { \
    bSlowerOnPagecross = true; \
    val = (READ << 1) | flagc; \
    flagc = (val > 0xFF); \
    WRITE(val); \
    SETNZ(val); \
}

#define ROLA { \
    val = (((uint16_t)A) << 1) | flagc; \
    flagc = (val > 0xFF); \
    A = val & 0xFF; \
    SETNZ(A); \
}

#define ROR_NMOS { \
    temp = READ; \
    val = (temp >> 1) | (flagc ? 0x80 : 0); \
    flagc = (temp & 1); \
    SETNZ(val); \
    WRITE(val); \
}

#define ROR_CMOS { \
    temp = READ; \
    val = (temp >> 1) | (flagc ? 0x80 : 0); \
    flagc = (temp & 1); \
    SETNZ(val); \
    WRITE(val); \
}

#define RORA { \
    val = (((uint16_t)A) >> 1) | (flagc ? 0x80 : 0); \
    flagc = (A & 1); \
    A = val & 0xFF; \
    SETNZ(A); \
}

#define RRA { \
    bSlowerOnPagecross = false; \
    temp = READ; \
    val = (temp >> 1) | (flagc ? 0x80 : 0); \
    flagc = (temp & 1); \
    WRITE(val); \
    temp = val; \
    if (S & AF_DECIMAL) { \
        val = (A & 0x0F) + (temp & 0x0F) + flagc; \
        if (val > 0x09) val += 0x06; \
        if (val <= 0x0F) val = (val & 0x0F) + (A & 0xF0) + (temp & 0xF0); \
        else val = (val & 0x0F) + (A & 0xF0) + (temp & 0xF0) + 0x10; \
        flagz = !((A + temp + flagc) & 0xFF); \
        flagn = (val & 0x80); \
        flagv = ((A ^ val) & 0x80) && !((A ^ temp) & 0x80); \
        if ((val & 0x1F0) > 0x90) val += 0x60; \
        flagc = ((val & 0xFF0) > 0xF0); \
        A = val & 0xFF; \
    } else { \
        val = A + temp + flagc; \
        flagc = (val > 0xFF); \
        flagv = (((A & 0x80) == (temp & 0x80)) && ((A & 0x80) != (val & 0x80))); \
        A = val & 0xFF; \
        SETNZ(A); \
    } \
}

#define RTI { \
    S = POP | AF_RESERVED | AF_BREAK; \
    AF_TO_EF; \
    PC = POP; \
    PC |= (((uint16_t)POP) << 8); \
}

#define RTS { \
    PC = POP; \
    PC |= (((uint16_t)POP) << 8); \
    ++PC; \
}

#define SAX { \
    temp = A & X; \
    val = READ; \
    flagc = (temp >= val); \
    X = temp - val; \
    SETNZ(X); \
}

#define SAY { \
    bSlowerOnPagecross = false; \
    val = Y & (((base >> 8) + 1) & 0xFF); \
    WRITE(val); \
}

#define SBC_NMOS { \
    bSlowerOnPagecross = true; \
    temp = READ; \
    temp2 = A - temp - !flagc; \
    if (S & AF_DECIMAL) { \
        val = (A & 0x0F) - (temp & 0x0F) - !flagc; \
        if (val & 0x10) val = ((val - 0x06) & 0x0F) | ((A & 0xF0) - (temp & 0xF0) - 0x10); \
        else val = (val & 0x0F) | ((A & 0xF0) - (temp & 0xF0)); \
        if (val & 0x100) val -= 0x60; \
        flagc = (temp2 < 0x100); \
        SETNZ(temp2 & 0xFF); \
        flagv = ((A ^ temp2) & 0x80) && ((A ^ temp) & 0x80); \
        A = val & 0xFF; \
    } else { \
        val = temp2; \
        flagc = (val < 0x100); \
        flagv = (((A & 0x80) != (temp & 0x80)) && ((A & 0x80) != (val & 0x80))); \
        A = val & 0xFF; \
        SETNZ(A); \
    } \
}

#define SBC_CMOS { \
    bSlowerOnPagecross = true; \
    temp = READ; \
    flagv = ((A ^ temp) & 0x80); \
    if (S & AF_DECIMAL) { \
        uExtraCycles++; \
        temp2 = 0x0F + (A & 0x0F) - (temp & 0x0F) + flagc; \
        if (temp2 < 0x10) { \
            val = 0; \
            temp2 -= 0x06; \
        } else { \
            val = 0x10; \
            temp2 -= 0x10; \
        } \
        val += 0xF0 + (A & 0xF0) - (temp & 0xF0); \
        if (val < 0x100) { \
            flagc = 0; \
            if (val < 0x80) flagv = 0; \
            val -= 0x60; \
        } else { \
            flagc = 1; \
            if (val >= 0x180) flagv = 0; \
        } \
        val += temp2; \
    } else { \
        val = 0xff + A - temp + flagc; \
        if (val < 0x100) { \
            flagc = 0; \
            if (val < 0x80) flagv = 0; \
        } else { \
            flagc = 1; \
            if (val >= 0x180) flagv = 0; \
        } \
    } \
    A = val & 0xFF; \
    SETNZ(A); \
}

#define SEC flagc = 1;
#define SED S |= AF_DECIMAL;
#define SEI S |= AF_INTERRUPT;

#define STA { \
    bSlowerOnPagecross = false; \
    WRITE(A); \
}

#define STX { \
    bSlowerOnPagecross = false; \
    WRITE(X); \
}

#define STY { \
    bSlowerOnPagecross = false; \
    WRITE(Y); \
}

#define STZ { \
    bSlowerOnPagecross = false; \
    WRITE(0); \
}

#define TAS { \
    bSlowerOnPagecross = false; \
    val = A & X; \
    SP = 0x100 | val; \
    val &= (((base >> 8) + 1) & 0xFF); \
    WRITE(val); \
}

#define TAX { \
    X = A; \
    SETNZ(X); \
}

#define TAY { \
    Y = A; \
    SETNZ(Y); \
}

#define TRB { \
    bSlowerOnPagecross = false; \
    val = READ; \
    flagz = !(A & val); \
    val &= ~A; \
    WRITE(val); \
}

#define TSB { \
    bSlowerOnPagecross = false; \
    val = READ; \
    flagz = !(A & val); \
    val |= A; \
    WRITE(val); \
}

#define TSX { \
    X = SP & 0xFF; \
    SETNZ(X); \
}

#define TXA { \
    A = X; \
    SETNZ(A); \
}

#define TXS SP = 0x100 | X;

#define TYA { \
    A = Y; \
    SETNZ(A); \
}

#define XAA { \
    A = X; \
    A &= READ; \
    SETNZ(A); \
}

#define XAS { \
    bSlowerOnPagecross = false; \
    val = X & (((base >> 8) + 1) & 0xFF); \
    WRITE(val); \
}

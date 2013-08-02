/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 19-Jul-2013
 * File: opcode.h
 * 
 * Description:
 * 
 *      6502 opcode implementations.  Important to note that this code
 *      is optimized for speed, not size.  In other words, instead of
 *      decoding addressing modes for instructions and the like, we just
 *      use a table.  In the future, I may add a flag for prioritizing
 *      size instead of speed.
 * 
 * Change Log:
 *      19-Jul-2013:
 *          File created.
 */

#ifndef VNES_OPCODE_H
#define VNES_OPCODE_H

#include "types.h"

VNES_Err Dispatch_Opcode(u8 opcode);

u8 Get_Opcode_Length(u8 opcode);

/* Addressing mode shorthands */

/* Implicit Mode
 * No additional value is stored in the instruction; the instruction
 * itself implies the information to be manipulated.
 * Example:
 *      TAX         ; Transfer accumulator to X register */
#define IP MODE_IMPLICIT

/* Accumulator Mode
 * Operate directly on the accumulator
 * Example: 
 *      LSR A       ; Logical shift right one bit */
#define AC MODE_ACCUMULATOR

/* Immediate Mode
 * Specify an 8-bit constant within instruction.
 * Example: 
 *      LDA #10     ; Load 10 ($0A) into the accumulator */
#define IM MODE_IMMEDIATE

/* Zero Page Mode
 * Specifies the lower byte of the address where the upper byte is $00.
 * Example:
 *      LDA $00     ; Load the accumulator from memory position $0000 */
#define ZP MODE_ZERO_PAGE

/* Zero Page Mode (X increment)
 * Same as Zero Page Mode, but adds the X register to the address first.
 * The resulting address is modulo $00FF (it wraps to zero-page).
 * Example:
 *      (X = $30)
 *      STY $10,X   ; Save the Y register at location $0040
 *      (X = $FF)
 *      STY $80,X   ; Save Y register into location $007F (not $017F) */
#define ZX MODE_ZERO_PAGE_X

/* Zero Page Mode (Y increment)
 * Same as Zero Page Mode (X increment), but only for the instructions
 * LDX and STX.
 * Example:
 *      (Y = $33)
 *      STX $17,Y   ; Save the x register at location $004A */
#define ZY MODE_ZERO_PAGE_Y

/* Relative Mode
 * For branch instructions, contains a SIGNED 8-bit offset, relative to
 * the programming counter, which is added only if the branch is taken.
 * Example:
 *      BEQ *+4     ; If zero flag is set, skip ahead two bytes. */
#define RE MODE_RELATIVE

/* Absolute Mode
 * These instructions contain a full 16-bit address to locate the target
 * location in memory.
 * Example:
 *      JMP $4321   ; Jump to the location $4321. */
#define AB MODE_ABSOLUTE

/* Absolute Mode (X increment)
 * Same as Absolute Mode, but adds the X register to the address to form
 * the target location.
 * Example:
 *      STA $3000,X ; Store accumulator between $3000 and $30FF */
#define AX MODE_ABSOLUTE_X

/* Absolute Mode (Y increment)
 * Same as Absolute Mode (X increment), but uses Y register instead.
 * Example:
 *      STA $3000,Y ; Store accumulator between $3000 and $30FF */
#define AY MODE_ABSOLUTE_Y

/* Indirect Mode
 * This mode is only used with JMP.  The instruction contains a 16-bit
 * address containing the location of the least significant byte of
 * another 16-bit address, which is the true target of the instruction.
 * Example:
 *      JMP ($FFFC)     ; Jump to the jump vector for power on/reset. */
#define IN MODE_INDIRECT

/* Indexed Indirect
 * Usually used with a table of addresses stored in the zero page. The
 * address of the table is stored in the instruction, and the X register
 * indexes to the correct location, to give the location of the least
 * significant byte of the address targeted.  This also wraps to the
 * zero page.
 * Example:
 *      LDA ($A0, X)    ; Load a byte indirectly from ($00A0 + X). */
#define IX MODE_INDEXED_INDIRECT

/* Indirect Indexed
 * Instruction contains the zero-page location of the least significant
 * byte of a 16-bit address.  The Y register is added to this value to
 * generate the actual target address. Indirection becomes before the
 * indexing.
 * Example:
 *      LDA ($A0),Y     ; Load a byte indirectly from ($00A0) + Y. */
#define IY MODE_INDIRECT_INDEXED

#define ADDRESS_MODES(apply)                                           \
    apply(MODE_IMPLICIT, 1)                                            \
    apply(MODE_ACCUMULATOR, 1)                                         \
    apply(MODE_IMMEDIATE, 2)                                           \
    apply(MODE_ZERO_PAGE, 2)                                           \
    apply(MODE_ZERO_PAGE_X, 2)                                         \
    apply(MODE_ZERO_PAGE_Y, 2)                                         \
    apply(MODE_RELATIVE, 2)                                            \
    apply(MODE_ABSOLUTE, 3)                                            \
    apply(MODE_ABSOLUTE_X, 3)                                          \
    apply(MODE_ABSOLUTE_Y, 3)                                          \
    apply(MODE_INDIRECT, 3)                                            \
    apply(MODE_INDEXED_INDIRECT, 2)                                    \
    apply(MODE_INDIRECT_INDEXED, 2)                                     

#define mode(name, length) name,
enum {
    ADDRESS_MODES(mode)
} address_modes;
#undef mode

typedef void(*op_func)(u8);

/* Used for distinguishing undocumented opcodes (set to 0). */
#define _ 0

INLINED void Do_Nmi(void);

/* Opcode X-macro definition.  Format is (name, mode, cycles).  UNS
 * stands for "unsupported," and may be needed later for some games.
 * Asterisks (*) next to hex value means that an additional cycle
 * will be necessary for page crossing. */
#define OPCODE_LIST(op)                                                \
                                                                       \
/*        00             01             02             03 */           \
       op(BRK, IP, 7) op(ORA, IX, 6) op(UNS, IP, _) op(UNS, IP, _)     \
/*        04             05             06             07 */           \
       op(UNS, IP, _) op(ORA, ZP, 3) op(ASL, ZP, 5) op(UNS, IP, _)     \
/*        08             09             0A             0B */           \
       op(PHP, IP, 3) op(ORA, IM, 2) op(ASL, AC, 2) op(UNS, IP, _)     \
/*        0C             0D             0E             0F */           \
       op(UNS, IP, _) op(ORA, AB, 4) op(ASL, AB, 6) op(UNS, IP, _)     \
                                                                       \
/*        10*            11*            12             13 */           \
       op(BPL, RE, 2) op(ORA, IY, 5) op(UNS, IP, _) op(UNS, IP, _)     \
/*        14             15             16             17 */           \
       op(UNS, IP, _) op(ORA, ZX, 4) op(ASL, ZX, 6) op(UNS, IP, _)     \
/*        18             19*            1A             1B */           \
       op(CLC, IP, 2) op(ORA, AY, 4) op(UNS, IP, _) op(UNS, IP, _)     \
/*        1C             1D*            1E             1F */           \
       op(UNS, IP, _) op(ORA, AX, 4) op(ASL, AX, 7) op(UNS, IP, _)     \
                                                                       \
/*        20             21             22             23 */           \
       op(JSR, AB, 6) op(AND, IX, 6) op(UNS, IP, _) op(UNS, IP, _)     \
/*        24             25             26             27 */           \
       op(BIT, ZP, 3) op(AND, ZP, 3) op(ROL, ZP, 5) op(UNS, IP, _)     \
/*        28             29             2A             2B */           \
       op(PLP, IP, 4) op(AND, IM, 2) op(ROL, AC, 2) op(UNS, IP, _)     \
/*        2C             2D             2E             2F */           \
       op(BIT, AB, 4) op(AND, AB, 4) op(ROL, AB, 6) op(UNS, IP, _)     \
                                                                       \
/*        30*            31*            32             33 */           \
       op(BMI, RE, 2) op(AND, IY, 5) op(UNS, IP, _) op(UNS, IP, _)     \
/*        34             35             36             37 */           \
       op(UNS, IP, _) op(AND, ZX, 4) op(ROL, ZX, 6) op(UNS, IP, _)     \
/*        38             39*            3A             3B */           \
       op(SEC, IP, 2) op(AND, AY, 4) op(UNS, IP, _) op(UNS, IP, _)     \
/*        3C             3D*            3E             3F */           \
       op(UNS, IP, _) op(AND, AX, 4) op(ROL, AX, 7) op(UNS, IP, _)     \
                                                                       \
/*        40             41             42             43 */           \
       op(RTI, IP, 6) op(EOR, IX, 6) op(UNS, IP, _) op(UNS, IP, _)     \
/*        44             45             46             47 */           \
       op(UNS, IP, _) op(EOR, ZP, 3) op(LSR, ZP, 5) op(UNS, IP, _)     \
/*        48             49             4A             4B */           \
       op(PHA, IP, 3) op(EOR, IM, 2) op(LSR, AC, 2) op(UNS, IP, _)     \
/*        4C             4D             4E             4F */           \
       op(JMP, AB, 3) op(EOR, AB, 4) op(LSR, AB, 6) op(UNS, IP, _)     \
                                                                       \
/*        50*            51*            52             53 */           \
       op(BVC, RE, 2) op(EOR, IY, 5) op(UNS, IP, _) op(UNS, IP, _)     \
/*        54             55             56             57 */           \
       op(UNS, IP, _) op(EOR, ZX, 4) op(LSR, ZX, 6) op(UNS, IP, _)     \
/*        58             59*            5A             5B */           \
       op(CLI, IP, 2) op(EOR, AY, 4) op(UNS, IP, _) op(UNS, IP, _)     \
/*        5C             5D*            5E             5F */           \
       op(UNS, IP, _) op(EOR, AX, 4) op(LSR, AX, 7) op(UNS, IP, _)     \
                                                                       \
/*        60             61             62             63 */           \
       op(RTS, IP, 6) op(ADC, IX, 6) op(UNS, IP, _) op(UNS, IP, _)     \
/*        64             65             66             67 */           \
       op(UNS, IP, _) op(ADC, ZP, 3) op(ROR, ZP, 5) op(UNS, IP, _)     \
/*        68             69             6A             6B */           \
       op(PLA, IP, 4) op(ADC, IM, 2) op(ROR, AC, 2) op(UNS, IP, _)     \
/*        6C             6D             6E             6F */           \
       op(JMP, IN, 5) op(ADC, AB, 4) op(ROR, AB, 6) op(UNS, IP, _)     \
                                                                       \
/*        70*            71*            72             73 */           \
       op(BVS, RE, 2) op(ADC, IY, 5) op(UNS, IP, _) op(UNS, IP, _)     \
/*        74             75             76             77 */           \
       op(UNS, IP, _) op(ADC, ZX, 4) op(ROR, ZX, 6) op(UNS, IP, _)     \
/*        78             79*            7A             7B */           \
       op(SEI, IP, 2) op(ADC, AY, 4) op(UNS, IP, _) op(UNS, IP, _)     \
/*        7C             7D*            7E             7F */           \
       op(UNS, IP, _) op(ADC, AX, 4) op(ROR, AX, 7) op(UNS, IP, _)     \
                                                                       \
/*        80             81             82             83 */           \
       op(UNS, IP, _) op(STA, IX, 6) op(UNS, IP, _) op(UNS, IP, _)     \
/*        84             85             86             87 */           \
       op(STY, ZP, 3) op(STA, ZP, 3) op(STX, ZP, 3) op(UNS, IP, _)     \
/*        88             89             8A             8B */           \
       op(DEY, IP, 2) op(UNS, IP, _) op(TXA, IP, 2) op(UNS, IP, _)     \
/*        8C             8D             8E             8F */           \
       op(STY, AB, 4) op(STA, AB, 4) op(STX, AB, 4) op(UNS, IP, _)     \
                                                                       \
/*        90*            91             92             93 */           \
       op(BCC, RE, 2) op(STA, IY, 6) op(UNS, IP, _) op(UNS, IP, _)     \
/*        94             95             96             97 */           \
       op(STY, ZX, 4) op(STA, ZX, 4) op(STX, ZY, 4) op(UNS, IP, _)     \
/*        98             99             9A             9B */           \
       op(TYA, IP, 2) op(STA, AY, 5) op(TXS, IP, 2) op(UNS, IP, _)     \
/*        9C             9D             9E             9F */           \
       op(UNS, IP, _) op(STA, AX, 5) op(UNS, IP, _) op(UNS, IP, _)     \
                                                                       \
/*        A0             A1             A2             A3 */           \
       op(LDY, IM, 2) op(LDA, IX, 6) op(LDX, IM, 2) op(UNS, IP, _)     \
/*        A4             A5             A6             A7 */           \
       op(LDY, ZP, 3) op(LDA, ZP, 3) op(LDX, ZP, 3) op(UNS, IP, _)     \
/*        A8             A9             AA             AB */           \
       op(TAY, IP, 2) op(LDA, IM, 2) op(TAX, IP, 2) op(UNS, IP, _)     \
/*        AC             AD             AE             AF */           \
       op(LDY, AB, 4) op(LDA, AB, 4) op(LDX, AB, 4) op(UNS, IP, _)     \
                                                                       \
/*        B0*            B1*            B2             B3 */           \
       op(BCS, RE, 2) op(LDA, IY, 5) op(UNS, IP, _) op(UNS, IP, _)     \
/*        B4             B5             B6             B7 */           \
       op(LDY, ZX, 4) op(LDA, ZX, 4) op(LDX, ZY, 4) op(UNS, IP, _)     \
/*        B8             B9*            BA             BB */           \
       op(CLV, IP, 2) op(LDA, AY, 4) op(TSX, IP, 2) op(UNS, IP, _)     \
/*        BC*            BD*            BE*            BF */           \
       op(LDY, AX, 4) op(LDA, AX, 4) op(LDX, AY, 4) op(UNS, IP, _)     \
                                                                       \
/*        C0             C1             C2             C3 */           \
       op(CPY, IM, 2) op(CMP, IX, 6) op(UNS, IP, _) op(UNS, IP, _)     \
/*        C4             C5             C6             C7 */           \
       op(CPY, ZP, 3) op(CMP, ZP, 3) op(DEC, ZP, 5) op(UNS, IP, _)     \
/*        C8             C9             CA             CB */           \
       op(INY, IP, 2) op(CMP, IM, 2) op(DEX, IP, 2) op(UNS, IP, _)     \
/*        CC             CD             CE             CF */           \
       op(CPY, AB, 4) op(CMP, AB, 4) op(DEC, AB, 6) op(UNS, IP, _)     \
                                                                       \
/*        D0*            D1*            D2             D3 */           \
       op(BNE, RE, 2) op(CMP, IY, 5) op(UNS, IP, _) op(UNS, IP, _)     \
/*        D4             D5             D6             D7 */           \
       op(UNS, IP, _) op(CMP, ZX, 4) op(DEC, ZX, 6) op(UNS, IP, _)     \
/*        D8             D9*            DA             DB */           \
       op(CLD, IP, 2) op(CMP, AY, 4) op(UNS, IP, _) op(UNS, IP, _)     \
/*        DC             DD*            DE             DF */           \
       op(UNS, IP, _) op(CMP, AX, 4) op(DEC, AX, 7) op(UNS, IP, _)     \
                                                                       \
/*        E0             E1             E2             E3 */           \
       op(CPX, IM, 2) op(SBC, IX, 6) op(UNS, IP, _) op(UNS, IP, _)     \
/*        E4             E5             E6             E7 */           \
       op(CPX, ZP, 3) op(SBC, ZP, 3) op(INC, ZP, 5) op(UNS, IP, _)     \
/*        E8             E9             EA             EB */           \
       op(INX, IP, 2) op(SBC, IM, 2) op(NOP, IP, 2) op(UNS, IP, _)     \
/*        EC             ED             EE             EF */           \
       op(CPX, AB, 4) op(SBC, AB, 4) op(INC, AB, 6) op(UNS, IP, _)     \
                                                                       \
/*        F0*            F1*            F2             F3 */           \
       op(BEQ, RE, 2) op(SBC, IY, 5) op(UNS, IP, _) op(UNS, IP, _)     \
/*        F4             F5             F6             F7 */           \
       op(UNS, IP, _) op(SBC, ZX, 4) op(INC, ZX, 6) op(UNS, IP, _)     \
/*        F8             F9*            FA             FB */           \
       op(SED, IP, 2) op(SBC, AY, 4) op(UNS, IP, _) op(UNS, IP, _)     \
/*        FC             FD*            FE             FF */           \
       op(UNS, IP, _) op(SBC, AX, 4) op(INC, AX, 7) op(UNS, IP, _)

#endif /* #ifndef VNES_OPCODE_H */

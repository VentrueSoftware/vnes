/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 19-Jul-2013
 * File: opcode.c
 * 
 * Description:
 * 
 *      6502 opcode implementations.
 * 
 * Change Log:
 *      19-Jul-2013:
 *          File created.
 */

#include "opcode.h"
#include "bitwise.h"
#include "mem.h"
#include "cpu.h"

typedef void(*op_func)(u8);

/* Aren't X-macros great?  Saves us a lot of upkeep here. */

/* Define the function prototypes */
#define OP(name, mode, cycles) void Do_##name(u8 opcode);
OPCODE_LIST(OP)
#undef OP

/* Define the jump table */
#define OP(name, mode, cycles) Do_##name,
const static op_func op_fn[] = {
    OPCODE_LIST(OP)
};
#undef OP

/* Define the cycle table (for cycle-accurate emulation) */
#define OP(name, mode, cycles) cycles,
const static u32 op_cyc[] = {
    OPCODE_LIST(OP)
};
#undef OP

/* Define addressing mode table */
#define OP(name, mode, cycles) mode,
const static u8 op_mode[] = {
    OPCODE_LIST(OP)
};
#undef OP

/* Define the string table of opcodes, for debugging purposes */
#define OP(name, mode, cycles) #name,
const static char *op_str[] = {
    OPCODE_LIST(OP)
};
#undef OP

/* Declare a helper function for doing subtract with carry.  Why, you
 * might ask?  Well, ADC, CMP, CPX, CPY, and SBC will all use it. */
inline static u8 Do_Add(u8 op1, u8 op2, u8 c);

/* Okay, now let's get to dispatching the opcodes */
VNES_Err Dispatch_Opcode(u8 opcode) {
    printf("Opcode 0x%02X translates to %s\n", opcode, op_str[opcode]);
    op_fn[opcode](opcode);
    return op_cyc[opcode];
}

/********* Begin the tediousness that is opcode definition ***********/
/* First, define a few macros for accumulator and other registers, for
 * ease of use. */
extern cpu_6502 cpu;

#define A (cpu.a)
#define X (cpu.x)
#define Y (cpu.y)
#define S (cpu.s)
#define P (cpu.p)
#define PC (cpu.pc)

#define GET_ADDRESS() Opcode_Get_Address(op_mode[opcode], 0)
#define GET_VALUE()   Opcode_Get_Value(op_mode[opcode], 0)
#define GET_ADDRESS1() Opcode_Get_Address(op_mode[opcode], 1)
#define GET_VALUE1()   Opcode_Get_Value(op_mode[opcode], 1)


#define GET(flag) FLAG_GET(P, flag)
#define SET(flags) FLAG_SET(P, flags)
#define CLR(flags) FLAG_CLEAR(P, flags)
#define FLG(cond, flags) FLAG_COND(cond, P, flags)
#define FLG_NZ(val)       \
    FLG(!(val), FLG_ZERO);\
    FLG((val) & FLG_SIGN, FLG_SIGN)

/* Obtain addresses and values based on addressing mode */
static u16 Opcode_Get_Address(u8 mode, u8 cross) {
	u16 addr = 0XFFFF;
	u8 op1 = Cpu_Fetch();
	u8 op2;
	
	switch (mode) {
		case ZP:
			return op1;
		break;
		case ZX:
			return (op1 + X) % 0x0100;
		break;
		case ZY:
			return (op1 + Y) % 0x0100;
		break;
		case RE:
			return PC + op1 - 2;
		break;
		case AB:
			op2 = Cpu_Fetch();
			return TO_U16(op1, op2);
		break;
		case AX:
			op2 = Cpu_Fetch();
			/* Check for page crossing */
			return TO_U16(op1, op2) + X;
		break;
		case AY:
			op2 = Cpu_Fetch();
			/* Check for page crossing */
			return TO_U16(op1, op2) + Y;
		break;
		case IN:
			op2 = Cpu_Fetch();
			return Mem_Fetch16(TO_U16(op1, op2));
		break;
		case IX:
			return Mem_Fetch16((u16)((op1 + X) % 0x0100));
		break;
		case IY:
			/* Check for page crossing */
			return Mem_Fetch16((u16)(op1)) + Y;
		break;
		default:
			neslog("Bad addressing mode: %d\n", mode);
	}
	return addr;
}

static u8 Opcode_Get_Value(u8 mode, u8 cross) {
	u16 addr;
	switch (mode) {
		case IM:
			return Cpu_Fetch();
		case AC:
			return A;
		default:
			addr = Opcode_Get_Address(mode, cross);
			return Mem_Fetch(addr);
	}
	return 0xFF;
}

/* Define a macro for opcodes to reduce overhead */
#define DEFINE_OP(name) void Do_##name(u8 opcode)

/* Unsupported opcodes */
DEFINE_OP(UNS) {}

/* Alphabetized for your convenience! */

/* opcode: ADC
 * Description: Add with carry.
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 on Page Cross: AX, AY, IY
 * Flags Affected: C, Z, V, N */
DEFINE_OP(ADC) {
    register u8 v = GET_VALUE();
	A = Do_Add(A, v, (P & FLG_CARRY));

    Cpu_Dump();
}

/* opcode: AND
 * Description: Logical AND
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 on Page Cross: AX, AY, IY
 * Flags Affected: Z, N */ 
DEFINE_OP(AND) {
    A &= GET_VALUE();
    
    /* Update flags */
    FLG_NZ(A);
}

/* opcode: ASL
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(ASL) {}

/* opcode: BCC
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BCC) {}

/* opcode: BCS
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BCS) {}

/* opcode: BEQ
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BEQ) {}

/* opcode: BIT
 * Description: Bit test
 * Address Modes: ZP, AB
 * +1 On Page Cross: 
 * Flags Affected: Z, V, N */ 
DEFINE_OP(BIT) {
    register u8 v = GET_VALUE();
    
    /* Update flags */
    FLG(!(A & v), FLG_ZERO);
    FLG((v & FLG_OVERFLOW), FLG_OVERFLOW);
    FLG((v & FLG_SIGN), FLG_SIGN);
}

/* opcode: BMI
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BMI) {}

/* opcode: BNE
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BNE) {}

/* opcode: BPL
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BPL) {}

/* opcode: BRK
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BRK) {}

/* opcode: BVC
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BVC) {}

/* opcode: BVS
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BVS) {}

/* opcode: CLC
 * Description: Clear Carry flag
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: C */ 
DEFINE_OP(CLC) {
    CLR(FLG_CARRY);
}

/* opcode: CLD
 * Description: Clear Decimal flag
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: D */ 
DEFINE_OP(CLD) {
    CLR(FLG_DECIMAL);
}

/* opcode: CLI
 * Description: Clear Interrupt Disable flag
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: I */ 
DEFINE_OP(CLI) {
    CLR(FLG_INT_DIS);
}

/* opcode: CLV
 * Description: Clear Overflow flag
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: V */ 
DEFINE_OP(CLV) {
    CLR(FLG_OVERFLOW);
}

/* opcode: CMP
 * Description: Comparison - like a SBC, but doesn't store the value.
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 On Page Cross: AX, AY, IY
 * Flags Affected: C, Z, N */ 
DEFINE_OP(CMP) {
    register u8 v = GET_VALUE();
    register u8 vflag = GET(FLG_OVERFLOW);	/* V doesn't change */
    Do_Add(A, ~v, 1);

	FLG(vflag, FLG_OVERFLOW);
}

/* opcode: CPX
 * Description: Compare X register
 * Address Modes: IM, ZP, AB
 * +1 On Page Cross: 
 * Flags Affected: C, Z, N */ 
DEFINE_OP(CPX) {
    register u8 v = GET_VALUE();
    register u8 vflag = GET(FLG_OVERFLOW);	/* V doesn't change */
    Do_Add(X, ~v, 1);

	FLG(vflag, FLG_OVERFLOW);
}

/* opcode: CPY
 * Description: Compare Y register
 * Address Modes: IM, ZP, AB
 * +1 On Page Cross: 
 * Flags Affected: C, Z, N */ 
DEFINE_OP(CPY) {
    register u8 v = GET_VALUE();
    register u8 vflag = GET(FLG_OVERFLOW);	/* V doesn't change */
    Do_Add(Y, ~v, 1);

	FLG(vflag, FLG_OVERFLOW);
}

/* opcode: DEC
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(DEC) {}

/* opcode: DEX
 * Description: Decrement X Register
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */ 
DEFINE_OP(DEX) {
    --X;
    FLG_NZ(X);
}

/* opcode: DEY
 * Description: Decrement Y Register
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */  
DEFINE_OP(DEY) {
    --Y;
    FLG_NZ(Y);
}

/* opcode: EOR
 * Description: Exclusive OR
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 On Page Cross: AX, AY, IY
 * Flags Affected: Z, N */ 
DEFINE_OP(EOR) {
    register u8 v = GET_VALUE();
    
    A ^= v;
    FLG_NZ(A);
}

/* opcode: INC
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(INC) {}

/* opcode: INX
 * Description: Increment X register
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */ 
DEFINE_OP(INX) {
    ++X;
    FLG_NZ(X);
}

/* opcode: INY
 * Description: Increment Y register
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */ 
DEFINE_OP(INY) {
    ++Y;
    FLG_NZ(Y);
}

/* opcode: JMP
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(JMP) {}

/* opcode: JSR
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(JSR) {}

/* opcode: LDA
 * Description: Load Accumulator
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 On Page Cross: AX, AY, IY
 * Flags Affected: Z, N */ 
DEFINE_OP(LDA) {
    A = GET_VALUE();
    FLG_NZ(A);
}

/* opcode: LDX
 * Description: Load X register
 * Address Modes: IM, ZP, ZY, AB, AY
 * +1 On Page Cross: AY
 * Flags Affected: N, Z */ 
DEFINE_OP(LDX) {
    X = GET_VALUE();
    FLG_NZ(X);
}

/* opcode: LDY
 * Description: Load Y register
 * Address Modes: IM, ZP, ZX, AB, X
 * +1 On Page Cross: AY
 * Flags Affected: N, Z */ 
DEFINE_OP(LDY) {
    Y = GET_VALUE();
    FLG_NZ(Y);
}

/* opcode: LSR
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(LSR) {}

/* opcode: NOP
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(NOP) {}

/* opcode: ORA
 * Description: Logical OR with Accumulator
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 On Page Cross: AX, AY, IY
 * Flags Affected: Z, N */ 
DEFINE_OP(ORA) {
    register u8 v = GET_VALUE();
    A |= v;
    FLG_NZ(A);
}

/* opcode: PHA
 * Description: Push Accumulator
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(PHA) {
    //Stack_Push(A);
}

/* opcode: PHP
 * Description: Push Processor status
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(PHP) {
    //Stack_Push(P);
}

/* opcode: PLA
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(PLA) {
    //A = Stack_Pull();
    FLG_NZ(A);
}

/* opcode: PLP
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected: ALLLLLL the flags! */ 
DEFINE_OP(PLP) {
    //P = Stack_Pull();
}

/* opcode: ROL
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(ROL) {}

/* opcode: ROR
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(ROR) {}

/* opcode: RTI
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(RTI) {}

/* opcode: RTS
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(RTS) {}

/* opcode: SBC
 * Description: Subtract with carry
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 On Page Cross: AX, AY, IY
 * Flags Affected: C, Z, N, V */ 
DEFINE_OP(SBC) {
    register u8 v = GET_VALUE();
    A = Do_Add(A, ~v, (P & FLG_CARRY));
    Cpu_Dump();
}

/* opcode: SEC
 * Description: Set Carry flag
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: C */ 
DEFINE_OP(SEC) {
    SET(FLG_CARRY);
}

/* opcode: SED
 * Description: Set Decimal mode flag
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: D */ 
DEFINE_OP(SED) {
    SET(FLG_DECIMAL);
}

/* opcode: SEI
 * Description: Set Interrupt disable flag
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: I */ 
DEFINE_OP(SEI) {
    SET(FLG_INT_DIS);
}

/* opcode: STA
 * Description: Store Accumulator
 * Address Modes: ZP, ZX, AB, AX, AY, IX, IY
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(STA) {
    
}

/* opcode: STX
 * Description: Store X register
 * Address Modes: ZP, ZY, AB
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(STX) {
    
}

/* opcode: STY
 * Description: Store Y register
 * Address Modes: ZP, ZX, AB
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(STY) {

}

/* opcode: TAX
 * Description: Transfer Accumulator to X register
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */ 
DEFINE_OP(TAX) {
    X = A;
    FLG_NZ(X);
}

/* opcode: TAY
 * Description: Transfer Accumulator to Y register
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */ 
DEFINE_OP(TAY) {
    Y = A;
    FLG_NZ(Y);
}

/* opcode: TSX
 * Description: Transfer Stack pointer to X register
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */ 
DEFINE_OP(TSX) {
    X = S;
    FLG_NZ(X);
}

/* opcode: TXA
 * Description: Transfer X register to Accumulator
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */ 
DEFINE_OP(TXA) {
    A = X;
    FLG_NZ(A);
}

/* opcode: TXS
 * Description: Transfer X register to Stack pointer
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: */ 
DEFINE_OP(TXS) {
    S = X;
}

/* opcode: TYA
 * Description: Transfer Y register to Accumulator
 * Address Modes: IP
 * +1 On Page Cross: 
 * Flags Affected: Z, N */ 
DEFINE_OP(TYA) {
    A = Y;
    FLG_NZ(A);
}


/* We might use these somewhere else, so undefine them. */

/* Do the addition */
inline static u8 Do_Add(u8 op1, u8 op2, u8 c) {
    register u16 s = (u16)op1 + (u16)op2 + (u16)c;
    
    /* Update Overflow and carry.  Overflow is a bit tricky. */
    FLG((op1 ^ s) & (op2 ^ s) & 0x80, FLG_OVERFLOW);
    FLG((s > 0xFF), FLG_CARRY);
    
    /* Update N and Z */
    FLG_NZ(s);
    
    return (u8)s;
}

#undef A
#undef X
#undef Y
#undef S
#undef P
#undef PC

#undef SET
#undef CLR
#undef FLG
#undef FLG_NZ


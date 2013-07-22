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
INLINED static u8 Do_Add(u8 op1, u8 op2, u8 c);

/* Helper function for branching; decreases verbosity of the source. */
INLINED static void Do_Branch(u8 cond, u8 opcode);

INLINED static void Push_Stack(u8 v);
INLINED static u8 Pull_Stack();

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

#define MODE (op_mode[opcode])

#define GET_ADDRESS() Opcode_Get_Address(MODE, 0)
#define GET_VALUE()   Opcode_Get_Value(MODE, 0)
#define GET_ADDRESS1() Opcode_Get_Address(MODE, 1)
#define GET_VALUE1()   Opcode_Get_Value(MODE, 1)

#define STACK_PAGE 0x0100

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
	//u8 op2;
	
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
			return PC + (i8)op1;
		break;
		case AB:
			return TO_U16(op1, Cpu_Fetch());
		break;
		case AX:
			addr = TO_U16(op1, Cpu_Fetch());
			/* Check for page crossing */
			if (cross && (PAGE_OF(addr) != PAGE_OF(addr + X))) {
				Cpu_Add_Cycles(1);
			}
			return addr + X;
		break;
		case AY:
			addr = TO_U16(op1, Cpu_Fetch());
			/* Check for page crossing */
			if (cross && (PAGE_OF(addr) != PAGE_OF(addr + Y))) {
				Cpu_Add_Cycles(1);
			}
			return addr + Y;
		break;
		case IN:
            /* Indirect has a bug where it wraps to the same page for
             * the jump vector if it falls on the boundary of a page. */
            addr = TO_U16(op1, Cpu_Fetch());
            if (op1 == 0xFF) {
                return TO_U16(Mem_Fetch(addr), Mem_Fetch(addr & 0xFF00));
            }
			return Mem_Fetch16(TO_U16(op1, Cpu_Fetch()));
		break;
		case IX:
			return Mem_Fetch16((u16)((op1 + X) % 0x0100));
		break;
		case IY:
			/* Check for page crossing */
			addr = Mem_Fetch16((u16)(op1));
			if (cross && (PAGE_OF(addr) != PAGE_OF(addr + Y))) {
				Cpu_Add_Cycles(1);
			}
			return addr + Y;
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
    register u8 v = GET_VALUE1();
	A = Do_Add(A, v, (P & FLG_CARRY));

    Cpu_Dump();
}

/* opcode: AND
 * Description: Logical AND
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 on Page Cross: AX, AY, IY
 * Flags Affected: Z, N */ 
DEFINE_OP(AND) {
    A &= GET_VALUE1();
    
    /* Update flags */
    FLG_NZ(A);
}

/* opcode: ASL
 * Description: Arithmetic shift left
 * Address Modes: AC ZP ZX AB AX
 * Flags Affected: C, Z, N */ 
DEFINE_OP(ASL) {
    /* No memory fetches needed in accumulator mode */
    if (MODE == AC) {
        FLG((A & FLG_SIGN), FLG_CARRY);
        A <<= 1;
        FLG_NZ(A);
    } else {
        /* Otherwise, we do a mem fetch and a mem set */
        register u16 addr = GET_ADDRESS();
        register u8 v = Mem_Fetch(addr);
        FLG((v & FLG_SIGN), FLG_CARRY);
        v <<= 1;
        FLG_NZ(v);
        Mem_Set(addr, v);
    }
}

/* opcode: BCC
 * Description: Branch if Carry Clear
 * Address Modes: RE */
DEFINE_OP(BCC) { Do_Branch(!(P & FLG_CARRY), opcode); }

/* opcode: BCS
 * Description: Branch if Carry Set
 * Address Modes: RE */ 
DEFINE_OP(BCS) { Do_Branch(P & FLG_CARRY, opcode); }

/* opcode: BEQ
 * Description: Branch if Equal
 * Address Modes: RE */ 
DEFINE_OP(BEQ) { Do_Branch(P & FLG_ZERO, opcode); }

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
 * Description: Branch if Minus (negative)
 * Address Modes: RE */ 
DEFINE_OP(BMI) { Do_Branch(P & FLG_SIGN, opcode); }

/* opcode: BNE
 * Description: Branch if not equal
 * Address Modes: RE */ 
DEFINE_OP(BNE) { Do_Branch(!(P & FLG_ZERO), opcode); }

/* opcode: BPL
 * Description: Branch if Plus (positive)
 * Address Modes: RE */ 
DEFINE_OP(BPL) { Do_Branch(!(P & FLG_SIGN), opcode); }

/* opcode: BRK
 * Description: 
 * Address Modes: 
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(BRK) {}

/* opcode: BVC
 * Description: Branch if Overflow clear
 * Address Modes: RE */ 
DEFINE_OP(BVC) { Do_Branch(!(P & FLG_OVERFLOW), opcode); }

/* opcode: BVS
 * Description: Branch if Overflow set
 * Address Modes: RE */ 
DEFINE_OP(BVS) { Do_Branch(P & FLG_OVERFLOW, opcode); }

/* opcode: CLC
 * Description: Clear Carry flag
 * Address Modes: IP
 * Flags Affected: C */ 
DEFINE_OP(CLC) { CLR(FLG_CARRY); }

/* opcode: CLD
 * Description: Clear Decimal flag
 * Address Modes: IP
 * Flags Affected: D */ 
DEFINE_OP(CLD) { CLR(FLG_DECIMAL); }

/* opcode: CLI
 * Description: Clear Interrupt Disable flag
 * Address Modes: IP
 * Flags Affected: I */ 
DEFINE_OP(CLI) { CLR(FLG_INT_DIS); }

/* opcode: CLV
 * Description: Clear Overflow flag
 * Address Modes: IP
 * Flags Affected: V */ 
DEFINE_OP(CLV) { CLR(FLG_OVERFLOW); }

/* opcode: CMP
 * Description: Comparison - like a SBC, but doesn't store the value.
 * Address Modes: IM, ZP, ZX, AB, AX, AY, IX, IY
 * +1 On Page Cross: AX, AY, IY
 * Flags Affected: C, Z, N */ 
DEFINE_OP(CMP) {
    register u8 v = GET_VALUE1();
    register u8 vflag = GET(FLG_OVERFLOW);	/* V doesn't change */
    Do_Add(A, ~v, 1);

	FLG(vflag, FLG_OVERFLOW);
}

/* opcode: CPX
 * Description: Compare X register
 * Address Modes: IM, ZP, AB
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
 * Flags Affected: C, Z, N */ 
DEFINE_OP(CPY) {
    register u8 v = GET_VALUE();
    register u8 vflag = GET(FLG_OVERFLOW);	/* V doesn't change */
    Do_Add(Y, ~v, 1);

	FLG(vflag, FLG_OVERFLOW);
}

/* opcode: DEC
 * Description: Decreases contents of memory by 1
 * Address Modes: ZP ZX AB AX
 * Flags Affected: Z, N  */ 
DEFINE_OP(DEC) {
    register u16 addr = GET_ADDRESS();
    register u8 v = Mem_Fetch(addr);
    --v;
    FLG_NZ(v);
    Mem_Set(addr, v);
}

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
    register u8 v = GET_VALUE1();
    
    A ^= v;
    FLG_NZ(A);
}

/* opcode: INC
 * Description: Increase contents of memory by 1
 * Address Modes: ZP, ZX, AB, AX
 * Flags Affected: Z, N */ 
DEFINE_OP(INC) {
    register u16 addr = GET_ADDRESS();
    register u8 v = Mem_Fetch(addr);
    ++v;
    FLG_NZ(v);
    Mem_Set(addr, v);
}

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
 * Description: Jump to target address
 * Address Modes: AB IN */ 
DEFINE_OP(JMP) {
    PC = GET_ADDRESS();
}

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
    A = GET_VALUE1();
    FLG_NZ(A);
}

/* opcode: LDX
 * Description: Load X register
 * Address Modes: IM, ZP, ZY, AB, AY
 * +1 On Page Cross: AY
 * Flags Affected: N, Z */ 
DEFINE_OP(LDX) {
    X = GET_VALUE1();
    FLG_NZ(X);
}

/* opcode: LDY
 * Description: Load Y register
 * Address Modes: IM, ZP, ZX, AB, X
 * +1 On Page Cross: AX
 * Flags Affected: N, Z */ 
DEFINE_OP(LDY) {
    Y = GET_VALUE1();
    FLG_NZ(Y);
}

/* opcode: LSR
 * Description: Logical shift right
 * Address Modes: AC, ZP, ZX, AB, AX
 * Flags Affected: C, Z, N */ 
DEFINE_OP(LSR) {
    /* No memory fetches needed if accumulator mode */
    if (MODE == AC) {
        FLG((A & FLG_CARRY), FLG_CARRY);
        A >>= 1;
        FLG_NZ(A);
    } else {
        /* Memory is fetched, in order to manipulate it. */
        register u16 addr = GET_ADDRESS();
        register u8 v = Mem_Fetch(addr);
        FLG((v & FLG_CARRY), FLG_CARRY);
        v >>= 1;
        FLG_NZ(v);
        Mem_Set(addr, v);
    }
}

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
    register u8 v = GET_VALUE1();
    A |= v;
    FLG_NZ(A);
}

/* opcode: PHA
 * Description: Push Accumulator to stack
 * Address Modes: IP */ 
DEFINE_OP(PHA) { Push_Stack(A); }

/* opcode: PHP
 * Description: Push Processor status to stack
 * Address Modes: IP */ 
DEFINE_OP(PHP) { Push_Stack(P); }

/* opcode: PLA
 * Description: Pull Accumulator from stack
 * Address Modes: IP
 * Flags Affected: Z, N */ 
DEFINE_OP(PLA) {
    A = Pull_Stack();
    FLG_NZ(A);
}

/* opcode: PLP
 * Description: Pull Processor status from stack
 * Address Modes: IP
 * Flags Affected: ALLLLLL the flags! */ 
DEFINE_OP(PLP) {
    P = Pull_Stack();
}

/* opcode: ROL
 * Description: Rotate left
 * Address Modes: AC, ZP, ZX, AB, AX
 * Flags Affected: C, Z, N */ 
DEFINE_OP(ROL) {
    /* Save the old carry value for use in the rotation */
    register u8 c = P & FLG_CARRY;
    
    /* If accumulator mode, no memory access required */
    if (MODE == AC) {
        FLG((A & FLG_SIGN), FLG_CARRY);
        A = (A << 1) | c;
        FLG_NZ(A);
    } else {
        register u16 addr = GET_ADDRESS();
        register u8 v = Mem_Fetch(addr);
        FLG((v & FLG_SIGN), FLG_CARRY);
        v = (v << 1) | c;
        FLG_NZ(v);
        Mem_Set(addr, v);
    }
}

/* opcode: ROR
 * Description: Rotate right
 * Address Modes: AC, ZP, ZX, AB, AX
 * Flags Affected: C, Z, N */ 
DEFINE_OP(ROR) {
    /* Save the old carry value for use in the rotation */
    register u8 c = (P & FLG_CARRY) << 7;
    
    /* If accumulator mode, no memory access required */
    if (MODE == AC) {
        FLG((A & FLG_CARRY), FLG_CARRY);
        A = (A >> 1) | c;
        FLG_NZ(A);
    } else {
        register u16 addr = GET_ADDRESS();
        register u8 v = Mem_Fetch(addr);
        FLG((v & FLG_CARRY), FLG_CARRY);
        v = (v >> 1) | c;
        FLG_NZ(v);
        Mem_Set(addr, v);
    }
}

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
 * Address Modes: ZP, ZX, AB, AX, AY, IX, IY */ 
DEFINE_OP(STA) {
    register u16 addr = GET_ADDRESS();
    Mem_Set(addr, A);
}

/* opcode: STX
 * Description: Store X register
 * Address Modes: ZP, ZY, AB */ 
DEFINE_OP(STX) {
    register u16 addr = GET_ADDRESS();
    Mem_Set(addr, X);
}

/* opcode: STY
 * Description: Store Y register
 * Address Modes: ZP, ZX, AB
 * +1 On Page Cross: 
 * Flags Affected:  */ 
DEFINE_OP(STY) {
    register u16 addr = GET_ADDRESS();
    Mem_Set(addr, Y);
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
INLINED static u8 Do_Add(u8 op1, u8 op2, u8 c) {
    register u16 s = (u16)op1 + (u16)op2 + (u16)c;
    
    /* Update Overflow and carry.  Overflow is a bit tricky. */
    FLG((op1 ^ s) & (op2 ^ s) & 0x80, FLG_OVERFLOW);
    FLG((s > 0xFF), FLG_CARRY);
    
    /* Update N and Z */
    FLG_NZ(s);
    
    return (u8)s;
}

/* Do the branch - makes source code shorter */
INLINED static void Do_Branch(u8 cond, u8 opcode) {
    register u16 addr = GET_ADDRESS();
    
    /* Determine whether we take the branch. +1 cycle if branch taken. */
    if (cond) {
        /* If we're crossing a page, we add another cycle. */
        Cpu_Add_Cycles((PAGE_OF(PC) == PAGE_OF(addr)) ? 1 : 2);
        PC = addr;
    }    
}

/* Stack-related functions */
INLINED static void Push_Stack(u8 v) {
    Mem_Set(STACK_PAGE | S, v);
    S--;
}

INLINED static u8 Pull_Stack() {
    return Mem_Fetch(STACK_PAGE | --S);
}

#undef A
#undef X
#undef Y
#undef S
#undef P
#undef PC

#undef MODE

#undef SET
#undef CLR
#undef FLG
#undef FLG_NZ


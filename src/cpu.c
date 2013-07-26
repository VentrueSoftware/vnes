/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 18-Jul-2013
 * File: cpu.c
 * 
 * Description:
 * 
 *      Defines the CPU state machine.  This emulates the behavior of
 *      the MOS Technology 6502, or more specifically, the second-source
 *      version by Ricoh, which lacks binary-coded decimal but provides
 *      on-chip sound generation, joypad reading, and DMA for sprites.
 * 
 * Change Log:
 *      18-Jul-2013:
 *          File created.
 */

#include "cpu.h"
#include "mem.h"
#include "opcode.h"
#include "bitwise.h"

#define CPU_STACK_INIT 0xFD
#define CPU_STATUS_INIT 0x34
#define CPU_PC_RESET Mem_Fetch16(0xFFFC)

/* Instance of the cpu */
cpu_6502 cpu;

/* Func: void cpu_init(void)
 * Desc: sets the cpu into it's initial state. Note that this is not the
 * same as pressing the reset button. */
INLINED void Cpu_Init(void) {
    cpu.a = cpu.x = cpu.y = 0;
    cpu.s = CPU_STACK_INIT;
    cpu.p = CPU_STATUS_INIT;
    
    cpu.pc = CPU_PC_RESET;
    neslog("CPU Initialized.\n");
}

/* Func: void cpu_reset(void)
 * Desc: Alters cpu state to emulate pressing the "reset" button. */
INLINED void Cpu_Reset(void) {
    cpu.s -= 3;
    /* FLAG_SET(cpu.p, IRQ_DISABLE); */
    cpu.pc = CPU_PC_RESET;
    neslog("CPU Reset.\n");
}

/* Func: u8 cpu_fetch(void)
 * Desc: Fetches the next byte from memory. */
INLINED u8 Cpu_Fetch(void) {
    return Mem_Fetch(cpu.pc++);
} 

INLINED void Cpu_Add_Cycles(u32 cycles) {
    cpu.cycles += cycles;
}

INLINED VNES_Err Cpu_Step(void) {
    return Dispatch_Opcode(Cpu_Fetch());
}

/* Func: VNES_Err Cpu_Run(void)
 * Desc: Runs the cpu, performing instructions, handling interrupts, and
 *       the like. */
void Cpu_Run(void) {
    cpu.state = 1;
    while (cpu.state) {
        /* Handle next instruction */
        if (!Cpu_Step()) {
			neslog("\n============[End of CPU Execution]===========\n");
			break;
		}
        
        /* Check scanline/interrupts */
    }
}

void Cpu_Dump(void) {
    return;
    printf(
        "[CPU] SV BDIZC\t  PC     A     X     Y     SP\n"
        "      %c%c%c%c%c%c%c%c\t0x%04X  0x%02X  0x%02X  0x%02X  0x%02X\n",
        (cpu.p & FLG_SIGN) ? 'x' : '-',
        (cpu.p & FLG_OVERFLOW) ? 'x' : '-',
        (cpu.p & FLG_NOT_USED) ? 'x' : '-',
        (cpu.p & FLG_BRK) ? 'x' : '-',
        (cpu.p & FLG_DECIMAL) ? 'x' : '-',
        (cpu.p & FLG_INT_DIS) ? 'x' : '-',
        (cpu.p & FLG_ZERO) ? 'x' : '-',
        (cpu.p & FLG_CARRY) ? 'x' : '-',
        cpu.pc, cpu.a, cpu.x, cpu.y, cpu.s);
}

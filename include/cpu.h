/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 18-Jul-2013
 * File: cpu.h
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

#ifndef VNES_CPU_H
#define VNES_CPU_H

#include "types.h"

#define FLG_SIGN        0x80
#define FLG_OVERFLOW    0x40
#define FLG_NOT_USED    0x20
#define FLG_BRK         0x10
#define FLG_DECIMAL     0x08
#define FLG_INT_DIS     0x04
#define FLG_ZERO        0x02
#define FLG_CARRY       0x01

typedef struct cpu_6502 {    
    u8 a;       /* Accumulator      */
    u8 x;       /* X Index Register */
    u8 y;       /* Y Index Register */
    u8 p;       /* Status Register  */
    u8 s;       /* Stack Pointer    */
    u16 pc;     /* Program Counter  */

    u8 state;   /* VNES CPU State   */
    u32 cycles; /* Total number of cycles */    
} cpu_6502;

/* Initialization Functions */
INLINED void Cpu_Init(void);
INLINED void Cpu_Reset(void);

/* Byte fetching */
INLINED u8 Cpu_Fetch(void);

INLINED void Cpu_Add_Cycles(u32 cycles);

INLINED VNES_Err Cpu_Step(void);

void Cpu_Run(void);

void Cpu_Dump(void);
#endif /* #ifndef VNES_CPU_H */

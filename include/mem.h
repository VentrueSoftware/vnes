/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 18-Jul-2013
 * File: mem.h
 * 
 * Description:
 * 
 *      Emulates the memory hierarchy for the NES.
 * 
 * Change Log:
 *      18-Jul-2013:
 *          File created.
 */

#ifndef VNES_MEM_H
#define VNES_MEM_H

#include "types.h"

/* Special addresses */
#define PPUCTRL   0x2000
#define PPUMASK   0x2001
#define PPUSTATUS 0x2002
#define OAMADDR   0x2003
#define OAMDATA   0x2004
#define PPUSCROLL 0x2005
#define PPUADDR   0x2006
#define PPUDATA   0x2007

INLINED void Mem_Init(void);
INLINED void Mem_Reset(void);

INLINED u8 Mem_Fetch(u16 address);
INLINED u16 Mem_Fetch16(u16 address);

INLINED void Mem_Set(u16 address, u8 value);
INLINED void Mem_Set16(u16 address, u16 value);

INLINED u8 *Mem_Get_Ptr(u16 address);

void Mem_Dump(void);

#endif /* #ifndef VNES_MEM_H */

/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 18-Jul-2013
 * File: mem.c
 * 
 * Description:
 * 
 *      Emulates the memory hierarchy for the NES.
 * 
 * Change Log:
 *      18-Jul-2013:
 *          File created.
 */

#include "mem.h"
#include "ppu.h"
#include "bitwise.h"
#include "cart.h"
#include <string.h>

#define INTERNAL_MEM_SIZE 0x800

static const u8 INTERNAL_MEM_INIT[] = {0xF7, 0xEF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF};

static u8 internal_mem[INTERNAL_MEM_SIZE];


/* Begin Functions */

INLINED void Mem_Init(void) {
    memset(internal_mem, 0xFF, INTERNAL_MEM_SIZE);
    memcpy(internal_mem + 8, INTERNAL_MEM_INIT, 8);
    neslog("Memory initialized.\n");
}

INLINED void Mem_Reset(void) {
    neslog("Memory reset.\n");
}

INLINED u8 Mem_Fetch(u16 address) {
    /* Memory mapping should go here, as things are implemented. */
    if (address < 0x2000) return internal_mem[address % INTERNAL_MEM_SIZE];
    else if (address > 0x4020) return Read_Cartridge_Prg(address);
    else if (address < 0x2008) return Read_Ppu(address);
    else {
        //printf("Unassigned memory partition mapped: 0x%04X\n", address);
    }
    return 0x00;
}

INLINED u16 Mem_Fetch16(u16 address) {
    /* Don't forget to test page wraparounds */
    return TO_U16(Mem_Fetch(address), Mem_Fetch(address + 1));
}

INLINED void Mem_Set(u16 address, u8 value) {
    /* Memory Mapping and the whatnot */
    if (address < 0x2000) internal_mem[address % INTERNAL_MEM_SIZE] = value;
    else if (address < 0x2008) {
        Write_Ppu(address, value);
    } else {
        //printf("Unassigned memory partition mapped: 0x%04X\n", address);
    }
}

INLINED void Mem_Set16(u16 address, u16 value) {
    Mem_Set(address, LB(value));
    Mem_Set(address + 1, UB(value));
}

INLINED u8 *Mem_Get_Ptr(u16 address) {
    return &(internal_mem[address % INTERNAL_MEM_SIZE]);
}

/* Dumps all of memory.  ALL of it. */
void Mem_Dump(void) {
    u16 address = 0;
    u16 line = 0;
    /* Print a header out */
    printf("-----------------------------------------------------------------------------------------\n");
    printf("  MEM  | 0x-0 0x-1 0x-2 0x-3 0x-4 0x-5 0x-6 0x-7 0x-8 0x-9 0x-A 0x-B 0x-C 0x-D 0x-E 0x-F \n");
    printf("-----------------------------------------------------------------------------------------");
    while (address < INTERNAL_MEM_SIZE) {
        
        if (!(address & 0x0F)) {
            printf("\n0x%03X- | ", address >> 4);
            line++;
        }
        
        printf("0x%02X ", Mem_Fetch(address++));
    }
    printf("\n");
}

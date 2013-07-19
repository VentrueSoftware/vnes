/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 18-Jul-2013
 * File: vnes.c
 * 
 * Description:
 * 
 *      NES Emulator main file.
 * 
 * Change Log:
 *      18-Jul-2013:
 *          File created.
 */

#include "types.h"
#include "cpu.h"
#include "mem.h"

void VNES_Init(void) {
    neslog("Starting emulation...");
    Cpu_Init();
    Mem_Init();
}

int main(int argc, char **argv) {
    VNES_Init();
    Mem_Set(0x0000, 0x69);
    Mem_Set(0x0001, 0x50);
    Mem_Set(0x0002, 0xE9);
    Mem_Set(0x0003, 0x10);
    Cpu_Run();
    Cpu_Dump();
    return 0;
}

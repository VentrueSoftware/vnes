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

#include <string.h>
#include "types.h"
#include "cpu.h"
#include "ppu.h"
#include "mem.h"
#include "dbg.h"
#include "cart.h"

void VNES_Init(void) {
    neslog("Starting emulation...");
    Cpu_Init();
    Ppu_Init();
    Mem_Init();
}

int main(int argc, char **argv) {
    if (argc > 1) {
		if (0 == strcmp(argv[1], "--test")) {
			extern cpu_6502 cpu;
			Load_Cartridge("roms/nestest.nes");
			VNES_Init();
			cpu.pc = 0xC000;
		} else {
			Load_Cartridge(argv[1]);
			VNES_Init();
		}
    }
    Start_Dbg();
    return 0;
}

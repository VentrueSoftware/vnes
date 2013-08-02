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
#include "render.h"

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
            extern ppu_2c02 ppu;
			Load_Cartridge("roms/nestest.nes");
			VNES_Init();
			cpu.pc = 0xC000;
            ppu.scanline = 241;
		} else {
			Load_Cartridge(argv[1]);
            if ((argc > 2) && 0 == strcmp(argv[2], "--ptdump")) {
                Dump_Pattern_Tables();
                return 0;
            }
			VNES_Init();
		}
    }
    Start_Dbg();
    return 0;
}

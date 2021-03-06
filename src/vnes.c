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
#include "display.h"

void VNES_Init(void) {
    neslog("Starting emulation...");
    Cpu_Init();
    Ppu_Init();
    Mem_Init();
}

int main(int argc, char **argv) {
    Load_Cartridge(argv[1]);
    VNES_Init();
    Start_Debug(0);
    return 0;
}

#if 0
int main(int argc, char **argv) {
    vnes_display *disp;
    if (argc > 1) {
        if (0 == strcmp(argv[1], "--disptest")) {
            u32 testsrc[16] = {0xFF00FF00, 0x00FF00FF, 0x00FF00FF, 0xFF00FF00, 0xFF00FF00, 0x00FF00FF, 0x00FF00FF, 0xFF00FF00, 0xFF00FF00, 0x00FF00FF, 0x00FF00FF, 0xFF00FF00, 0xFF00FF00, 0x00FF00FF, 0x00FF00FF, 0xFF00FF00};
            Open_Display(&disp, 512, 480);
            Set_Display_Source(disp, testsrc, 4, 4);
            Display_Loop(disp);
            return 0;
        }
        Load_Cartridge(argv[1]);
        VNES_Init();
        Start_Debug(0);
		else if (0 == strcmp(argv[1], "--test")) {
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
#endif 

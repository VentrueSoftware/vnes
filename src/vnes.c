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
#include "dbg.h"
#include "cart.h"

void VNES_Init(void) {
    neslog("Starting emulation...");
    Cpu_Init();
    Mem_Init();
}

int main(int argc, char **argv) {
    if (argc > 1) {
        Load_Cartridge(argv[1]);
    }
    VNES_Init();
    Start_Dbg();
    return 0;
}

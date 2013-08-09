/* Project: VNES
 * Author: Kurt Sassenrath
 * Created: 19-Jul-2013
 * File: dbg.c
 * 
 * Description:
 * 
 *      VNES Emulator debug console.
 * 
 * Change Log:
 *      22-Jul-2013:
 *          File created.
 *      26-Jul-2013:
 *          Removed ncurses GUI requirement.  It still uses ncurses, but
 *          only for things like hiding input and minor formatting.
 *      08-Aug-2013:
 *          Begin rewriting debugger - support either display event loop
 *          or ncurses event loop, based on options provided.
 */

#include <string.h>
#include "dbg.h"
#include "bitwise.h"
#include "display.h"
#include "render.h"
#include "ppu.h"
#include "cpu.h"

/* From ppu.c */
extern ppu_2c02 ppu;
extern cpu_6502 cpu;

#define NO_GFX 0x00000001

vnes_display *disp__;

INLINED int Handle_Debug_Input(vnes_display *disp, const char *cmd);

void Start_Debug(u32 flags) {
    if (IS_SET(flags, NO_GFX)) {
        /* Ncurses debug loop */
    } else {
        u32 *render_buffer = Get_Render_Buffer();
        Open_Display(&disp__, NES_RES_X * 2, NES_RES_Y * 2);
        Display_Loop(disp__, Handle_Debug_Input);
    }
}

void End_Debug(int sig) {
    Close_Display(disp__);
}

INLINED int Handle_Debug_Input(vnes_display *disp, const char *cmd) {
    /* Handle single-character input */
    if (!cmd[1]) {
        switch (*cmd) {
            case 'q': End_Debug(0); return 0;
            case 'f': {
                printf("Rendering next frame...\n");
                ppu.frame_check = 1;
                while (ppu.frame_check) {
                    Cpu_Step();
                }
                if (ppu.mask & (SHOW_BG | SHOW_SPRITES)) {
                    Set_Display_Source(disp, Get_Render_Buffer(), NES_RES_X, NES_RES_Y);
                    Update_Display(disp);
                    Set_Display_Title(disp, "[VNES - %ux%u] Frame: %u", disp->src.width, disp->src.height, ppu.frame);
                }
                break;
            }
        }
    }
    return 1;
}

void Log_Line(const char *format, ...) {}

/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 23-Jul-2013
 * File: dbg-gui.c
 * 
 * Description:
 * 
 *      VNES Emulator debugger GUI layout and draw functions.  The GUI
 *      main layout looks like this:
 * 
 *  ╔═════════════════════[Memory Viewer]══════════════════════╗
 *  ║ Offset   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ║
 *  ║        ┌─────────────────────────────────────────────────╢
 *  ║ 0x0130 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x0140 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x0150 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x0160 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x0170 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x0180 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x0190 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x01a0 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x01b0 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ║ 0x01c0 │ ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ║
 *  ╚════════╧═════════════════════════════════════════════════╝
 *  ╔═══════════════════════[CPU Info]═════════════════════════╗
 *  ║  PC   OP        INST            A   X   Y   SP  NV-BDIZC ║
 *  ║ 013A  FF        UNS             90  00  00  FC  x-xx-x-x ║
 *  ╚══════════════════════════════════════════════════════════╝
 * 
 * Change Log:
 *      23-Jul-2013:
 *          File created.
 */

#include <assert.h>
#include <string.h>
#include "dbg-gui.h"

int main(int argc, char **argv) {
    pane *pane_main, *pane_memframe, *pane_memdata, *pane_cpuinfo;
    
    initscr();
    nonl();
    cbreak();
    noecho();
    curs_set(0);    

    if (has_colors())
    {
        start_color();

        init_pair(1, COLOR_WHITE,   COLOR_BLACK);
        init_pair(2, COLOR_GREEN,  COLOR_BLACK);
    }

    assert((pane_main = Pane_Create(ROOT_PANE)));
    assert((pane_memframe = Pane_Create(pane_main, 0, 1, 1, 60, 13)));
    assert((pane_memdata = Pane_Create(pane_memframe, 0, 9, 2, PANE_MERGE, PANE_MERGE)));
    assert((pane_cpuinfo = Pane_Create(pane_main, 0, 1, 14, 60, 4)));

    assert((Pane_Title(pane_main, "[VNES Viewer]")));
    assert((Pane_Title(pane_memframe, "[Memory Viewer]")));
    assert((Pane_Title(pane_cpuinfo, "[CPU Info]")));

    assert(Pane_Draw(pane_main));

    while (1);
}

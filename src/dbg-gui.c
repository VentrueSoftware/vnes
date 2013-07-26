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

#include <signal.h>
#include <assert.h>
#include <string.h>
#include "dbg-gui.h"
#include "types.h"

#define COLOR_HIGHLIGHT 2

pane *pane_main, *pane_memframe, *pane_memdata, *pane_cpuinfo;

void Resize_Dbg_Gui(int signal) {
	Pane_Resize(pane_main);
    Pane_Draw(pane_main);
}

void Init_Dbg_Gui(void) {
	/* Initialize ncurses */
    initscr();
    nonl();
    cbreak();
    noecho();
    curs_set(0);    

	/* Initialize color pairs used by debugger */
    if (has_colors())
    {
        start_color();

        init_pair(1, COLOR_WHITE,   COLOR_BLACK);
        init_pair(COLOR_HIGHLIGHT, COLOR_GREEN,  COLOR_BLACK);
    }

	/* Set resize signal handler */
	signal(SIGWINCH, Resize_Dbg_Gui);

	/* Create the panes */
    pane_main = Pane_Create(ROOT_PANE);
    pane_memframe = Pane_Create(pane_main, 0, 1, 1, PANE_STRETCH, 14);
    pane_memdata = Pane_Create(pane_memframe, 0, 9, 2, PANE_MERGE, PANE_MERGE);
    pane_cpuinfo = Pane_Create(pane_main, 0, 1, 15, 60, 4);

	/* Set pane titles */
    pane_main->title = "VNES Debugger";
    pane_memframe->title = "[Memory Viewer]";
    pane_cpuinfo->title = "[CPU Info]";

	/* Draw the main pane. */
    Pane_Draw(pane_main);
}

void Update_Memframe(u16 start) {
	u16 i;
	Pane_Mv_Print(pane_memframe, 1, 1, " Offset   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ");
	for (i = 0; i < 10; i++) {
		Pane_Mv_Print(pane_memframe, 3 + i, 1, " 0x%04X ", (u16)((start & 0xFFF0) + (i * 16)));
	}
	Pane_Draw(pane_memframe);
}

void Update_Memdata_Page(u8 *start, u8 *end) {
	int i = 0;
    if (end > start) {
        for (; i < (u16)(end - start) && i < 16 * 10; i++) {
            Pane_Mv_Print(pane_memdata, 1 + (i / 16), 2 + ((i % 16) * 3), "%02X ", start[i]);
        }
    }
	for (; i < 16 * 10; i++) {
		Pane_Mv_Print(pane_memdata, 1 + (i / 16), 2 + ((i % 16) * 3), "   ");
	}
	Pane_Draw(pane_memdata);
}

void Highlight_Inst(u16 offset, u16 length) {
    Pane_Highlight(pane_memdata, 1 + (offset / 16), 1 + ((offset % 16) * 3), length * 3, COLOR_HIGHLIGHT);
    Pane_Draw(pane_memdata);
}

u8 test[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6
};

int main(int argc, char **argv) {
	u16 i = 0;
	Init_Dbg_Gui();
	while(1) {
		i+=16;
		Update_Memframe(i);
		Update_Memdata_Page(test + (i & 0xFFF0), test + sizeof(test));
        Highlight_Inst(i / 16, (i % 3) + 1);
		wgetch(pane_memdata->win);
	}
}

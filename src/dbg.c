/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 19-Jul-2013
 * File: dbg.c
 * 
 * Description:
 * 
 *      VNES Emulator debugger.  This interactive console allows you to
 *      step through CPU instructions, hex-dump areas of memory, modify
 *      contents directly in memory, and  the status of the various
 *      components of the emulator.
 * 
 * Change Log:
 *      22-Jul-2013:
 *          File created.
 */

#include <curses.h> /* Only available with ncurses library for now */
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "dbg.h"
#include "types.h"

#define MEM_TITLE "[Memory Viewer]"
#define CPU_TITLE "[CPU Info]"


static vnes_dbg dbg;

static void Initialize_Dbg_Windows(void);

/* Redraw routines */
static void Redraw_Debugger(void);
static void Redraw_Mem(void);
static void Redraw_Cpu(void);

static void finish(int sig)
{
    if (dbg.mem.win) { delwin(dbg.mem.win); }	
    if (dbg.cpu.win) { delwin(dbg.cpu.win); }	
    endwin();
    /* do your non-curses wrapup here */

    exit(0);
}

static void resize(int sig) {
    if (dbg.mem.win) { delwin(dbg.mem.win); }	
    if (dbg.cpu.win) { delwin(dbg.cpu.win); }	
	endwin();
	
	Initialize_Dbg_Windows();
	Redraw_Debugger();
}

static void Dbg(void) {
	int c = 0;
	while ((c = wgetch(dbg.cpu.win)) != 'q') {
		switch (c) {
			case 's':
				dbg.cpu.pc++;
				Redraw_Mem();
				Redraw_Cpu();
				break;
			case 'g':
				//Dbg_Goto();
			default: continue;
		}
        wrefresh(dbg.mem.win);
        wrefresh(dbg.cpu.win);
	}
}

void Initialize_Dbg(void) {
    u8 hexvals[10 * 16];
    signal(SIGINT, finish);      /* arrange interrupts to terminate */
    signal(SIGWINCH, resize);      /* arrange interrupts to terminate */

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
    
    dbg.mem.datap = hexvals;

	Initialize_Dbg_Windows();
    Redraw_Debugger();

	Dbg();

    finish(0);               /* we're done */
}

/* Subwindow initialization */
static void Initialize_Dbg_Windows(void) {
    getmaxyx(stdscr, dbg.term.height, dbg.term.width);
    
    dbg.mem.win = newwin(12, 60, 1, 1);
    dbg.mem.width = 60;
    dbg.mem.height = 12;	
    
    dbg.cpu.win = newwin(4, 60, 13, 1);
    dbg.cpu.width = 60;
    dbg.cpu.height = 12;  
}

/* Redraw Subroutines */
static void Redraw_Debugger(void) {
    Redraw_Mem();
    Redraw_Cpu();
}

static void Redraw_Mem(void) {
	int i, j, curr;
    wbkgd(dbg.mem.win, COLOR_PAIR(1));
    wborder(dbg.mem.win, 0, 0, 0, 0, 0, 0, 0, 0);
    mvwprintw(dbg.mem.win, 0, (dbg.mem.width - sizeof(MEM_TITLE)) / 2, MEM_TITLE);
    
    for (i = 0; i < 10; i++) {
        mvwprintw(dbg.mem.win, i + 1, 2, "0x%04x : ", 0x0100 + (i * 16));
        for (j = 0; j < 16; j++) {
			curr = 0;
			if (dbg.cpu.pc == dbg.mem.base + (i * 16) + j) {
				curr = 1;
				wattron(dbg.mem.win, COLOR_PAIR(2));
			}
            mvwprintw(dbg.mem.win, i + 1, (j * 3) + 11, "%02x ", dbg.mem.datap[(i * 16) + j]);
            if (curr) wattroff(dbg.mem.win, COLOR_PAIR(2));
        }
    }
    wrefresh(dbg.mem.win);
}
/* Format:
 ┌───────────────────────[CPU Info]─────────────────────────┐
 │  PC   OP        INST            A   X   Y   SP  CZIDB-VN │
 │ F30F  69 44 XX  ADC #44         44  FF  01 $FD  x--xx--x │
 └──────────────────────────────────────────────────────────┘
*/
static void Redraw_Cpu(void) {
	int x = 1, y = 1;
	wbkgd(dbg.cpu.win, COLOR_PAIR(1));
    wborder(dbg.cpu.win, 0, 0, 0, 0, 0, 0, 0, 0);
    mvwprintw(dbg.cpu.win, 0, (dbg.cpu.width - sizeof(CPU_TITLE)) / 2, CPU_TITLE);
    
    /* Draw CPU info */
    mvwprintw(dbg.cpu.win, 1, 1, "  PC   OP        INST            A   X   Y   SP  CZIDB-VN ");
    mvwprintw(dbg.cpu.win, 2, 1, " %04X  ", dbg.cpu.pc);
    wprintw(dbg.cpu.win, "%02X", dbg.mem.datap[dbg.cpu.pc]);
    wrefresh(dbg.cpu.win);
}

int main(int argc, char **argv) {
    Initialize_Dbg();
    return 0;
}

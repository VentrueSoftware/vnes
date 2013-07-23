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
#include "cpu.h"
#include "mem.h"
#include "opcode.h"

#define MEM_TITLE "[Memory Viewer]"
#define CPU_TITLE "[CPU Info]"

#define MEM_WIN_SIZE (16 * 10)

extern cpu_6502 cpu;

extern const char *op_str[];
extern const u8 op_mode[];

static vnes_dbg dbg;
static char asmstr[16];

static void Initialize_Dbg_Windows(void);
static char *To_Asm_String(u8 *ops, u8 size);

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

static char *To_Asm_String(u8 *ops, u8 size) {
    sprintf(asmstr, "%s ", op_str[ops[0]]);
    switch (op_mode[ops[0]]) {
        case IP: break;
        case AC: sprintf(asmstr + 4, "A"); break;
        case IM: sprintf(asmstr + 4, "#$%02X", ops[1]); break;
        case ZP: sprintf(asmstr + 4, "$%02X", ops[1]); break;
        case ZX: sprintf(asmstr + 4, "$%02X + X", ops[1]); break;
        case ZY: sprintf(asmstr + 4, "$%02X + Y", ops[1]); break;
        case RE: sprintf(asmstr + 4, "%+d", (i8)ops[1]); break;
        case AB: sprintf(asmstr + 4, "$%02X%02X", ops[2], ops[1]); break;
        case AX: sprintf(asmstr + 4, "$%02X%02X + X", ops[2], ops[1]); break;
        case AY: sprintf(asmstr + 4, "$%02X%02X + Y", ops[2], ops[1]); break;
        case IN: sprintf(asmstr + 4, "($%02X%02X)", ops[2], ops[1]); break;
        case IX: sprintf(asmstr + 4, "($%02X + X)", ops[1]); break;
        case IY: sprintf(asmstr + 4, "($%02X) + Y", ops[1]); break;
    }
    return asmstr;
}

static void Dbg(void) {
	int c = 0;
	while ((c = wgetch(dbg.cpu.win)) != 'q') {
		switch (c) {
			case 's':
				Cpu_Step();
				Redraw_Cpu();
				Redraw_Mem();
				break;
			case 'g':
				//Dbg_Goto();
			default: continue;
		}

	}
}

void Initialize_Dbg(void) {
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
    
    dbg.mem.size = MEM_WIN_SIZE;
    dbg.mem.datap = Mem_Get_Ptr(0);

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
    Redraw_Cpu();
    Redraw_Mem();
}

static void Redraw_Mem(void) {
    int pc_in_mem = (cpu.pc >= dbg.mem.base && cpu.pc <= dbg.mem.base + dbg.mem.size) ? 1 : 0;
	int i, j, curr;
    wbkgd(dbg.mem.win, COLOR_PAIR(1));
    wborder(dbg.mem.win, 0, 0, 0, 0, 0, 0, 0, 0);
    mvwprintw(dbg.mem.win, 0, (dbg.mem.width - sizeof(MEM_TITLE)) / 2, MEM_TITLE);
    
    for (i = 0; i < 10; i++) {
        mvwprintw(dbg.mem.win, i + 1, 2, "0x%04x : ", dbg.mem.base + (i * 16));
        for (j = 0; j < 16; j++) {
            int addr = dbg.mem.base + (i * 16) + j;
            if (pc_in_mem) {
                curr = 0;
                if (cpu.pc <= addr && addr < cpu.pc + dbg.cpu.inst_size) {
                    curr = 1;
                    wattron(dbg.mem.win, COLOR_PAIR(2));
                }
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
	int i = 0;
    dbg.cpu.pc = cpu.pc;
    dbg.cpu.inst[0] = Mem_Fetch(cpu.pc);
    dbg.cpu.inst_size = Get_Opcode_Length(dbg.cpu.inst[0]);
	wbkgd(dbg.cpu.win, COLOR_PAIR(1));
    wborder(dbg.cpu.win, 0, 0, 0, 0, 0, 0, 0, 0);
    mvwprintw(dbg.cpu.win, 0, (dbg.cpu.width - sizeof(CPU_TITLE)) / 2, CPU_TITLE);
    
    /* Draw CPU info */
    mvwprintw(dbg.cpu.win, 1, 1, "  PC   OP        INST            A   X   Y   SP  NV-BDIZC ");
    mvwprintw(dbg.cpu.win, 2, 1, " %04X  ", cpu.pc);
    wprintw(dbg.cpu.win, "%02X ", dbg.cpu.inst[0]);
    for (i = 1; i < dbg.cpu.inst_size; i++) {
        dbg.cpu.inst[i] = Mem_Fetch(cpu.pc + i);
        wprintw(dbg.cpu.win, "%02X ", dbg.cpu.inst[i]);
    }
    for (; i < 3; i++) {
        wprintw(dbg.cpu.win, "   ");
    }
    
    wprintw(dbg.cpu.win, "                 ");
    mvwprintw(dbg.cpu.win, 2, 18, "%s ", To_Asm_String(dbg.cpu.inst, dbg.cpu.inst_size));

    /* Draw registers */
    mvwprintw(dbg.cpu.win, 2, 34, "%02X  %02X  %02X  %02X  ", cpu.a, cpu.x, cpu.y, cpu.s);
    wprintw(dbg.cpu.win, "%c%c%c%c%c%c%c%c", 
        (cpu.p & FLG_SIGN) ? 'x' : '-',
        (cpu.p & FLG_OVERFLOW) ? 'x' : '-',
        (cpu.p & FLG_NOT_USED) ? 'x' : '-',
        (cpu.p & FLG_BRK) ? 'x' : '-',
        (cpu.p & FLG_DECIMAL) ? 'x' : '-',
        (cpu.p & FLG_INT_DIS) ? 'x' : '-',
        (cpu.p & FLG_ZERO) ? 'x' : '-',
        (cpu.p & FLG_CARRY) ? 'x' : '-');
        
    wrefresh(dbg.cpu.win);
}

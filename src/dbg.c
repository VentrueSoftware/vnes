/* 
 * Project: VNES
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
 */

#include <errno.h>
#include <curses.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include "dbg.h"
#include "types.h"
#include "cpu.h"
#include "ppu.h"
#include "mem.h"
#include "opcode.h"

#define INFO_COLOR 2
#define ERROR_COLOR 3

/* Useful globals from opcode.c */
extern const char *op_str[];
extern const u8 op_mode[];
extern const u8 mode_length[];
extern const op_func op_fn[];

/* Useful globals from cpu.c */
extern cpu_6502 cpu;

/* Useful globals from ppu.c */
extern ppu_2c02 ppu;

/* Static buffer for stringifying an instruction */
#define MAX_ASM_LEN 16
static char asmstrbuf__[MAX_ASM_LEN];

/* File that acts as a buffer for the log. */
static const char *logfilename = ".vnes.log";
static FILE *logfp;

/* ncurses windows */
static WINDOW *logwin;
static WINDOW *statline;

static int error_attr;
static int info_attr;

/* Local function declarations */
static void Init_Ncurses(void);
static void Open_Log(void);
static char *Stringify_Instruction(u8 *ops, u8 size);
static void Log_Instruction(void);

/* Start debugger console */
void Start_Dbg(void) {
    int cmd = 0;
    signal(SIGINT, End_Dbg);
    
    Init_Ncurses();
    Open_Log();
    
    Log_Instruction();
    
    //Accept_Dbg_Input();
    while (cmd != 'q') {
        cmd = getch();
        switch (cmd) {
            case 's': 
                Cpu_Step();
                Log_Instruction();
            break;
            case 'r': {
                u8 op;
                while (op_fn[(op = Mem_Fetch(cpu.pc))] != op_fn[0xFF]) {
                    Cpu_Step();
                    Log_Instruction();
                }
            }
            default:
            break;
        }
    }
    End_Dbg(0);
}

/* End the debugger */
void End_Dbg(int signal) {
    /* Clean up the ncurses windows */
    delwin(logwin);
    delwin(statline);
    endwin();
    
    /* Close the log */
    if (logfp) fclose(logfp);
    exit(0);
}

/* Report an error to the debug console */
void Show_Error(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    wmove(statline, 0, 0);
    wbkgd(statline, error_attr);
    wprintw(statline, "[VNES Debugger] ");
    vw_printw(statline, fmt, args);
    wrefresh(statline);
    
    va_end(args);
    getch();
}


/* Local function Definitions */

/* Initializes Ncurses and sets attributes based on its capabilities. */
static void Init_Ncurses(void) {
    int x, y;
    
    /* Set ncurses attributes */
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, y, x);

    /* Try to see if we have color; if we don't, we use "reverse" */
    if (has_colors()) {
        start_color();
        
        init_pair(INFO_COLOR, COLOR_WHITE, COLOR_BLUE);
        init_pair(ERROR_COLOR, COLOR_WHITE, COLOR_RED);
        
        info_attr = COLOR_PAIR(INFO_COLOR);
        error_attr = COLOR_PAIR(ERROR_COLOR);
    } else {
        info_attr = error_attr = A_REVERSE;
    }
    
    /* Initialize windows */
    logwin = newwin(y, x, 0, 0);
    statline = newwin(1, x, y - 1, 0);
    
    
    scrollok(logwin, TRUE);
    refresh();
}

static void Open_Log(void) {
    /* Open log file for reading/writing */
    logfp = fopen(logfilename, "w+");
    
    /* If we couldn't open the log file, we error */
    if (!logfp) {
        Show_Error("Error opening log: %s\n", strerror(errno));
		End_Dbg(0);
	}
}

/* Converts an instruction into its assembler equivalent. */
static char *Stringify_Instruction(u8 *ops, u8 size) {
    sprintf(asmstrbuf__, "%s ", op_str[ops[0]]);
    switch (op_mode[ops[0]]) {
        case IP: break;
        case AC: sprintf(asmstrbuf__ + 4, "A"); break;
        case IM: sprintf(asmstrbuf__ + 4, "#$%02X", ops[1]); break;
        case ZP: sprintf(asmstrbuf__ + 4, "$%02X", ops[1]); break;
        case ZX: sprintf(asmstrbuf__ + 4, "$%02X + X", ops[1]); break;
        case ZY: sprintf(asmstrbuf__ + 4, "$%02X + Y", ops[1]); break;
        case RE: sprintf(asmstrbuf__ + 4, "$%04X", 2 + cpu.pc + (i8)ops[1]); break;
        case AB: sprintf(asmstrbuf__ + 4, "$%02X%02X", ops[2], ops[1]); break;
        case AX: sprintf(asmstrbuf__ + 4, "$%02X%02X + X", ops[2], ops[1]); break;
        case AY: sprintf(asmstrbuf__ + 4, "$%02X%02X + Y", ops[2], ops[1]); break;
        case IN: sprintf(asmstrbuf__ + 4, "($%02X%02X)", ops[2], ops[1]); break;
        case IX: sprintf(asmstrbuf__ + 4, "($%02X + X)", ops[1]); break;
        case IY: sprintf(asmstrbuf__ + 4, "($%02X) + Y", ops[1]); break;
    }
    return asmstrbuf__;
}

void Log_Line(const char *format, ...) {
//#if 0    
    va_list args;
    va_start(args, format);
    
    vw_printw(logwin, format, args);
    wprintw(logwin, "\n");
    vfprintf(logfp, format, args);
    fprintf(logfp, "\n");
    wrefresh(logwin);
    getch();
    va_end(args);
//#endif
}

/* Log an instruction */
void Log_Instruction(void) {
#if 1
    char line[128];
    u8 ops[3], len, i;

    sprintf(line, "%04X ", cpu.pc);

    /* Get ops */
    ops[0] = Mem_Fetch(cpu.pc);
    len = mode_length[op_mode[ops[0]]];
    sprintf(line + 5, " %02X", ops[0]);
    for (i = 1; i < len; i++) {
        ops[i] = Mem_Fetch(cpu.pc + i);
        sprintf(line + 5 + (3 * i), " %02X", ops[i]);
    }
    for (; i < 3; i++) {
        sprintf(line + 5 + (3 * i), "   ");
    }
    sprintf(line + 14, "  %-32sA:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%3u SL:%3d PPUADDR:%04X",
        Stringify_Instruction(ops, len),
        cpu.a, cpu.x, cpu.y, cpu.p, cpu.s, ppu.cycles, ppu.scanline, ppu.addr);
    
    //wprintw(logwin, "%s\n", line);
    //wrefresh(logwin);

    wbkgd(statline, info_attr);
    //mvwprintw(statline, 0, 0, "[VNES debugger]");
    //wrefresh(statline);

    fprintf(logfp, "%s\n", line);
#endif
}


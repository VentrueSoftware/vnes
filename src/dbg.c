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

#include <curses.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "dbg.h"
#include "types.h"
#include "cpu.h"
#include "mem.h"
#include "opcode.h"

/* Static buffer for stringifying an instruction */
#define MAX_ASM_LEN 16
static char asmstrbuf__[MAX_ASM_LEN];

/* From opcode.c */
extern const char *op_str[];
extern const u8 op_mode[];
extern const u8 mode_length[];
extern const op_func op_fn[];

/* From cpu.c */
extern cpu_6502 cpu;

void End_Dbg(int signal) {
    endwin();
    exit(0);
}

static char *Stringify_Instruction(u8 *ops, u8 size);
static char *Stringify_Instruction(u8 *ops, u8 size) {
    sprintf(asmstrbuf__, "%s ", op_str[ops[0]]);
    switch (op_mode[ops[0]]) {
        case IP: break;
        case AC: sprintf(asmstrbuf__ + 4, "A"); break;
        case IM: sprintf(asmstrbuf__ + 4, "#$%02X", ops[1]); break;
        case ZP: sprintf(asmstrbuf__ + 4, "$%02X", ops[1]); break;
        case ZX: sprintf(asmstrbuf__ + 4, "$%02X + X", ops[1]); break;
        case ZY: sprintf(asmstrbuf__ + 4, "$%02X + Y", ops[1]); break;
        case RE: sprintf(asmstrbuf__ + 4, "%+d", (i8)ops[1]); break;
        case AB: sprintf(asmstrbuf__ + 4, "$%02X%02X", ops[2], ops[1]); break;
        case AX: sprintf(asmstrbuf__ + 4, "$%02X%02X + X", ops[2], ops[1]); break;
        case AY: sprintf(asmstrbuf__ + 4, "$%02X%02X + Y", ops[2], ops[1]); break;
        case IN: sprintf(asmstrbuf__ + 4, "($%02X%02X)", ops[2], ops[1]); break;
        case IX: sprintf(asmstrbuf__ + 4, "($%02X + X)", ops[1]); break;
        case IY: sprintf(asmstrbuf__ + 4, "($%02X) + Y", ops[1]); break;
    }
    return asmstrbuf__;
}

void Log_Instruction(void) {
    char line[88];
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
    sprintf(line + 14, "  %-32sA:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:XXX SL:XXX",
        Stringify_Instruction(ops, len),
        cpu.a, cpu.x, cpu.y, cpu.p, cpu.s);
    }
    printw("%s\n", line);
    refresh();
}

void Start_Dbg(void) {
    int cmd = 0;
    signal(SIGINT, End_Dbg);
    initscr();
    noecho();
    
    Log_Instruction();
    
    while (cmd != 'q') {
        cmd = getch();
        switch (cmd) {
            case 's': 
                Cpu_Step();
                Log_Instruction();
            break;
            case 'c': {
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

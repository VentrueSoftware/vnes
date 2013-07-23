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
 *      contents directly in memory, and view the status of the various
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

static vnes_dbg dbg;
static u8 hexvals[10][16];

static void finish(int sig)
{
    if (hexwin) {
        delwin(hexwin);
    };
    endwin();
    /* do your non-curses wrapup here */

    exit(0);
}

static void Redraw_Debugger(void) {
    
}

static void refresh_scr(int sig) {
    endwin();
    refresh();
}

#define HEXTITLE "[Memory Viewer]"

void Initialize_Dbg(void) {
    int num = 0, i, j;
    int h, w;
    signal(SIGINT, finish);      /* arrange interrupts to terminate */

    initscr();
    nonl();
    cbreak();
    noecho();
    curs_set(0);

    getmaxyx(stdscr, dbg.term.height, dbg.term.width);
    
    dbg.memview.win = newwin(12, 60, 0, 1);
    dbg.memview.width = 60;
    dbg.memview.height = 12;
    
    signal(SIGWINCH, refresh_scr);

    if (has_colors())
    {
        start_color();

        /*
         * Simple color assignment, often all we need.  Color pair 0 cannot
	 * be redefined.  This example uses the same value for the color
	 * pair as for the foreground color, though of course that is not
	 * necessary:
         */
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
        init_pair(8, COLOR_WHITE,   COLOR_GREEN);
    }
    //wmove(hexwin, 0, 0);
    wbkgd(hexwin, COLOR_PAIR(8));
    wborder(hexwin, 0, 0, 0, 0, 0, 0, 0, 0);
    mvwprintw(hexwin, 0, (59 - strlen(HEXTITLE)) / 2, HEXTITLE);
    
    for (i = 0; i < 10; i++) {
        mvwprintw(hexwin, i + 1, 1, "0x%04x : ", 0x0100 + (i * 16));
        for (j = 0; j < 16; j++) {
            mvwprintw(hexwin, i + 1, (j * 3) + 10, "%02x ", hexvals[i][j]);
        }
    }
    for (;;)
    {
        int c = wgetch(hexwin);     /* refresh, accept single keystroke of input */
        num++;

        /* process the command keystroke */
        //mvwaddch(hexwin, 0, num, c);
//        refresh();
        wrefresh(hexwin);
    }

    finish(0);               /* we're done */
}

int main(int argc, char **argv) {
    Initialize_Dbg();
    return 0;
}

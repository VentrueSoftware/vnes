/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 23-Jul-2013
 * File: dbg-pane.c
 * 
 * Description:
 * 
 *      Pane layout implementation, for use in ncurses-based debugger. 
 *      Handles some of the boilerplate for window layouts, and has
 *      some built-in border handling.
 * 
 * Change Log:
 *      23-Jul-2013:
 *          File created.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "dbg-pane.h"


pane *Pane_Create(pane *parent, int flags, int x, int y, int width, int height) {
    pane *p = (pane *)malloc(sizeof(pane));
    if (!p) return 0;

    bzero(p, sizeof(pane));

    /* Creation of the root pane is a bit different */
    if (!parent) {
        p->win = stdscr;
        getmaxyx(stdscr, p->ah, p->aw);
        p->flags = flags;
        return p;
    }
    
    /* If the parent is full, error  */
    if (parent->nchildren == PANE_MAX_CHILDREN) goto err;
    
    /* Set properties */
    p->parent = parent;
    p->flags = flags;
    
    Pane_Set_Dimensions(p, x, y, width, height);

    /* Create the WINDOW and attach it to the parent's children */
    p->win = derwin(parent->win, p->ah, p->aw, p->ay, p->ax);
    parent->children[(parent->nchildren)++] = p;
    
    return p;
err:
    free(p);
    return 0;
}

void Pane_Set_Dimensions(pane *p, int x, int y, int width, int height) {
    /* Set the actual x, y, w, h values */
    switch ((p->x = x)) {
        case PANE_MERGE: p->ax = 0; break;
        case PANE_STRETCH: p->ax = 1; break;
        default: p->ax = x;
    }
    switch ((p->y = y)) {
        case PANE_MERGE: p->ay = 0; break;
        case PANE_STRETCH: p->ay = 1; break;
        default: p->ay = y;
    }
    switch ((p->w = width)) {
        case PANE_MERGE: p->aw = p->parent->aw - p->ax; break;
        case PANE_STRETCH: p->aw = p->parent->aw - p->ax - 1; break;
        default: p->aw = width;
    }
    switch ((p->h = height)) {
        case PANE_MERGE: p->ah = p->parent->ah - p->ay; break;
        case PANE_STRETCH: p->ah = p->parent->ah - p->ay - 1; break;
        default: p->ah = height;
    }
}

int Pane_Delete(pane *p) {
	int i;
	if (!p) return 1;
    
    /* Delete children first */
	for (i = 0; i < p->nchildren; i++) {
		Pane_Delete(p->children[i]);
	}
    
    /* Don't delete the root pane's WINDOW as its stdscr */
	if (p->win != stdscr) delwin(p->win);
	free(p);
	return 1;
}

int Pane_Resize(pane *p) {
	int i;
	if (!p) return 1;
	
	if (p->win == stdscr) {
		endwin();
        getmaxyx(stdscr, p->ah, p->aw);
		refresh();
	} else {
        wresize(p->win, p->h, p->w);
#if 0
		delwin(p->win);
        wres
        Pane_Set_Dimensions(p, p->x, p->y, p->w, p->h);
		p->win = derwin(p->parent->win, p->ah, p->aw, p->ay, p->ax);
#endif 
	}
	
	for (i = 0; i < p->nchildren; i++) {
		Pane_Resize(p->children[i]);
	}
	//werase(p->win);
	Pane_Draw(p);
	
	return 1;
}

int Pane_Draw(pane *p) {
    int i;
    if (!p || !p->win) return 0;
    
    if (p->flags & PANE_NO_BORDER) {
        wborder(p->win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    } else {
        Pane_Border(p);
    }
    
    if (p->title) {
        int offset;
        if (p->flags & PANE_TITLE_LEFT) offset = 1;
        else if (p->flags & PANE_TITLE_RIGHT) offset = p->aw - strlen(p->title) - 2;
        else offset = (p->aw - strlen(p->title)) / 2;
        mvwprintw(p->win, 0, offset, p->title);
    }
    
    wprintw(p->win, " %dx%d", p->aw, p->ah);
    
    
    wrefresh(p->win);

    for (i = 0; i < p->nchildren; i++) {
		Pane_Draw(p->children[i]);
    }
        
    return 1;
}

void Pane_Border(pane *p) {
    int lt = 0, rt = 0, lb = 0, rb = 0;
    if (!p->parent || (p->parent->flags & PANE_NO_BORDER)) goto draw;
    if (p->x == PANE_MERGE) {
        if (p->ay) {
            lt = ACS_LTEE;
            if (p->ay + p->ah < p->parent->ah) lb = ACS_LTEE;
        }
    } else {
        if (p->y == PANE_MERGE) lt = ACS_TTEE;
        if (p->h == PANE_MERGE) lb = ACS_BTEE;
    }
    if (p->w == PANE_MERGE) {
        if (p->ay) {
            rt = ACS_RTEE;
            if (p->ay + p->ah < p->parent->ah) rb = ACS_RTEE;
        }
    } else {
        if (p->y == PANE_MERGE) rt = ACS_TTEE;
        if (p->h == PANE_MERGE) rb = ACS_BTEE;
    }
draw:
    wborder(p->win, 0, 0, 0, 0, lt, rt, lb, rb);
}

void Pane_Print(pane *p, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vw_printw(p->win, fmt, args);
    va_end(args);
}

void Pane_Mv_Print(pane *p, int y, int x, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    wmove(p->win, y, x);
    vw_printw(p->win, fmt, args);
    va_end(args);
}

void Pane_Highlight(pane *p, int y, int x, int length, int color_pair) {
    char *buf = (char *)malloc((length + 1) * sizeof(char));
    mvwinnstr(p->win, y, x, buf, length);
    buf[length] = '\0';
    wattron(p->win, COLOR_PAIR(color_pair));
    Pane_Mv_Print(p, y, x, "%s", buf);
    wattroff(p->win, COLOR_PAIR(color_pair));
    free(buf);
}

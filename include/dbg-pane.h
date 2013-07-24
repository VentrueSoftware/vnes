/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 23-Jul-2013
 * File: dbg-pane.h
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

#ifndef VNES_DBG_PANE_H
#define VNES_DBG_PANE_H

#include <curses.h>

#define PANE_MAX_CHILDREN 5

#define PANE_STRETCH -1      /* Expands to fill the width of the parent */
#define PANE_MERGE -2        /* Merges with border of parent */

#define PANE_NO_BORDER 0x01
#define PANE_TITLE_LEFT 0x02
#define PANE_TITLE_RIGHT 0x03


typedef struct pane {
    WINDOW *win;
    struct pane *parent;
    
    int nchildren;
    struct pane *children[PANE_MAX_CHILDREN];
    
    char *title;    /* If NULL, no title */
    
    /* Offset and dimensions set by user */
    int x;  /* Relative to parent */
    int y;  /* Relative to parent */
    int w;
    int h;
    
    int flags;  /* Border type, title position etc. */

    /* Actual offset and dimensions, used in border handling for MERGE
     * type (sub)windows */
    int ax;
    int ay;
    int aw;
    int ah;    
} pane;

/* Pane API */
#define ROOT_PANE 0, (PANE_NO_BORDER | PANE_TITLE_LEFT), 0, 0, 0, 0
pane *Pane_Create(pane *parent, int flags, int x, int y, int width, int height);
int Pane_Delete(pane *p);

int Pane_Resize(pane *p);

int Pane_Draw(pane *p);
void Pane_Border(pane *p);

#endif /* #ifndef VNES_DBG_PANE_H */

/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 19-Jul-2013
 * File: dbg.h
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

#ifndef VNES_DBG_H
#define VNES_DBG_H

#include <curses.h>
#include "types.h"

/* Debugger structure */
typedef struct vnes_dbg {
    /* Overall terminal attributes */
    struct {
        int width;
        int height;
    } term;
    
    /* Memory viewer */
    struct {
        u16 base;
        u16 size;
        u8 *datap;  /* More efficient memory lookup */
        int width;
        int height;
        WINDOW *win;
    } mem;

	/* CPU Info */
	struct {
        u8 pc;
		u8 inst[3];
        u8 inst_size;
		int width;
		int height;
		WINDOW *win;
	} cpu;
} vnes_dbg;

/* Debugger windows and utilities - unsure what goes here yet. */
void Initialize_Dbg(void);

#endif /* #ifndef VNES_DBG_H */

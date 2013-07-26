/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 19-Jul-2013
 * File: dbg.h
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

#ifndef VNES_DBG_H
#define VNES_DBG_H

#include "types.h"

/* Initialize Debugger */
void Start_Dbg(void);

#endif /* #ifndef VNES_DBG_H */

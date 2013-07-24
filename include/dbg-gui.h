/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 23-Jul-2013
 * File: dbg-gui.h
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

#ifndef VNES_DBG_GUI_H
#define VNES_DBG_GUI_H

#include "dbg-pane.h"

#endif /* #ifndef VNES_DBG_GUI_H */

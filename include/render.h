/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 01-Aug-2013
 * File: render.h
 * 
 * Description:
 * 
 *      Screen rendering functionality.
 * 
 * Change Log:
 *      01-Aug-2013:
 *          File created.
 */

#ifndef VNES_RENDER_H
#define VNES_RENDER_H

#include "types.h"

#define NES_RES_X 256
#define NES_RES_Y 240

INLINED u32 Sample_Nes_Palette(u8 index);

void Render_Scanline(i16 scanline);
void Dump_Render(char *file);
void Dump_Pattern_Tables(void);
void Dump_Name_Tables(void);
void Dump_Attr_Tables(void);
#endif /* #ifndef VNES_RENDER_H */

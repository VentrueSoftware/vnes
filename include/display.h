/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 08-Aug-2013
 * File: display.h
 * 
 * Description:
 * 
 *      Display interface for rendering NES frames.  Eventually, it plans
 *      to have implementations for:
 *          * OpenGL (Texture)..............[INCOMPLETE]
 *          * OpenGL (FrameBuffer)..........[NOT STARTED]
 *          * DirectFB......................[NOT STARTED]
 *          * DirectX.......................[NOT STARTED]
 * 
 * Change Log:
 *      08-Aug-2013:
 *          File created.
 */

#ifndef VNES_DISPLAY_H
#define VNES_DISPLAY_H

#include "types.h"

struct win_impl;

typedef struct vnes_display {
    struct win_impl *win;
    struct {
        /* format will go here */
        void *data;     /* Pointer to source data */
        u16   width;    /* Width of source */
        u16   height;   /* Height of source */
    } src;
} vnes_display;

int Open_Display(vnes_display **disp);
void Display_Loop(vnes_display *disp);

#endif /* #ifndef VNES_DISPLAY_H */

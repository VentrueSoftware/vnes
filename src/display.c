/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 08-Aug-2013
 * File: display.c
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
 
#include "impl/d-opengl.c"

void Set_Display_Source(vnes_display *disp, void *source, u16 width, u16 height) {
    disp->src.data = source;
    disp->src.width = width;
    disp->src.height = height;
    /* Implementation-specific source setting callback */
    Set_Display_Source_Impl(disp);
}

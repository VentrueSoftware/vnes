/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 01-Aug-2013
 * File: render.c
 * 
 * Description:
 * 
 *      Screen rendering functionality.
 * 
 * Change Log:
 *      01-Aug-2013:
 *          File created.
 */

#include <string.h>
#include "bitwise.h"
#include "render.h"
#include "ppu.h"

/* From ppu.h */
extern ppu_2c02 ppu;

static const u32 nes_palette[] = {
	0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000,
	0x881400, 0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000,
	0x000000, 0x000000, 0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC,
	0xE40058, 0xF83800, 0xE45C10, 0xAC7C00, 0x00B800, 0x00A800, 0x00A844,
	0x008888, 0x000000, 0x000000, 0x000000, 0xF8F8F8, 0x3CBCFC, 0x6888FC,
	0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044, 0xF8B800, 0xB8F818,
	0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000, 0xFCFCFC,
	0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
	0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000,
	0x000000
};

static u32 render_data[NES_RES_X * NES_RES_Y];

INLINED u32 Sample_Nes_Palette(u8 index) {
    return nes_palette[index];
}

/* Local declarations */
static void Render_Background(u16 scanline, u16 *background);

/* Rendering Function Definitions */

/* Render_Scanline renders background, sprites (front, back, and zero)
 * and then passes them to a compositor.   */
void Render_Scanline(u16 scanline) {
    u16 render_buffer[256 * 4];
    u16 *background = render_buffer,
        *spr_front = render_buffer + 256,
        *spr_back  = render_buffer + (2 * 256),
        *spr_zero  = render_buffer + (3 * 256);
        
    /* Enforce cleared memory */
    bzero(render_buffer, 256 * 4);
    
    if (ppu.mask & SHOW_BG) Render_Background(scanline, background);
    //if (ppu.mask & SHOW_SPRITES) Render_Sprites(scanline, spr_front, spr_back, spr_zero);
    
    /* Compositor renders directly into the screen buffer. */
    /* Composite_Scanline(scanline, background, spr_front, spr_back, spr_zero); */
}

/* Render_Background renders the background at the particular scanline */
static void Render_Background(u16 scanline, u16 *background) {
    u16 clip;           /* Clip offset */
    u16 i;              /* Iterator variable */
    u8 nt_index,        /* Name table index */
       current_pixel,   /* Current Pixel value */
       current_attr;    /* Attribute table value */
    
    clip = IS_SET(ppu.mask, CLIP_BG) ? 8 : 0;
    for (i = 0; i < 264; i++) {
        nt_index = (ppu.addr >> 10) & 0x03;
    }
}

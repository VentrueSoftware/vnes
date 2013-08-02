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

/* Render_Background renders the background at the particular scanline.
 * Since this is rather confusing, I'm using more verbose variable names
 * and commenting the shit out of this code. */
static void Render_Background(u16 scanline, u16 *background) {
    u16 clip;           /* Clip offset */
    u16 i;              /* Iterator variable */
    //u16 y_offset;       /* Y offset, used for tile and pattern offset */
    //u16 x_offset;       /* X offset, used in tile offset and pixel calculation */
    u16 tile_no;        /* Tile number, as specified in name table. */
    u16 pattern_base;   /* Base pattern table */
    u16 pattern_offset; /* Pattern table offset */
    u8 nt_index,        /* Name table index */
       current_pixel,   /* Current Pixel value */
       current_attr;    /* Attribute table value */
    
    /* The base pattern table is 0x0000 if BG_PTRN_TABLE = 0, 
     * 0x1000 otherwise. */
    pattern_base = IS_SET(ppu.ctrl, BG_PTRN_TABLE) ? 0x1000 : 0x0000;

    /* If CLIP_BG is set in PPUMASK, the left-most 8 pixels are not
     * rendered. */
    clip = IS_SET(ppu.mask, CLIP_BG) ? 8 : 0;
    
    
    for (i = 0; i < 264; i++) {
        /* Calculate some useful offsets. */
        //x_offset =  | ppu.scrollx;
        /* Calculate the name table index (nt_index).
         * The name table index calculated here is simply used to select
         * the correct name table (0, 1, 2, or 3) from memory.  Conveniently
         * enough, There's a correlation:  
         * 
         *      0x2000 -> 0x23FF = name table 0
         *      0x2400 -> 0x27FF = name table 1
         *      0x2800 -> 0x2BFF = name table 2
         *      0x2C00 -> 0x2FFF = name table 3
         * 
         * In case it isn't clear, bits 10 and 11 of the address provide
         * the name table index. Hence, the shift 10 bits followed by
         * a logical AND 0x03. */
        nt_index = (ppu.addr >> 10) & 0x03;
        
        /* Calculate the tile number.
         * Each tile of the NES's display is stored in a name table as
         * an index into the pattern table.  The name table map (ppu.nt_map)
         * is used to account for name table mirroring.  From there, we
         * can use the least significant 10 bits to get the offset of the
         * tile we are rendering. */
        tile_no = ppu.nt_map[nt_index][ppu.addr & 0x03FF];
        
        /* Calculate the pattern's offset.
         * Since pattern entries are 16 bytes in size, we shift the 
         * tile_number left by 4 to arrive at the correct pattern entry.
         * Lastly, we need to calculate which row of the pattern table
         * we're going to be rendering on this scanline, which is handled
         * by the PPUSCROLL's y value. */
        pattern_offset = pattern_base + (tile_no << 4) + ppu.scrolly;
        
        /* Calculate the current attribute table entry 
         * Attribute table entries start at 0x03C0 from the name table's
         * beginning. */
    }
}

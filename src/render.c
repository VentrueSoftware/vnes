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
#include "cart.h"

/* From ppu.h */
extern ppu_2c02 ppu;

static const u32 nes_palette[] = {
    0xFF7C7C7C, 0xFF0000FC, 0xFF0000BC, 0xFF4428BC, 0xFF940084, 0xFFA80020, 0xFFA81000,
    0xFF881400, 0xFF503000, 0xFF007800, 0xFF006800, 0xFF005800, 0xFF004058, 0xFF000000,
    0xFF000000, 0xFF000000, 0xFFBCBCBC, 0xFF0078F8, 0xFF0058F8, 0xFF6844FC, 0xFFD800CC,
    0xFFE40058, 0xFFF83800, 0xFFE45C10, 0xFFAC7C00, 0xFF00B800, 0xFF00A800, 0xFF00A844,
    0xFF008888, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFF8F8F8, 0xFF3CBCFC, 0xFF6888FC,
    0xFF9878F8, 0xFFF878F8, 0xFFF85898, 0xFFF87858, 0xFFFCA044, 0xFFF8B800, 0xFFB8F818,
    0xFF58D854, 0xFF58F898, 0xFF00E8D8, 0xFF787878, 0xFF000000, 0xFF000000, 0xFFFCFCFC,
    0xFFA4E4FC, 0xFFB8B8F8, 0xFFD8B8F8, 0xFFF8B8F8, 0xFFF8A4C0, 0xFFF0D0B0, 0xFFFCE0A8,
    0xFFF8D878, 0xFFD8F878, 0xFFB8F8B8, 0xFFB8F8D8, 0xFF00FCFC, 0xFFF8D8F8, 0xFF000000,
    0xFF000000
};

static u32 render_data[NES_RES_X * NES_RES_Y];
static u16 render_nums[NES_RES_X * NES_RES_Y];

INLINED u32 Sample_Nes_Palette(u8 index) {
    return nes_palette[index];
}

/* Local declarations */
static void Render_Background(u16 *background);
static void Composite_Scanline(i16 scanline, u16 *background, u16 *spr_back, u16 *spr_front);


/* Rendering Function Definitions */

/* Render_Scanline renders background, sprites (front, back, and zero)
 * and then passes them to a compositor.   */
void Render_Scanline(i16 scanline) {
    u16 render_buffer[256 * 3];
    u16 *background = render_buffer,
        *spr_back = render_buffer + 256,
        *spr_front  = render_buffer + (2 * 256);
        
    /* Enforce cleared memory */
    bzero(render_buffer, 256 * 4);
    
    Render_Background(background);
    //if (ppu.mask & SHOW_SPRITES) Render_Sprites(scanline, spr_front, spr_back);
    
    /* Compositor renders directly into the screen buffer. */
    if (scanline > -1 && scanline < 240) {
        Composite_Scanline(scanline, background, spr_front, spr_back);
    } else if (scanline == -1) {
        /* Update v_addr */
        ppu.v_addr = (ppu.v_addr & 0x041F) | (ppu.t_addr & ~0x041F);
    }
}

/* Render_Background renders the background at the particular scanline.
 * Since this is rather confusing, I'm using more verbose variable names
 * and commenting the shit out of this code. */
static void Render_Background(u16 *background) {
    u16 clip_amount;    /* Clip offset */
    u16 i;              /* Iterator variable */
    u16 tile_no;        /* Tile number, as specified in name table. */
    u16 pattern_base;   /* Base pattern table */
    u16 pattern_offset; /* Pattern table offset */
    u8 nt_index,        /* Name table index */
       current_pixel,   /* Current Pixel value */
       current_attr;    /* Attribute table value */
    u8 found = 0;
    /* The base pattern table is 0x0000 if BG_PTRN_TABLE = 0, 
     * 0x1000 otherwise. */
    pattern_base = IS_SET(ppu.ctrl, BG_PTRN_TABLE) ? 0x1000 : 0x0000;

    /* If CLIP_BG is set in PPUMASK, the left-most 8 pixels are not
     * rendered. */
    clip_amount = IS_SET(ppu.mask, CLIP_BG) ? 0 : 8;
    
    
    for (i = 0; i < 264; i++) {
        /* Check to see if the pixel should be rendered. This means
         * that it is within clip_amount and 256, and that it has a lower
         * two bits that are non-zero. The first of those conditions can
         * be checked here.  We don't do this restriction in the loop
         * condition, because we still need to update registers after
         * each iteration. */
        if (i < clip_amount) goto update;
        
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
        if (0 == ((ppu.v_addr >> 10) & 3)) ppu.v_addr |= 0x2000;
        nt_index = (ppu.v_addr >> 10) & 0x0003;

        /* Calculate the tile number.
         * Each tile of the NES's display is stored in a name table as
         * an index into the pattern table.  The name table map (ppu.nt_map)
         * is used to account for name table mirroring.  From there, we
         * can use the least significant 10 bits to get the offset of the
         * tile we are rendering. */
        tile_no = ppu.nt_map[nt_index][ppu.v_addr & 0x03FF];
        
        /* Calculate the pattern's offset.
         * Since pattern entries are 16 bytes in size, we shift the 
         * tile_number left by 4 to arrive at the correct pattern entry.
         * Lastly, we need to calculate which row of the pattern table
         * we're going to be rendering on this scanline, which is handled
         * by the PPUSCROLL's y value. */
        pattern_offset = pattern_base + (tile_no << 4) + ppu.scrolly;
        
        /* Calculate the current attribute table entry 
         * The attribute table stores the 2 high bits of the palette index.
         * Attribute table entries start at 0x03C0 from the name table's
         * beginning.  Attribute table entires represent a 32x32 pixel
         * area, which means there are 256 / 32 = 8 bytes per row.  The 
         * x component of the PPU address (5 bits) is a multiple of 8 in
         * terms of x-coordinate pixels, and with 32 pixels per entry, 
         * you can just shift the five least significant bits two to the
         * right: 
         * 
         *         x = (ppu.v_addr & 0x001F) >> 2
         * 
         * For the y component, which also is in multiples of 8,
         * needs to be divided by 4, to get the y index, then multiplied by
         * 8 to skip to the right "row", as this is a one-dimensional array.
         * The net is a shift left of 1, while disregarding the two least
         * significant bits of y. Here's an example:
         * 
         *         (0b11111 >> 2) << 1 = 0b00111 << 1 = 0b01110 
         * 
         * The end result?  We first only grab the 3 most significant bits
         * of y, to account for a divide by 4.  We have a total of a shift
         * right 5 to get the bits from the address, followed by a shift
         * left 1 to account for the divide by 4, then multiply by 8. 
         * Therefore, the formula is:
         * 
         *         y = (ppu.v_addr & 0x0380) >> 4
         * */
                                                        /* Y component */            /* X component */
        current_attr = 
#if 0
        ppu.nt_map[nt_index][0x03C0 | (ppu.v_addr & 0x0C00) | ((ppu.v_addr >> 4) ^ 0x38) | ((ppu.v_addr >> 2) & 0x07)];
#else
        ppu.nt_map[nt_index][0x03C0 + ((ppu.v_addr & 0x0380) >> 4) + ((ppu.v_addr & 0x001F) >> 2)];
#endif        
        /* Once we acquire the attribute table entry, we need to grab the
         * two bits that are relevant to the 16x16 area we are in.  This can
         * be achieved by looking at the second bit of the x and y components
         * of the PPU address; The second bit of x is the second bit of the
         * PPU address, so an AND of 0x0002 will obtain that bit.  The y
         * bit requires an and of 0byyy[y]yxxxxx = 0x0040.  We need to
         * shift that one 4 bits.  Since we are looking at 4 quadrants
         * with 8 bits of data, we effectively shift by a multiple of 2
         * of the combined bits, which is why the x bit isn't shifted right
         * 1.  Finally, we AND with 0x03 to only obtain the two bits of the
         * byte that we want. */
                             /* Y component */            /* X component */
        current_attr >>= ((ppu.v_addr & 0x0040) >> 4) | (ppu.v_addr & 0x0002);
        current_attr &= 0x03;
         
        /* The two low bits are calculated from the pattern table itself. This
         * is where the precision from scrollx and scrolly help. */
        current_pixel = ((Read_Cartridge_Chr(pattern_offset) >> (7 - ppu.scrollx)) & 1)
                       | (((Read_Cartridge_Chr(pattern_offset + 8) >> (7 - ppu.scrollx)) & 1) << 1)
                       | (current_attr << 2);
        
        /* Check that the lower two bits are set.  If they are, we can
         * render this pixel to the background line. */
        if (current_pixel & 0x03) {
            //if (!found) {found = 1; Log_Line("Woot: %02X", current_pixel);}
            background[i] = current_pixel | 0x3F00;
            render_nums[(ppu.scanline * NES_RES_X) + i] = tile_no;
        }
update:
        /* Update the x scroll position and the PPU address.  Every 8
         * pixels, we have to increment the PPU address, so we check
         * the x component of PPUSCROLL */
        ppu.scrollx++;
        if (8 == ppu.scrollx) {
            ppu.scrollx = 0;
            /* Reconstruct the address to account for new shifts in value:
             * Every 32 bytes, we change name tables; this is achieved
             * by checking, pre-increment, for all x bits to be set to 1,
             * which is 0b11111 = 0x1F = 31.  We flip bit 10, using EOR
             * of 0x0400.  We then reconstruct the new address, using
             * the upper 10 bits and the lower 5, incremented. */
            if ((ppu.v_addr & 0x001F) == 0x001F) {
                ppu.v_addr &= ~0x001F;
                ppu.v_addr ^= 0x0400;
            } else {
                ppu.v_addr++;
            }
        }
    }
    
    /* Dot 256 increments the scanline */
    /* Update the y scroll position and the PPU address, as a result.
     * Every 8 lines, we have to increment to a new y component for
     * the PPU address. */
    if (7 == ppu.scrolly) {
        register u16 y = (ppu.v_addr & 0x03E0) >> 5;
        /* Swap vertical nametable. */
        if (y == 29) {
            y = 0;
            ppu.v_addr ^= 0x0800;
        } else if (y == 31) {
            y = 0;  /* nametable doesn't get swapped */
        } else {
            y++;
        }
        ppu.v_addr = (ppu.v_addr & 0xFC1F) | (y << 5);
    }
    ppu.scrolly = (ppu.scrolly + 1) & 7;    /* & 7 <=> % 8 */
    
    /* Dot 257 */
    ppu.v_addr = (ppu.v_addr & 0x3BE0) | (ppu.t_addr & 0x041F);

}

static void Composite_Scanline(i16 scanline, u16 *background, u16 *spr_back, u16 *spr_front) {
    u16 i;
    //char buf[256];
    /* For now, only the background gets drawn. */
    for (i = 0; i < 256; i++) {
        //buf[i] = (background[i]) ? '0' + background[i] : ' ';
        render_data[(scanline * NES_RES_X) + i] = nes_palette[ppu.palette[background[i] & 0x000F]];
    }
}

void Dump_Render(char *file) {
    int x, y;
    FILE *fp = fopen(file, "w");
    FILE *fp2 = fopen("index.gfx", "w");
#if 0
    for (y = 0; y < 0x2000; y++) {
        fprintf(fp, "%02X ", ppu.nt[y]);
        if (y && !(y % 0x10)) fprintf(fp, "\n");
    }
#else
    fwrite(render_data, sizeof(u32), NES_RES_X * NES_RES_Y, fp);
    for (y = 0; y < 240; y++) {
        for (x = 0; x < 256; x++) {
            fprintf(fp2, "%04x ", render_nums[(y * NES_RES_X) + x]);
        }
        fprintf(fp2, "\n");
    }
#endif
    fclose(fp);
    fclose(fp2);
}

/* Dumps the pattern table in a 16x16 grid of 8x8 patterns. */
u32 pt_palette[4] = {0, 0x55555555, 0xCCCCCCCC, 0xFFFFFFFF};
void Dump_Pattern_Tables(void) {
    u16 base = 0, i, j;
    /* Iterate through all of the pattern table data */
    i16 x, y;
    for (base = 0; base < 2; base++) {
        for (i = 0; i < 16; i++) {
            for (y = 0; y < 8; y++) {
                for (j = 0; j < 16; j++) {
                    /* Get the pattern data for this particular line. */
                    u8 t1 = Read_Cartridge_Chr((base * 0x1000) + (i * 0x100) + (j * 0x10) + y);
                    u8 t2 = Read_Cartridge_Chr((base * 0x1000) + (i * 0x100) + (j * 0x10) + 8 + y);
                    
                    /* Composite the pattern data, bit by bit */
                    for (x = 7; x > -1; x--) {
                        u8 index = (t1 >> (x) & 1) | (((t2 >> x) & 1) << 1);
                        fwrite(pt_palette + index, sizeof(u32), 1, stdout);
                    }
                }
            }
        }
    }
}

/* Dumps the name table into a file */
void Dump_Name_Tables(void) {
    u8 index; u16 x, y;
    FILE *fp = fopen("nt.map", "w");
    for (index = 0; index < 4; index++) {
        fprintf(fp, "[Name table %u]\n", index);
        for (y = 0; y < 240; y++) {
            for (x = 0; x < 256; x++) {
                fprintf(fp, "%03u ", ppu.nt_map[index][((y / 8) * 32) + (x / 8)]);
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void Dump_Attr_Tables(void) {
    u8 index; u16 x, y;
    FILE *fp = fopen("at.data", "w");
    for (index = 0; index < 4; index++) {
        fprintf(fp, "[Name table %u]\n", index);
        for (y = 0; y < 8; y++) {
            fprintf(fp, "\t");
            for (x = 0; x < 8; x++) {
                fprintf(fp, "%03x ", ppu.nt_map[index][0x3C0 + (y * 8) + x]);
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

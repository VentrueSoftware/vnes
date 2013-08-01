/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 31-Jul-2013
 * File: ppu.c
 * 
 * Description:
 * 
 *      Picture Processing Unit implementation.
 * 
 * Change Log:
 *      31-Jul-2013:
 *          File created.
 */

#include "ppu.h"
#include "bitwise.h"
#include "cart.h"

#define MIRROR_HORIZONTAL   0
#define MIRROR_VERTICAL     1
#define MIRROR_FOUR_SCREEN  2

/* Local function declarations */
static u8 Read_Vram(u16 addr);
static void Write_Vram(u16 addr, u8 byte);

/* Render functions */
static void Render_Scanline(void);

ppu_2c02 ppu;

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


/* Initialize PPU */
INLINED void Ppu_Init(void) {
    ppu.status = 0;
    ppu.mask = 0;
    ppu.ctrl = 0;
    ppu.oamaddr = 0;
    ppu.latch = 0;
    ppu.addr = 0;
    ppu.scanline = 241;
}

INLINED void Set_Nametable_Mirroring(u8 mode) {
    switch (mode) {
        case MIRROR_HORIZONTAL:
            ppu.nt_map[0] = ppu.nt_map[1] = ppu.nt;
            ppu.nt_map[2] = ppu.nt_map[3] = ppu.nt + 0x400;
            break;
        case MIRROR_VERTICAL:
            ppu.nt_map[0] = ppu.nt_map[2] = ppu.nt;
            ppu.nt_map[1] = ppu.nt_map[3] = ppu.nt + 0x400;
            break;
        case MIRROR_FOUR_SCREEN:
            ppu.nt_map[0] = ppu.nt;
            ppu.nt_map[1] = ppu.nt + 0x400;
            ppu.nt_map[2] = ppu.nt + 0x800;
            ppu.nt_map[3] = ppu.nt + 0xC00;
            break;
    }
}

INLINED void Ppu_Add_Cycles(u32 cycles) {
	ppu.cycles += cycles;
	/* Check for rendering code */
	if (ppu.cycles > 340) {
		ppu.cycles -= 340;
		Render_Scanline();
	}
}

/* READS */

/* Get PPUSTATUS/  This also clears the VBLANK_STARTED flag and
 * the address latch for PPUSCROLL and PPUADDR */
INLINED u8 Read_Ppu_Status(void) {
    register u8 status = ppu.status;
    FLAG_CLEAR(ppu.status, VBLANK_STARTED);
    ppu.latch = 0;
    return status;
}

/* Read OAMDATA */
INLINED u8 Read_Oam_Data(void) {
    return ppu.oam[ppu.oamaddr];
}

/* Read PPUDATA */
INLINED u8 Read_Ppu_Data(void) {
    register u8 value = 0;
    value = Read_Vram(ppu.addr);
    ppu.addr += (ppu.status & VRAM_INCREMENT) ? 32 : 1;
    return value;
}


/* WRITES */

/* Set the PPUCTRL flags. */
INLINED void Write_Ppu_Ctrl(u8 byte) {
    ppu.ctrl = byte;
}

/* Set the PPUMASK flags. */
INLINED void Write_Ppu_Mask(u8 byte) {
    ppu.mask = byte;
}

/* Set the OAMADDR */
INLINED void Write_Ppu_Oam_Addr(u8 byte) {
    ppu.oamaddr = byte;
}

/* Set the OAMDATA */
INLINED void Write_Ppu_Oam_Data(u8 byte) {
    ppu.oam[ppu.oamaddr++] = byte;
}

/* Set PPUSCROLL */
INLINED void Write_Ppu_Scroll(u8 byte) {
    if (0 == ppu.latch) {
        ppu.latch = 1;
        ppu.scroll = ((u16)(byte)) << 8;
    } else {
        ppu.latch = 0;
        ppu.scroll |= byte;
    }
}

/* Set PPUADDR */
INLINED void Write_Ppu_Addr(u8 byte) {
    if (0 == ppu.latch) {
        ppu.latch = 1;
        ppu.addr = ((u16)(byte)) << 8;
    } else {
        ppu.latch = 0;
        ppu.addr |= byte;
    }
}

/* Set PPUDATA */
INLINED void Write_Ppu_Data(u8 byte) {
    Write_Vram(ppu.addr, byte);
    ppu.addr += (ppu.ctrl & VRAM_INCREMENT) ? 32 : 1;
}

/* Local function declarations */

/* Read from VRAM */
static u8 Read_Vram(u16 addr) {
    register u8 value = 0;
    addr &= 0x3FFF;
    if (addr < 0x2000) return Read_Cartridge_Chr(addr);
    else if (addr < 0x3F00) {
        /* Resolve address using nametable mirror map and offset. */
        register u8 index = (addr % 0x1000) / 0x400;
        register u16 offset = addr % 0x400;
        /* We technically lag by one fetch. */
        value = ppu.vram_value;
        ppu.vram_value = ppu.nt_map[index][offset];
        return value;
    } else if (addr < 0x3F20) {
        /* Palette data */
        return ppu.palette[addr % 0x10];
    }
    return 0xFF;
}

static void Write_Vram(u16 addr, u8 value) {
    addr &= 0x3FFF;
    if (addr < 0x2000) {/* Write_Cartridge_Chr(addr, value) */}
    else if (addr < 0x3F00) {
        /* Resolve address using nametable mirror map and offset. */
        register u8 index = (addr % 0x1000) / 0x400;
        register u16 offset = addr % 0x400;
        ppu.nt_map[index][offset] = value;
    } else if (addr < 0x3F20) {
        /* Palette data */
        ppu.palette[addr % 0x10] = value;
    }    
}

/* Rendering Function Definitions */
static void Render_Scanline(void) {
	ppu.scanline = (ppu.scanline == 260) ? -1 : ppu.scanline + 1;
}

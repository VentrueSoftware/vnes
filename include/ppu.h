/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 31-Jul-2013
 * File: ppu.h
 * 
 * Description:
 * 
 *      Picture Processing Unit implementation.
 * 
 * Change Log:
 *      31-Jul-2013:
 *          File created.
 */
#ifndef VNES_PPU_H
#define VNES_PPU_H

#include "types.h"

/* CPU's Memory-mapped PPU Registers */
#define PPUCTRL   0x2000
#define PPUMASK   0x2001
#define PPUSTATUS 0x2002
#define OAMADDR   0x2003
#define OAMDATA   0x2004
#define PPUSCROLL 0x2005
#define PPUADDR   0x2006
#define PPUDATA   0x2007

/* PPUCTRL Flags */
#define NAMETABLE_BASE      0x03
#define VRAM_INCREMENT      0x04
#define SPRITE_PTRN_TABLE   0x08
#define BG_PTRN_TABLE       0x10
#define SPRITE_SIZE         0x20
#define EXT_PIN_SELECT      0x40
#define NMI_ON_VBLANK       0x80

/* PPUMASK Flags */
#define MASK_GRAYSCALE      0x01
#define CLIP_BG             0x02
#define CLIP_SPRITES        0x04
#define SHOW_BG             0x08
#define SHOW_SPRITES        0x10
#define INTENSIFY_REDS      0x20
#define INTENSIFY_GREENS    0x40
#define INTENSIFY_BLUES     0x80

/* PPUSTATUS flags */
#define LSB_OF_PPU      0x1F
#define SPRITE_OVERFLOW 0x20
#define SPRITE0_HIT     0x40
#define VBLANK_STARTED  0x80

typedef struct ppu_2c02 {
	/* PPU Emulation Info */
	i16 scanline;
	u32 cycles;
	
    u8 last_write;  /* The value last written to PPU */
    
    /* Registers */
    u8 ctrl;        /* PPUCTRL */
    u8 mask;        /* PPUMASK */
    u8 status;      /* PPUSTATUS */
    u8 oamaddr;     /* OAM Address */
    
    u8 latch;       /* Address Latch for PPUSCROLL/PPUADDR */
    
    u16 scrollx; 	/* Scroll x */
    u16 scrolly;    /* Scroll y */
    
    u16 addr;       /* VRAM Address (PPUADDR) */
    u16 v_addr;	    /* VRAM Address (PPUADDR) */
    u16 t_addr;	    /* Temporary VRAM address */
    
    u8 vram_value;  /* VRAM value last fetched. */
    
    /* Data storage */
    u8 nt[0x2000];
    u8 *nt_map[4];
    u8 palette[0x10];
    u8 oam[0x100];
} ppu_2c02;

INLINED void Ppu_Init(void);
INLINED void Set_Nametable_Mirroring(u8 mode);
INLINED void Ppu_Add_Cycles(u32 cycles);

/* Reads coming from CPU */
u8 Read_Ppu(u16 addr);

/* Writes coming from CPU */
void Write_Ppu(u16 addr, u8 value);

#endif /* #ifndef VNES_PPU_H */

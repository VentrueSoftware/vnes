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

#include <string.h>
#include "ppu.h"
#include "cpu.h"
#include "bitwise.h"
#include "cart.h"
#include "render.h"

#define PPU_POWERUP_NTSC 29658

#define MIRROR_HORIZONTAL   0
#define MIRROR_VERTICAL     1
#define MIRROR_FOUR_SCREEN  2

/* Local function declarations */

static INLINED u8 Read_Ppu_Status(void);
static INLINED u8 Read_Oam_Data(void);
static INLINED u8 Read_Ppu_Data(void);

static INLINED void Write_Ppu_Ctrl(u8 byte);
static INLINED void Write_Ppu_Mask(u8 byte);
static INLINED void Write_Ppu_Oam_Addr(u8 byte);
static INLINED void Write_Ppu_Oam_Data(u8 byte);
static INLINED void Write_Ppu_Scroll(u8 byte);
static INLINED void Write_Ppu_Addr(u8 byte);
static INLINED void Write_Ppu_Data(u8 byte);

/* VRAM Operations */
static u8 Read_Vram(u16 addr);
static void Write_Vram(u16 addr, u8 byte);

        extern void Log_Line(const char *format, ...);

ppu_2c02 ppu;

/* Initialize PPU */
INLINED void Ppu_Init(void) {
    int i;
    ppu.status = VBLANK_STARTED | SPRITE_OVERFLOW;
    ppu.mask = 0;
    ppu.ctrl = 0;
    ppu.oamaddr = 0;
    ppu.latch = 0;
    ppu.v_addr = 0;
    ppu.scanline = 0;
    memset(ppu.nt, 0xFF, sizeof(u8) * 0x2000);
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
        case MIRROR_FOUR_SCREEN: default:
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
        
        ppu.scanline = (ppu.scanline == 260) ? -1 : ppu.scanline + 1;
        
        Render_Scanline(ppu.scanline);
        if (ppu.scanline == 241) {
            Log_Line("Palette: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                ppu.palette[0], ppu.palette[1], ppu.palette[2], ppu.palette[3], ppu.palette[4], ppu.palette[5], 
                ppu.palette[6], ppu.palette[7], ppu.palette[8], ppu.palette[9], ppu.palette[10], ppu.palette[11], 
                ppu.palette[12], ppu.palette[13], ppu.palette[14], ppu.palette[15]);
            FLAG_SET(ppu.status, VBLANK_STARTED);
            if (IS_SET(ppu.ctrl, NMI_ON_VBLANK)) {
                Log_Line("Executing NMI!");
                Cpu_Nmi();
            }
            Dump_Render("screen.data");
            Dump_Name_Tables();
            Dump_Attr_Tables();
            Log_Line("Frame written. CPU cycles: %u", Cpu_Get_Cycles());
        }
    }
}

/* Read/Write */
u8 Read_Ppu(u16 addr) {
    switch (addr) {
        case PPUSTATUS: return Read_Ppu_Status();
        case OAMDATA: return Read_Oam_Data();
        case PPUDATA: return Read_Ppu_Data();
        default:
            printf("Unrecognized PPU Register read address (0x%04X)!\n", addr);
    }
    return 0xFF;
}

void Write_Ppu(u16 addr, u8 value) {
    ppu.last_write = value;
    //Log_Line("Writing to PPU address %04x, value %02x", addr, value);
    switch (addr) {
        case PPUCTRL:
            if (Cpu_Get_Cycles() > PPU_POWERUP_NTSC) Write_Ppu_Ctrl(value); 
        break;
        case PPUMASK: 
            if (Cpu_Get_Cycles() > PPU_POWERUP_NTSC) Write_Ppu_Mask(value);
        break;
        case OAMADDR: Write_Ppu_Oam_Addr(value); break;
        case OAMDATA: Write_Ppu_Oam_Data(value); break;
        case PPUSCROLL: 
            if (Cpu_Get_Cycles() > PPU_POWERUP_NTSC) Write_Ppu_Scroll(value);
        break;
        case PPUADDR: 
            if (Cpu_Get_Cycles() > PPU_POWERUP_NTSC) Write_Ppu_Addr(value); 
        break;
        case PPUDATA: Write_Ppu_Data(value); break;
        default: 
            printf("Unrecognized PPU Register address (0x%04X)!\n", addr);
    }
}

/* Local function declarations */

/* Get PPUSTATUS/  This also clears the VBLANK_STARTED flag and
 * the address latch for PPUSCROLL and PPUADDR */
static INLINED u8 Read_Ppu_Status(void) {
    register u8 status = ppu.status;
    status |= ppu.last_write & LSB_OF_PPU;
    FLAG_CLEAR(ppu.status, VBLANK_STARTED);
    ppu.latch = 0;
    return status;
}

/* Read OAMDATA */
static INLINED u8 Read_Oam_Data(void) {
    return ppu.oam[ppu.oamaddr];
}

/* Read PPUDATA */
static INLINED u8 Read_Ppu_Data(void) {
    register u8 value = 0;
    value = Read_Vram(ppu.v_addr);
    ppu.v_addr += (ppu.status & VRAM_INCREMENT) ? 32 : 1;
    return value;
}


/* WRITES */
/* Set the PPUCTRL flags. */
static INLINED void Write_Ppu_Ctrl(u8 value) {
    Log_Line("Writing to PPUCTRL, value: %02X", value);
    ppu.ctrl = value;
    ppu.t_addr = (ppu.t_addr & 0xF3FF) | (((u16)(value & 0x03)) << 10);
}

/* Set the PPUMASK flags. */
static INLINED void Write_Ppu_Mask(u8 value) {
    ppu.mask = value;
}

/* Set the OAMADDR */
static INLINED void Write_Ppu_Oam_Addr(u8 value) {
    ppu.oamaddr = value;
}

/* Set the OAMDATA */
static INLINED void Write_Ppu_Oam_Data(u8 value) {
    ppu.oam[ppu.oamaddr++] = value;
}

/* Set PPUSCROLL
 * Important note: scrollx and scrolly are the demux'd equivalent of
 * the PPUSCROLL register.  This register controls the offset within
 * a tile.  Since tiles are 8x8, it only makes sense to limit values to
 * 0-7, which is done with a logical AND with 0x07. */
static INLINED void Write_Ppu_Scroll(u8 value) {
    if (0 == ppu.latch) {
        ppu.t_addr = (ppu.t_addr & 0xFFE0) | ((value & 0xF8) >> 3);
        ppu.scrollx = value & 0x07;
        ppu.latch = 1;
    } else {
        ppu.t_addr = (ppu.t_addr & 0xFC1F) | ((value & 0xF8) >> 3);
        ppu.scrolly = value & 0x07;
        ppu.latch = 0;
    }
}

/* Set PPUADDR */
static INLINED void Write_Ppu_Addr(u8 value) {
    if (0 == ppu.latch) {
        ppu.t_addr = (((u16)(value) & 0x3F) << 8) | (ppu.t_addr & 0x00FF);
        ppu.t_addr = ppu.t_addr & 0x3FFF;
        ppu.latch = 1;
    } else {
        ppu.t_addr = (ppu.t_addr & 0xFF00) | (u16)(value);
        ppu.v_addr = ppu.t_addr;
        ppu.latch = 0;
    }
}

/* Set PPUDATA */
INLINED void Write_Ppu_Data(u8 value) {
    Write_Vram(ppu.v_addr, value);
    //Log_Line("Wrote value %02X to %04X", value, ppu.v_addr);
    ppu.v_addr += (ppu.ctrl & VRAM_INCREMENT) ? 32 : 1;
}

/* Read from VRAM */
static u8 Read_Vram(u16 addr) {
    register u8 value = 0;
    addr &= 0x3FFF;
    if (addr < 0x2000) return Read_Cartridge_Chr(addr);
    else if (addr < 0x3F00) {
        /* Resolve address using nametable mirror map and offset. */
        register u8 index = (addr >> 10) & 0x03;
        register u16 offset = addr & 0x3FF;
        /* We technically lag by one fetch. */
        value = ppu.vram_value;
        ppu.vram_value = ppu.nt_map[index][offset];
        return value;
    } else {
        /* Palette data */
        return ppu.palette[addr & 0x0F];
    }
    return 0xFF;
}

static void Write_Vram(u16 addr, u8 value) {
    addr &= 0x3FFF;
    if (addr < 0x2000) {
#if 0
        Log_Line("Invalid write address %04X: %02X", addr, value);/* Write_Cartridge_Chr(addr, value) */
#endif
    }
    else if (addr < 0x3F00) {
        /* Resolve address using nametable mirror map and offset. */
        register u8 index = (addr >> 10) & 0x03;
        register u16 offset = addr & 0x3FF;
        ppu.nt_map[index][offset] = value;
    } else {
        /* Palette data */
        //Log_Line("Adding palette data. %04x = %02x", addr, value);
        ppu.palette[addr & 0x0F] = value;
    }    
}

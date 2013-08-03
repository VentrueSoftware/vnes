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
#include "cpu.h"
#include "bitwise.h"
#include "cart.h"
#include "render.h"

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
    ppu.status = 0;
    ppu.mask = 0;
    ppu.ctrl = 0;
    ppu.oamaddr = 0;
    ppu.latch = 0;
    ppu.addr = 0;
    ppu.scanline = 0;
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
        
        /* Advance scanline */
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
            Log_Line("Frame written.");
            Dump_Name_Tables();
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
        case PPUCTRL: Write_Ppu_Ctrl(value); break;
        case PPUMASK: Write_Ppu_Mask(value); break;
        case OAMADDR: Write_Ppu_Oam_Addr(value); break;
        case OAMDATA: Write_Ppu_Oam_Data(value); break;
        case PPUSCROLL: Write_Ppu_Scroll(value); break;
        case PPUADDR: Write_Ppu_Addr(value); break;
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
    value = Read_Vram(ppu.addr);
    ppu.addr += (ppu.status & VRAM_INCREMENT) ? 32 : 1;
    return value;
}


/* WRITES */
/* Set the PPUCTRL flags. */
static INLINED void Write_Ppu_Ctrl(u8 value) {
    Log_Line("Writing to PPUCTRL, value: %02X", value);
    ppu.ctrl = value;
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
        ppu.latch = 1;
        ppu.scrollx = value & 0x07;
    } else {
        ppu.latch = 0;
        ppu.scrolly = value & 0x07;
    }
}

/* Set PPUADDR */
static INLINED void Write_Ppu_Addr(u8 value) {
    if (0 == ppu.latch) {
        ppu.latch = 1;
        ppu.addr = ((u16)(value)) << 8;
    } else {
        ppu.latch = 0;
        ppu.addr |= value;
    }
}

/* Set PPUDATA */
INLINED void Write_Ppu_Data(u8 value) {
    Write_Vram(ppu.addr, value);
    //Log_Line("Wrote value %02X to %04X", value, ppu.addr);
    ppu.addr += (ppu.ctrl & VRAM_INCREMENT) ? 32 : 1;
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
    if (addr < 0x2000) {Log_Line("Invalid write address %04X: %02X", addr, value);/* Write_Cartridge_Chr(addr, value) */}
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

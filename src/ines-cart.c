/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 30-Jul-2013
 * File: ines-cart.c
 * 
 * Description:
 * 
 *      iNES Cartridge format loader/accessor.
 * 
 * Change Log:
 *      30-Jul-2013:
 *          File created.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ines-cart.h"

/* Load/Free PRG/CHR ROM pages */
static void Load_iNES_Pages(ines_cart *cart, FILE *fp);
static void Free_iNES_Pages(ines_cart *cart);

/* For now, we define mapper 0 functions in this file. */
static u8 Read_iNES0_Prg(icart *cart, u16 address);
static u8 Read_iNES0_Chr(icart *cart, u16 address);

static u8 Write_iNES0_Prg(icart *cart, u16 address, u8 value);
static u8 Write_iNES0_Chr(icart *cart, u16 address, u8 value);

static void Unload_iNES0(icart *cart);

icart *Load_iNES(FILE *fp) {
    ines_cart *cart;
    u8 header[12];      /* Header container */
    u8 mapper_id;       /* Mapper ID extracted from header */
    
    if (12 != fread(header, sizeof(u8), 12, fp)) {
        /* Error(Unexpected end of file) */
        return 0;
    }
    
    mapper_id = (header[3] >> 4) | (header[2] >> 4);
    /* Later, we'll want a jump table here for different mappers */
    if (0 == mapper_id) {
        cart = (ines_cart *)malloc(sizeof(ines_cart));
        bzero(cart, sizeof(ines_cart));
        
        cart->type = CART_INES;
        cart->Read_Prg = Read_iNES0_Prg;
        cart->Read_Chr = Read_iNES0_Chr;
        cart->Write_Prg = Write_iNES0_Prg;
        cart->Write_Chr = Write_iNES0_Chr;
        cart->Unload = Unload_iNES0;
    } else {
        return 0;
    }
    
    cart->mapper_id = mapper_id;
    cart->prg_pages = header[0];
    cart->chr_pages = header[1];
    
    /* Set mirror mode */{
    extern void Set_Nametable_Mirroring(u8 mode);
    cart->flags = ((header[2] & 0x08) >> 2) | (header[2] & 0x01);
    Set_Nametable_Mirroring(cart->flags & 0x03);
    }/* Set other flags */
    cart->flags |= (header[2] & 0x02) ? INES_HAS_SAVERAM : 0;
    cart->flags |= (header[2] & 0x04) ? INES_HAS_TRAINER : 0;
    cart->flags |= (header[5] & 0x01) ? INES_IS_PAL : 0;
    
    Load_iNES_Pages(cart, fp);
    
    printf(
        "Cartridge loaded:\n"
        "\tMapper: %u\n"
        "\tPRG Pages: %u\n"
        "\tCHR Pages: %u\n"
        "\tMirror Mode: %u\n"
        "\tHas Trainer: %s\n"
        "\tHas Save RAM: %s\n"
        "\tVideo type: %s\n",
        cart->mapper_id, cart->prg_pages, cart->chr_pages, cart->flags & INES_MIRROR_MASK,
        (cart->flags & INES_HAS_TRAINER) ? "yes" : "no",
        (cart->flags & INES_HAS_SAVERAM) ? "yes" : "no",
        (cart->flags & INES_IS_PAL) ? "PAL" : "NTSC");
    
    return (icart *)cart;
}

/* PRG/CHR Page loading */

static void Load_iNES_Pages(ines_cart *cart, FILE *fp) {
    u8 i;
    
    /* Read PRG and CHR ROMs */
    cart->prg_rom = (u8 **)malloc(sizeof(u8 *) * cart->prg_pages);
    bzero(cart->prg_rom, sizeof(u8 *) * cart->prg_pages);
    for (i = 0; i < cart->prg_pages; i++) {
        cart->prg_rom[i] = (u8 *)malloc(sizeof(u8) * INES_PRG_PAGE_SIZE);
        if (INES_PRG_PAGE_SIZE != fread(cart->prg_rom[i], sizeof(u8), INES_PRG_PAGE_SIZE, fp)) {
            /* Error(Unexpected end of file while reading PRG data) */
            //goto err;
        }
    }

    cart->chr_rom = (u8 **)malloc(sizeof(u8 *) * cart->chr_pages);
    bzero(cart->chr_rom, sizeof(u8 *) * cart->chr_pages);
    for (i = 0; i < cart->chr_pages; i++) {
        cart->chr_rom[i] = (u8 *)malloc(sizeof(u8) * INES_CHR_PAGE_SIZE);
        if (INES_CHR_PAGE_SIZE != fread(cart->chr_rom[i], sizeof(u8), INES_CHR_PAGE_SIZE, fp)) {
            /* Error(Unexpected end of file while reading CHR data) */
            //goto err;
        }
    }
}

static void Free_iNES_Pages(ines_cart *cart) {
    u8 i;
    
    if (cart) {
        if (cart->prg_rom) {
            for (i = 0; i < cart->prg_pages; i++) {
                free(cart->prg_rom[i]);
            }
            free(cart->prg_rom);
        }
        if (cart->chr_rom) {
            for (i = 0; i < cart->chr_pages; i++) {
                free(cart->chr_rom[i]);
            }
            free(cart->chr_rom);
        }
    }
}

/* For now, we define mapper 0 functions in this file. */
static u8 Read_iNES0_Prg(icart *cart, u16 address) {
    ines_cart *c = (ines_cart *)cart;
    u16 index = ((address - 0x8000) / INES_PRG_PAGE_SIZE) % c->prg_pages;
    return c->prg_rom[index][(address - 0x8000) % INES_PRG_PAGE_SIZE];
}
static u8 Read_iNES0_Chr(icart *cart, u16 address) {
    ines_cart *c = (ines_cart *)cart;
    u16 index = (address / INES_CHR_PAGE_SIZE);
    return c->chr_rom[index][address % INES_PRG_PAGE_SIZE];
}

static u8 Write_iNES0_Prg(icart *cart, u16 address, u8 value) {
    return 0;
}
static u8 Write_iNES0_Chr(icart *cart, u16 address, u8 value) {
    return 0;
}

static void Unload_iNES0(icart *cart) {
    ines_cart *c = (ines_cart *)cart;
    Free_iNES_Pages(c);
    free(c);
}

/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 30-Jul-2013
 * File: ines-cart.h
 * 
 * Description:
 * 
 *      iNES Cartridge format loader/accessor.
 * 
 * Change Log:
 *      30-Jul-2013:
 *          File created.
 */

#ifndef VNES_INES_CART_H
#define VNES_INES_CART_H

#include "icart.h"

#define CART_INES 0x01

/* iNES cartridge constants, flags, masks, etc. */
#define INES_PRG_PAGE_SIZE (16 * 1024)
#define INES_CHR_PAGE_SIZE (8 * 1024)

#define INES_MIRROR_MASK        0x03
#define INES_MIRROR_VERTICAL    0x00
#define INES_MIRROR_HORIZONTAL  0x01
#define INES_MIRROR_FOUR_SCREEN 0x10

#define INES_HAS_TRAINER 0x04
#define INES_HAS_SAVERAM 0x08
#define INES_IS_PAL      0x10

#define VNES_INES_CART_INTERFACE                                       \
    VNES_CART_INTERFACE                                                \
    u8 mapper_id;                                                      \
    u8 prg_pages;                                                      \
    u8 chr_pages;                                                      \
                                                                       \
    u8 **prg_rom;                                                      \
    u8 **chr_rom;                                                      \
                                                                       \
    u8 flags;       /* Mirror type, trainer, save RAM, NTSC/PAL */     \
    u8 *trainer;                                                       \
    
typedef struct ines_cart {
    VNES_INES_CART_INTERFACE
} ines_cart;

icart *Load_iNES(FILE *fp);

#endif /* #ifndef VNES_INES_CART_H */

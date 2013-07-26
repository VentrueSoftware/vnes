/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 25-Jul-2013
 * File: cart.h
 * 
 * Description:
 * 
 *      Cartridge loader interface.
 * 
 * Change Log:
 *      25-Jul-2013:
 *          File created.
 */

#ifndef VNES_CART_H
#define VNES_CART_H

#include "types.h"

/* Forward declarations */
struct vnes_icart;
typedef struct vnes_icart vnes_icart;

/* Read/write handler definitions */
typedef u8 (*cart_read_fn)(vnes_icart, u16);
typedef u8 (*cart_write_fn)(vnes_icart, u16);

/* VNES Cartridge Interface.  VNES_CART_INTERFACE must be the first part
 * of any struct that implements a mapper for a particular cartridge.
 * This is quite flexible and can be extended to work with non iNES
 * formats (raw 6502 binaries, for instance). */
struct vnes_icart {
#define VNES_CART_INTERFACE                                            \
    u8 mapper_id;                                                      \
    u8 prg_pages;                                                      \
    u8 chr_pages;                                                      \
    u8 **prg_rom;                                                      \
    u8 **chr_rom;                                                      \
                                                                       \
    /* Read/write function handlers */                                 \
    cart_read_fn read_prg;                                             \
    cart_read_fn read_chr;                                             \
                                                                       \
    cart_write_fn write_prg;                                           \
    cart_write_fn write_chr;                                           \
    
};

vnes_icart *Load_Cartridge(char *filename);

#endif /* #ifndef VNES_CART_H */

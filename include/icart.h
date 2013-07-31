/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 30-Jul-2013
 * File: icart.h
 * 
 * Description:
 * 
 *      Cartridge interface.
 * 
 * Change Log:
 *      30-Jul-2013:
 *          File created.
 */

#ifndef VNES_ICART_H
#define VNES_ICART_H

#include "types.h"

/* Forward declarations */
struct icart;
typedef struct icart icart;

/* Read/write handler definitions */
typedef u8 (*cart_read)(icart *, u16);
typedef u8 (*cart_write)(icart *, u16, u8);
typedef void (*cart_delete)(icart *);

/* VNES Cartridge Interface.  VNES_CART_INTERFACE must be the first part
 * of any struct that implements a mapper for a particular cartridge.
 * This is quite flexible and can be extended to work with non iNES
 * formats (raw 6502 binaries, for instance). */

#define VNES_CART_INTERFACE                                            \
    /* Cartridge type */                                               \
    u8 type;                                                           \
    /* Read function handlers */                                       \
    cart_read Read_Prg;                                                \
    cart_read Read_Chr;                                                \
                                                                       \
    /* Write function handlers */                                      \
    cart_write Write_Prg;                                              \
    cart_write Write_Chr;                                              \
                                                                       \
    /* Unload/delete function handler */                               \
    cart_delete Unload;                                                \

struct icart {
    VNES_CART_INTERFACE
};

#endif /* #ifndef VNES_ICART_H */

/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 25-Jul-2013
 * File: cart.h
 * 
 * Description:
 * 
 *      Cartridge load/access methods.
 * 
 * Change Log:
 *      30-Jul-2013:
 *          Moved cartridge interface to its own header file.  This file
 *          only provides the wrappers to the cartridge.
 *      25-Jul-2013:
 *          File created.
 */

#ifndef VNES_CART_H
#define VNES_CART_H

#include "types.h"

void Load_Cartridge(char *filename);

u8 Read_Cartridge_Prg(u16 address);
u8 Read_Cartridge_Chr(u16 address);

u8 Write_Cartridge_Prg(u16 address, u8 value);
u8 Write_Cartridge_Chr(u16 address, u8 value);

void Unload_Cartridge(void);

#endif /* #ifndef VNES_CART_H */

/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 26-Jul-2013
 * File: cart.c
 * 
 * Description:
 * 
 *      Cartridge loader interface.
 * 
 * Change Log:
 *      26-Jul-2013:
 *          File created.
 */

#include <stdio.h>
#include "cart.h"
#include "icart.h"
#include "ines-cart.h"
#include "types.h"
#include "bitwise.h"

icart *g_cart = 0;

/* Load cartridge, with different options based on file format */
void Load_Cartridge(char *filename) {
    u8 format[4];
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        /* Error(Failed to open file) */
        return;
    }
    
    fread(format, sizeof(u8), 4, fp);
    if (0x4E == format[0] && 0x45 == format[1] && 0x53 == format[2] &&
        0x1A == format[3]
    ) {
            g_cart = Load_iNES(fp);
    }
    fclose(fp);
}

u8 Read_Cartridge_Prg(u16 address) {
    if (g_cart) {
        return g_cart->Read_Prg(g_cart, address);
    }
    return 0xFF;
}

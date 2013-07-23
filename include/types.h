/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 18-Jul-2013
 * File: types.h
 * 
 * Description:
 * 
 *      Common type definitions used throughout vnes.
 * 
 * Change Log:
 *      18-Jul-2013:
 *          File created.
 */

#ifndef VNES_TYPES_H
#define VNES_TYPES_H

#include <stdio.h>

#ifdef USE_INLINING
#define INLINED inline
#else
#define INLINED 
#endif /* #ifdef USE_INLINING */

#ifdef USE_STDINT
#include <stdint.h>

typedef uint8_t     u8;
typedef int8_t      i8;
typedef uint16_t    u16;
typedef int16_t     i16;
typedef uint32_t    u32;
typedef int32_t     i32;

#else

typedef unsigned char   u8;
typedef char            i8;
typedef unsigned short  u16;
typedef short           i16;
typedef unsigned int    u32;
typedef int             i32;

#endif /* #ifdef USE_STDINT */

#define neslog(...) /*printf*/

#define VNES_Err int

#endif /* #ifndef VNES_TYPES_H */

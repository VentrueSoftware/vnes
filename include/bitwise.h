/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 18-Jul-2013
 * File: bitwise.h
 * 
 * Description:
 * 
 *      Common bit manipulation macros (flag set/clear, etc.)
 * 
 * Change Log:
 *      18-Jul-2013:
 *          File created.
 */

#ifndef VNES_BITWISE_H
#define VNES_BITWISE_H

#define FLAG_GET(var, flag) (((var) & (flag)) ? 1 : 0)

#define FLAG_SET(var, flags) \
    ((var) |= (flags))
    
#define FLAG_CLEAR(var, flags) \
    ((var) &= ~(flags))

#define FLAG_COND(cond, var, flags) \
    (cond) ? FLAG_SET(var, flags) : FLAG_CLEAR(var, flags)

#define IS_SET(var, flag) \
    ((var) & (flag))

#define PAGE_OF(x) \
	((x) & 0xFF00)

/* Byte-related manipulation */
#define TO_U16(lb, ub) \
    ((u16)((lb) | ((ub) << 8)))

#define LB(v) \
    ((u8)((v) & 0xFF))

#define UB(v) \
    ((u8)((v) >> 8))
#endif /* #ifndef VNES_BITWISE_H */

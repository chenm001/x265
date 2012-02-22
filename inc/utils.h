/*****************************************************************************
 * utils.h: Util functions
 *****************************************************************************
 * Copyright (C) 2011-2012 x265 project
 *
 * Authors: Min Chen <chenm003@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at chenm003@163.com.
 *****************************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__

static int xLog2(UInt32 x)
{
    UInt32 log2Size = 0;
    while(x > 0) {
        x >>= 1;
        log2Size++;
    }
    return(log2Size);
}

static Int32 Clip3( Int32 minVal, Int32 maxVal, Int32 a )
{
    if ( a < minVal )
        a = minVal;
    else if ( a > maxVal )
        a = maxVal;
    return a;
}

#define xSHR(x, n)      ( (n)>=32 ? 0 : ((x)>>(n)) )
#define xSHL(x, n)      ( (n)>=32 ? 0 : ((x)<<(n)) )
#define MALLOC(n)       malloc(n)
#define FREE(p)         free(p)
#define ASIZE(x)        ( sizeof(x)/sizeof((x)[0]) )

#endif /* __UTILS_H__ */

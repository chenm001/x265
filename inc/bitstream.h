/*****************************************************************************
 * bitstream.h: Bitstream functions
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

#ifndef __BS_H__
#define __BS_H__

#include "utils.h"

#ifdef __INTEL_COMPILER
    #pragma warning(disable: 177)   // declared but never referenced
#endif


// ***************************************************************************
typedef struct X265_t X265_t;

typedef struct X265_BitStream {
    UInt8      *pucBits;
    UInt32      dwCache;
    Int         nCachedBits;
    UInt8      *pucBits0;
} X265_BitStream;


#define putBits32(dst, x)    *(UInt32*)(dst) = (x);
#define BSWAP32(x)          ( (x<<24) + ((x<<8)&0xff0000) + ((x>>8)&0xff00) + (x>>24) )
#define flushCache(dst, x, bits)    { \
    int _i; \
    for(_i=0; _i < (bits)>>3; _i++) { \
        const UInt _tmp = (x) >> 24; \
        (x) <<= 8; \
        if (   (dst)[-1] == 0 \
            && (dst)[-2] == 0 \
            && _tmp <= 3 ) \
            *(dst)++ = 0x03; \
        *(dst)++ = _tmp; \
    } \
}

// ***************************************************************************
static void xBitStreamInit(X265_BitStream *pBS, UInt8 *pucBuffer, Int nBufferSize)
{
    assert( nBufferSize > 0 );

    pBS->pucBits        =
    pBS->pucBits0       = pucBuffer;
    pBS->dwCache        = 0;
    pBS->nCachedBits    = 0;
}

static void xPutBits32(X265_BitStream *pBS, UInt32 uiBits)
{
    assert( pBS->nCachedBits % 8 == 0 );
    putBits32(pBS->pucBits, uiBits);
    pBS->pucBits += 4;
}

static void xPutBits(X265_BitStream *pBS, UInt32 uiBits, Int nNumBits)
{
    Int nShift = 32 - pBS->nCachedBits - nNumBits;
    
    assert((nNumBits >= 0) && (nNumBits <= 32));
    assert(xSHR(uiBits, nNumBits) == 0);

    if (nShift >= 0) {
        pBS->dwCache     |= xSHL(uiBits, nShift);
        pBS->nCachedBits += nNumBits;
    }
    else {
        UInt32 dwCache = pBS->dwCache;
        dwCache |= xSHR(uiBits, -nShift);

        flushCache(pBS->pucBits, dwCache, 32);
        
        pBS->dwCache = xSHL(uiBits, (32 + nShift));
        pBS->nCachedBits = -nShift;
    }
}

static void xWriteAlignZero(X265_BitStream *pBS)
{
    Int nShift = 8 - (pBS->nCachedBits % 8);
    xPutBits(pBS, 0, nShift);
}

static void xWriteAlignOne(X265_BitStream *pBS)
{
    Int nShift = 8 - (pBS->nCachedBits % 8);
    xPutBits(pBS, (1<<nShift)-1, nShift);
}

static void xWriteRBSPTrailingBits(X265_BitStream *pBS)
{
    xPutBits(pBS, 1, 1);
    xWriteAlignZero(pBS);
}

static Int32 xBitFlush(X265_BitStream *pBS)
{
    flushCache(pBS->pucBits, pBS->dwCache, pBS->nCachedBits);
    pBS->nCachedBits &= 7;
    return (pBS->pucBits - pBS->pucBits0) + (pBS->nCachedBits + 7) / 8;
}


// ***************************************************************************
static void xWriteCode( X265_BitStream *pBS, UInt32 uiCode, UInt32 uiLength )
{
    assert ( uiLength > 0 );
    xPutBits( pBS, uiCode, uiLength );
}

static void xWriteUvlc( X265_BitStream *pBS, UInt32 uiCode )
{
    UInt32 uiLength = xLog2(++uiCode) - 1;

    xPutBits( pBS, 0,       uiLength);
    xPutBits( pBS, uiCode, (uiLength+1));
}

static UInt32 xConvertToUInt( Int32 iValue )
{
    return ( iValue > 0) ? (iValue<<1)-1 : -iValue<<1;
}

static void xWriteSvlc( X265_BitStream *pBS, Int32 iCode )
{
    UInt uiCode;
  
    uiCode = xConvertToUInt( iCode );
    xWriteUvlc( pBS, uiCode );
}

static void xWriteFlag( X265_BitStream *pBS, UInt32 uiCode )
{
    xPutBits( pBS, uiCode, 1 );
}

#endif /* __BS_H__ */

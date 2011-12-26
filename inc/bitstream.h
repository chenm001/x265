/*****************************************************************************
 * bitstream.h: Bitstream functions
 *****************************************************************************
 * Copyright (C) 2011 x265 project
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


#define BSWAP32(x)          ( (x<<24) + ((x<<8)&0xff0000) + ((x>>8)&0xff00) + (x>>24) )
#define putCache(dst, x)    *(UInt32*)(dst) = BSWAP32(x)

// ***************************************************************************
static void xBitStreamInit(X265_BitStream *pBS, UInt8 *pucBuffer, Int nBufferSize)
{
    assert( nBufferSize > 0 );

    pBS->pucBits        =
    pBS->pucBits0       = pucBuffer;
    pBS->dwCache        = 0;
    pBS->nCachedBits    = 0;
}

static void xPutBit(X265_BitStream *pBS, Int nBit)
{
    assert((nBit == 0) || (nBit == 1));
    
    if (pBS->nCachedBits < 32) {
        pBS->dwCache |= nBit << (31 - pBS->nCachedBits);
        pBS->nCachedBits++;
    }
    else {
        putCache(pBS->pucBits, pBS->dwCache);
        pBS->pucBits += 4;
        
        pBS->dwCache = nBit ? (1 << 31) : 0;
        pBS->nCachedBits = 1;
    }
}		

static void xPutBits(X265_BitStream *pBS, UInt32 uiBits, Int nNumBits)
{
    Int nShift = 32 - pBS->nCachedBits - nNumBits;
    
    assert((nNumBits >= 0) && (nNumBits < 32));
    assert((uiBits >> nNumBits) == 0);

    if (nShift >= 0) {
        pBS->dwCache     |= (uiBits << nShift);
        pBS->nCachedBits += nNumBits;
    }
    else {
        UInt32 dwCache = pBS->dwCache;
        dwCache |= uiBits >> -nShift;
        
        putCache(pBS->pucBits, dwCache);
        pBS->pucBits += 4;
        
        pBS->dwCache = uiBits << (32 + nShift);
        pBS->nCachedBits = -nShift;
    }
}

static void xWriteAlignZero(X265_BitStream *pBS)
{
    Int nShift = 8 - (pBS->nCachedBits % 8);

    xPutBits(pBS, 0, nShift);
}

static void xWriteRBSPTrailingBits(X265_BitStream *pBS)
{
    xPutBit(pBS, 1);
    xWriteAlignZero(pBS);
}

static Int32 xBitFlush(X265_BitStream *pBS)
{
    putCache(pBS->pucBits, pBS->dwCache);
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
    UInt32 uiLength = xLog2(++uiCode);

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
    xPutBit( pBS, uiCode );
}

#endif /* __BS_H__ */

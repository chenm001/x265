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
    Int nShift = (8 - pBS->nCachedBits) & 7;
    xPutBits(pBS, 0, nShift);
}

static void xWriteAlignOne(X265_BitStream *pBS)
{
    Int nShift = (8 - pBS->nCachedBits) & 7;
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

// ***************************************************************************
// Cabac Table
// ***************************************************************************
#define MAX_NUM_CTX_MOD             256       ///< maximum number of supported contexts

#define NUM_SPLIT_FLAG_CTX            3       ///< number of context models for split flag
#define NUM_SKIP_FLAG_CTX             3       ///< number of context models for skip flag

#define NUM_MERGE_FLAG_EXT_CTX        1       ///< number of context models for merge flag of merge extended
#define NUM_MERGE_IDX_EXT_CTX         1       ///< number of context models for merge index of merge extended

#define NUM_ALF_CTRL_FLAG_CTX         1       ///< number of context models for ALF control flag
#define NUM_PART_SIZE_CTX             4       ///< number of context models for partition size
#define NUM_CU_AMP_CTX                1       ///< number of context models for partition size (AMP)
#define NUM_PRED_MODE_CTX             1       ///< number of context models for prediction mode

#define NUM_ADI_CTX                   1       ///< number of context models for intra prediction

#define NUM_CHROMA_PRED_CTX           2       ///< number of context models for intra prediction (chroma)
#define NUM_INTER_DIR_CTX             4       ///< number of context models for inter prediction direction
#define NUM_MV_RES_CTX                2       ///< number of context models for motion vector difference

#define NUM_REF_NO_CTX                4       ///< number of context models for reference index
#define NUM_TRANS_SUBDIV_FLAG_CTX     10      ///< number of context models for transform subdivision flags
#define NUM_QT_CBF_CTX                5       ///< number of context models for QT CBF
#define NUM_QT_ROOT_CBF_CTX           1       ///< number of context models for QT ROOT CBF
#define NUM_DELTA_QP_CTX              3       ///< number of context models for dQP

#define NUM_SIG_CG_FLAG_CTX           2       ///< number of context models for MULTI_LEVEL_SIGNIFICANCE

#define NUM_SIG_FLAG_CTX              48      ///< number of context models for sig flag

#define NUM_SIG_FLAG_CTX_LUMA         27      ///< number of context models for luma sig flag
#define NUM_SIG_FLAG_CTX_CHROMA       21      ///< number of context models for chroma sig flag
#define NUM_LAST_FLAG_XY_CTX          15      ///< number of context models for last coefficient position

#define NUM_ONE_FLAG_CTX              24      ///< number of context models for greater than 1 flag
#define NUM_ONE_FLAG_CTX_LUMA         16      ///< number of context models for greater than 1 flag of luma
#define NUM_ONE_FLAG_CTX_CHROMA        8      ///< number of context models for greater than 1 flag of chroma
#define NUM_ABS_FLAG_CTX               6      ///< number of context models for greater than 2 flag
#define NUM_ABS_FLAG_CTX_LUMA          4      ///< number of context models for greater than 2 flag of luma
#define NUM_ABS_FLAG_CTX_CHROMA        2      ///< number of context models for greater than 2 flag of chroma

#define NUM_MVP_IDX_CTX                2       ///< number of context models for MVP index

#define CNU                          154      ///< dummy initialization value for unused context models 'Context model Not Used'


#endif /* __BS_H__ */

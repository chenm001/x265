/*****************************************************************************
 * encode.cpp: Main for encode
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

#include "x265.h"

// ***************************************************************************
// * Interface Functions
// ***************************************************************************
void xEncInit( X265_t *h )
{
    const UInt32 uiWidth    = h->usWidth;
    const UInt32 uiHeight   = h->usHeight;
    const UInt32 uiYSize    = uiWidth * uiHeight;
    int i;

    for( i=0; i < MAX_REF_NUM; i++ ) {
        UInt8 *ptr = (UInt8 *)MALLOC(uiYSize * 3 / 2);
        assert(ptr != NULL);
        h->refn[i].pucY = (UInt8 *)ptr;
        h->refn[i].pucU = (UInt8 *)ptr + uiYSize;
        h->refn[i].pucV = (UInt8 *)ptr + uiYSize * 5 / 4;
    }
}

Int32 xEncEncode( X265_t *h, X265_Frame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize )
{
    const UInt32 uiWidth    = h->usWidth;
    const UInt32 uiHeight   = h->usHeight;
    X265_BitStream *pBS     = &h->bs;
    X265_Cache     *pCache  = &h->cache;
    Int x, y;

    /// Copy to local
    h->pFrame = pFrame;

    /// Initial local
    xBitStreamInit( pBS, pucOutBuf, uiBufSize );

    /// Write SPS Header
    xPutBits32(pBS, 0x01000000);
    xPutBits(pBS, 0x67, 8);
    xWriteSPS(h);
    xBitFlush(pBS);

    /// Write PPS Header
    xPutBits32(pBS, 0x01000000);
    xPutBits(pBS, 0x68, 8);
    xWritePPS(h);
    xBitFlush(pBS);

    /// Write Silces Header
    xPutBits32(pBS, 0x65010000);
    //xPutBits(pBS, 0, 3); // temporal_id
    //xPutBits(pBS, 1, 1); // output_flag
    //xPutBits(pBS, 1, 4); // reserved_one_4bits
    xPutBits(pBS, 0x11, 8);
    xWriteSliceHeader(h);

    /// Encode loop
    xEncCahceInit( h );
    for( y=0; y < uiHeight; y+=h->ucMaxCUWidth ) {
        h->uiCUY = y;
        xEncCahceInitLine( h );
        for( x=0; x < uiHeight; x+=h->ucMaxCUWidth ) {
            // Stage 0: Init internal
            h->uiCUX = x;

            // Stage 1a: Load image to cache
            xEncCacheLoadCU( h, x, y );

            // Stage 1b: Load Intra PU Reference Samples
            // TODO: ASSUME one PU only
            xEncIntraLoadRef( h, x, y, h->ucMaxCUWidth );
        }
    }

    return xBitFlush( pBS );
}

// ***************************************************************************
// * Internal Functions
// ***************************************************************************

// ***************************************************************************
// * Cache Manage Functions
// ***************************************************************************
void xEncCahceInit( X265_t *h )
{
    X265_Cache *pCache  = &h->cache;
    memset( pCache, 0, sizeof(X265_Cache) );
    memset( pCache->pcTopModeY, MODE_INVALID, sizeof(pCache->pcTopModeY) );
    memset( pCache->pcTopModeU, MODE_INVALID, sizeof(pCache->pcTopModeU) );
    memset( pCache->pcTopModeV, MODE_INVALID, sizeof(pCache->pcTopModeV) );
}

void xEncCahceInitLine( X265_t *h )
{
    X265_Cache *pCache  = &h->cache;
    pCache->uiOffset    = 0;
    memset( pCache->pcLeftModeY, MODE_INVALID, sizeof(pCache->pcLeftModeY) );
    memset( pCache->pcLeftModeU, MODE_INVALID, sizeof(pCache->pcLeftModeU) );
    memset( pCache->pcLeftModeV, MODE_INVALID, sizeof(pCache->pcLeftModeV) );
}

void xEncCacheLoadCU( X265_t *h, UInt uiX, UInt uiY )
{
    X265_Cache  *pCache     = &h->cache;
    X265_Frame  *pFrame     = h->pFrame;
    const UInt   nCUWidth   = h->ucMaxCUWidth;
    const UInt32 uiWidth    = h->usWidth;
    const UInt32 uiOffsetY  = uiWidth * uiY + uiX;
    const UInt32 uiOffsetC  = uiWidth * uiY / 4 + uiX / 2;
    UInt8 *pucSY = pFrame->pucY + uiOffsetY;
    UInt8 *pucSU = pFrame->pucU + uiOffsetC;
    UInt8 *pucSV = pFrame->pucV + uiOffsetC;
    UInt8 *pucDY = pCache->pucPixY;
    UInt8 *pucDU = pCache->pucPixU;
    UInt8 *pucDV = pCache->pucPixV;
    Int y;

    for( y=0; y < h->ucMaxCUWidth; y++ ) {
        memcpy( pucDY, pucSY, nCUWidth     );
        memcpy( pucDU, pucSU, nCUWidth / 2 );
        memcpy( pucDV, pucSV, nCUWidth / 2 );
        pucSY += uiWidth;
        pucSU += uiWidth / 2;
        pucSV += uiWidth / 2;
        pucDY += MAX_CU_SIZE;
        pucDU += MAX_CU_SIZE / 2;
        pucDV += MAX_CU_SIZE / 2;
    }
}

// ***************************************************************************
// * IntraPred Functions
// ***************************************************************************
void xEncIntraLoadRef( X265_t *h, UInt32 uiX, UInt32 uiY, UInt nSize )
{
    X265_Cache  *pCache         = &h->cache;
    const UInt   nMinTUSize     =  (1 << h->ucQuadtreeTULog2MinSize);
    const UInt32 uiOffset       =  pCache->uiOffset;
    const UInt   nIdx           = (uiY / MIN_CU_SIZE) * (MAX_CU_SIZE / MIN_CU_SIZE) + (uiX / MIN_CU_SIZE);
    const UInt8 *pucTopPixY     = &pCache->pucTopPixY[uiOffset + uiX];
    const UInt8 *pucLeftPixY    =  pCache->pucLeftPixY + uiY;
    const UInt8 *pucTopLeftY    =  pCache->pucTopLeftY;
    const  Int8 *pcTopModeY     = &pCache->pcTopModeY[uiOffset + uiX];
    const  Int8 *pcLeftModeY    =  pCache->pcLeftModeY + uiY;

    /// T(op), B(ottom), L(eft), R(ight)
    const UInt   bT             = (pcTopModeY [uiX] != MODE_INVALID);
    const UInt   bL             = (pcLeftModeY[uiY] != MODE_INVALID);
    const UInt   bLT            = bT && bL;
    const UInt   bTR            = (pcTopModeY [uiX + nSize] != MODE_INVALID);
    const UInt   bLB            = (pcLeftModeY[uiY + nSize] != MODE_INVALID);
    const UInt8  bValid[5]      = {bLB, bL, bLT, bT, bTR};
    const UInt   nBlkOffset[5]  = {0, nSize, 2*nSize, 2*nSize+1, 3*nSize+1};
    UInt8 *pucRefY0 = pCache->pucPixRef[0];
    UInt8 *pucRefY1 = pCache->pucPixRef[1];
    UInt8  ucPadding;
    Int i, n;

    // TODO: I ASSUME( CU = PU = TU ) here, do more!
    assert( (uiX == 0) && (uiY == 0) && (nSize == h->ucMaxCUWidth) );

    // Default to DC when all reference invalid
    if ( (bT | bL | bLT | bTR | bLB) == 0 )
        memset( pucRefY0, 0x80, nSize * 4 + 1 );
    else {
        // Copy the reconst pixel when valid
        if (bLB) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffset[0]] = pucLeftPixY[nSize * 2 - 1 - i];
            }
        }
        if (bLB) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffset[1]] = pucLeftPixY[nSize - 1 - i];
            }
        }
        if (bLT) {
            pucRefY0[nBlkOffset[2]] = pucTopLeftY[ucTopLeftIdx[nIdx]];
        }
        if (bT) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffset[3]] = pucTopPixY[i + 0 * nSize];
            }
        }
        if (bTR) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffset[4]] = pucTopPixY[i + 1 * nSize];
            }
        }

#if PADDING_INTRA
        // Padding from Right to Left
        for( n=0; n<ASIZE(bValid); n++ ) {
            if (bValid[n])
                break;
        }
        ucPadding = pucRefY0[nBlkOffset[n]];
        for( i=0; i<n*nSize; i++ ) {
            pucRefY0[i] = ucPadding;
        }

        // Padding from Left to Right
        for( ; n<ASIZE(bValid); n++ ) {
            if (!bValid[n]) {
                assert( n > 0 );
                const UInt nBlkLast = nBlkOffset[n-1] + ((n-1) == 2 ? 1 : nSize) - 1;
                ucPadding = pucRefY0[nBlkLast];
                for( i=0; i<nSize; i++ ) {
                    pucRefY0[i] = ucPadding;
                }
            }
        }
#else
#error Please sync the code!
#endif

        // Filter with [1 2 1]
        pucRefY1[0      ] = pucRefY0[0];
        pucRefY1[4*nSize] = pucRefY0[4*nSize];
        for( i=1; i<4*nSize; i++ ) {
            pucRefY1[i] = (pucRefY0[i - 1] + 2 * pucRefY0[i] + pucRefY0[i + 1] + 2) >> 2;
        }
    }
}


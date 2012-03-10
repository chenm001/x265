/*****************************************************************************
 * encode.cpp: Main for encode
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

#include "x265.h"

#define CHECK_TV    1

#if (CHECK_TV)
// ========== [  98]( 0): Size=32, Avail=(1,1)
#define TV_SIZE     (32)
UInt8 tv_top[TV_SIZE*2+1] = {
    0x56,
    0x63, 0x61, 0x5D, 0x57, 0x53, 0x52, 0x53, 0x57,
    0x5A, 0x5D, 0x5C, 0x59, 0x55, 0x52, 0x52, 0x54,
    0x57, 0x59, 0x5C, 0x5D, 0x5E, 0x5E, 0x5E, 0x5D,
    0x5A, 0x55, 0x50, 0x4C, 0x4B, 0x4B, 0x4D, 0x4E,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E 
};
UInt8 tv_left[TV_SIZE*2] = {
    0x60, 0x61, 0x63, 0x64, 0x63, 0x62, 0x60, 0x5F,
    0x5F, 0x5F, 0x60, 0x5F, 0x5D, 0x5A, 0x57, 0x55,
    0x54, 0x51, 0x40, 0x53, 0x60, 0x52, 0x51, 0xAC,
    0xAD, 0x93, 0x83, 0x90, 0xA7, 0xA4, 0x90, 0x0E,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
};

UInt8 tv_top1[TV_SIZE*2+1] = {
    0x5C,
    0x5F, 0x61, 0x5D, 0x58, 0x54, 0x53, 0x54, 0x57,
    0x5A, 0x5C, 0x5C, 0x59, 0x55, 0x53, 0x53, 0x54,
    0x57, 0x59, 0x5C, 0x5D, 0x5E, 0x5E, 0x5E, 0x5D,
    0x5A, 0x55, 0x50, 0x4D, 0x4B, 0x4C, 0x4D, 0x4E,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E,
    0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E,
};

UInt8 tv_left1[TV_SIZE*2] = {
    0x5E, 0x61, 0x63, 0x64, 0x63, 0x62, 0x60, 0x5F,
    0x5F, 0x5F, 0x60, 0x5F, 0x5D, 0x5A, 0x57, 0x55,
    0x54, 0x4E, 0x49, 0x52, 0x59, 0x55, 0x68, 0x96,
    0xA6, 0x96, 0x8A, 0x93, 0xA1, 0xA0, 0x75, 0x2F,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,

};
#endif

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
    Int i;

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
            const UInt   nCUSize     = h->ucMaxCUWidth;
            const UInt   nLog2CUSize = xLog2(nCUSize-1);
            UInt32 uiBestSad;
            UInt   nBestMode;
            UInt   nMode;

            // Stage 0: Init internal
            h->uiCUX = x;

            // Stage 1a: Load image to cache
            xEncCacheLoadCU( h, x, y );

            // Stage 1b: Load Intra PU Reference Samples
            // TODO: ASSUME one PU only
            xEncIntraLoadRef( h, 0, 0, h->ucMaxCUWidth );

            // Stage 2a: Decide Intra
            // TODO: Support LM mode
            // TODO: Support more size
            uiBestSad = MAX_SAD;
            nBestMode = 0;
            for( nMode=0; nMode<NUM_INTRA_MODE; nMode++ ) {
                UInt32 uiSad;
                xEncIntraPredLuma( h, nMode, nCUSize );
                uiSad = xSadN[nLog2CUSize-2](
                            nCUSize,
                            pCache->pucPixY, MAX_CU_SIZE,
                            pCache->pucPredY, MAX_CU_SIZE
                        );
                if ( uiSad < uiBestSad ) {
                    uiBestSad = uiSad;
                    nBestMode = nMode;
                }
            }
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
UInt xGetTopLeftIndex( UInt32 uiX, UInt32 uiY )
{
    UInt nOffsetX = (uiX % MAX_CU_SIZE) / MIN_CU_SIZE;
    UInt nOffsetY = (uiY % MAX_CU_SIZE) / MIN_CU_SIZE;
    UInt nIdx     = nOffsetY * MAX_PU_XY + nOffsetX;

    if ( nOffsetX == 0 )
        nIdx += MAX_PU_XY;

    return nIdx - 1;
}

void xEncIntraLoadRef( X265_t *h, UInt32 uiX, UInt32 uiY, UInt nSize )
{
    X265_Cache  *pCache         = &h->cache;
    const UInt   nMinTUSize     =  (1 << h->ucQuadtreeTULog2MinSize);
    const UInt32 uiOffset       =  pCache->uiOffset;
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

    // Save bValid flag for other functions
    memcpy( pCache->bValid, bValid, sizeof(bValid) );

#if !(CHECK_TV)
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
            pucRefY0[nBlkOffset[2]] = pucTopLeftY[xGetTopLeftIndex(uiX, uiY)];
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

        // Filter with [1 2 1]
        pucRefY1[0      ] = pucRefY0[0];
        pucRefY1[4*nSize] = pucRefY0[4*nSize];
        for( i=1; i<4*nSize; i++ ) {
            pucRefY1[i] = (pucRefY0[i - 1] + 2 * pucRefY0[i] + pucRefY0[i + 1] + 2) >> 2;
        }
    }
#else
    assert( nSize == TV_SIZE );
    for( i=0; i<TV_SIZE*2; i++ ) {
        pucRefY0[i] = tv_left[TV_SIZE*2-i-1];
    }
    for( i=0; i<TV_SIZE*2+1; i++ ) {
        pucRefY0[TV_SIZE*2+i] = tv_top[i];
    }

    // Filter with [1 2 1]
    pucRefY1[0      ] = pucRefY0[0];
    pucRefY1[4*nSize] = pucRefY0[4*nSize];
    for( i=1; i<4*nSize; i++ ) {
        pucRefY1[i] = (pucRefY0[i - 1] + 2 * pucRefY0[i] + pucRefY0[i + 1] + 2) >> 2;
    }

    int bPassed = TRUE;
    for( i=0; i<TV_SIZE*2; i++ ) {
        if ( pucRefY1[i] != tv_left1[TV_SIZE*2-i-1] ) {
            bPassed = FALSE;
            fprintf( stderr, "Detect Wrong at %d, %02X -> %02X\n", i, tv_left[TV_SIZE*2-i-1], pucRefY1[i] );
        }
    }
    for( i=0; i<TV_SIZE*2; i++ ) {
        if ( pucRefY1[i] != tv_left1[TV_SIZE*2-i-1] ) {
            bPassed = FALSE;
            fprintf( stderr, "Detect Wrong at %d, %02X -> %02X\n", i, tv_left[TV_SIZE*2-i-1], pucRefY1[i] );
        }
    }
    if ( bPassed )
        fprintf( stderr, "Intra Reference Pixel Filter Passed!\n");
    else {
        fprintf( stderr, "Intra Reference Pixel Filter Failed!\n");
        abort();
    }
#endif
}

void xPredIntraPlanar(
    UInt8   *pucRef,
    UInt8   *pucDst,
    Int      nDstStride,
    UInt     nSize
)
{
    UInt nLog2Size = xLog2(nSize - 1);
    UInt8 *pucLeft = pucRef + 2 * nSize - 1;
    UInt8 *pucTop  = pucRef + 2 * nSize + 1;
    Int i, j;
    Int bottomLeft, topRight;
    Int horPred;
    Int leftColumn[MAX_CU_SIZE+1], topRow[MAX_CU_SIZE+1], bottomRow[MAX_CU_SIZE+1], rightColumn[MAX_CU_SIZE+1];
    UInt offset2D = nSize;
    UInt shift1D = nLog2Size;
    UInt shift2D = shift1D + 1;

    // Get left and above reference column and row
    for( i=0; i<nSize+1; i++) {
        topRow[i]     = pucTop[i];
        leftColumn[i] = pucLeft[-i];
    }

    // Prepare intermediate variables used in interpolation
    bottomLeft = leftColumn[nSize];
    topRight   = topRow[nSize];
    for( i=0; i<nSize; i++ ) {
        bottomRow[i]   = bottomLeft - topRow[i];
        rightColumn[i] = topRight   - leftColumn[i];
        topRow[i]      <<= shift1D;
        leftColumn[i]  <<= shift1D;
    }

    // Generate prediction signal
    for( i=0; i<nSize; i++ ) {
        horPred = leftColumn[i] + offset2D;
        for( j=0; j<nSize; j++ ) {
            horPred += rightColumn[i];
            topRow[j] += bottomRow[j];
            pucDst[i*nDstStride+j] = ( (horPred + topRow[j]) >> shift2D );
        }
    }
}

UInt8 xPredIntraGetDCVal(
    UInt8   *pucRef,
    UInt     nSize
)
{
    UInt8 *pucLeft = pucRef + 2 * nSize - 1;
    UInt8 *pucTop  = pucRef + 2 * nSize + 1;
    UInt32 uiSumTop = 0;
    UInt32 uiSumLeft = 0;
    UInt8 ucDcVal;
    int i;

    for( i=0; i<nSize; i++ ) {
        uiSumTop  += pucTop [ i];
        uiSumLeft += pucLeft[-i];
    }
    ucDcVal = (uiSumTop + uiSumLeft + nSize) / (nSize + nSize);
    return ucDcVal;
}

void xPredIntraDc(
    UInt8   *pucRef,
    UInt8   *pucDst,
    Int      nDstStride,
    UInt     nSize
)
{
    UInt8 *pucLeft = pucRef + 2 * nSize - 1;
    UInt8 *pucTop  = pucRef + 2 * nSize + 1;
    UInt8 ucDcVal = xPredIntraGetDCVal( pucRef, nSize );
    int i;

    // Fill DC Val
    for( i=0; i<nSize; i++ ) {
        memset( &pucDst[i * nDstStride], ucDcVal, nSize );
    }

    // DC Filtering
    pucDst[0] = ( pucTop[0] + pucLeft[0] + 2 * pucDst[0] + 2 ) >> 2;
    for( i=1; i<nSize; i++ ) {
        pucDst[i           ] = ( pucTop [ i] + 3 * pucDst[i           ] + 2 ) >> 2;
        pucDst[i*nDstStride] = ( pucLeft[-i] + 3 * pucDst[i*nDstStride] + 2 ) >> 2;
    }
}

void xPredIntraAng(
    UInt8   *pucRef,
    UInt8   *pucDst,
    Int      nDstStride,
    UInt     nSize,
    UInt     nMode
)
{
    UInt   dirMode          = g_aucIntraPredOrder[nMode];
    Int    nIntraPredAngle  = g_aucIntraPredAngle[dirMode];
    Int    nInvAngle        = g_aucInvAngle[dirMode];
    UInt   bModeVer         = (dirMode > 0) && (dirMode < 18);
    UInt8 *pucLeft          = pucRef + 2 * nSize - 1;
    UInt8 *pucTop           = pucRef + 2 * nSize + 1;
    UInt8 *pucTopLeft       = pucRef + 2 * nSize;
    UInt8  ucRefBuf[2*MAX_CU_SIZE+1];
    UInt8 *pucRefMain       = ucRefBuf + ( (nIntraPredAngle < 0) ? MAX_CU_SIZE : 0 );
    UInt8 *pucRefSide       = bModeVer ? pucLeft : pucTop;
    int x;

    assert( dirMode > 0 );

    // 8-36 and 8-39
    for( x=0; x<nSize+1; x++ ) {
        pucRefMain[x] = bModeVer ? pucTopLeft[x] : pucTopLeft[-x];
    }

    if (nIntraPredAngle < 0) {
        // Fix my inv left buffer index
        Int iOffset = 128;
        if ( bModeVer ) {
            iOffset   = -128;
            nInvAngle = -nInvAngle;
        }
        // 8-37 or 8-40
        for( x=-1; x>(nSize*nIntraPredAngle)>>5; x-- ) {
            iOffset += nInvAngle;
            pucRefMain[x] = pucRefSide[iOffset >> 8];
        }
    }
    else {
        // 8-38 or 8-41
        for( x=nSize+1; x<2*nSize+1; x++ ) {
            pucRefMain[x] = bModeVer ? pucTop[x] : pucLeft[-x];
        }
    }
}

void xEncIntraPredLuma( X265_t *h, UInt nMode, UInt nSize )
{
    X265_Cache  *pCache     = &h->cache;
    UInt        nLog2Size   = xLog2(nSize - 1);
    UInt        bFilter     = g_aucIntraFilterType[nLog2Size-2][nMode];
    UInt8       *pucRefY    = pCache->pucPixRef[bFilter];
    UInt8       *pucDstY    = pCache->pucPredY;

    if ( nMode == PLANAR_IDX ) {
        xPredIntraPlanar(
            pucRefY,
            pucDstY,
            MAX_CU_SIZE,
            nSize
        );
    }
    else if ( nMode == DC_IDX ) {
        xPredIntraDc(
            pucRefY,
            pucDstY,
            MAX_CU_SIZE,
            nSize
        );
    }
    else {
        xPredIntraAng(
            pucRefY,
            pucDstY,
            MAX_CU_SIZE,
            nSize,
            nMode
        );
    }
}


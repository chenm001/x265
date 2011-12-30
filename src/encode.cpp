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
    memset( &h->cache, 0, sizeof(h->cache) );
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
    for( y=0; y < uiHeight; y+=h->ucMaxCUWidth ) {
        for( x=0; x < uiHeight; x+=h->ucMaxCUWidth ) {
            // Stage 1: Load image to cache
            xEncLoadCache( h, x, y );

        }
    }

    return xBitFlush( pBS );
}

// ***************************************************************************
// * Internal Functions
// ***************************************************************************
void xEncLoadCache( X265_t *h, UInt uiX, UInt uiY )
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

/*****************************************************************************
 * pixel.cpp: Pixel operator functions
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

// ***************************************************************************
// * SAD Functions
// ***************************************************************************
UInt32 xSad64xN( Int N, UInt8 *pSrc, UInt nStrideSrc, UInt8 *pRef, UInt nStrideRef )
{
    Int x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<64; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

UInt32 xSad32xN( Int N, UInt8 *pSrc, UInt nStrideSrc, UInt8 *pRef, UInt nStrideRef )
{
    Int x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<32; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

xSad *xSadN[MAX_CU_DEPTH+2] = {
    NULL,       /*  2 x N */
    NULL,       /*  4 x N */
    NULL,       /*  8 x N */
    NULL,       /* 16 x N */
    xSad32xN,   /* 32 x N */
    xSad64xN,   /* 64 x N */
};


// ***************************************************************************
// * DCT Functions
// ***************************************************************************

/** 4x4 forward transform implemented using partial butterfly structure (1D)
 *  \param pSrc   input data (residual)
 *  \param pDst   output data (transpose transform coefficients)
 *  \param nLines transform lines
 *  \param nShift specifies right shift after 1D transform
 */
void xDCT4( Int16 *pSrc, Int16 *pDst, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* Even and Odd */
        Int32 s03 = pSrc[i*MAX_CU_SIZE+0] + pSrc[i*MAX_CU_SIZE+3];
        Int32 d03 = pSrc[i*MAX_CU_SIZE+0] - pSrc[i*MAX_CU_SIZE+3];
        Int32 s12 = pSrc[i*MAX_CU_SIZE+1] + pSrc[i*MAX_CU_SIZE+2];
        Int32 d12 = pSrc[i*MAX_CU_SIZE+1] - pSrc[i*MAX_CU_SIZE+2];

        pDst[0*MAX_CU_SIZE+i] = ( g_aiT4[0*4+0]*s03 + g_aiT4[0*4+1]*s12 + rnd ) >> nShift;
        pDst[2*MAX_CU_SIZE+i] = ( g_aiT4[2*4+0]*s03 + g_aiT4[2*4+1]*s12 + rnd ) >> nShift;
        pDst[1*MAX_CU_SIZE+i] = ( g_aiT4[1*4+0]*d03 + g_aiT4[1*4+1]*d12 + rnd ) >> nShift;
        pDst[3*MAX_CU_SIZE+i] = ( g_aiT4[3*4+0]*d03 + g_aiT4[3*4+1]*d12 + rnd ) >> nShift;
    }
}

void xDST4( Int16 *pSrc, Int16 *pDst, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<4; i++ ) {
        // Intermediate Variables
        Int32 c0 = pSrc[i*MAX_CU_SIZE+0] + pSrc[i*MAX_CU_SIZE+3];
        Int32 c1 = pSrc[i*MAX_CU_SIZE+1] + pSrc[i*MAX_CU_SIZE+3];
        Int32 c2 = pSrc[i*MAX_CU_SIZE+0] - pSrc[i*MAX_CU_SIZE+1];
        Int32 c3 = 74* pSrc[i*MAX_CU_SIZE+2];
        Int32 c4 = (pSrc[i*MAX_CU_SIZE+0]+ pSrc[i*MAX_CU_SIZE+1] - pSrc[i*MAX_CU_SIZE+3]);

        pDst[0*MAX_CU_SIZE+i] =  ( 29 * c0 + 55 * c1 + c3 + rnd ) >> nShift;
        pDst[1*MAX_CU_SIZE+i] =  ( 74 * c4                + rnd ) >> nShift;
        pDst[2*MAX_CU_SIZE+i] =  ( 29 * c2 + 55 * c0 - c3 + rnd ) >> nShift;
        pDst[3*MAX_CU_SIZE+i] =  ( 55 * c2 - 29 * c1 + c3 + rnd ) >> nShift;
    }
}

void xDCT8( Int16 *pSrc, Int16 *pDst, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* Even and Odd */
        Int32 s07 = pSrc[i*MAX_CU_SIZE+0] + pSrc[i*MAX_CU_SIZE+7];
        Int32 d07 = pSrc[i*MAX_CU_SIZE+0] - pSrc[i*MAX_CU_SIZE+7];
        Int32 s16 = pSrc[i*MAX_CU_SIZE+1] + pSrc[i*MAX_CU_SIZE+6];
        Int32 d16 = pSrc[i*MAX_CU_SIZE+1] - pSrc[i*MAX_CU_SIZE+6];
        Int32 s25 = pSrc[i*MAX_CU_SIZE+2] + pSrc[i*MAX_CU_SIZE+5];
        Int32 d25 = pSrc[i*MAX_CU_SIZE+2] - pSrc[i*MAX_CU_SIZE+5];
        Int32 s34 = pSrc[i*MAX_CU_SIZE+3] + pSrc[i*MAX_CU_SIZE+4];
        Int32 d34 = pSrc[i*MAX_CU_SIZE+3] - pSrc[i*MAX_CU_SIZE+4];

        /* EE and EO */
        Int32 EE0 = s07 + s34;
        Int32 EO0 = s07 - s34;
        Int32 EE1 = s16 + s25;
        Int32 EO1 = s16 - s25;

        pDst[0*MAX_CU_SIZE+i] = (g_aiT8[0*8+0]*EE0 + g_aiT8[0*8+1]*EE1 + rnd) >> nShift;
        pDst[4*MAX_CU_SIZE+i] = (g_aiT8[4*8+0]*EE0 + g_aiT8[4*8+1]*EE1 + rnd) >> nShift;
        pDst[2*MAX_CU_SIZE+i] = (g_aiT8[2*8+0]*EO0 + g_aiT8[2*8+1]*EO1 + rnd) >> nShift;
        pDst[6*MAX_CU_SIZE+i] = (g_aiT8[6*8+0]*EO0 + g_aiT8[6*8+1]*EO1 + rnd) >> nShift;

        pDst[1*MAX_CU_SIZE+i] = (g_aiT8[1*8+0]*d07 + g_aiT8[1*8+1]*d16 + g_aiT8[1*8+2]*d25 + g_aiT8[1*8+3]*d34 + rnd) >> nShift;
        pDst[3*MAX_CU_SIZE+i] = (g_aiT8[3*8+0]*d07 + g_aiT8[3*8+1]*d16 + g_aiT8[3*8+2]*d25 + g_aiT8[3*8+3]*d34 + rnd) >> nShift;
        pDst[5*MAX_CU_SIZE+i] = (g_aiT8[5*8+0]*d07 + g_aiT8[5*8+1]*d16 + g_aiT8[5*8+2]*d25 + g_aiT8[5*8+3]*d34 + rnd) >> nShift;
        pDst[7*MAX_CU_SIZE+i] = (g_aiT8[7*8+0]*d07 + g_aiT8[7*8+1]*d16 + g_aiT8[7*8+2]*d25 + g_aiT8[7*8+3]*d34 + rnd) >> nShift;
    }
}

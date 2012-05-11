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
UInt32 xSad64xN( const UInt N, const UInt8 *pSrc, const UInt nStrideSrc, const UInt8 *pRef, const UInt nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<64; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

UInt32 xSad32xN( const UInt N, const UInt8 *pSrc, const UInt nStrideSrc, const UInt8 *pRef, const UInt nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<32; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

UInt32 xSad16xN( const UInt N, const UInt8 *pSrc, const UInt nStrideSrc, const UInt8 *pRef, const UInt nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<16; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

UInt32 xSad8xN( const UInt N, const UInt8 *pSrc, const UInt nStrideSrc, const UInt8 *pRef, UInt const nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<8; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

UInt32 xSad4xN( const UInt N, const UInt8 *pSrc, const UInt nStrideSrc, const UInt8 *pRef, const UInt nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<4; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

xSad *xSadN[MAX_CU_DEPTH+1] = {
    xSad4xN,    /*  4 x N */
    xSad8xN,    /*  8 x N */
    xSad16xN,   /* 16 x N */
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
void xDCT4( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* Even and Odd */
        Int32 s03 = pSrc[i*nStride+0] + pSrc[i*nStride+3];
        Int32 d03 = pSrc[i*nStride+0] - pSrc[i*nStride+3];
        Int32 s12 = pSrc[i*nStride+1] + pSrc[i*nStride+2];
        Int32 d12 = pSrc[i*nStride+1] - pSrc[i*nStride+2];

        pDst[0*nStride+i] = ( g_aiT4[0*4+0]*s03 + g_aiT4[0*4+1]*s12 + rnd ) >> nShift;
        pDst[2*nStride+i] = ( g_aiT4[2*4+0]*s03 + g_aiT4[2*4+1]*s12 + rnd ) >> nShift;
        pDst[1*nStride+i] = ( g_aiT4[1*4+0]*d03 + g_aiT4[1*4+1]*d12 + rnd ) >> nShift;
        pDst[3*nStride+i] = ( g_aiT4[3*4+0]*d03 + g_aiT4[3*4+1]*d12 + rnd ) >> nShift;
    }
}

void xDST4( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<4; i++ ) {
        // Intermediate Variables
        Int32 c0 = pSrc[i*nStride+0] + pSrc[i*nStride+3];
        Int32 c1 = pSrc[i*nStride+1] + pSrc[i*nStride+3];
        Int32 c2 = pSrc[i*nStride+0] - pSrc[i*nStride+1];
        Int32 c3 = 74* pSrc[i*nStride+2];
        Int32 c4 = (pSrc[i*nStride+0]+ pSrc[i*nStride+1] - pSrc[i*nStride+3]);

        pDst[0*nStride+i] =  ( 29 * c0 + 55 * c1 + c3 + rnd ) >> nShift;
        pDst[1*nStride+i] =  ( 74 * c4                + rnd ) >> nShift;
        pDst[2*nStride+i] =  ( 29 * c2 + 55 * c0 - c3 + rnd ) >> nShift;
        pDst[3*nStride+i] =  ( 55 * c2 - 29 * c1 + c3 + rnd ) >> nShift;
    }
}

void xDCT8( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* Even and Odd */
        Int32 s07 = pSrc[i*nStride+0] + pSrc[i*nStride+7];
        Int32 d07 = pSrc[i*nStride+0] - pSrc[i*nStride+7];
        Int32 s16 = pSrc[i*nStride+1] + pSrc[i*nStride+6];
        Int32 d16 = pSrc[i*nStride+1] - pSrc[i*nStride+6];
        Int32 s25 = pSrc[i*nStride+2] + pSrc[i*nStride+5];
        Int32 d25 = pSrc[i*nStride+2] - pSrc[i*nStride+5];
        Int32 s34 = pSrc[i*nStride+3] + pSrc[i*nStride+4];
        Int32 d34 = pSrc[i*nStride+3] - pSrc[i*nStride+4];

        /* EE and EO */
        Int32 EE0 = s07 + s34;
        Int32 EO0 = s07 - s34;
        Int32 EE1 = s16 + s25;
        Int32 EO1 = s16 - s25;

        pDst[0*nStride+i] = (g_aiT8[0*8+0]*EE0 + g_aiT8[0*8+1]*EE1 + rnd) >> nShift;
        pDst[4*nStride+i] = (g_aiT8[4*8+0]*EE0 + g_aiT8[4*8+1]*EE1 + rnd) >> nShift;
        pDst[2*nStride+i] = (g_aiT8[2*8+0]*EO0 + g_aiT8[2*8+1]*EO1 + rnd) >> nShift;
        pDst[6*nStride+i] = (g_aiT8[6*8+0]*EO0 + g_aiT8[6*8+1]*EO1 + rnd) >> nShift;

        pDst[1*nStride+i] = (g_aiT8[1*8+0]*d07 + g_aiT8[1*8+1]*d16 + g_aiT8[1*8+2]*d25 + g_aiT8[1*8+3]*d34 + rnd) >> nShift;
        pDst[3*nStride+i] = (g_aiT8[3*8+0]*d07 + g_aiT8[3*8+1]*d16 + g_aiT8[3*8+2]*d25 + g_aiT8[3*8+3]*d34 + rnd) >> nShift;
        pDst[5*nStride+i] = (g_aiT8[5*8+0]*d07 + g_aiT8[5*8+1]*d16 + g_aiT8[5*8+2]*d25 + g_aiT8[5*8+3]*d34 + rnd) >> nShift;
        pDst[7*nStride+i] = (g_aiT8[7*8+0]*d07 + g_aiT8[7*8+1]*d16 + g_aiT8[7*8+2]*d25 + g_aiT8[7*8+3]*d34 + rnd) >> nShift;
    }
}

void xDCT16( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* Even and Odd */
        Int32 E0 = pSrc[i*nStride+0] + pSrc[i*nStride+15];
        Int32 O0 = pSrc[i*nStride+0] - pSrc[i*nStride+15];
        Int32 E1 = pSrc[i*nStride+1] + pSrc[i*nStride+14];
        Int32 O1 = pSrc[i*nStride+1] - pSrc[i*nStride+14];
        Int32 E2 = pSrc[i*nStride+2] + pSrc[i*nStride+13];
        Int32 O2 = pSrc[i*nStride+2] - pSrc[i*nStride+13];
        Int32 E3 = pSrc[i*nStride+3] + pSrc[i*nStride+12];
        Int32 O3 = pSrc[i*nStride+3] - pSrc[i*nStride+12];
        Int32 E4 = pSrc[i*nStride+4] + pSrc[i*nStride+11];
        Int32 O4 = pSrc[i*nStride+4] - pSrc[i*nStride+11];
        Int32 E5 = pSrc[i*nStride+5] + pSrc[i*nStride+10];
        Int32 O5 = pSrc[i*nStride+5] - pSrc[i*nStride+10];
        Int32 E6 = pSrc[i*nStride+6] + pSrc[i*nStride+ 9];
        Int32 O6 = pSrc[i*nStride+6] - pSrc[i*nStride+ 9];
        Int32 E7 = pSrc[i*nStride+7] + pSrc[i*nStride+ 8];
        Int32 O7 = pSrc[i*nStride+7] - pSrc[i*nStride+ 8];

        /* EE and EO */
        Int32 EE0 = E0 + E7;
        Int32 EO0 = E0 - E7;
        Int32 EE1 = E1 + E6;
        Int32 EO1 = E1 - E6;
        Int32 EE2 = E2 + E5;
        Int32 EO2 = E2 - E5;
        Int32 EE3 = E3 + E4;
        Int32 EO3 = E3 - E4;

        /* EEE and EEO */
        Int32 EEE0 = EE0 + EE3;
        Int32 EEO0 = EE0 - EE3;
        Int32 EEE1 = EE1 + EE2;
        Int32 EEO1 = EE1 - EE2;

        pDst[ 0*nStride+i] = (g_aiT16[ 0*16+0]*EEE0 + g_aiT16[ 0*16+1]*EEE1 + rnd) >> nShift;
        pDst[ 8*nStride+i] = (g_aiT16[ 8*16+0]*EEE0 + g_aiT16[ 8*16+1]*EEE1 + rnd) >> nShift;
        pDst[ 4*nStride+i] = (g_aiT16[ 4*16+0]*EEO0 + g_aiT16[ 4*16+1]*EEO1 + rnd) >> nShift;
        pDst[12*nStride+i] = (g_aiT16[12*16+0]*EEO0 + g_aiT16[12*16+1]*EEO1 + rnd) >> nShift;

        pDst[ 2*nStride+i] = (g_aiT16[ 2*16+0]*EO0 + g_aiT16[ 2*16+1]*EO1 + g_aiT16[ 2*16+2]*EO2 + g_aiT16[ 2*16+3]*EO3 + rnd) >> nShift;
        pDst[ 6*nStride+i] = (g_aiT16[ 6*16+0]*EO0 + g_aiT16[ 6*16+1]*EO1 + g_aiT16[ 6*16+2]*EO2 + g_aiT16[ 6*16+3]*EO3 + rnd) >> nShift;
        pDst[10*nStride+i] = (g_aiT16[10*16+0]*EO0 + g_aiT16[10*16+1]*EO1 + g_aiT16[10*16+2]*EO2 + g_aiT16[10*16+3]*EO3 + rnd) >> nShift;
        pDst[14*nStride+i] = (g_aiT16[14*16+0]*EO0 + g_aiT16[14*16+1]*EO1 + g_aiT16[14*16+2]*EO2 + g_aiT16[14*16+3]*EO3 + rnd) >> nShift;

        pDst[ 1*nStride+i] = (g_aiT16[ 1*16+0]*O0 + g_aiT16[ 1*16+1]*O1 + g_aiT16[ 1*16+2]*O2 + g_aiT16[ 1*16+3]*O3 +
                              g_aiT16[ 1*16+4]*O4 + g_aiT16[ 1*16+5]*O5 + g_aiT16[ 1*16+6]*O6 + g_aiT16[ 1*16+7]*O7 + rnd) >> nShift;
        pDst[ 3*nStride+i] = (g_aiT16[ 3*16+0]*O0 + g_aiT16[ 3*16+1]*O1 + g_aiT16[ 3*16+2]*O2 + g_aiT16[ 3*16+3]*O3 +
                              g_aiT16[ 3*16+4]*O4 + g_aiT16[ 3*16+5]*O5 + g_aiT16[ 3*16+6]*O6 + g_aiT16[ 3*16+7]*O7 + rnd) >> nShift;
        pDst[ 5*nStride+i] = (g_aiT16[ 5*16+0]*O0 + g_aiT16[ 5*16+1]*O1 + g_aiT16[ 5*16+2]*O2 + g_aiT16[ 5*16+3]*O3 +
                              g_aiT16[ 5*16+4]*O4 + g_aiT16[ 5*16+5]*O5 + g_aiT16[ 5*16+6]*O6 + g_aiT16[ 5*16+7]*O7 + rnd) >> nShift;
        pDst[ 7*nStride+i] = (g_aiT16[ 7*16+0]*O0 + g_aiT16[ 7*16+1]*O1 + g_aiT16[ 7*16+2]*O2 + g_aiT16[ 7*16+3]*O3 +
                              g_aiT16[ 7*16+4]*O4 + g_aiT16[ 7*16+5]*O5 + g_aiT16[ 7*16+6]*O6 + g_aiT16[ 7*16+7]*O7 + rnd) >> nShift;
        pDst[ 9*nStride+i] = (g_aiT16[ 9*16+0]*O0 + g_aiT16[ 9*16+1]*O1 + g_aiT16[ 9*16+2]*O2 + g_aiT16[ 9*16+3]*O3 +
                              g_aiT16[ 9*16+4]*O4 + g_aiT16[ 9*16+5]*O5 + g_aiT16[ 9*16+6]*O6 + g_aiT16[ 9*16+7]*O7 + rnd) >> nShift;
        pDst[11*nStride+i] = (g_aiT16[11*16+0]*O0 + g_aiT16[11*16+1]*O1 + g_aiT16[11*16+2]*O2 + g_aiT16[11*16+3]*O3 +
                              g_aiT16[11*16+4]*O4 + g_aiT16[11*16+5]*O5 + g_aiT16[11*16+6]*O6 + g_aiT16[11*16+7]*O7 + rnd) >> nShift;
        pDst[13*nStride+i] = (g_aiT16[13*16+0]*O0 + g_aiT16[13*16+1]*O1 + g_aiT16[13*16+2]*O2 + g_aiT16[13*16+3]*O3 +
                              g_aiT16[13*16+4]*O4 + g_aiT16[13*16+5]*O5 + g_aiT16[13*16+6]*O6 + g_aiT16[13*16+7]*O7 + rnd) >> nShift;
        pDst[15*nStride+i] = (g_aiT16[15*16+0]*O0 + g_aiT16[15*16+1]*O1 + g_aiT16[15*16+2]*O2 + g_aiT16[15*16+3]*O3 +
                              g_aiT16[15*16+4]*O4 + g_aiT16[15*16+5]*O5 + g_aiT16[15*16+6]*O6 + g_aiT16[15*16+7]*O7 + rnd) >> nShift;
    }
}

void xDCT32( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i,k;
    Int32 E[16],O[16];
    Int32 EE[8],EO[8];
    Int32 EEE[4],EEO[4];
    Int32 EEEE[2],EEEO[2];
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* E and O */
        for( k=0; k<16; k++ ) {
            E[k] = pSrc[i*nStride+k] + pSrc[i*nStride+31-k];
            O[k] = pSrc[i*nStride+k] - pSrc[i*nStride+31-k];
        }
        /* EE and EO */
        for( k=0; k<8; k++ ) {
            EE[k] = E[k] + E[15-k];
            EO[k] = E[k] - E[15-k];
        }
        /* EEE and EEO */
        for( k=0; k<4; k++ ) {
            EEE[k] = EE[k] + EE[7-k];
            EEO[k] = EE[k] - EE[7-k];
        }
        /* EEEE and EEEO */
        EEEE[0] = EEE[0] + EEE[3];
        EEEO[0] = EEE[0] - EEE[3];
        EEEE[1] = EEE[1] + EEE[2];
        EEEO[1] = EEE[1] - EEE[2];

        // 0, 8, 16, 24
        pDst[ 0*nStride+i] = (g_aiT32[ 0*32+0]*EEEE[0] + g_aiT32[ 0*32+1]*EEEE[1] + rnd) >> nShift;
        pDst[16*nStride+i] = (g_aiT32[16*32+0]*EEEE[0] + g_aiT32[16*32+1]*EEEE[1] + rnd) >> nShift;
        pDst[ 8*nStride+i] = (g_aiT32[ 8*32+0]*EEEO[0] + g_aiT32[ 8*32+1]*EEEO[1] + rnd) >> nShift;
        pDst[24*nStride+i] = (g_aiT32[24*32+0]*EEEO[0] + g_aiT32[24*32+1]*EEEO[1] + rnd) >> nShift;

        // 4, 12, 20, 28
        for( k=4; k<32; k+=8 ) {
            pDst[k*nStride+i] = (g_aiT32[k*32+0]*EEO[0] + g_aiT32[k*32+1]*EEO[1] + g_aiT32[k*32+2]*EEO[2] + g_aiT32[k*32+3]*EEO[3] + rnd) >> nShift;
        }

        // 2, 6, 10, 14, 18, 22, 26, 30
        for( k=2; k<32; k+=4 ) {
            pDst[k*nStride+i] = (g_aiT32[k*32+0]*EO[0] + g_aiT32[k*32+1]*EO[1] + g_aiT32[k*32+2]*EO[2] + g_aiT32[k*32+3]*EO[3] +
                                 g_aiT32[k*32+4]*EO[4] + g_aiT32[k*32+5]*EO[5] + g_aiT32[k*32+6]*EO[6] + g_aiT32[k*32+7]*EO[7] + rnd) >> nShift;
        }

        // 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31
        for( k=1; k<32; k+=2 ) {
            pDst[k*nStride+i] = (g_aiT32[k*32+ 0]*O[ 0] + g_aiT32[k*32+ 1]*O[ 1] + g_aiT32[k*32+ 2]*O[ 2] + g_aiT32[k*32+ 3]*O[ 3] +
                                 g_aiT32[k*32+ 4]*O[ 4] + g_aiT32[k*32+ 5]*O[ 5] + g_aiT32[k*32+ 6]*O[ 6] + g_aiT32[k*32+ 7]*O[ 7] +
                                 g_aiT32[k*32+ 8]*O[ 8] + g_aiT32[k*32+ 9]*O[ 9] + g_aiT32[k*32+10]*O[10] + g_aiT32[k*32+11]*O[11] +
                                 g_aiT32[k*32+12]*O[12] + g_aiT32[k*32+13]*O[13] + g_aiT32[k*32+14]*O[14] + g_aiT32[k*32+15]*O[15] + rnd) >> nShift;
        }
    }
}


/** 4x4 inverse transform implemented using partial butterfly structure (1D)
 *  \param pSrc   input data (residual)
 *  \param pDst   output data (transpose transform coefficients)
 *  \param nLines transform lines
 *  \param nShift specifies right shift after 1D transform
 */
void xInvDCT4( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 = g_aiT4[1*4+0]*pSrc[1*nStride+i] + g_aiT4[3*4+0]*pSrc[3*nStride+i];
        Int32 O1 = g_aiT4[1*4+1]*pSrc[1*nStride+i] + g_aiT4[3*4+1]*pSrc[3*nStride+i];
        Int32 E0 = g_aiT4[0*4+0]*pSrc[0*nStride+i] + g_aiT4[2*4+0]*pSrc[2*nStride+i];
        Int32 E1 = g_aiT4[0*4+1]*pSrc[0*nStride+i] + g_aiT4[2*4+1]*pSrc[2*nStride+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        pDst[i*nStride+0] = (E0 + O0 + rnd) >> nShift;
        pDst[i*nStride+1] = (E1 + O1 + rnd) >> nShift;
        pDst[i*nStride+2] = (E1 - O1 + rnd) >> nShift;
        pDst[i*nStride+3] = (E0 - O0 + rnd) >> nShift;

        pDst[i*nStride+0] = Clip3( -32768, 32767, pDst[i*nStride+0] );
        pDst[i*nStride+1] = Clip3( -32768, 32767, pDst[i*nStride+1] );
        pDst[i*nStride+2] = Clip3( -32768, 32767, pDst[i*nStride+2] );
        pDst[i*nStride+3] = Clip3( -32768, 32767, pDst[i*nStride+3] );
    }
}

void xInvDST4( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i;
    int rnd = 1 << (nShift-1);

    for( i=0; i<4; i++ ) {
        // Intermediate Variables
        Int32 c0 = pSrc[0*nStride+i] + pSrc[2*nStride+i];
        Int32 c1 = pSrc[2*nStride+i] + pSrc[3*nStride+i];
        Int32 c2 = pSrc[0*nStride+i] - pSrc[3*nStride+i];
        Int32 c3 = 74* pSrc[1*nStride+i];
        Int32 c4 = pSrc[0*nStride+i] - pSrc[2*nStride+i] + pSrc[3*nStride+i];

        pDst[i*nStride+0] = ( 29 * c0 + 55 * c1 + c3 + rnd ) >> nShift;
        pDst[i*nStride+1] = ( 55 * c2 - 29 * c1 + c3 + rnd ) >> nShift;
        pDst[i*nStride+2] = ( 74 * c4                + rnd ) >> nShift;
        pDst[i*nStride+3] = ( 55 * c0 + 29 * c2 - c3 + rnd ) >> nShift;

        pDst[i*nStride+0] = Clip3( -32768, 32767, pDst[i*nStride+0] );
        pDst[i*nStride+1] = Clip3( -32768, 32767, pDst[i*nStride+1] );
        pDst[i*nStride+2] = Clip3( -32768, 32767, pDst[i*nStride+2] );
        pDst[i*nStride+3] = Clip3( -32768, 32767, pDst[i*nStride+3] );
  }
}

void xInvDCT8( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 = g_aiT8[1*8+0]*pSrc[1*nStride+i] + g_aiT8[3*8+0]*pSrc[3*nStride+i] + g_aiT8[5*8+0]*pSrc[5*nStride+i] + g_aiT8[7*8+0]*pSrc[7*nStride+i];
        Int32 O1 = g_aiT8[1*8+1]*pSrc[1*nStride+i] + g_aiT8[3*8+1]*pSrc[3*nStride+i] + g_aiT8[5*8+1]*pSrc[5*nStride+i] + g_aiT8[7*8+1]*pSrc[7*nStride+i];
        Int32 O2 = g_aiT8[1*8+2]*pSrc[1*nStride+i] + g_aiT8[3*8+2]*pSrc[3*nStride+i] + g_aiT8[5*8+2]*pSrc[5*nStride+i] + g_aiT8[7*8+2]*pSrc[7*nStride+i];
        Int32 O3 = g_aiT8[1*8+3]*pSrc[1*nStride+i] + g_aiT8[3*8+3]*pSrc[3*nStride+i] + g_aiT8[5*8+3]*pSrc[5*nStride+i] + g_aiT8[7*8+3]*pSrc[7*nStride+i];

        Int32 EO0 = g_aiT8[2*8+0]*pSrc[2*nStride+i] + g_aiT8[6*8+0]*pSrc[6*nStride+i];
        Int32 EO1 = g_aiT8[2*8+1]*pSrc[2*nStride+i] + g_aiT8[6*8+1]*pSrc[6*nStride+i];
        Int32 EE0 = g_aiT8[0*8+0]*pSrc[0*nStride+i] + g_aiT8[4*8+0]*pSrc[4*nStride+i];
        Int32 EE1 = g_aiT8[0*8+1]*pSrc[0*nStride+i] + g_aiT8[4*8+1]*pSrc[4*nStride+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        Int32 E0 = EE0 + EO0;
        Int32 E3 = EE0 - EO0;
        Int32 E1 = EE1 + EO1;
        Int32 E2 = EE1 - EO1;

        pDst[i*nStride+0] = (E0 + O0 + rnd) >> nShift;
        pDst[i*nStride+1] = (E1 + O1 + rnd) >> nShift;
        pDst[i*nStride+2] = (E2 + O2 + rnd) >> nShift;
        pDst[i*nStride+3] = (E3 + O3 + rnd) >> nShift;
        pDst[i*nStride+4] = (E3 - O3 + rnd) >> nShift;
        pDst[i*nStride+5] = (E2 - O2 + rnd) >> nShift;
        pDst[i*nStride+6] = (E1 - O1 + rnd) >> nShift;
        pDst[i*nStride+7] = (E0 - O0 + rnd) >> nShift;
        pDst[i*nStride+0] = Clip3( -32768, 32767, pDst[i*nStride+0] );
        pDst[i*nStride+1] = Clip3( -32768, 32767, pDst[i*nStride+1] );
        pDst[i*nStride+2] = Clip3( -32768, 32767, pDst[i*nStride+2] );
        pDst[i*nStride+3] = Clip3( -32768, 32767, pDst[i*nStride+3] );
        pDst[i*nStride+4] = Clip3( -32768, 32767, pDst[i*nStride+4] );
        pDst[i*nStride+5] = Clip3( -32768, 32767, pDst[i*nStride+5] );
        pDst[i*nStride+6] = Clip3( -32768, 32767, pDst[i*nStride+6] );
        pDst[i*nStride+7] = Clip3( -32768, 32767, pDst[i*nStride+7] );
    }
}

void xInvDCT16( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i;
    int rnd = 1<<(nShift-1);

    for( i=0; i<nLines; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 =   g_aiT16[ 1*16+0]*pSrc[ 1*nStride+i] + g_aiT16[ 3*16+0]*pSrc[ 3*nStride+i] + g_aiT16[ 5*16+0]*pSrc[ 5*nStride+i] + g_aiT16[ 7*16+0]*pSrc[ 7*nStride+i]
                   + g_aiT16[ 9*16+0]*pSrc[ 9*nStride+i] + g_aiT16[11*16+0]*pSrc[11*nStride+i] + g_aiT16[13*16+0]*pSrc[13*nStride+i] + g_aiT16[15*16+0]*pSrc[15*nStride+i];
        Int32 O1 =   g_aiT16[ 1*16+1]*pSrc[ 1*nStride+i] + g_aiT16[ 3*16+1]*pSrc[ 3*nStride+i] + g_aiT16[ 5*16+1]*pSrc[ 5*nStride+i] + g_aiT16[ 7*16+1]*pSrc[ 7*nStride+i]
                   + g_aiT16[ 9*16+1]*pSrc[ 9*nStride+i] + g_aiT16[11*16+1]*pSrc[11*nStride+i] + g_aiT16[13*16+1]*pSrc[13*nStride+i] + g_aiT16[15*16+1]*pSrc[15*nStride+i];
        Int32 O2 =   g_aiT16[ 1*16+2]*pSrc[ 1*nStride+i] + g_aiT16[ 3*16+2]*pSrc[ 3*nStride+i] + g_aiT16[ 5*16+2]*pSrc[ 5*nStride+i] + g_aiT16[ 7*16+2]*pSrc[ 7*nStride+i]
                   + g_aiT16[ 9*16+2]*pSrc[ 9*nStride+i] + g_aiT16[11*16+2]*pSrc[11*nStride+i] + g_aiT16[13*16+2]*pSrc[13*nStride+i] + g_aiT16[15*16+2]*pSrc[15*nStride+i];
        Int32 O3 =   g_aiT16[ 1*16+3]*pSrc[ 1*nStride+i] + g_aiT16[ 3*16+3]*pSrc[ 3*nStride+i] + g_aiT16[ 5*16+3]*pSrc[ 5*nStride+i] + g_aiT16[ 7*16+3]*pSrc[ 7*nStride+i]
                   + g_aiT16[ 9*16+3]*pSrc[ 9*nStride+i] + g_aiT16[11*16+3]*pSrc[11*nStride+i] + g_aiT16[13*16+3]*pSrc[13*nStride+i] + g_aiT16[15*16+3]*pSrc[15*nStride+i];
        Int32 O4 =   g_aiT16[ 1*16+4]*pSrc[ 1*nStride+i] + g_aiT16[ 3*16+4]*pSrc[ 3*nStride+i] + g_aiT16[ 5*16+4]*pSrc[ 5*nStride+i] + g_aiT16[ 7*16+4]*pSrc[ 7*nStride+i]
                   + g_aiT16[ 9*16+4]*pSrc[ 9*nStride+i] + g_aiT16[11*16+4]*pSrc[11*nStride+i] + g_aiT16[13*16+4]*pSrc[13*nStride+i] + g_aiT16[15*16+4]*pSrc[15*nStride+i];
        Int32 O5 =   g_aiT16[ 1*16+5]*pSrc[ 1*nStride+i] + g_aiT16[ 3*16+5]*pSrc[ 3*nStride+i] + g_aiT16[ 5*16+5]*pSrc[ 5*nStride+i] + g_aiT16[ 7*16+5]*pSrc[ 7*nStride+i]
                   + g_aiT16[ 9*16+5]*pSrc[ 9*nStride+i] + g_aiT16[11*16+5]*pSrc[11*nStride+i] + g_aiT16[13*16+5]*pSrc[13*nStride+i] + g_aiT16[15*16+5]*pSrc[15*nStride+i];
        Int32 O6 =   g_aiT16[ 1*16+6]*pSrc[ 1*nStride+i] + g_aiT16[ 3*16+6]*pSrc[ 3*nStride+i] + g_aiT16[ 5*16+6]*pSrc[ 5*nStride+i] + g_aiT16[ 7*16+6]*pSrc[ 7*nStride+i]
                   + g_aiT16[ 9*16+6]*pSrc[ 9*nStride+i] + g_aiT16[11*16+6]*pSrc[11*nStride+i] + g_aiT16[13*16+6]*pSrc[13*nStride+i] + g_aiT16[15*16+6]*pSrc[15*nStride+i];
        Int32 O7 =   g_aiT16[ 1*16+7]*pSrc[ 1*nStride+i] + g_aiT16[ 3*16+7]*pSrc[ 3*nStride+i] + g_aiT16[ 5*16+7]*pSrc[ 5*nStride+i] + g_aiT16[ 7*16+7]*pSrc[ 7*nStride+i]
                   + g_aiT16[ 9*16+7]*pSrc[ 9*nStride+i] + g_aiT16[11*16+7]*pSrc[11*nStride+i] + g_aiT16[13*16+7]*pSrc[13*nStride+i] + g_aiT16[15*16+7]*pSrc[15*nStride+i];

        Int32 EO0 = g_aiT16[ 2*16+0]*pSrc[ 2*nStride+i] + g_aiT16[ 6*16+0]*pSrc[ 6*nStride+i] + g_aiT16[10*16+0]*pSrc[10*nStride+i] + g_aiT16[14*16+0]*pSrc[14*nStride+i];
        Int32 EO1 = g_aiT16[ 2*16+1]*pSrc[ 2*nStride+i] + g_aiT16[ 6*16+1]*pSrc[ 6*nStride+i] + g_aiT16[10*16+1]*pSrc[10*nStride+i] + g_aiT16[14*16+1]*pSrc[14*nStride+i];
        Int32 EO2 = g_aiT16[ 2*16+2]*pSrc[ 2*nStride+i] + g_aiT16[ 6*16+2]*pSrc[ 6*nStride+i] + g_aiT16[10*16+2]*pSrc[10*nStride+i] + g_aiT16[14*16+2]*pSrc[14*nStride+i];
        Int32 EO3 = g_aiT16[ 2*16+3]*pSrc[ 2*nStride+i] + g_aiT16[ 6*16+3]*pSrc[ 6*nStride+i] + g_aiT16[10*16+3]*pSrc[10*nStride+i] + g_aiT16[14*16+3]*pSrc[14*nStride+i];

        Int32 EEO0 = g_aiT16[4*16+0]*pSrc[4*nStride+i] + g_aiT16[12*16+0]*pSrc[12*nStride+i];
        Int32 EEO1 = g_aiT16[4*16+1]*pSrc[4*nStride+i] + g_aiT16[12*16+1]*pSrc[12*nStride+i];
        Int32 EEE0 = g_aiT16[0*16+0]*pSrc[0*nStride+i] + g_aiT16[ 8*16+0]*pSrc[ 8*nStride+i];
        Int32 EEE1 = g_aiT16[0*16+1]*pSrc[0*nStride+i] + g_aiT16[ 8*16+1]*pSrc[ 8*nStride+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        Int32 EE0 = EEE0 + EEO0;
        Int32 EE3 = EEE0 - EEO0;
        Int32 EE1 = EEE1 + EEO1;
        Int32 EE2 = EEE1 - EEO1;

        Int32 E0 = EE0 + EO0;
        Int32 E7 = EE0 - EO0;
        Int32 E1 = EE1 + EO1;
        Int32 E6 = EE1 - EO1;
        Int32 E2 = EE2 + EO2;
        Int32 E5 = EE2 - EO2;
        Int32 E3 = EE3 + EO3;
        Int32 E4 = EE3 - EO3;

        pDst[i*nStride+ 0] = (E0 + O0 + rnd) >> nShift;
        pDst[i*nStride+15] = (E0 - O0 + rnd) >> nShift;
        pDst[i*nStride+ 1] = (E1 + O1 + rnd) >> nShift;
        pDst[i*nStride+14] = (E1 - O1 + rnd) >> nShift;
        pDst[i*nStride+ 2] = (E2 + O2 + rnd) >> nShift;
        pDst[i*nStride+13] = (E2 - O2 + rnd) >> nShift;
        pDst[i*nStride+ 3] = (E3 + O3 + rnd) >> nShift;
        pDst[i*nStride+12] = (E3 - O3 + rnd) >> nShift;
        pDst[i*nStride+ 4] = (E4 + O4 + rnd) >> nShift;
        pDst[i*nStride+11] = (E4 - O4 + rnd) >> nShift;
        pDst[i*nStride+ 5] = (E5 + O5 + rnd) >> nShift;
        pDst[i*nStride+10] = (E5 - O5 + rnd) >> nShift;
        pDst[i*nStride+ 6] = (E6 + O6 + rnd) >> nShift;
        pDst[i*nStride+ 9] = (E6 - O6 + rnd) >> nShift;
        pDst[i*nStride+ 7] = (E7 + O7 + rnd) >> nShift;
        pDst[i*nStride+ 8] = (E7 - O7 + rnd) >> nShift;

        pDst[i*nStride+ 0] = Clip3( -32768, 32767, pDst[i*nStride+ 0] );
        pDst[i*nStride+ 1] = Clip3( -32768, 32767, pDst[i*nStride+ 1] );
        pDst[i*nStride+ 2] = Clip3( -32768, 32767, pDst[i*nStride+ 2] );
        pDst[i*nStride+ 3] = Clip3( -32768, 32767, pDst[i*nStride+ 3] );
        pDst[i*nStride+ 4] = Clip3( -32768, 32767, pDst[i*nStride+ 4] );
        pDst[i*nStride+ 5] = Clip3( -32768, 32767, pDst[i*nStride+ 5] );
        pDst[i*nStride+ 6] = Clip3( -32768, 32767, pDst[i*nStride+ 6] );
        pDst[i*nStride+ 7] = Clip3( -32768, 32767, pDst[i*nStride+ 7] );
        pDst[i*nStride+ 8] = Clip3( -32768, 32767, pDst[i*nStride+ 8] );
        pDst[i*nStride+ 9] = Clip3( -32768, 32767, pDst[i*nStride+ 9] );
        pDst[i*nStride+10] = Clip3( -32768, 32767, pDst[i*nStride+10] );
        pDst[i*nStride+11] = Clip3( -32768, 32767, pDst[i*nStride+11] );
        pDst[i*nStride+12] = Clip3( -32768, 32767, pDst[i*nStride+12] );
        pDst[i*nStride+13] = Clip3( -32768, 32767, pDst[i*nStride+13] );
        pDst[i*nStride+14] = Clip3( -32768, 32767, pDst[i*nStride+14] );
        pDst[i*nStride+15] = Clip3( -32768, 32767, pDst[i*nStride+15] );
    }
}

/** 32x32 inverse transform implemented using partial butterfly structure (1D)
 *  \param pSrc   input data (residual)
 *  \param pDst   output data (transpose transform coefficients)
 *  \param nLines transform lines
 *  \param nShift specifies right shift after 1D transform
 */
void xInvDCT32( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift )
{
    int i,k;
    Int32 E[16],O[16];
    Int32 EE[8],EO[8];
    Int32 EEE[4],EEO[4];
    Int32 EEEE[2],EEEO[2];
    int rnd = 1<<(nShift-1);

    for( i=0; i<32; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for( k=0; k<16; k++ ) {
            O[k] =   g_aiT32[ 1*32+k]*pSrc[ 1*nStride+i] + g_aiT32[ 3*32+k]*pSrc[ 3*nStride+i]
                   + g_aiT32[ 5*32+k]*pSrc[ 5*nStride+i] + g_aiT32[ 7*32+k]*pSrc[ 7*nStride+i]
                   + g_aiT32[ 9*32+k]*pSrc[ 9*nStride+i] + g_aiT32[11*32+k]*pSrc[11*nStride+i]
                   + g_aiT32[13*32+k]*pSrc[13*nStride+i] + g_aiT32[15*32+k]*pSrc[15*nStride+i]
                   + g_aiT32[17*32+k]*pSrc[17*nStride+i] + g_aiT32[19*32+k]*pSrc[19*nStride+i]
                   + g_aiT32[21*32+k]*pSrc[21*nStride+i] + g_aiT32[23*32+k]*pSrc[23*nStride+i]
                   + g_aiT32[25*32+k]*pSrc[25*nStride+i] + g_aiT32[27*32+k]*pSrc[27*nStride+i]
                   + g_aiT32[29*32+k]*pSrc[29*nStride+i] + g_aiT32[31*32+k]*pSrc[31*nStride+i];
        }

        for( k=0; k<8; k++ ) {
            EO[k] =   g_aiT32[ 2*32+k]*pSrc[ 2*nStride+i] + g_aiT32[ 6*32+k]*pSrc[ 6*nStride+i]
                    + g_aiT32[10*32+k]*pSrc[10*nStride+i] + g_aiT32[14*32+k]*pSrc[14*nStride+i]
                    + g_aiT32[18*32+k]*pSrc[18*nStride+i] + g_aiT32[22*32+k]*pSrc[22*nStride+i]
                    + g_aiT32[26*32+k]*pSrc[26*nStride+i] + g_aiT32[30*32+k]*pSrc[30*nStride+i];
        }

        for( k=0; k<4; k++ ) {
            EEO[k] =   g_aiT32[ 4*32+k]*pSrc[ 4*nStride+i] + g_aiT32[12*32+k]*pSrc[12*nStride+i]
                     + g_aiT32[20*32+k]*pSrc[20*nStride+i] + g_aiT32[28*32+k]*pSrc[28*nStride+i];
        }
        EEEO[0] = g_aiT32[8*32+0]*pSrc[8*nStride+i] + g_aiT32[24*32+0]*pSrc[24*nStride+i];
        EEEO[1] = g_aiT32[8*32+1]*pSrc[8*nStride+i] + g_aiT32[24*32+1]*pSrc[24*nStride+i];
        EEEE[0] = g_aiT32[0*32+0]*pSrc[0*nStride+i] + g_aiT32[16*32+0]*pSrc[16*nStride+i];
        EEEE[1] = g_aiT32[0*32+1]*pSrc[0*nStride+i] + g_aiT32[16*32+1]*pSrc[16*nStride+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        EEE[0] = EEEE[0] + EEEO[0];
        EEE[3] = EEEE[0] - EEEO[0];
        EEE[1] = EEEE[1] + EEEO[1];
        EEE[2] = EEEE[1] - EEEO[1];

        EE[0] = EEE[0] + EEO[0];
        EE[7] = EEE[0] - EEO[0];
        EE[1] = EEE[1] + EEO[1];
        EE[6] = EEE[1] - EEO[1];
        EE[5] = EEE[2] - EEO[2];
        EE[2] = EEE[2] + EEO[2];
        EE[3] = EEE[3] + EEO[3];
        EE[4] = EEE[3] - EEO[3];

        for( k=0; k<8; k++ ) {
            E[k  ] = EE[k  ] + EO[k  ];
            E[k+8] = EE[7-k] - EO[7-k];
        }

        for( k=0; k<16; k++ ) {
            pDst[i*nStride+k   ] = (E[k   ] + O[k   ] + rnd) >> nShift;
            pDst[i*nStride+k+16] = (E[15-k] - O[15-k] + rnd) >> nShift;
            pDst[i*nStride+k   ] = Clip3( -32768, 32767, pDst[i*nStride+k   ]);
            pDst[i*nStride+k+16] = Clip3( -32768, 32767, pDst[i*nStride+k+16]);
        }
    }
}

xDCT *xDctN[MAX_CU_DEPTH+1] = {
    xDST4,
    xDCT4,
    xDCT8,
    xDCT16,
    xDCT32
};

xDCT *xInvDctN[MAX_CU_DEPTH+1] = {
    xInvDST4,
    xInvDCT4,
    xInvDCT8,
    xInvDCT16,
    xInvDCT32
};


// ***************************************************************************
// * Interface Functions
// ***************************************************************************
UInt32 xQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, X265_SliceType eSType )
{
    int x, y;
    UInt32 uiAcSum = 0;
    const UInt nQpDiv6 = nQP / 6;
    const UInt nQpMod6 = nQP % 6;
    UInt uiLog2TrSize = xLog2( iWidth - 1);
    UInt uiQ = g_quantScales[nQpMod6];
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - uiLog2TrSize;  // Represents scaling through forward transform
    Int iQBits = QUANT_SHIFT + nQpDiv6 + iTransformShift;

    Int32 iRnd = (eSType == SLICE_I ? 171 : 85) << (iQBits-9);

    for( y=0; y < iHeight; y++ ) {
        for( x=0; x < iWidth; x++ ) {
            Int iLevel;
            Int  iSign;
            UInt nBlockPos = y * nStride + x;
            iLevel  = pSrc[nBlockPos];
            iSign   = (iLevel < 0 ? -1: 1);      

            iLevel = ( abs(iLevel) * uiQ + iRnd ) >> iQBits;
            uiAcSum += iLevel;
            iLevel *= iSign;        
            pDst[nBlockPos] = iLevel;
            pDst[nBlockPos] = Clip3(-32768, 32767, pDst[nBlockPos]);
        }
    }
    #if (CHECK_TV)
    {
        Int16 *P = (nStride == MAX_CU_SIZE) ? tv_quant : tv_quantC[tv_nIdxC][tv_nModeC];
        for( y=0; y<iHeight; y++ ) {
            for( x=0; x<iWidth; x++ ) {
                if (pDst[y * nStride + x] != P[y * nStride + x] ) {
                    printf( " Quant(%d,%d) %04X -> %04X Failed!\n", y, x, P[y * nStride + x] & 0xFFFF, pDst[y * nStride + x] & 0xFFFF );
                    abort();
                }
            }
        }
    }
    #endif
    return uiAcSum;
}

void xDeQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, X265_SliceType eSType )
{
    int x, y;
    const UInt nQpDiv6 = nQP / 6;
    const UInt nQpMod6 = nQP % 6;
    UInt uiLog2TrSize = xLog2( iWidth - 1 );
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - uiLog2TrSize;  // Represents scaling through forward transform
    Int nShift = nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
    Int32 iRnd = 1 << (nShift-1);
    Int iScale = g_invQuantScales[nQpMod6] << nQpDiv6;

    for( y=0; y < iHeight; y++ ) {
        for( x=0; x < iWidth; x++ ) {
            UInt nBlockPos = y * nStride + x;
            Int32 iCoeffQ = ( pSrc[nBlockPos] * iScale + iRnd ) >> nShift;
            pDst[nBlockPos] = Clip3(-32768, 32767, iCoeffQ);
        }
    }
    #if (CHECK_TV)
    {
        Int16 *P = (nStride == MAX_CU_SIZE) ? tv_iquant : tv_iquantC[tv_nIdxC][tv_nModeC];
        for( y=0; y<iHeight; y++ ) {
            for( x=0; x<iWidth; x++ ) {
                if (pDst[y * nStride + x] != P[y * nStride + x] ) {
                    printf( " DeQuant(%d,%d) %04X -> %04X Failed!\n", y, x, P[y * nStride + x] & 0xFFFF, pDst[y * nStride + x] & 0xFFFF );
                    abort();
                }
            }
        }
    }
    #endif
}

void xSubDct(
    Int16 *pDst,
    UInt8 *pSrc,
    UInt8 *pRef, UInt nStride,
    Int16 *piTmp0, Int16 *piTmp1,
    Int iWidth, Int iHeight, UInt nMode
)
{
    const Int nLog2Width  = xLog2( iWidth - 1 );
    const Int nLog2Height = xLog2( iHeight - 1 );
    const Int bUseDst     = (iWidth + iHeight == 8) && (nMode != MODE_INVALID);
    const Int bUseDstHor  = bUseDst && (!nMode || (nMode>=2  && nMode <= 25));
    const Int bUseDstVer  = bUseDst && (!nMode || (nMode>=11 && nMode <= 34));
    int i, j;

    // The TU must be less than or equal to 5
    assert( (nLog2Width <= 5) && (nLog2Height <= 5) );

    // Sub
    for( i=0; i<iHeight; i++ ) {
        for( j=0; j<iWidth; j++ ) {
            piTmp0[i * nStride + j] = pSrc[i * nStride + j] - pRef[i * nStride + j];
        }
    }
    #if (CHECK_TV)
    {
        Int16 *P = (nStride == MAX_CU_SIZE) ? tv_resi : tv_resiC[tv_nIdxC][tv_nModeC];
        for( i=0; i<iHeight; i++ ) {
            for( j=0; j<iWidth; j++ ) {
                if (piTmp0[i * nStride + j] != P[i * nStride + j] ) {
                    printf( " Residual(%d,%d) %04X -> %04X Failed!\n", i, j, P[i * nStride + j] & 0xFFFF, piTmp0[i * nStride + j] & 0xFFFF );
                    abort();
                }
            }
        }
    }
    #endif

    xDctN[nLog2Width  - 1 - bUseDstHor]( piTmp1, piTmp0, nStride, iHeight, nLog2Width -1 );
    xDctN[nLog2Height - 1 - bUseDstVer]( pDst,   piTmp1, nStride, iWidth,  nLog2Height+6 );

    #if (CHECK_TV)
    {
        Int16 *P = (nStride == MAX_CU_SIZE) ? tv_trans : tv_transC[tv_nIdxC][tv_nModeC];
        for( i=0; i<iHeight; i++ ) {
            for( j=0; j<iWidth; j++ ) {
                if (pDst[i * nStride + j] != P[i * nStride + j] ) {
                    printf( " Transform(%d,%d) %04X -> %04X Failed!\n", i, j, P[i * nStride + j] & 0xFFFF, pDst[i * nStride + j] & 0xFFFF );
                    abort();
                }
            }
        }
    }
    #endif
}

void xIDctAdd(
    UInt8 *pDst,
    Int16 *pSrc,
    UInt8 *pRef, UInt nStride,
    Int16 *piTmp0, Int16 *piTmp1,
    Int iWidth, Int iHeight, UInt nMode
)
{
    const Int nLog2Width  = xLog2( iWidth - 1 );
    const Int nLog2Height = xLog2( iHeight - 1 );
    const Int bUseDst     = (iWidth + iHeight == 8) && (nMode != MODE_INVALID);
    const Int bUseDstHor  = bUseDst && (!nMode || (nMode>=11 && nMode <= 34));
    const Int bUseDstVer  = bUseDst && (!nMode || (nMode>=2  && nMode <= 25));
    int i, j;

    xInvDctN[nLog2Width  - 1 - bUseDstHor]( piTmp0, pSrc,   nStride, iHeight, SHIFT_INV_1ST );
    xInvDctN[nLog2Height - 1 - bUseDstVer]( piTmp1, piTmp0, nStride, iWidth,  SHIFT_INV_2ND );

    #if (CHECK_TV)
    {
        Int16 *P = (nStride == MAX_CU_SIZE) ? tv_itrans : tv_itransC[tv_nIdxC][tv_nModeC];
        for( i=0; i<iHeight; i++ ) {
            for( j=0; j<iWidth; j++ ) {
                if (piTmp1[i * nStride + j] != P[i * nStride + j] ) {
                    printf( " ITransform(%d,%d) %04X -> %04X Failed!\n", i, j, P[i * nStride + j] & 0xFFFF, piTmp1[i * nStride + j] & 0xFFFF );
                    abort();
                }
            }
        }
    }
    #endif

    // Add
    for( i=0; i<iHeight; i++ ) {
        for( j=0; j<iWidth; j++ ) {
            pDst[i * nStride + j] = Clip( piTmp1[i * nStride + j] + pRef[i * nStride + j] );
        }
    }

    #if (CHECK_TV)
    {
        UInt8 *P = (nStride == MAX_CU_SIZE) ? tv_rec : tv_recC[tv_nIdxC][tv_nModeC];
        for( i=0; i<iHeight; i++ ) {
            for( j=0; j<iWidth; j++ ) {
                if (pDst[i * nStride + j] != P[i * nStride + j] ) {
                    printf( " Reconst(%d,%d) %04X -> %04X Failed!\n", i, j, P[i * nStride + j] & 0xFFFF, pDst[i * nStride + j] & 0xFFFF );
                    abort();
                }
            }
        }
    }
    #endif
}

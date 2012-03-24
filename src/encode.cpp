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
    #if (CHECK_TV)
    if ( tInitTv( "CHEN_TV.TXT" ) < 0)
        abort();
    #endif
}

Int32 xEncEncode( X265_t *h, X265_Frame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize )
{
    const UInt32    uiWidth     = h->usWidth;
    const UInt32    uiHeight    = h->usHeight;
    X265_BitStream *pBS         = &h->bs;
    X265_Cache     *pCache      = &h->cache;
          Int       nQP         = h->iQP;
          Int       nQPC        = g_aucChromaScale[nQP];
    const Int32     lambda      = nQP;
          UInt8    *pucMostModeY= pCache->ucMostModeY;
          UInt8    *pucPixY     = pCache->pucPixY;
          UInt8    *pucRecY     = pCache->pucRecY;
          UInt8    *pucPredY    = pCache->pucPredY;
          UInt8    *pucPixC[2]  = { pCache->pucPixU, pCache->pucPixV };
          UInt8    *pucRecC[2]  = { pCache->pucRecU, pCache->pucRecV };
          UInt8    *pucPredC[2] = { pCache->pucPredC[0], pCache->pucPredC[1] };
          Int16    *piTmp0      = pCache->piTmp[0];
          Int16    *piTmp1      = pCache->piTmp[1];
          Int16    *piCoefY     = pCache->piCoefY;
          Int16    *piCoefC[2]  = { pCache->piCoefU, pCache->piCoefV };
          UInt8    *pucMostModeC= pCache->ucMostModeC;
          UInt      realModeC;
    Int x, y;
    Int i;
    UInt32 uiSum;

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
        for( x=0; x < uiWidth; x+=h->ucMaxCUWidth ) {
            const UInt   nCUSize     = h->ucMaxCUWidth;
            const UInt   nLog2CUSize = xLog2(nCUSize-1);
            UInt32 uiBestSadY, uiBestSadC;
            UInt   nBestModeY, nBestModeC;
            UInt   nMode;

            // Stage 0: Init internal
            h->uiCUX = x;

            #if (CHECK_TV)
            tGetVector();
            #endif

            // Stage 1a: Load image to cache
            xEncCacheLoadCU( h, x, y );
            #if (CHECK_TV)
            // Check Y
            {
                int x, y;
                for( y=0; y<nCUSize; y++ ) {
                    for( x=0; x<nCUSize; x++ ) {
                        if ( pCache->pucPixY[y * MAX_CU_SIZE + x] != tv_orig[y * MAX_CU_SIZE + x] ) {
                            fprintf( stderr, "Orig Pixel Y Wrong, (%d,%d), %02X -> %02X\n", y, x, tv_orig[y * MAX_CU_SIZE + x], pCache->pucPixY[y * MAX_CU_SIZE + x] );
                            abort();
                        }
                    }
                }
            }
            // Check U and V
            {
                int x, y;
                for( y=0; y<nCUSize/2; y++ ) {
                    for( x=0; x<nCUSize/2; x++ ) {
                        if ( pCache->pucPixU[y * MAX_CU_SIZE/2 + x] != tv_origC[0][y * MAX_CU_SIZE/2 + x] ) {
                            fprintf( stderr, "Orig Pixel U Wrong, (%d,%d), %02X -> %02X\n", y, x, tv_origC[0][y * MAX_CU_SIZE/2 + x], pCache->pucPixY[y * MAX_CU_SIZE/2 + x] );
                            abort();
                        }
                        if ( pCache->pucPixV[y * MAX_CU_SIZE/2 + x] != tv_origC[1][y * MAX_CU_SIZE/2 + x] ) {
                            fprintf( stderr, "OrigC Pixel V Wrong, (%d,%d), %02X -> %02X\n", y, x, tv_origC[1][y * MAX_CU_SIZE/2 + x], pCache->pucPixY[y * MAX_CU_SIZE/2 + x] );
                            abort();
                        }
                    }
                }
            }
            #endif

            // Stage 1b: Load Intra PU Reference Samples
            // TODO: ASSUME one PU only
            xEncIntraLoadRef( h, 0, 0, h->ucMaxCUWidth );

            // Stage 2a: Decide Intra
            // TODO: Support LM mode
            // TODO: Support more size
            uiBestSadY = MAX_SAD;
            nBestModeY = 0;
            for( nMode=0; nMode<35; nMode++ ) {
                UInt32 uiSad;

                #if (CHECK_TV)
                memset( pucPredY, 0xCD, sizeof(pCache->pucPredY) );
                #endif

                xEncIntraPredLuma( h, nMode, nCUSize );

                #if (CHECK_TV)
                {
                    int x, y;
                    for( y=0; y<nCUSize; y++ ) {
                        for( x=0; x<nCUSize; x++ ) {
                            if ( pucPredY[y * MAX_CU_SIZE + x] != tv_pred[nMode][y * MAX_CU_SIZE + x] ) {
                                fprintf( stderr, "Intra Pred Y Wrong, Mode %d at (%d,%d), %02X -> %02X\n", nMode, y, x, tv_pred[nMode][y*nCUSize+x], pCache->pucPredY[y * MAX_CU_SIZE + x] );
                                abort();
                            }
                        }
                    }
                }
                #endif

                if ( nMode == pucMostModeY[0] )
                    uiSad = 1 * lambda;
                else if ( nMode == pucMostModeY[1] || nMode == pucMostModeY[2] )
                    uiSad = 2 * lambda;
                else
                    uiSad = 3 * lambda;
                uiSad += xSadN[nLog2CUSize-2](
                            nCUSize,
                            pucPixY, MAX_CU_SIZE,
                            pucPredY, MAX_CU_SIZE
                        );

                if ( uiSad < uiBestSadY ) {
                    uiBestSadY = uiSad;
                    nBestModeY = nMode;
                }

                #if (CHECK_TV)
                if ( uiSad != tv_sad[nMode] ) {
                    printf( " Sad %d -> %d Failed!\n", tv_sad[nMode], uiSad );
                    abort();
                }
                #endif
            }
            #if (CHECK_TV)
            if ( nBestModeY != tv_bestmode ) {
                printf( " BestMode %d -> %d Failed!\n", tv_bestmode, nBestModeY );
                abort();
            }

            // GetAllowedChromaMode
            pucMostModeC[0] = PLANAR_IDX;
            pucMostModeC[1] = VER_IDX;
            pucMostModeC[2] = HOR_IDX;
            pucMostModeC[3] = DC_IDX;
            pucMostModeC[4] = nBestModeY;
            for( i=0;i<4; i++ ) {
                if ( pucMostModeC[i] == nBestModeY ) {
                    pucMostModeC[i] = 34;
                    break;
                }
            }

            uiBestSadC = MAX_SAD;
            for( nMode=0; nMode<5; nMode++ ) {
                UInt32 uiSad = 0;
                realModeC = pucMostModeC[nMode];

                #if (CHECK_TV)
                memset( pCache->pucPredC, 0xCD, sizeof(pCache->pucPredC) );
                #endif

                xEncIntraPredChroma( h, realModeC, nCUSize >> 1 );

                #if (CHECK_TV)
                {
                    int x, y;
                    for( y=0; y<nCUSize; y++ ) {
                        for( x=0; x<nCUSize; x++ ) {
                            if ( pucPredC[0][y * MAX_CU_SIZE/2 + x] != tv_predC[0][nMode][y * MAX_CU_SIZE/2 + x] ) {
                                fprintf( stderr, "Intra Pred U Wrong, Mode %d at (%d,%d), %02X -> %02X\n", nMode, y, x, tv_predC[0][nMode][y*nCUSize+x], pCache->pucPredC[0][y * MAX_CU_SIZE/2 + x] );
                                //abort();
                                goto _exit;
                            }
                            if ( pucPredC[1][y * MAX_CU_SIZE/2 + x] != tv_predC[1][nMode][y * MAX_CU_SIZE/2 + x] ) {
                                fprintf( stderr, "Intra Pred V Wrong, Mode %d at (%d,%d), %02X -> %02X\n", nMode, y, x, tv_predC[1][nMode][y*nCUSize+x], pCache->pucPredC[1][y * MAX_CU_SIZE/2 + x] );
                                //abort();
                                goto _exit;
                            }
                        }
                    }
_exit:;
                }
                #endif
                uiSad += xSadN[nLog2CUSize-2-1](
                            nCUSize / 2,
                            pucPixC[0], MAX_CU_SIZE/2,
                            pucPredC[0], MAX_CU_SIZE/2
                        );
                uiSad += xSadN[nLog2CUSize-2-1](
                            nCUSize / 2,
                            pucPixC[1], MAX_CU_SIZE/2,
                            pucPredC[1], MAX_CU_SIZE/2
                        );
                if ( uiSad < uiBestSadC ) {
                    uiBestSadC = uiSad;
                    nBestModeC = nMode;
                }
            }
            realModeC = pucMostModeC[nBestModeC];
            #if (CHECK_TV)
            {
                // Check Chroma Mode and Sad
                assert( uiBestSadC == tv_sadC );
                assert( (nBestModeC == 4 && realModeC == tv_bestmode) || (pucMostModeC[nBestModeC] == tv_bestmodeC) );
                tv_nModeC = nBestModeC;
            }
            #endif

            // Stage 3a: Encode CU
            pCache->nBestModeY = nBestModeY;
            pCache->nBestModeC = nBestModeC;
            // Y
            xEncIntraPredLuma( h, nBestModeY, nCUSize );
            xSubDct( piTmp0,
                     pucPixY,
                     pucPredY, MAX_CU_SIZE,
                     piTmp0, piTmp1,
                     nCUSize, nCUSize, nBestModeY );
            uiSum = xQuant( piCoefY, piTmp0, MAX_CU_SIZE, nQP, nCUSize, nCUSize, SLICE_I );

            // Cr and Cb
            xEncIntraPredChroma( h, realModeC, nCUSize >> 1 );
            for( i=0; i<2; i++ ) {
                #if (CHECK_TV)
                tv_nIdxC = i;
                #endif
                xSubDct( piTmp0,
                         pucPixC[i],
                         pucPredC[i], MAX_CU_SIZE/2,
                         piTmp0, piTmp1,
                         nCUSize/2, nCUSize/2, realModeC );
                uiSum += xQuant( piCoefC[i], piTmp0, MAX_CU_SIZE/2, nQPC, nCUSize/2, nCUSize/2, SLICE_I );
            }

            // Stage 3b: Decode CU
            if ( uiSum ) {
                xDeQuant( piTmp0, piCoefY, MAX_CU_SIZE, nQP, nCUSize, nCUSize, SLICE_I );
                xIDctAdd( pucRecY,
                          piTmp0,
                          pucPredY, MAX_CU_SIZE,
                          piTmp1, piTmp0,
                          nCUSize, nCUSize, nBestModeY );
                // Cr and Cb
                for( i=0; i<2; i++ ) {
                    #if (CHECK_TV)
                    tv_nIdxC = i;
                    #endif
                    xDeQuant( piTmp0, piCoefC[i], MAX_CU_SIZE/2, nQPC, nCUSize/2, nCUSize/2, SLICE_I );
                    xIDctAdd( pucRecC[i],
                              piTmp0,
                              pucPredC[i], MAX_CU_SIZE/2,
                              piTmp1, piTmp0,
                              nCUSize/2, nCUSize/2, realModeC );
                }
            }
            else {
                for( i=0; i<nCUSize; i++ ) {
                    memcpy( &pucRecY[i*MAX_CU_SIZE], &pucPredY[i*MAX_CU_SIZE], nCUSize );
                }
            }

            // Stage 4: Write CU
            // TBD

            // Stage 5: Update context
            xEncCacheUpdate( h, 0, 0, nCUSize, nCUSize );
            pCache->uiOffset += nCUSize;

            printf( "CU(%2d,%2d) Passed!\n", y/h->ucMaxCUWidth, x/h->ucMaxCUWidth );
            #endif
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
    memset( pCache->pucTopModeY, MODE_INVALID, sizeof(pCache->pucTopModeY) );
}

void xEncCahceInitLine( X265_t *h )
{
    X265_Cache *pCache  = &h->cache;
    pCache->uiOffset    = 0;
    memset( pCache->pucLeftModeY, MODE_INVALID, sizeof(pCache->pucLeftModeY) );
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

void xEncCacheUpdate( X265_t *h, UInt32 uiX, UInt32 uiY, UInt nWidth, UInt nHeight )
{
    X265_Cache  *pCache         = &h->cache;
    const UInt32 uiOffset       =  pCache->uiOffset;
          UInt8 *pucTopPixY     = &pCache->pucTopPixY[(uiOffset + uiX)];
          UInt8 *pucTopPixU     = &pCache->pucTopPixU[(uiOffset + uiX)/2];
          UInt8 *pucTopPixV     = &pCache->pucTopPixV[(uiOffset + uiX)/2];
          UInt8 *pucLeftPixY    =  pCache->pucLeftPixY + uiY;
          UInt8 *pucLeftPixU    =  pCache->pucLeftPixU + uiY / 2;
          UInt8 *pucLeftPixV    =  pCache->pucLeftPixV + uiY / 2;
          UInt8 *pucTopLeftY    =  pCache->pucTopLeftY;
          UInt8 *pucTopLeftU    =  pCache->pucTopLeftU;
          UInt8 *pucTopLeftV    =  pCache->pucTopLeftV;
          UInt8 *pucTopModeY    = &pCache->pucTopModeY[(uiOffset + uiX) / MIN_CU_SIZE];
          UInt8 *pucLeftModeY   =  pCache->pucLeftModeY + (uiY / MIN_CU_SIZE);
    const UInt8 *pucRecY        =  pCache->pucRecY;
    const UInt8 *pucRecU        =  pCache->pucRecU;
    const UInt8 *pucRecV        =  pCache->pucRecV;
    const UInt8 nBestModeY      =  pCache->nBestModeY;

    Int x, y;

    // Update TopLeft
    for( x=0; x<nWidth; x+=MIN_CU_SIZE ) {
        pucTopLeftY[x/MIN_CU_SIZE] = pucTopPixY[x + MIN_CU_SIZE - 1];
    }
    for( x=0; x<nWidth/2; x+=MIN_CU_SIZE ) {
        pucTopLeftU[x/MIN_CU_SIZE] = pucTopPixU[x + MIN_CU_SIZE - 1];
        pucTopLeftV[x/MIN_CU_SIZE] = pucTopPixV[x + MIN_CU_SIZE - 1];
    }

    // Update Top Y
    for( x=0; x<nWidth; x++ ) {
        pucTopPixY [x            ] = pucRecY[(nHeight-1) * MAX_CU_SIZE + x];
        pucTopModeY[x/MIN_CU_SIZE] = nBestModeY;
    }
    for( x=0; x<nWidth/2; x++ ) {
        pucTopPixU[x] = pucRecU[(nHeight/2-1) * MAX_CU_SIZE/2 + x];
        pucTopPixV[x] = pucRecV[(nHeight/2-1) * MAX_CU_SIZE/2 + x];
    }

    // Update Left Y
    for( x=0; x<nHeight; x++ ) {
        pucLeftPixY [x            ] = pucRecY[x * MAX_CU_SIZE + (nWidth - 1)];
        pucLeftModeY[x/MIN_CU_SIZE] = nBestModeY;
    }
    for( x=0; x<nHeight/2; x++ ) {
        pucLeftPixU[x] = pucRecU[x * MAX_CU_SIZE/2 + (nWidth/2 - 1)];
        pucLeftPixV[x] = pucRecV[x * MAX_CU_SIZE/2 + (nWidth/2 - 1)];
    }
}

// ***************************************************************************
// * IntraPred Functions
// ***************************************************************************
UInt xGetTopLeftIndex( UInt32 uiX, UInt32 uiY )
{
    UInt nOffsetX = uiX / MIN_CU_SIZE;
    UInt nOffsetY = uiY / MIN_CU_SIZE;
    UInt nIdx     = nOffsetY * MAX_PU_XY + nOffsetX;

    if ( nOffsetX == 0 )
        nIdx += MAX_PU_XY;

    return nIdx - 1;
}

void xPaddingRef( UInt8 *pucRef, const UInt8 bValid[5], const UInt nBlkOffset[6] )
{
    Int     i, n;
    UInt8   ucPadding;
    const Int nValid = 5;

    // Padding from Right to Left
    for( n=0; n<nValid; n++ ) {
        if (bValid[n])
            break;
    }
    ucPadding = pucRef[nBlkOffset[n]];
    for( i=0; i<nBlkOffset[n]; i++ ) {
        pucRef[i] = ucPadding;
    }

    // Padding from Left to Right
    for( ; n<nValid; n++ ) {
        if (!bValid[n]) {
            assert( n > 0 );
            const UInt nBlkAddr = nBlkOffset[n];
            const UInt nBlkSize = nBlkOffset[n + 1] - nBlkOffset[n];
            ucPadding = pucRef[nBlkAddr - 1];
            for( i=0; i<nBlkSize; i++ ) {
                pucRef[nBlkAddr + i] = ucPadding;
            }
        }
    }
}

void xEncIntraLoadRef( X265_t *h, UInt32 uiX, UInt32 uiY, UInt nSize )
{
    X265_Cache  *pCache         = &h->cache;
    const UInt   nMinTUSize     =  (1 << h->ucQuadtreeTULog2MinSize);
    const UInt32 uiOffset       =  pCache->uiOffset;
    const UInt8 *pucTopPixY     = &pCache->pucTopPixY[(uiOffset + uiX)];
    const UInt8 *pucTopPixU     = &pCache->pucTopPixU[(uiOffset + uiX)/2];
    const UInt8 *pucTopPixV     = &pCache->pucTopPixV[(uiOffset + uiX)/2];
    const UInt8 *pucLeftPixY    =  pCache->pucLeftPixY + uiY;
    const UInt8 *pucLeftPixU    =  pCache->pucLeftPixU + uiY / 2;
    const UInt8 *pucLeftPixV    =  pCache->pucLeftPixV + uiY / 2;
    const UInt8 *pucTopLeftY    =  pCache->pucTopLeftY;
    const UInt8 *pucTopLeftU    =  pCache->pucTopLeftU;
    const UInt8 *pucTopLeftV    =  pCache->pucTopLeftV;
    const UInt8 *pucTopModeY    = &pCache->pucTopModeY[(uiOffset + uiX) / MIN_CU_SIZE];
    const UInt8 *pucLeftModeY   =  pCache->pucLeftModeY + (uiY / MIN_CU_SIZE);
          UInt8 *pucRefY0       =  pCache->pucPixRef[0];
          UInt8 *pucRefY1       =  pCache->pucPixRef[1];
          UInt8 *pucRefU        =  pCache->pucPixRefC[0];
          UInt8 *pucRefV        =  pCache->pucPixRefC[1];
          UInt8 *pucMostModeY   =  pCache->ucMostModeY;
    const UInt   nSizeC         =  (nSize >> 1);

    /// T(op), B(ottom), L(eft), R(ight)
    const UInt   bT             = (pucTopModeY [uiX] != MODE_INVALID);
    const UInt   bL             = (pucLeftModeY[uiY] != MODE_INVALID);
    const UInt   bLT            = bT && bL;
    const UInt   bTR            = (pucTopModeY [(uiX + nSize) / MIN_CU_SIZE] != MODE_INVALID);
    const UInt   bLB            = (pucLeftModeY[(uiY + nSize) / MIN_CU_SIZE] != MODE_INVALID);
    const UInt8  bValid[5]      = {bLB, bL, bLT, bT, bTR};
    const UInt   nBlkOffsetY[6] = {0, nSize,  2*nSize,  2*nSize +1, 3*nSize +1, 4*nSize +1};
    const UInt   nBlkOffsetC[6] = {0, nSizeC, 2*nSizeC, 2*nSizeC+1, 3*nSizeC+1, 4*nSizeC+1};
    Int i, n;

    // TODO: I ASSUME( CU = PU = TU ) here, do more!
    assert( (uiX == 0) && (uiY == 0) && (nSize == h->ucMaxCUWidth) );

    // Save bValid flag for other functions
    memcpy( pCache->bValid, bValid, sizeof(bValid) );

    // Default to DC when all reference invalid
    if ( (bT | bL | bLT | bTR | bLB) == 0 ) {
        memset( pucRefY0, 0x80, nSize  * 4 + 1 );
        memset( pucRefY1, 0x80, nSize  * 4 + 1 );
        memset( pucRefU,  0x80, nSizeC * 4 + 1 );
        memset( pucRefV,  0x80, nSizeC * 4 + 1 );
    }
    else {
        // Copy the reconst pixel when valid
        if (bLB) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffsetY[0]] = pucLeftPixY[nSize * 2 - 1 - i];
            }
            for( i=0; i<nSizeC; i++ ) {
                pucRefU[i + nBlkOffsetC[0]] = pucLeftPixU[nSizeC * 2 - 1 - i];
                pucRefV[i + nBlkOffsetC[0]] = pucLeftPixV[nSizeC * 2 - 1 - i];
            }
        }
        if (bL) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffsetY[1]] = pucLeftPixY[nSize - 1 - i];
            }
            for( i=0; i<nSizeC; i++ ) {
                pucRefU[i + nBlkOffsetC[1]] = pucLeftPixU[nSizeC - 1 - i];
                pucRefV[i + nBlkOffsetC[1]] = pucLeftPixV[nSizeC - 1 - i];
            }
        }
        if (bLT) {
            UInt offsetY = ((uiX == 0 ? nSize  : uiX) / MIN_CU_SIZE) - 1;
            UInt offsetC = ((uiX == 0 ? nSizeC : uiX) / MIN_CU_SIZE) - 1;
            pucRefY0[nBlkOffsetY[2]] = pucTopLeftY[ offsetY ];
            pucRefU[nBlkOffsetC[2]] = pucTopLeftU[ offsetC ];
            pucRefV[nBlkOffsetC[2]] = pucTopLeftV[ offsetC ];
        }
        if (bT) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffsetY[3]] = pucTopPixY[i + 0 * nSize];
            }
            for( i=0; i<nSizeC; i++ ) {
                pucRefU[i + nBlkOffsetC[3]] = pucTopPixU[i + 0 * nSizeC];
                pucRefV[i + nBlkOffsetC[3]] = pucTopPixV[i + 0 * nSizeC];
            }
        }
        if (bTR) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffsetY[4]] = pucTopPixY[i + 1 * nSize];
            }
            for( i=0; i<nSizeC; i++ ) {
                pucRefU[i + nBlkOffsetC[4]] = pucTopPixU[i + 1 * nSizeC];
                pucRefV[i + nBlkOffsetC[4]] = pucTopPixV[i + 1 * nSizeC];
            }
        }

        xPaddingRef( pucRefY0, bValid, nBlkOffsetY );
        xPaddingRef( pucRefU,  bValid, nBlkOffsetC );
        xPaddingRef( pucRefV,  bValid, nBlkOffsetC );

        // Filter with [1 2 1]
        pucRefY1[0      ] = pucRefY0[0];
        pucRefY1[4*nSize] = pucRefY0[4*nSize];
        for( i=1; i<4*nSize; i++ ) {
            pucRefY1[i] = (pucRefY0[i - 1] + 2 * pucRefY0[i] + pucRefY0[i + 1] + 2) >> 2;
        }
    }

    // Most Mode
    UInt8 ucLeftMode = bL        ? pucLeftModeY[uiX] : DC_IDX;
    UInt8 ucTopMode  = bT && uiY ? pucTopModeY [uiX] : DC_IDX;

    if ( ucLeftMode == ucTopMode ) {
        if ( ucLeftMode > 1 ) {
            // angular modes
            pucMostModeY[0] = ucLeftMode;
            pucMostModeY[1] = ((ucLeftMode + 29) % 32) + 2;
            pucMostModeY[2] = ((ucLeftMode -  1) % 32) + 2;
        }
        else {
            // non angular modes
            pucMostModeY[0] = PLANAR_IDX;
            pucMostModeY[1] = DC_IDX;
            pucMostModeY[2] = VER_IDX; 
        }
    }
    else {
        pucMostModeY[0] = ucLeftMode;
        pucMostModeY[1] = ucTopMode;
        if ( ucLeftMode && ucTopMode )
            pucMostModeY[2] = PLANAR_IDX;
        else
            pucMostModeY[2] = ( ucLeftMode + ucTopMode ) < 2 ? VER_IDX : DC_IDX;
    }


#if (CHECK_TV)
    assert( nSize == tv_size );

    int bPassed = TRUE;
    for( n=0; n<2; n++ ) {
        // Check Left
        for( i=0; i<tv_size*2; i++ ) {
            if ( pucRefY0[i] != tv_left[0][tv_size*2-i-1] ) {
                bPassed = FALSE;
                fprintf( stderr, "Detect Intra Reference Left[%d] Wrong at %d, %02X -> %02X\n", n, i, tv_left[n][tv_size*2-i-1], pCache->pucPixRef[n][i] );
                break;
            }
        }
        // Check TopLeft, Top and TopRight
        for( i=0; i<tv_size*2+1; i++ ) {
            if ( pucRefY0[tv_size*2+i] != tv_top[0][i] ) {
                bPassed = FALSE;
                fprintf( stderr, "Detect Intra Reference  Top[%d] Wrong at %d, %02X -> %02X\n", n, i, tv_top[n][i], pCache->pucPixRef[n][tv_size*2+i] );
                break;
            }
        }
    }
    // Check MPM3
    for( i=0; i<3; i++ ) {
        if ( pucMostModeY[i] != tv_mostmode[i] ) {
            bPassed = FALSE;
            fprintf( stderr, "Detect Intra Most Mode[%d] Wrong %d -> %d\n", i, tv_mostmode[i], pCache->pucPixRef[n][i] );
            break;
        }
    }
    // Check U
    for( i=0; i<tv_size/2*4+1; i++ ) {
        if ( pucRefU[i] != tv_refC[0][i] ) {
            bPassed = FALSE;
            fprintf( stderr, "Detect Intra Reference Cr[%d] Wrong at %d, %02X -> %02X\n", n, i, tv_refC[0][i], pCache->pucPixRefC[0][i] );
            break;
        }
    }
    // Check V
    for( i=0; i<tv_size/2*4+1; i++ ) {
        if ( pucRefV[i] != tv_refC[1][i] ) {
            bPassed = FALSE;
            fprintf( stderr, "Detect Intra Reference Cb[%d] Wrong at %d, %02X -> %02X\n", n, i, tv_refC[1][i], pCache->pucPixRefC[1][i] );
            break;
        }
    }

    if ( bPassed ) {
        //fprintf( stderr, "Intra Reference Pixel Filter Passed!\n");
    }
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
    UInt8 bottomLeft, topRight;
    Int16 horPred;
    Int16 leftColumn[MAX_CU_SIZE+1], topRow[MAX_CU_SIZE+1], bottomRow[MAX_CU_SIZE+1], rightColumn[MAX_CU_SIZE+1];
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
    UInt     nSize,
    UInt     bLuma
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

    // DC Filtering ( 8.4.3.1.5 )
    if ( bLuma ) {
        pucDst[0] = ( pucTop[0] + pucLeft[0] + 2 * pucDst[0] + 2 ) >> 2;
        for( i=1; i<nSize; i++ ) {
            pucDst[i           ] = ( pucTop [ i] + 3 * pucDst[i           ] + 2 ) >> 2;
            pucDst[i*nDstStride] = ( pucLeft[-i] + 3 * pucDst[i*nDstStride] + 2 ) >> 2;
        }
    }
}

void xPredIntraAng(
    UInt8   *pucRef,
    UInt8   *pucDst,
    Int      nDstStride,
     Int     nSize,
    UInt     nMode,
    UInt     bLuma
)
{
    UInt   bModeHor          = (nMode < 18);
    Int    nIntraPredAngle  = g_aucIntraPredAngle[nMode];
    Int    nInvAngle        = g_aucInvAngle[nMode];
    UInt8 *pucLeft          = pucRef + 2 * nSize - 1;
    UInt8 *pucTop           = pucRef + 2 * nSize + 1;
    UInt8 *pucTopLeft       = pucRef + 2 * nSize;
    UInt8  ucRefBuf[2*MAX_CU_SIZE+1];
    UInt8 *pucRefMain       = ucRefBuf + ( (nIntraPredAngle < 0) ? MAX_CU_SIZE : 0 );
    Int    x, k;

    if ( nMode == 26 )
        printf("");

    // (8-47) and (8-50)
    for( x=0; x<nSize+1; x++ ) {
        pucRefMain[x] = bModeHor ? pucTopLeft[-x] : pucTopLeft[x];
    }

    if (nIntraPredAngle < 0) {
        Int iSum = 128;
        // (8-48) or (8-51)
        for( x=-1; x>(nSize*nIntraPredAngle)>>5; x-- ) {
            iSum += nInvAngle;
            // Fix my inv left buffer index
            Int nOffset = bModeHor ? (iSum >> 8) : -(iSum >> 8);
            pucRefMain[x] = pucTopLeft[ nOffset ];
        }
    }
    else {
        // (8-49) or (8-52)
        for( x=nSize+1; x<2*nSize+1; x++ ) {
            pucRefMain[x] = bModeHor ? pucTopLeft[-x] : pucTopLeft[x];
        }
    }

    // 8.4.3.1.6
    Int deltaPos=0;
    Int refMainIndex;
    
    for( k=0; k<nSize; k++ ) {
        deltaPos += nIntraPredAngle;
        Int iIdx  = deltaPos >> 5;  // (8-53)
        Int iFact = deltaPos & 31;  // (8-54)
        
        if (iFact)
        {
            // Do linear filtering
            for( x=0; x<nSize; x++ ) {
                refMainIndex           = x+iIdx+1;
                pucDst[k*nDstStride+x] = ( ((32-iFact)*pucRefMain[refMainIndex]+iFact*pucRefMain[refMainIndex+1]+16) >> 5 );
            }
        }
        else
        {
            // Just copy the integer samples
            for( x=0; x<nSize; x++) {
                pucDst[k*nDstStride+x] = pucRefMain[iIdx+1+x];
            }
        }
    }

    // Filter if this is IntraPredAngle zero mode
    // see 8.4.3.1.3 and 8.4.3.1.4
    if ( bLuma && (nIntraPredAngle == 0) )
    {
        Int offset = bModeHor ? 1 : -1;
        for( x=0; x<nSize; x++ ) {
            pucDst[x*nDstStride] = Clip ( pucDst[x*nDstStride] + (( pucTopLeft[(x+1)*offset] - pucTopLeft[0] ) >> 1) );
        }
    }

    // Matrix Transpose if this is the horizontal mode
    if ( bModeHor ) {
      UInt8 tmp;
      for (k=0;k<nSize-1;k++)
      {
        for (x=k+1;x<nSize;x++)
        {
          tmp                 = pucDst[k*nDstStride+x];
          pucDst[k*nDstStride+x] = pucDst[x*nDstStride+k];
          pucDst[x*nDstStride+k] = tmp;
        }
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
            nSize,
            TRUE
        );
    }
    else {
        xPredIntraAng(
            pucRefY,
            pucDstY,
            MAX_CU_SIZE,
            nSize,
            nMode,
            TRUE
        );
    }
}

void xEncIntraPredChroma( X265_t *h, UInt nMode, UInt nSize )
{
    X265_Cache  *pCache     = &h->cache;

    if ( nMode == PLANAR_IDX ) {
        xPredIntraPlanar(
            pCache->pucPixRefC[0],
            pCache->pucPredC[0],
            MAX_CU_SIZE / 2,
            nSize
        );
        xPredIntraPlanar(
            pCache->pucPixRefC[1],
            pCache->pucPredC[1],
            MAX_CU_SIZE / 2,
            nSize
        );
    }
    else if ( nMode == DC_IDX ) {
        xPredIntraDc(
            pCache->pucPixRefC[0],
            pCache->pucPredC[0],
            MAX_CU_SIZE / 2,
            nSize,
            FALSE
        );
        xPredIntraDc(
            pCache->pucPixRefC[1],
            pCache->pucPredC[1],
            MAX_CU_SIZE / 2,
            nSize,
            FALSE
        );
    }
    else {
        xPredIntraAng(
            pCache->pucPixRefC[0],
            pCache->pucPredC[0],
            MAX_CU_SIZE / 2,
            nSize,
            nMode,
            FALSE
            );
        xPredIntraAng(
            pCache->pucPixRefC[1],
            pCache->pucPredC[1],
            MAX_CU_SIZE / 2,
            nSize,
            nMode,
            FALSE
        );
    }
}


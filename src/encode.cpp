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
#ifdef CHECK_SEI
#include "md5.h"
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

    for( i=0; i < MAX_REF_NUM+1; i++ ) {
        UInt8 *ptr = (UInt8 *)MALLOC(uiYSize * 3 / 2);
        assert(ptr != NULL);
        h->refn[i].pucY = (UInt8 *)ptr;
        h->refn[i].pucU = (UInt8 *)ptr + uiYSize;
        h->refn[i].pucV = (UInt8 *)ptr + uiYSize * 5 / 4;
    }
    h->iPoc = -1;
    #if (CHECK_TV)
    if( tInitTv( "CHEN_TV.TXT" ) < 0)
        abort();
    #endif
}

void xEncFree( X265_t *h )
{
    int i;
    for( i=0; i < MAX_REF_NUM+1; i++ ) {
        assert( h->refn[i].pucY != NULL );
        FREE( h->refn[i].pucY );
    }
    memset( h->refn, 0, sizeof(h->refn) );
}

// ***************************************************************************
// * Internal Functions
// ***************************************************************************
UInt32 countNonZeroCoeffs( Int16 *psCoef, UInt nStride, UInt nSize )
{
    Int count = 0;
    UInt x, y;

    for( y=0; y<nSize; y++ ) {
        for( x=0; x<nSize; x++ ) {
            count += (psCoef[y*nStride+x] != 0);
        }
    }

    return count;
}

UInt getCoefScanIdx( UInt nWidth, UInt8 bIsIntra, UInt8 bIsLuma, UInt nLumaMode )
{
    UInt uiCTXIdx;
    UInt nScanIdx;
    UInt uiDirMode;
    
    if( !bIsIntra ) {
        return SCAN_ZIGZAG;
    }
    
    switch( nWidth ) {
    case  2: uiCTXIdx = 6; break;
    case  4: uiCTXIdx = 5; break;
    case  8: uiCTXIdx = 4; break;
    case 16: uiCTXIdx = 3; break;
    case 32: uiCTXIdx = 2; break;
    case 64: uiCTXIdx = 1; break;
    default: uiCTXIdx = 0; break;
    }
    
    if( bIsLuma ) {
        nScanIdx = SCAN_ZIGZAG;
        if( uiCTXIdx > 3 && uiCTXIdx < 6 ) {
            //if multiple scans supported for PU size
            nScanIdx = abs((Int) nLumaMode - VER_IDX) < 5 ? 1 : (abs((Int)nLumaMode - HOR_IDX) < 5 ? 2 : 0);
        }
    }
    else {
        nScanIdx = SCAN_ZIGZAG;
        if( uiCTXIdx > 4 && uiCTXIdx < 7 ) {
            //if multiple scans supported for PU size
            nScanIdx = abs((Int) nLumaMode - VER_IDX) < 5 ? 1 : (abs((Int)nLumaMode - HOR_IDX) < 5 ? 2 : 0);
        }
    }

    return nScanIdx;
}

Int getSigCtxInc(Int16 *psCoeff, Int posX, Int posY, Int blockType, Int nSize, UInt8 bIsLuma)
{
    const Int width  = nSize;
    const Int height = nSize;
    const UInt nStride = (MAX_CU_SIZE >> (bIsLuma ? 0 : 1));

    if( blockType == 2 ) {
        //LUMA map
        const Int ctxIndMap4x4Luma[15] = {
            0, 1, 4, 5,
            2, 3, 4, 5,
            6, 6, 8, 8,
            7, 7, 8
        };
        //CHROMA map
        const Int ctxIndMap4x4Chroma[15] = {
            0, 1, 2, 4,
            1, 1, 2, 4,
            3, 3, 5, 5,
            4, 4, 5
        };
        
        if( bIsLuma ) {
            return ctxIndMap4x4Luma[ 4 * posY + posX ];
        }
        else {
            return ctxIndMap4x4Chroma[ 4 * posY + posX ];
        }
    }
    else if( blockType == 3 ) {
        const Int map8x8[16] = {
            0,  1,  2,  3,
            4,  5,  6,  3,
            8,  6,  6,  7,
            9,  9,  7,  7
        };
        
        Int offset = bIsLuma ? 9 : 6;
        
        if( posX + posY == 0 ) {
            return offset + 10;
        }
        else {
            return offset + map8x8[4 * (posY >> 1) + (posX >> 1)];
        }
    }
    
    Int offset = bIsLuma ? 20 : 17;
    if( posX + posY == 0 ) {
        return offset;
    }
    Int thredHighFreq = 3*(MAX(width, height)>>4);
    if( (posX>>2) + (posY>>2) >= thredHighFreq ) {
        return bIsLuma ? 24 : 18;
    }
    
    const Int16 *pData = psCoeff + posY * nStride + posX;
    
    Int cnt = 0;
    if( posX < width - 1 ) {
        cnt += (pData[1] != 0);
        if( posY < height - 1 ) {
            cnt += (pData[nStride+1] != 0);
        }
        if( posX < width - 2 ) {
            cnt += (pData[2] != 0);
        }
    }
    if( posY < height - 1 ) {
        if( ( ( posX & 3 ) || ( posY & 3 ) ) && ( ( (posX+1) & 3 ) || ( (posY+2) & 3 ) ) ) {
            cnt += pData[nStride] != 0;
        }
        if( posY < height - 2 && cnt < 4 ) {
            cnt += pData[2*nStride] != 0;
        }
    }
    
    cnt = ( cnt + 1 ) >> 1;
    return (( bIsLuma && ((posX>>2) + (posY>>2)) > 0 ) ? 4 : 1) + offset + cnt;
}

UInt getSigCoeffGroupCtxInc(const UInt8 *uiSigCoeffGroupFlag, const UInt nCGPosX, const UInt nCGPosY, const UInt scanIdx, UInt nSize)
{
    UInt uiRight = 0;
    UInt uiLower = 0;
    UInt width = nSize;
    UInt height = nSize;
    
    width >>= 2;
    height >>= 2;
    // 8x8
    if( nSize == 8 ) {
        if( scanIdx == SCAN_HOR ) {
            width = 1;
            height = 4;
        }
        else if( scanIdx == SCAN_VER ) {
            width = 4;
            height = 1;
        }
    }
    if( nCGPosX < width - 1 ) {
        uiRight = (uiSigCoeffGroupFlag[ nCGPosY * width + nCGPosX + 1 ] != 0);
    }
    if( nCGPosY < height - 1 ) {
        uiLower = (uiSigCoeffGroupFlag[ (nCGPosY  + 1 ) * width + nCGPosX ] != 0);
    }
    return (uiRight || uiLower);
}

void codeLastSignificantXY( X265_Cabac *pCabac, X265_BitStream *pBS, UInt nPosX, UInt nPosY, UInt nSize, UInt8 bIsLuma, UInt nScanIdx )
{
    const UInt nLog2Size = xLog2( nSize - 1 );

    // swap
    if( nScanIdx == SCAN_VER ) {
        UInt tmp = nPosX;
        nPosY = nPosX;
        nPosX = tmp;
    }

    UInt nCtxLast;
    UInt nCtxX = OFF_LAST_X_CTX + (bIsLuma ? 0 : NUM_LAST_FLAG_XY_CTX);
    UInt nCtxY = OFF_LAST_Y_CTX + (bIsLuma ? 0 : NUM_LAST_FLAG_XY_CTX);
    UInt uiGroupIdxX    = g_uiGroupIdx[ nPosX ];
    UInt uiGroupIdxY    = g_uiGroupIdx[ nPosY ];
    
    // posX
    Int iLog2WidthCtx = bIsLuma ? nLog2Size-2 : 0;
    const UInt8 *puiCtxIdxX = g_uiLastCtx + ( iLog2WidthCtx * ( iLog2WidthCtx + 3 ) );
    for( nCtxLast = 0; nCtxLast < uiGroupIdxX; nCtxLast++ ) {
        xCabacEncodeBin( pCabac, pBS, 1, nCtxX + ( bIsLuma ? puiCtxIdxX[nCtxLast] : (nCtxLast>>(nLog2Size-2)) ) );
    }
    if( uiGroupIdxX < g_uiGroupIdx[nSize-1]) {
        xCabacEncodeBin( pCabac, pBS, 0, nCtxX + ( bIsLuma ? puiCtxIdxX[nCtxLast] : (nCtxLast>>(nLog2Size-2)) ) );
    }
    
    // posY
    Int iLog2HeightCtx = bIsLuma ? nLog2Size-2 : 0;
    const UInt8 *puiCtxIdxY = g_uiLastCtx + ( iLog2WidthCtx * ( iLog2WidthCtx + 3 ) );
    for( nCtxLast = 0; nCtxLast < uiGroupIdxY; nCtxLast++ ) {
        xCabacEncodeBin( pCabac, pBS, 1, nCtxY + ( bIsLuma ? puiCtxIdxY[nCtxLast] : (nCtxLast>>(nLog2Size-2)) ) );
    }
    if( uiGroupIdxY < g_uiGroupIdx[ nSize - 1 ]) {
        xCabacEncodeBin( pCabac, pBS, 0, nCtxY + ( bIsLuma ? puiCtxIdxY[nCtxLast] : (nCtxLast>>(nLog2Size-2)) ) );
    }

    if( uiGroupIdxX > 3 ) {      
        UInt nCount = ( uiGroupIdxX - 2 ) >> 1;
        nPosX       = nPosX - g_uiMinInGroup[ uiGroupIdxX ];
        xCabacEncodeBinsEP( pCabac, pBS, nPosX, nCount );
    }
    if( uiGroupIdxY > 3 ) {      
        UInt nCount = ( uiGroupIdxY - 2 ) >> 1;
        nPosY       = nPosY - g_uiMinInGroup[ uiGroupIdxY ];
        xCabacEncodeBinsEP( pCabac, pBS, nPosY, nCount );
    }
}

void xEncodeCoeffNxN( X265_Cabac *pCabac, X265_BitStream *pBS, Int16 *psCoef, UInt nSize, UInt nDepth, UInt8 bIsLuma, UInt nLumaMode )
{
    const UInt      nStride      = (MAX_CU_SIZE >> (bIsLuma ? 0 : 1));
          UInt      nLog2Size    = xLog2( nSize - 1 );
          UInt32    uiNumSig     = countNonZeroCoeffs( psCoef, nStride, nSize );
          UInt      nScanIdx     = getCoefScanIdx( nSize, TRUE, bIsLuma, nLumaMode );
    const UInt16   *scan         = NULL;
    const UInt16   *scanCG       = NULL;
    const UInt      uiShift      = MLS_CG_SIZE >> 1;
    const UInt      uiNumBlkSide = nSize >> uiShift;
    const Int       blockType    = nLog2Size;
    UInt8 uiSigCoeffGroupFlag[MLS_GRP_NUM];
    Int idx;

    // Map zigzag to diagonal scan
    if( nScanIdx == SCAN_ZIGZAG ) {
        nScanIdx = SCAN_DIAG;
    }

    // CHECK_ME: I think the size of 64x64 can't be here, but the HM say that 128x128 can be here?
    assert( nLog2Size <= 5 );
    scan   = g_ausScanIdx[ nScanIdx ][ nLog2Size - 1 ];
    scanCG = g_ausScanIdx[ nScanIdx ][ nLog2Size < 3 ? 0 : 1 ];
    if( nLog2Size == 3 ) {
      scanCG = g_sigLastScan8x8[ nScanIdx ];
    }
    else if( nLog2Size == 5 ) {
      scanCG = g_sigLastScanCG32x32;
    }

    memset( uiSigCoeffGroupFlag, 0, sizeof(uiSigCoeffGroupFlag) );

    // Find position of last coefficient
    Int scanPosLast = -1;
    Int iRealPos = -1;
    Int posLast;
    do {
        posLast = scan[ ++scanPosLast ];

        // get L1 sig map
        UInt nPosY    = posLast >> nLog2Size;
        UInt nPosX    = posLast - ( nPosY << nLog2Size );
        UInt nBlkIdx  = uiNumBlkSide * (nPosY >> uiShift) + (nPosX >> uiShift);

        iRealPos = nPosY * nStride + nPosX;

        if( nSize == 8 && (nScanIdx == SCAN_HOR || nScanIdx == SCAN_VER) ) {
            if( nScanIdx == SCAN_HOR ) {
                nBlkIdx = nPosY >> 1;
            }
            else if( nScanIdx == SCAN_VER ) {
                nBlkIdx = nPosX >> 1;
            }
        }

        if( psCoef[iRealPos] ) {
            uiSigCoeffGroupFlag[nBlkIdx] = 1;
        }

        uiNumSig -= ( psCoef[iRealPos] != 0 );
    } while( uiNumSig > 0 );

    // Code position of last coefficient
    UInt posLastY = posLast >> nLog2Size;
    UInt posLastX = posLast - ( posLastY << nLog2Size );
    codeLastSignificantXY( pCabac, pBS, posLastX, posLastY, nSize, bIsLuma, nScanIdx );

    //===== code significance flag =====
    UInt nBaseCoeffGroupCtx = OFF_SIG_CG_FLAG_CTX + (bIsLuma ? 0 : NUM_SIG_CG_FLAG_CTX);
    UInt nBaseCtx = OFF_SIG_FLAG_CTX + (bIsLuma ? 0 : NUM_SIG_FLAG_CTX_LUMA);
    
    const Int  iLastScanSet      = scanPosLast >> LOG2_SCAN_SET_SIZE;
    UInt uiNumOne                = 0;
    UInt uiGoRiceParam           = 0;
    Int  iScanPosSig             = scanPosLast;
    Int  iSubSet;
    
    for( iSubSet = iLastScanSet; iSubSet >= 0; iSubSet-- ) {
        Int numNonZero = 0;
        Int  iSubPos     = iSubSet << LOG2_SCAN_SET_SIZE;
        uiGoRiceParam    = 0;
        Int16 absCoeff[16];
        UInt32 coeffSigns = 0;
        
        if( iScanPosSig == scanPosLast ) {
            absCoeff[0] = abs( psCoef[iRealPos] );
            coeffSigns  = ( psCoef[iRealPos] < 0 );
            numNonZero  = 1;
            iScanPosSig--;
        }
        
        // encode significant_coeffgroup_flag
        Int iCGBlkPos = scanCG[ iSubSet ];
        Int iCGPosY   = iCGBlkPos / uiNumBlkSide;
        Int iCGPosX   = iCGBlkPos - (iCGPosY * uiNumBlkSide);
        if( nSize == 8 && (nScanIdx == SCAN_HOR || nScanIdx == SCAN_VER) ) {
            iCGPosY = (nScanIdx == SCAN_HOR ? iCGBlkPos : 0);
            iCGPosX = (nScanIdx == SCAN_VER ? iCGBlkPos : 0);
        }
        if( iSubSet == iLastScanSet || iSubSet == 0) {
            uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
        }
        else {
            UInt nSigCoeffGroup   = (uiSigCoeffGroupFlag[ iCGBlkPos ] != 0);
            UInt nCtxSig  = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, nScanIdx, nSize );
            xCabacEncodeBin( pCabac, pBS, nSigCoeffGroup, nBaseCoeffGroupCtx + nCtxSig );
        }
        
        // encode significant_coeff_flag
        if( uiSigCoeffGroupFlag[ iCGBlkPos ] ) {
            UInt nBlkPos, nPosY, nPosX, nSig, nCtxSig;
            UInt nRealBlkPos;
            for( ; iScanPosSig >= iSubPos; iScanPosSig-- ) {
                nBlkPos     = scan[ iScanPosSig ]; 
                nPosY       = nBlkPos >> nLog2Size;
                nPosX       = nBlkPos - ( nPosY << nLog2Size );
                nRealBlkPos = nPosY * nStride + nPosX;
                nSig        = (psCoef[ nRealBlkPos ] != 0);
                if( (iScanPosSig != iSubPos) || iSubSet == 0 || numNonZero ) {
                    nCtxSig  = getSigCtxInc( psCoef, nPosX, nPosY, blockType, nSize, bIsLuma );
                    xCabacEncodeBin( pCabac, pBS, nSig, nBaseCtx + nCtxSig );
                }
                if( nSig ) {
                    absCoeff[numNonZero] = abs( psCoef[nRealBlkPos] );
                    coeffSigns = (coeffSigns << 1) + ( psCoef[nRealBlkPos] < 0 );
                    numNonZero++;
                }
            }
        }
        else {
            iScanPosSig = iSubPos - 1;
        }
        
        if( numNonZero > 0 ) {
            UInt c1 = 1;
            UInt uiCtxSet = (iSubSet > 0 && bIsLuma) ? 2 : 0;
            
            if( uiNumOne > 0 ) {
                uiCtxSet++;
            }
            
            uiNumOne       >>= 1;
            UInt nBaseCtxMod = OFF_ONE_FLAG_CTX + 4 * uiCtxSet + ( bIsLuma ? 0 : NUM_ONE_FLAG_CTX_LUMA);
            
            Int numC1Flag = MIN(numNonZero, C1FLAG_NUMBER);
            Int firstC2FlagIdx = 16;
            for( idx = 0; idx < numC1Flag; idx++ ) {
                UInt uiSymbol = absCoeff[ idx ] > 1;
                xCabacEncodeBin( pCabac, pBS, uiSymbol, nBaseCtxMod + c1 );
                if( uiSymbol ) {
                    c1 = 0;
                    firstC2FlagIdx = MIN(firstC2FlagIdx, idx);
                }
                else if( c1 != 0 ) {
                    c1 = MIN(c1+1, 3);
                }
            }

            if( c1 == 0 ) {
                nBaseCtxMod = OFF_ABS_FLAG_CTX + uiCtxSet + (bIsLuma ? 0 : NUM_ABS_FLAG_CTX_LUMA);
                if( firstC2FlagIdx != 16 ) {
                    UInt symbol = absCoeff[ firstC2FlagIdx ] > 2;
                    xCabacEncodeBin( pCabac, pBS, symbol, nBaseCtxMod + 0 );
                }
            }

            xCabacEncodeBinsEP( pCabac, pBS, coeffSigns, numNonZero );

            Int iFirstCoeff2 = 1;    
            if( c1 == 0 || numNonZero > C1FLAG_NUMBER ) {
                for( idx = 0; idx < numNonZero; idx++ ) {
                    Int baseLevel = (idx < C1FLAG_NUMBER) ? (2 + iFirstCoeff2 ) : 1;
                    
                    if( absCoeff[ idx ] >= baseLevel ) {
                        xWriteGoRiceExGolomb( pCabac, pBS, absCoeff[ idx ] - baseLevel, uiGoRiceParam ); 
                    }
                    if( absCoeff[ idx ] >= 2 ) {
                        iFirstCoeff2 = 0;
                        uiNumOne++;
                    }
                }        
            }
        }
        else {
            uiNumOne >>= 1;
        }
    }
}

UInt getCtxQtCbf( UInt bLuma )
{
    // TODO: we use one layer only
    if( bLuma ) {
        return 1;
    }
    else {
        return 0;
    }
}

void xWriteCU( X265_t *h, UInt nDepth, UInt bLastCU )
{
    X265_BitStream *pBS         = &h->bs;
    X265_Cabac     *pCabac      = &h->cabac;
    X265_Cache     *pCache      = &h->cache;
    UInt8          *pucMostModeY= pCache->ucMostModeY;
    UInt8          *pCbf        = pCache->bCbf;
    UInt            nCUWidth    = h->ucMaxCUWidth;
    UInt            nModeY      = pCache->nBestModeY;
    UInt            nModeYBak   = nModeY;

    // Check Depth
    if( nDepth != (UInt)h->ucMaxCUDepth - 1 ) {
        // TODO: Split CU
        assert( 0 );
    }

    if( h->eSliceType == SLICE_I ) {
        // codePartSize
        // TODO: all are SIZE_2Nx2N now
        xCabacEncodeBin( pCabac, pBS, 1, OFF_PART_SIZE_CTX );

        // codeIntraDirLumaAng
        Int iPredIdx;
        if( nModeY == pCache->ucMostModeY[0] )
            iPredIdx = 0;
        else if( nModeY == pCache->ucMostModeY[1] )
            iPredIdx = 1;
        else if( nModeY == pCache->ucMostModeY[2] )
            iPredIdx = 2;
        else
            iPredIdx = -1;

        xCabacEncodeBin( pCabac, pBS, (iPredIdx != -1), OFF_INTRA_PRED_CTX );
        if( iPredIdx != -1 ) {
            // 0 -> 0
            // 1 -> 10
            // 2 -> 11
            if( iPredIdx == 0 )
                xCabacEncodeBinEP( pCabac, pBS, 0 );
            else
                xCabacEncodeBinsEP( pCabac, pBS, iPredIdx+1, 2 );
        }
        else {
            // Sort MostModeY
            UInt8 tmp;
#define SwapMode(a, b) \
            tmp = pCache->ucMostModeY[(a)]; \
            pCache->ucMostModeY[(a)] = pCache->ucMostModeY[(b)]; \
            pCache->ucMostModeY[(b)] = tmp

            if( pCache->ucMostModeY[0] > pCache->ucMostModeY[1] ) {
                SwapMode(0, 1);
            }
            if( pCache->ucMostModeY[0] > pCache->ucMostModeY[2] ) {
                SwapMode(0, 2);
            }
            if( pCache->ucMostModeY[1] > pCache->ucMostModeY[2] ) {
                SwapMode(1, 2);
            }
#undef SwapMode
            if(nModeY > pCache->ucMostModeY[2])
                nModeY--;
            if(nModeY > pCache->ucMostModeY[1])
                nModeY--;
            if(nModeY > pCache->ucMostModeY[0])
                nModeY--;
            xCabacEncodeBinsEP( pCabac, pBS, nModeY, 5 );
        }

        // codeIntraDirChroma
        UInt nModeC = pCache->nBestModeC;
        xCabacEncodeBin( pCabac, pBS, (nModeC != NUM_CHROMA_MODE - 1), OFF_CHROMA_PRED_CTX );
        if ( nModeC != NUM_CHROMA_MODE - 1 ) {
            if ( h->bUseLMChroma )
                xCabacEncodeBin( pCabac, pBS, (nModeC != NUM_CHROMA_MODE - 2), OFF_CHROMA_PRED_CTX+1 );
            // Non DM_CHROMA_IDX and LM_CHROMA_IDX
            if( nModeC < NUM_CHROMA_MODE - 2 ) {
                xCabacEncodeBinsEP( pCabac, pBS, nModeC, 2 );
            }
        }

        // TODO: codeTransformSubdivFlag
        // Currently, we are not use TU Split.
        assert( h->ucQuadtreeTUMaxDepthIntra == 1 );

        // TODO:
        // Currently, we always one layer and not use TU split
        const UInt8 bFirstChromaCbf = TRUE;

        // Cbf
        xCabacEncodeBin( pCabac, pBS, pCbf[1], OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + getCtxQtCbf(FALSE) );
        xCabacEncodeBin( pCabac, pBS, pCbf[2], OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + getCtxQtCbf(FALSE) );
        xCabacEncodeBin( pCabac, pBS, pCbf[0], OFF_QT_CBF_CTX + 0*NUM_QT_CBF_CTX + getCtxQtCbf(TRUE ) );

        // Coeff
        if(pCbf[0]) {
            xEncodeCoeffNxN( pCabac, pBS, pCache->psCoefY, nCUWidth,   nDepth, TRUE,  nModeYBak );
        }
        if(pCbf[1]) {
            xEncodeCoeffNxN( pCabac, pBS, pCache->psCoefU, nCUWidth/2, nDepth, FALSE, 0         );
        }
        if(pCbf[2]) {
            xEncodeCoeffNxN( pCabac, pBS, pCache->psCoefV, nCUWidth/2, nDepth, FALSE, 0         );
        }

        // FinishCU
        xCabacEncodeTerminatingBit( pCabac, pBS, bLastCU );
    }
    else {
        // TODO: No P-Slice now!
        assert(0);
    }
}

Int32 xEncEncode( X265_t *h, X265_Frame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize )
{
    const UInt32    uiWidth     = h->usWidth;
    const UInt32    uiHeight    = h->usHeight;
    const UInt32    nMaxCuWidth = h->ucMaxCUWidth;
    X265_Cabac     *pCabac      = &h->cabac;
    X265_BitStream *pBS         = &h->bs;
    X265_Cache     *pCache      = &h->cache;
          Int       nQP         = h->iQP;
          Int       nQPC        = g_aucChromaScale[nQP];
    const Int32     lambda      = nQP;
          UInt8    *pucMostModeY= pCache->ucMostModeY;
          UInt8    *pCbf        = pCache->bCbf;
          UInt8    *pucPixY     = pCache->pucPixY;
          UInt8    *pucRecY     = pCache->pucRecY;
          UInt8    *pucPredY    = pCache->pucPredY;
          UInt8    *pucPixC[2]  = { pCache->pucPixU, pCache->pucPixV };
          UInt8    *pucRecC[2]  = { pCache->pucRecU, pCache->pucRecV };
          UInt8    *pucPredC[2] = { pCache->pucPredC[0], pCache->pucPredC[1] };
          Int16    *piTmp0      = pCache->piTmp[0];
          Int16    *piTmp1      = pCache->piTmp[1];
          Int16    *piCoefY     = pCache->psCoefY;
          Int16    *piCoefC[2]  = { pCache->psCoefU, pCache->psCoefV };
          UInt8    *pucMostModeC= pCache->ucMostModeC;
          UInt      realModeC;
    UInt x, y;
    UInt i;
    UInt32 uiSumY, uiSumC[2];
    #ifdef CHECK_SEI
    UInt nOffsetSEI = 0;
    #endif

    /// Copy to local
    h->pFrameCur = pFrame;
    h->pFrameRec = &h->refn[MAX_REF_NUM];

    /// Initial local
    h->iPoc++;
    xBitStreamInit( pBS, pucOutBuf, uiBufSize );

    /// Write SPS Header
    xPutBits32(pBS, 0x01000000);
    xPutBits(pBS, 0x47, 8);
    xPutBits(pBS, 0x01, 8); // temporal_id and reserved_one_5bits
    xWriteSPS(h);
    xBitFlush(pBS);

    /// Write PPS Header
    xPutBits32(pBS, 0x01000000);
    xPutBits(pBS, 0x48, 8);
    xPutBits(pBS, 0x01, 8); // temporal_id and reserved_one_5bits
    xWritePPS(h);
    xBitFlush(pBS);

    #ifdef CHECK_SEI
    /// Write SEI Header
    xPutBits32(pBS, 0x06010000);
    xPutBits(pBS, 0x01,        8); // temporal_id and reserved_one_5bits
    xPutBits(pBS, 0xFF01,     16); // PICTURE_DIGEST
    xPutBits(pBS, 0x11,        8); // Payload length
    xPutBits(pBS, 0x00,        8); // Method = MD5
    nOffsetSEI = xBitFlush(pBS);
    xPutBits(pBS, 0xA0A1A2A3, 32);
    xPutBits(pBS, 0xB4B5B6B7, 32);
    xPutBits(pBS, 0xC8C9CACB, 32);
    xPutBits(pBS, 0xDCDDDEDF, 32);
    xWriteRBSPTrailingBits(pBS);
    xBitFlush(pBS);
    #endif

    /// Write Silces Header
    // FIX_ME: the HM-6.1 can't decoder my IDR only stream,
    //         they want CDR, so I do this chunk
    xPutBits32(pBS, (h->iPoc == 0 ? 0x45010000 : 0x41010000));
    xPutBits(pBS, 0x01, 8); // temporal_id and reserved_one_5bits
    xWriteSliceHeader(h);

    /// Encode loop
    xEncCahceInit( h );
    xCabacInit( h );
    xCabacReset( &h->cabac );
    for( y=0; y < uiHeight; y+=nMaxCuWidth ) {
        h->uiCUY = y;
        xEncCahceInitLine( h, y );
        for( x=0; x < uiWidth; x+=nMaxCuWidth ) {
            const UInt   bLastCU     = (y == uiHeight - nMaxCuWidth) && (x == uiWidth - nMaxCuWidth);
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
                UInt x, y;
                for( y=0; y<nCUSize; y++ ) {
                    for( x=0; x<nCUSize; x++ ) {
                        if( pCache->pucPixY[y * MAX_CU_SIZE + x] != tv_orig[y * MAX_CU_SIZE + x] ) {
                            fprintf( stderr, "Orig Pixel Y Wrong, (%d,%d), %02X -> %02X\n", y, x, tv_orig[y * MAX_CU_SIZE + x], pCache->pucPixY[y * MAX_CU_SIZE + x] );
                            abort();
                        }
                    }
                }
            }
            // Check U and V
            {
                UInt x, y;
                for( y=0; y<nCUSize/2; y++ ) {
                    for( x=0; x<nCUSize/2; x++ ) {
                        if( pCache->pucPixU[y * MAX_CU_SIZE/2 + x] != tv_origC[0][y * MAX_CU_SIZE/2 + x] ) {
                            fprintf( stderr, "Orig Pixel U Wrong, (%d,%d), %02X -> %02X\n", y, x, tv_origC[0][y * MAX_CU_SIZE/2 + x], pCache->pucPixY[y * MAX_CU_SIZE/2 + x] );
                            abort();
                        }
                        if( pCache->pucPixV[y * MAX_CU_SIZE/2 + x] != tv_origC[1][y * MAX_CU_SIZE/2 + x] ) {
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
                    UInt x, y;
                    for( y=0; y<nCUSize; y++ ) {
                        for( x=0; x<nCUSize; x++ ) {
                            if( pucPredY[y * MAX_CU_SIZE + x] != tv_pred[nMode][y * MAX_CU_SIZE + x] ) {
                                fprintf( stderr, "Intra Pred Y Wrong, Mode %d at (%d,%d), %02X -> %02X\n", nMode, y, x, tv_pred[nMode][y*nCUSize+x], pCache->pucPredY[y * MAX_CU_SIZE + x] );
                                abort();
                            }
                        }
                    }
                }
                #endif

                if( nMode == pucMostModeY[0] )
                    uiSad = 1 * lambda;
                else if( nMode == pucMostModeY[1] || nMode == pucMostModeY[2] )
                    uiSad = 2 * lambda;
                else
                    uiSad = 3 * lambda;
                uiSad += xSadN[nLog2CUSize-2](
                            nCUSize,
                            pucPixY, MAX_CU_SIZE,
                            pucPredY, MAX_CU_SIZE
                        );

                if( uiSad < uiBestSadY ) {
                    uiBestSadY = uiSad;
                    nBestModeY = nMode;
                }

                #if (CHECK_TV)
                if( uiSad != tv_sad[nMode] ) {
                    printf( " Sad %d -> %d Failed!\n", tv_sad[nMode], uiSad );
                    abort();
                }
                #endif
            }
            #if (CHECK_TV)
            if( nBestModeY != tv_bestmode ) {
                printf( " BestMode %d -> %d Failed!\n", tv_bestmode, nBestModeY );
                abort();
            }
            #endif

            // Stage 3a: Encode CU(Y)
            // Move to here because LMChroma need reconst Y samples
            xEncIntraPredLuma( h, nBestModeY, nCUSize );
            xSubDct( piTmp0,
                     pucPixY,
                     pucPredY, MAX_CU_SIZE,
                     piTmp0, piTmp1,
                     nCUSize, nCUSize, nBestModeY );
            uiSumY = xQuant( piCoefY, piTmp0, MAX_CU_SIZE, nQP, nCUSize, nCUSize, SLICE_I );

            // Stage 3b: Decode CU
            // Move to here because LMChroma need reconst Y samples
            if( uiSumY ) {
                xDeQuant( piTmp0, piCoefY, MAX_CU_SIZE, nQP, nCUSize, nCUSize, SLICE_I );
                xIDctAdd( pucRecY,
                          piTmp0,
                          pucPredY, MAX_CU_SIZE,
                          piTmp1, piTmp0,
                          nCUSize, nCUSize, nBestModeY );
            }
            else {
                for( i=0; i<nCUSize; i++ ) {
                    memcpy( &pucRecY[i*MAX_CU_SIZE], &pucPredY[i*MAX_CU_SIZE], nCUSize );
                }
            }

            #if (CHECK_TV)
            // Check LMChroma Reference
            {
                UInt x;
                for( x=0; x<nCUSize/2; x++ ) {
                    if( pCache->pucLeftPixM[x] != tv_refLM[0 * tv_sizeC + x] ) {
                        fprintf( stderr, "IntraPred_LM Wrong, Left[%d], %02X -> %02X\n", x, tv_refLM[0 * tv_sizeC + x], pCache->pucLeftPixM[x] );
                        abort();
                    }
                }
                for( x=0; x<nCUSize/2; x++ ) {
                    if( pCache->pucTopPixM[pCache->uiOffset/2+x] != tv_refLM[1 * tv_sizeC + x] ) {
                        fprintf( stderr, "IntraPred_LM Wrong, Top[%d], %02X -> %02X\n", x, tv_refLM[1 * tv_sizeC + x], pCache->pucTopPixM[pCache->uiOffset/2+x] );
                        abort();
                    }
                }
            }
            #endif

            // GetAllowedChromaMode
            pucMostModeC[0] = PLANAR_IDX;
            pucMostModeC[1] = VER_IDX;
            pucMostModeC[2] = HOR_IDX;
            pucMostModeC[3] = DC_IDX;
            pucMostModeC[4] = LM_CHROMA_IDX;
            pucMostModeC[5] = nBestModeY;
            for( i=0;i<4; i++ ) {
                if( pucMostModeC[i] == nBestModeY ) {
                    pucMostModeC[i] = 34;
                    break;
                }
            }

            uiBestSadC = MAX_SAD;
            for( nMode=0; nMode<NUM_CHROMA_MODE; nMode++ ) {
                UInt32 uiSumSad;
                UInt32 uiSad[2];
                realModeC = pucMostModeC[nMode];

                if ( !h->bUseLMChroma && (realModeC == LM_CHROMA_IDX) )
                    continue;

                #if (CHECK_TV)
                memset( pCache->pucPredC, 0xCD, sizeof(pCache->pucPredC) );
                #endif

                xEncIntraPredChroma( h, realModeC, nCUSize >> 1 );

                #if (CHECK_TV)
                {
                    UInt x, y;
                    for( y=0; y<nCUSize/2; y++ ) {
                        for( x=0; x<nCUSize/2; x++ ) {
                            if( pucPredC[0][y * MAX_CU_SIZE/2 + x] != tv_predC[0][nMode][y * MAX_CU_SIZE/2 + x] ) {
                                fprintf( stderr, "Intra Pred U Wrong, Mode %d at (%d,%d), %02X -> %02X\n", nMode, y, x, tv_predC[0][nMode][y*nCUSize+x], pCache->pucPredC[0][y * MAX_CU_SIZE/2 + x] );
                                //abort();
                                goto _exit;
                            }
                            if( pucPredC[1][y * MAX_CU_SIZE/2 + x] != tv_predC[1][nMode][y * MAX_CU_SIZE/2 + x] ) {
                                fprintf( stderr, "Intra Pred V Wrong, Mode %d at (%d,%d), %02X -> %02X\n", nMode, y, x, tv_predC[1][nMode][y*nCUSize+x], pCache->pucPredC[1][y * MAX_CU_SIZE/2 + x] );
                                //abort();
                                goto _exit;
                            }
                        }
                    }
_exit:;
                }
                #endif
                uiSad[0] = xSadN[nLog2CUSize-2-1](
                            nCUSize / 2,
                            pucPixC[0], MAX_CU_SIZE/2,
                            pucPredC[0], MAX_CU_SIZE/2
                        );

                uiSad[1] = xSadN[nLog2CUSize-2-1](
                            nCUSize / 2,
                            pucPixC[1], MAX_CU_SIZE/2,
                            pucPredC[1], MAX_CU_SIZE/2
                        );

                #if (CHECK_TV)
                assert( uiSad[0] == tv_sadC[0][nMode] );
                assert( uiSad[1] == tv_sadC[1][nMode] );
                #endif

                uiSumSad = uiSad[0] + uiSad[1];
                if( uiSumSad < uiBestSadC ) {
                    uiBestSadC = uiSumSad;
                    nBestModeC = nMode;
                }
            }
            realModeC = pucMostModeC[nBestModeC];
            #if (CHECK_TV)
            {
                // Check Chroma Mode and Sad
                assert( uiBestSadC == tv_BestSadC );
                assert( (nBestModeC == NUM_CHROMA_MODE-1 && realModeC == tv_bestmode) || (pucMostModeC[nBestModeC] == tv_bestmodeC) );
                tv_nModeC = nBestModeC;
            }
            #endif

            // Stage 3a: Encode CU
            pCache->nBestModeY = nBestModeY;
            pCache->nBestModeC = nBestModeC;
            // Move to front because LMChroma need reconst Y samples
            /*
            // Y
            xEncIntraPredLuma( h, nBestModeY, nCUSize );
            xSubDct( piTmp0,
                     pucPixY,
                     pucPredY, MAX_CU_SIZE,
                     piTmp0, piTmp1,
                     nCUSize, nCUSize, nBestModeY );
            uiSumY = xQuant( piCoefY, piTmp0, MAX_CU_SIZE, nQP, nCUSize, nCUSize, SLICE_I );
            */

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
                uiSumC[i] = xQuant( piCoefC[i], piTmp0, MAX_CU_SIZE/2, nQPC, nCUSize/2, nCUSize/2, SLICE_I );
            }
            pCbf[0] = (uiSumY    != 0);
            pCbf[1] = (uiSumC[0] != 0);
            pCbf[2] = (uiSumC[1] != 0);

            // Stage 3b: Decode CU
            // Move to front because LMChroma need reconst Y samples
            /*
            if( uiSumY ) {
                xDeQuant( piTmp0, piCoefY, MAX_CU_SIZE, nQP, nCUSize, nCUSize, SLICE_I );
                xIDctAdd( pucRecY,
                          piTmp0,
                          pucPredY, MAX_CU_SIZE,
                          piTmp1, piTmp0,
                          nCUSize, nCUSize, nBestModeY );
            }
            else {
                for( i=0; i<nCUSize; i++ ) {
                    memcpy( &pucRecY[i*MAX_CU_SIZE], &pucPredY[i*MAX_CU_SIZE], nCUSize );
                }
            }
            */
            // Cr and Cb
            for( i=0; i<2; i++ ) {
                #if (CHECK_TV)
                tv_nIdxC = i;
                #endif
                if( uiSumC[i] ) {
                    xDeQuant( piTmp0, piCoefC[i], MAX_CU_SIZE/2, nQPC, nCUSize/2, nCUSize/2, SLICE_I );
                    xIDctAdd( pucRecC[i],
                              piTmp0,
                              pucPredC[i], MAX_CU_SIZE/2,
                              piTmp1, piTmp0,
                              nCUSize/2, nCUSize/2, realModeC );
                }
                else {
                    UInt k;
                    for( k=0; k<nCUSize/2; k++ ) {
                        memcpy( &pucRecC[i][k*MAX_CU_SIZE/2], &pucPredC[i][k*MAX_CU_SIZE/2], nCUSize/2 );
                    }
                }
            }

            // Stage 4: Write CU
            xWriteCU( h, 0, bLastCU );

            // Stage 5a: Update reconstr
            xEncCacheStoreCU( h, x, y );

            // Stage 5b: Update context
            xEncCacheUpdate( h, 0, 0, nCUSize, nCUSize );
            pCache->uiOffset += nCUSize;

            #if (CHECK_TV)
            printf( "CU(%2d,%2d) Passed!\n", y/h->ucMaxCUWidth, x/h->ucMaxCUWidth );
            #endif
        }
    }
    xCabacFlush( pCabac, pBS );
    xWriteSliceEnd( h );

    #ifdef CHECK_SEI
    MD5Context ctx;
    MD5Init( &ctx );
    MD5Update( &ctx, h->pFrameRec->pucY, uiWidth*uiHeight*3/2 );
    MD5Final( &ctx, &pucOutBuf[nOffsetSEI] );
    #endif

    #ifdef WRITE_REC
    // Save Restruct
    {
        static FILE *fpx=NULL;
        if ( fpx == NULL )
            fpx = fopen("OX.YUV", "wb");
        assert( fpx != NULL );
        fwrite(h->pFrameRec->pucY, 1, uiWidth*uiHeight*3/2, fpx);
        fflush(fpx);
        //fclose(fpx);
    }
    #endif

    // Update Reference Frame Pointer
    X265_Frame tmp = h->refn[MAX_REF_NUM];
    for( i=MAX_REF_NUM; i>0; i-- ) {
        h->refn[i] = h->refn[i-1];
    }
    h->refn[0] = tmp;

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
    memset( pCache->pucTopPixM, 0x80, sizeof(pCache->pucTopPixM) );
}

void xEncCahceInitLine( X265_t *h, UInt y )
{
    X265_Cache *pCache  = &h->cache;
    pCache->uiOffset    = 0;
    memset( pCache->pucLeftModeY, MODE_INVALID, sizeof(pCache->pucLeftModeY) );
    memset( pCache->pucLeftPixM, (y == 0 ? 0x80 : pCache->pucTopPixY[0]), sizeof(pCache->pucLeftPixM) );
}

void xEncCacheLoadCU( X265_t *h, UInt uiX, UInt uiY )
{
    X265_Cache  *pCache     = &h->cache;
    X265_Frame  *pFrame     = h->pFrameCur;
    const UInt   nCUWidth   = h->ucMaxCUWidth;
    const UInt32 uiWidth    = h->usWidth;
    const UInt32 uiOffsetY  = uiWidth * uiY + uiX;
    const UInt32 uiOffsetC  = uiWidth * uiY / 4 + uiX / 2;
    UInt8 *pucSY0 = pFrame->pucY + uiOffsetY + 0*uiWidth;
    UInt8 *pucSY1 = pFrame->pucY + uiOffsetY + 1*uiWidth;
    UInt8 *pucSU  = pFrame->pucU + uiOffsetC;
    UInt8 *pucSV  = pFrame->pucV + uiOffsetC;
    UInt8 *pucDY0 = pCache->pucPixY + 0*MAX_CU_SIZE;
    UInt8 *pucDY1 = pCache->pucPixY + 1*MAX_CU_SIZE;
    UInt8 *pucDU  = pCache->pucPixU;
    UInt8 *pucDV  = pCache->pucPixV;
    Int y;

    for( y=0; y < h->ucMaxCUWidth/2; y++ ) {
        memcpy( pucDY0, pucSY0, nCUWidth     );
        memcpy( pucDY1, pucSY1, nCUWidth     );
        memcpy( pucDU,  pucSU,  nCUWidth / 2 );
        memcpy( pucDV,  pucSV,  nCUWidth / 2 );
        pucSY0 += uiWidth * 2;
        pucSY1 += uiWidth * 2;
        pucSU  += uiWidth / 2;
        pucSV  += uiWidth / 2;
        pucDY0 += MAX_CU_SIZE * 2;
        pucDY1 += MAX_CU_SIZE * 2;
        pucDU  += MAX_CU_SIZE / 2;
        pucDV  += MAX_CU_SIZE / 2;
    }
}

void xEncCacheStoreCU( X265_t *h, UInt uiX, UInt uiY )
{
    X265_Cache  *pCache     = &h->cache;
    X265_Frame  *pFrame     = h->pFrameRec;
    const UInt   nCUWidth   = h->ucMaxCUWidth;
    const UInt32 uiWidth    = h->usWidth;
    const UInt32 uiOffsetY  = uiWidth * uiY + uiX;
    const UInt32 uiOffsetC  = uiWidth * uiY / 4 + uiX / 2;
    UInt8 *pucSY0 = pCache->pucRecY + 0*MAX_CU_SIZE;
    UInt8 *pucSY1 = pCache->pucRecY + 1*MAX_CU_SIZE;
    UInt8 *pucSU  = pCache->pucRecU;
    UInt8 *pucSV  = pCache->pucRecV;
    UInt8 *pucDY0 = pFrame->pucY + uiOffsetY + 0*uiWidth;
    UInt8 *pucDY1 = pFrame->pucY + uiOffsetY + 1*uiWidth;
    UInt8 *pucDU  = pFrame->pucU + uiOffsetC;
    UInt8 *pucDV  = pFrame->pucV + uiOffsetC;
    Int y;

    for( y=0; y < h->ucMaxCUWidth/2; y++ ) {
        memcpy( pucDY0, pucSY0, nCUWidth     );
        memcpy( pucDY1, pucSY1, nCUWidth     );
        memcpy( pucDU,  pucSU,  nCUWidth / 2 );
        memcpy( pucDV,  pucSV,  nCUWidth / 2 );
        pucSY0 += MAX_CU_SIZE * 2;
        pucSY1 += MAX_CU_SIZE * 2;
        pucSU  += MAX_CU_SIZE / 2;
        pucSV  += MAX_CU_SIZE / 2;
        pucDY0 += uiWidth*2;
        pucDY1 += uiWidth*2;
        pucDU  += uiWidth / 2;
        pucDV  += uiWidth / 2;
    }
}

void xEncCacheUpdate( X265_t *h, UInt32 uiX, UInt32 uiY, UInt nWidth, UInt nHeight )
{
    X265_Cache  *pCache         = &h->cache;
    const UInt32 uiOffset       =  pCache->uiOffset;
          UInt8 *pucTopPixY     = &pCache->pucTopPixY[(uiOffset + uiX)];
          UInt8 *pucTopPixU     = &pCache->pucTopPixU[(uiOffset + uiX)/2];
          UInt8 *pucTopPixV     = &pCache->pucTopPixV[(uiOffset + uiX)/2];
          UInt8 *pucTopPixM     = &pCache->pucTopPixM[(uiOffset + uiX)/2];
          UInt8 *pucLeftPixY    =  pCache->pucLeftPixY + uiY;
          UInt8 *pucLeftPixU    =  pCache->pucLeftPixU + uiY / 2;
          UInt8 *pucLeftPixV    =  pCache->pucLeftPixV + uiY / 2;
          UInt8 *pucLeftPixM    =  pCache->pucLeftPixM + uiY / 2;
          UInt8 *pucTopLeftY    =  pCache->pucTopLeftY;
          UInt8 *pucTopLeftU    =  pCache->pucTopLeftU;
          UInt8 *pucTopLeftV    =  pCache->pucTopLeftV;
          UInt8 *pucTopModeY    = &pCache->pucTopModeY[(uiOffset + uiX) / MIN_CU_SIZE];
          UInt8 *pucLeftModeY   =  pCache->pucLeftModeY + (uiY / MIN_CU_SIZE);
    const UInt8 *pucRecY        =  pCache->pucRecY;
    const UInt8 *pucRecLMLeft   =  pCache->pucRecY + nWidth - 2;
    const UInt8 *pucRecU        =  pCache->pucRecU;
    const UInt8 *pucRecV        =  pCache->pucRecV;
    const UInt8 nBestModeY      =  pCache->nBestModeY;
    const UInt8 bT              =  pCache->bValid[VALID_T];
    const UInt8 bL              =  pCache->bValid[VALID_L];

    UInt x, y;

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

    // Calculate LMChroma Reference Left
    for( y=0; y<nHeight/2; y++ ) {
        UInt32 L0 = pucRecLMLeft[ (y*2+0)*MAX_CU_SIZE ];
        UInt32 L1 = pucRecLMLeft[ (y*2+1)*MAX_CU_SIZE ];
        pucLeftPixM[y] = (UInt8)( (L0 + L1) >> 1 );
    }
    // Calculate LMChroma Reference Top
    for( x=0; x<nWidth/2; x++ ) {
        UInt32 T0 = pucTopPixY[ (x*2-1) ];
        UInt32 T1 = pucTopPixY[ (x*2  ) ];
        UInt32 T2 = pucTopPixY[ (x*2+1) ];
        if ( x == 0 && !bL )
            T0 = T1;
        pucTopPixM[x] = (UInt8)( (T0 + 2*T1 + T2 + 2) >> 2 );
    }
    // Extend invalid Top pixel by Left for next CU
    if ( !bT ) {
        memset( pucTopPixM+nWidth/2, pucRecLMLeft[0], nWidth/2*sizeof(pucTopPixM[0]) );
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

    if( nOffsetX == 0 )
        nIdx += MAX_PU_XY;

    return nIdx - 1;
}

void xPaddingRef( UInt8 *pucRef, const UInt8 bValid[5], const UInt nBlkOffset[6] )
{
    UInt    i, n;
    UInt8   ucPadding;
    const Int nValid = 5;

    // Padding from Right to Left
    for( n=0; n<nValid; n++ ) {
        if( bValid[n] )
            break;
    }
    ucPadding = pucRef[nBlkOffset[n]];
    for( i=0; i<nBlkOffset[n]; i++ ) {
        pucRef[i] = ucPadding;
    }

    // Padding from Left to Right
    for( ; n<nValid; n++ ) {
        if( !bValid[n] ) {
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
    UInt i, n;

    // TODO: I ASSUME( CU = PU = TU ) here, do more!
    assert( (uiX == 0) && (uiY == 0) && (nSize == h->ucMaxCUWidth) );

    // Save bValid flag for other functions
    memcpy( pCache->bValid, bValid, sizeof(bValid) );

    // Default to DC when all reference invalid
    if( (bT | bL | bLT | bTR | bLB) == 0 ) {
        memset( pucRefY0, 0x80, nSize  * 4 + 1 );
        memset( pucRefY1, 0x80, nSize  * 4 + 1 );
        memset( pucRefU,  0x80, nSizeC * 4 + 1 );
        memset( pucRefV,  0x80, nSizeC * 4 + 1 );
    }
    else {
        // Copy the reconst pixel when valid
        if( bLB ) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffsetY[0]] = pucLeftPixY[nSize * 2 - 1 - i];
            }
            for( i=0; i<nSizeC; i++ ) {
                pucRefU[i + nBlkOffsetC[0]] = pucLeftPixU[nSizeC * 2 - 1 - i];
                pucRefV[i + nBlkOffsetC[0]] = pucLeftPixV[nSizeC * 2 - 1 - i];
            }
        }
        if( bL ) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffsetY[1]] = pucLeftPixY[nSize - 1 - i];
            }
            for( i=0; i<nSizeC; i++ ) {
                pucRefU[i + nBlkOffsetC[1]] = pucLeftPixU[nSizeC - 1 - i];
                pucRefV[i + nBlkOffsetC[1]] = pucLeftPixV[nSizeC - 1 - i];
            }
        }
        if( bLT ) {
            UInt offsetY = ((uiX == 0 ? nSize  : uiX) / MIN_CU_SIZE) - 1;
            UInt offsetC = ((uiX == 0 ? nSizeC : uiX) / MIN_CU_SIZE) - 1;
            pucRefY0[nBlkOffsetY[2]] = pucTopLeftY[ offsetY ];
            pucRefU[nBlkOffsetC[2]] = pucTopLeftU[ offsetC ];
            pucRefV[nBlkOffsetC[2]] = pucTopLeftV[ offsetC ];
        }
        if( bT ) {
            for( i=0; i<nSize; i++ ) {
                pucRefY0[i + nBlkOffsetY[3]] = pucTopPixY[i + 0 * nSize];
            }
            for( i=0; i<nSizeC; i++ ) {
                pucRefU[i + nBlkOffsetC[3]] = pucTopPixU[i + 0 * nSizeC];
                pucRefV[i + nBlkOffsetC[3]] = pucTopPixV[i + 0 * nSizeC];
            }
        }
        if( bTR ) {
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

    if( ucLeftMode == ucTopMode ) {
        if( ucLeftMode > 1 ) {
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
        if( ucLeftMode && ucTopMode )
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
            if( pucRefY0[i] != tv_left[0][tv_size*2-i-1] ) {
                bPassed = FALSE;
                fprintf( stderr, "Detect Intra Reference Left[%d] Wrong at %d, %02X -> %02X\n", n, i, tv_left[n][tv_size*2-i-1], pCache->pucPixRef[n][i] );
                break;
            }
        }
        // Check TopLeft, Top and TopRight
        for( i=0; i<tv_size*2+1; i++ ) {
            if( pucRefY0[tv_size*2+i] != tv_top[0][i] ) {
                bPassed = FALSE;
                fprintf( stderr, "Detect Intra Reference  Top[%d] Wrong at %d, %02X -> %02X\n", n, i, tv_top[n][i], pCache->pucPixRef[n][tv_size*2+i] );
                break;
            }
        }
    }
    // Check MPM3
    for( i=0; i<3; i++ ) {
        if( pucMostModeY[i] != tv_mostmode[i] ) {
            bPassed = FALSE;
            fprintf( stderr, "Detect Intra Most Mode[%d] Wrong %d -> %d\n", i, tv_mostmode[i], pCache->pucPixRef[n][i] );
            break;
        }
    }
    // Check U
    for( i=0; i<tv_size/2*4+1; i++ ) {
        if( pucRefU[i] != tv_refC[0][i] ) {
            bPassed = FALSE;
            fprintf( stderr, "Detect Intra Reference Cr[%d] Wrong at %d, %02X -> %02X\n", n, i, tv_refC[0][i], pCache->pucPixRefC[0][i] );
            break;
        }
    }
    // Check V
    for( i=0; i<tv_size/2*4+1; i++ ) {
        if( pucRefV[i] != tv_refC[1][i] ) {
            bPassed = FALSE;
            fprintf( stderr, "Detect Intra Reference Cb[%d] Wrong at %d, %02X -> %02X\n", n, i, tv_refC[1][i], pCache->pucPixRefC[1][i] );
            break;
        }
    }

    if( bPassed ) {
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
    for( i=0; i<(Int)nSize+1; i++) {
        topRow[i]     = pucTop[i];
        leftColumn[i] = pucLeft[-i];
    }

    // Prepare intermediate variables used in interpolation
    bottomLeft = pucLeft[-(Int)nSize];
    topRight   = pucTop[nSize];
    for( i=0; i<(Int)nSize; i++ ) {
        bottomRow[i]   = bottomLeft - topRow[i];
        rightColumn[i] = topRight   - leftColumn[i];
        topRow[i]      <<= shift1D;
        leftColumn[i]  <<= shift1D;
    }

    // Generate prediction signal
    for( i=0; i<(Int)nSize; i++ ) {
        horPred = leftColumn[i] + offset2D;
        for( j=0; j<(Int)nSize; j++ ) {
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

    for( i=0; i<(Int)nSize; i++ ) {
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
    for( i=0; i<(Int)nSize; i++ ) {
        memset( &pucDst[i * nDstStride], ucDcVal, nSize );
    }

    // DC Filtering ( 8.4.3.1.5 )
    if( bLuma ) {
        pucDst[0] = ( pucTop[0] + pucLeft[0] + 2 * pucDst[0] + 2 ) >> 2;
        for( i=1; i<(Int)nSize; i++ ) {
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

    // (8-47) and (8-50)
    for( x=0; x<nSize+1; x++ ) {
        pucRefMain[x] = bModeHor ? pucTopLeft[-x] : pucTopLeft[x];
    }

    if( nIntraPredAngle < 0 ) {
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
        
        if( iFact ) {
            // Do linear filtering
            for( x=0; x<nSize; x++ ) {
                refMainIndex           = x+iIdx+1;
                pucDst[k*nDstStride+x] = ( ((32-iFact)*pucRefMain[refMainIndex]+iFact*pucRefMain[refMainIndex+1]+16) >> 5 );
            }
        }
        else {
            // Just copy the integer samples
            for( x=0; x<nSize; x++) {
                pucDst[k*nDstStride+x] = pucRefMain[iIdx+1+x];
            }
        }
    }

    // Filter if this is IntraPredAngle zero mode
    // see 8.4.3.1.3 and 8.4.3.1.4
    if( bLuma && (nIntraPredAngle == 0) )
    {
        Int offset = bModeHor ? 1 : -1;
        for( x=0; x<nSize; x++ ) {
            pucDst[x*nDstStride] = Clip ( pucDst[x*nDstStride] + (( pucTopLeft[(x+1)*offset] - pucTopLeft[0] ) >> 1) );
        }
    }

    // Matrix Transpose if this is the horizontal mode
    if( bModeHor ) {
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

void xPredIntraLM(
    UInt8   *pucRefC,
    UInt8   *pucRefM_L,
    UInt8   *pucRefM_T,
    UInt8   *pucPredLM,
    UInt8   *pucDst,
    UInt     nSize
)
{
    // Table 8-7 ¨C Specification of lmDiv
    static const UInt16 lmDiv[64] = {
        32768, 16384, 10923, 8192, 6554, 5461, 4681, 4096,
         3641,  3277,  2979, 2731, 2521, 2341, 2185, 2048,
         1928,  1820,  1725, 1638, 1560, 1489, 1425, 1365,
         1311,  1260,  1214, 1170, 1130, 1092, 1057, 1024,
          993,   964,   936,  910,  886,  862,  840,  819,
          799,   780,   762,  745,  728,  712,  697,  683,
          669,   655,   643,  630,  618,  607,  596,  585,
          575,   565,   555,  546,  537,  529,  520,  512,

    };
    const UInt nLog2Size = xLog2( nSize - 1 );
    UInt8 *pucRefC_L = pucRefC + 2 * nSize - 1;
    UInt8 *pucRefC_T = pucRefC + 2 * nSize + 1;
    Int i;
    Int32 L=0, C=0, LL=0, LC=0;

    // (8-58) k3 = MAX(0, 8 + xLog2( nSize - 1 ) - 14) = 0;
    UInt k2 = xLog2(2*nSize - 1);   // (8-66)

    for( i=0; i<(Int)nSize; i++ ) {
        UInt32 L0 = ( pucRefM_L[ i] + pucRefM_T[i] );
        UInt32 C0 = ( pucRefC_L[-i] + pucRefC_T[i] );
        L  += L0;   // (8-62)
        C  += C0;   // (8-63)
        LL += (pucRefM_L[i] * pucRefM_L[ i]) + (pucRefM_T[i] * pucRefM_T[i]);    // (8-64)
        LC += (pucRefM_L[i] * pucRefC_L[-i]) + (pucRefM_T[i] * pucRefC_T[i]);    // (8-65)
    }

    Int a1 = ( LC << k2 ) - L * C;
    Int a2 = ( LL << k2 ) - L * L;

    // CHECK_ME: This algorithm have many error on Document, so I use HM's.
    Int32 a1s = a1;
    Int32 a2s = a2;
    Int32 a, b;
    Int iScaleShiftA2 = 0;
    Int iScaleShiftA1 = 0;
    UInt nShift = 13;

    iScaleShiftA1 = MAX(0, xLog2( abs( a1 ) ) - 15);    // (8-70)?
    iScaleShiftA2 = MAX(0, xLog2( abs( a2 ) ) -  6);    // (8-71)?

    Int iScaleShiftA = iScaleShiftA2 + 15 - nShift - iScaleShiftA1;

    a2s = a2 >> iScaleShiftA2;
    a1s = a1 >> iScaleShiftA1;

    a = (a2s < 1) ? 0 : a1s * lmDiv[a2s - 1];

    assert( iScaleShiftA >= 0 );
    a = Clip3( -32768, 32767, (a >> iScaleShiftA) );
    
    Int minA = -(1 << 6);
    Int maxA =  (1 << 6) - 1;
    if( a < minA || a > maxA ) {
        UInt n = 15 - xLog2( (a >= 0 ? a : ~a) );
        a >>= (9-n);
        nShift -= (9-n);
    }
    b = (  C - ( ( a * L ) >> nShift ) + ( 1 << ( k2 - 1 ) ) ) >> k2;
    
    UInt x, y;
    for( y=0; y<nSize; y++ ) {
        for( x=0; x<nSize; x++ ) {
            Int32 T = ((pucPredLM[y*MAX_CU_SIZE/2+x] * a) >> nShift) + b;
            pucDst[y*MAX_CU_SIZE/2+x] = Clip(T);
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

    if( nMode == PLANAR_IDX ) {
        xPredIntraPlanar(
            pucRefY,
            pucDstY,
            MAX_CU_SIZE,
            nSize
        );
    }
    else if( nMode == DC_IDX ) {
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

    if( nMode == PLANAR_IDX ) {
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
    else if( nMode == DC_IDX ) {
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
    else if ( nMode == LM_CHROMA_IDX ) {
        UInt8 *pucPredLM = pCache->pucPredC[2];
        UInt8 *pucRecY   = pCache->pucRecY;
        UInt32 uiOffset  = pCache->uiOffset/2;
        UInt x, y;

        // Calculate LMChroma Pred Pixel
        for( y=0; y<nSize; y++ ) {
            for( x=0; x<nSize; x++ ) {
                UInt8 P0 = pucRecY[(2*y+0)*MAX_CU_SIZE+(2*x)];
                UInt8 P1 = pucRecY[(2*y+1)*MAX_CU_SIZE+(2*x)];
                pucPredLM[y*MAX_CU_SIZE/2+x] = (P0 + P1) >> 1;
            }
        }

        xPredIntraLM(
            pCache->pucPixRefC[0],
            pCache->pucLeftPixM,
            pCache->pucTopPixM + uiOffset,
                    pucPredLM,
            pCache->pucPredC[0],
            nSize
        );
        xPredIntraLM(
            pCache->pucPixRefC[1],
            pCache->pucLeftPixM,
            pCache->pucTopPixM + uiOffset,
                    pucPredLM,
            pCache->pucPredC[1],
            nSize
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


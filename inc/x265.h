/*****************************************************************************
 * x265.h: Define struct and macro
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

#ifndef __X265_H__
#define __X265_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "bitstream.h"
#include "utils.h"

/// supported slice type
typedef enum {
    SLICE_I = 0,
    SLICE_P = 1,
    SLICE_B = 2
} X265_SliceType;

/// reference list index
typedef enum {
    REF_PIC_LIST_X = -1,    ///< reference list invalid
    REF_PIC_LIST_0 =  0,    ///< reference list 0
    REF_PIC_LIST_1 =  1,    ///< reference list 1
    REF_PIC_LIST_C =  2,    ///< combined reference list for uni-prediction in B-Slices
} X265_RefPicList;

/// chroma formats (according to semantics of chroma_format_idc)
typedef enum {
    CHROMA_400  = 0,
    CHROMA_420  = 1,
    CHROMA_422  = 2,
    CHROMA_444  = 3
} eChromaFormat;

/// reference frame
typedef struct X265_Frame {
    UInt8   *pucY;
    UInt8   *pucU;
    UInt8   *pucV;
} X265_Frame;

/// cache for every processor
typedef enum {
    MODE_INVALID    = 255,
    MODE_PLANE      =   0,
    MODE_VER        =   1,
    MODE_HOR        =   2,
    MODE_DC         =   3,
} eIntraMode;

typedef struct X265_Cache {
    /// context
    UInt32  uiOffset;
    UInt8   pucTopPixY[MAX_WIDTH    ];
    UInt8   pucTopPixU[MAX_WIDTH / 2];
    UInt8   pucTopPixV[MAX_WIDTH / 2];
    UInt8   pucTopModeY[MAX_WIDTH / MIN_CU_SIZE   ];
    UInt8   pucLeftPixY[MAX_CU_SIZE    ];
    UInt8   pucLeftPixU[MAX_CU_SIZE / 2];
    UInt8   pucLeftPixV[MAX_CU_SIZE / 2];
    UInt8   pucLeftModeY[(MAX_CU_SIZE + MAX_CU_SIZE)    ];
    UInt8   pucTopLeftY[MAX_PU_XY    ];
    UInt8   pucTopLeftU[MAX_PU_XY / 4];
    UInt8   pucTopLeftV[MAX_PU_XY / 4];

    /// current
    UInt8   pucPixY[MAX_CU_SIZE * MAX_CU_SIZE    ];
    UInt8   pucPixU[MAX_CU_SIZE * MAX_CU_SIZE / 4];
    UInt8   pucPixV[MAX_CU_SIZE * MAX_CU_SIZE / 4];
    UInt8   pucRecY[MAX_CU_SIZE * MAX_CU_SIZE    ];
    UInt8   pucRecU[MAX_CU_SIZE * MAX_CU_SIZE / 4];
    UInt8   pucRecV[MAX_CU_SIZE * MAX_CU_SIZE / 4];
    UInt8   nBestModeY;
    UInt8   nBestModeC;

    /// IntraPred buffer
    UInt8   pucPixRef[2][4*MAX_CU_SIZE+1];          //< 0:ReconPixel, 1:Filtered
    UInt8   pucPixRefC[2][4*MAX_CU_SIZE/2+1];          //< 0:ReconPixel, 1:Filtered
    UInt8   pucPredY[MAX_CU_SIZE * MAX_CU_SIZE];
    UInt8   pucPredC[2][MAX_CU_SIZE * MAX_CU_SIZE/4];
    UInt8   ucMostModeY[3];
    UInt8   ucMostModeC[5];
    UInt8   bValid[5];

    /// Encode coeff buffer
    Int16   piCoefY[MAX_CU_SIZE*MAX_CU_SIZE];
    Int16   piCoefU[MAX_CU_SIZE*MAX_CU_SIZE/4];
    Int16   piCoefV[MAX_CU_SIZE*MAX_CU_SIZE/4];


    /// Temp buffer
    Int16   piTmp[2][MAX_CU_SIZE*MAX_CU_SIZE];
} X265_Cache;

typedef struct {
    UInt32  uiLow;
    UInt32  uiRange;
    Int32   iBitsLeft;
    UInt8   ucCache;
    UInt32  uiNumBytes;
} X265_Cabac;

/// main handle
typedef struct X265_t {
    // Local
    X265_BitStream  bs;
    X265_Cabac      cabac;
    X265_SliceType  eSliceType;
    X265_Frame      refn[MAX_REF_NUM];
    X265_Frame      *pFrame;
    X265_Cache      cache;
    Int32           iPoc;
    Int32           iQP;
    UInt32          uiCUX;
    UInt32          uiCUY;

    // Interface
    // Profile
    UInt8   ucProfileIdc;
    UInt8   ucLevelIdc;

    // Params
    UInt16  usWidth;
    UInt16  usHeight;
    UInt8   ucMaxCUWidth;
    UInt8   ucMinCUWidth;
    UInt8   ucMaxCUDepth;
    UInt8   ucQuadtreeTULog2MinSize;
    UInt8   ucQuadtreeTULog2MaxSize;
    UInt8   ucQuadtreeTUMaxDepthInter;
    UInt8   ucQuadtreeTUMaxDepthIntra;
    UInt8   ucMaxNumRefFrames;
    UInt8   ucBitsForPOC;
    UInt8   ucMaxNumMergeCand;
    UInt8   ucTSIG;

    // Feature
    UInt8   bUseNewRefSetting;
    UInt8   bUseSAO;
    UInt8   bUseLMChroma;
    UInt8   bMRG;
    UInt8   bLoopFilterDisable;
    UInt8   bSignHideFlag;
    UInt8   bEnableTMVPFlag;
} X265_t;


// ***************************************************************************
// * bitstream.cpp
// ***************************************************************************
void xWriteSPS( X265_t *h );
void xWritePPS( X265_t *h );
void xWriteSliceHeader( X265_t *h );
Int32 xPutRBSP(UInt8 *pucDst, UInt8 *pucSrc, UInt32 uiLength);
void xCabacReset( X265_Cabac *pCabac );
void xCabacFlush( X265_Cabac *pCabac, X265_BitStream *pBS );
UInt xCabacGetNumWrittenBits( X265_Cabac *pCabac, X265_BitStream *pBS );
void xCabacEncodeBin( X265_Cabac *pCabac, X265_BitStream *pBS, UInt binValue, UInt8 *pCtxState );
void xCabacEncodeBinEP( X265_Cabac *pCabac, X265_BitStream *pBS, UInt binValue );
void xCabacEncodeBinsEP( X265_Cabac *pCabac, X265_BitStream *pBS, UInt binValues, Int numBins );


// ***************************************************************************
// * set.cpp
// ***************************************************************************
void xDefaultParams( X265_t *h );
int xCheckParams( X265_t *h );

// ***************************************************************************
// * Table.cpp
// ***************************************************************************
extern const UInt8 g_aucIntraFilterType[5][NUM_INTRA_MODE-1];
extern const Int8 g_aucIntraPredAngle[NUM_INTRA_MODE-1];
extern const Int16 g_aucInvAngle[NUM_INTRA_MODE-1];
extern const Int8 g_aiT4[4*4];
extern const Int8 g_aiT8[8*8];
extern const Int8 g_aiT16[16*16];
extern const Int8 g_aiT32[32*32];
extern const Int16 g_quantScales[6];
extern const UInt8 g_invQuantScales[6];
extern const UInt8 g_aucChromaScale[52];
extern const UInt8 g_aucNextStateMPS[128];
extern const UInt8 g_aucNextStateLPS[128];
extern const UInt8 g_aucLPSTable[64][4];
extern const UInt8 g_aucRenormTable[32];


// ***************************************************************************
// * Encode.cpp
// ***************************************************************************
void xEncInit( X265_t *h );
Int32 xEncEncode( X265_t *h, X265_Frame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize );
void xEncCahceInit( X265_t *h );
void xEncCahceInitLine( X265_t *h );
void xEncCacheLoadCU( X265_t *h, UInt uiX, UInt uiY );
void xEncCacheUpdate( X265_t *h, UInt32 uiX, UInt32 uiY, UInt nWidth, UInt nHeight );
void xEncIntraLoadRef( X265_t *h, UInt32 uiX, UInt32 uiY, UInt nSize );
UInt xGetTopLeftIndex( UInt32 uiX, UInt32 uiY );
void xEncIntraPredLuma( X265_t *h, UInt nMode, UInt nSize );
void xEncIntraPredChroma( X265_t *h, UInt nMode, UInt nSize );

// ***************************************************************************
// * Pixel.cpp
// ***************************************************************************
typedef UInt32 xSad( Int N, UInt8 *pSrc, UInt nStrideSrc, UInt8 *pRef, UInt nStrideRef );
extern xSad *xSadN[MAX_CU_DEPTH+1];
typedef void xDCT( Int16 *pDst, Int16 *pSrc, UInt nStride, Int nLines, Int nShift );
extern xDCT *xDctN[MAX_CU_DEPTH+1];
extern xDCT *xInvDctN[MAX_CU_DEPTH+1];
void xSubDct ( Int16 *pDst, UInt8 *pSrc, UInt8 *pRef, UInt nStride, Int16 *piTmp0, Int16 *piTmp1, Int iWidth,  Int iHeight, UInt nMode );
void xIDctAdd( UInt8 *pDst, Int16 *pSrc, UInt8 *pRef, UInt nStride, Int16 *piTmp0, Int16 *piTmp1, Int iWidth, Int iHeight, UInt nMode );
UInt32 xQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, X265_SliceType eSType );
void xDeQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, X265_SliceType eSType );

// ***************************************************************************
// * TestVec.cpp
// ***************************************************************************
extern UInt32 tv_size;
extern UInt8  tv_top[2][MAX_CU_SIZE*2+1];
extern UInt8  tv_left[2][MAX_CU_SIZE*2];
extern UInt8  tv_pred[35][MAX_CU_SIZE*MAX_CU_SIZE];
extern UInt8  tv_orig[MAX_CU_SIZE*MAX_CU_SIZE];
extern UInt8  tv_bpred[MAX_CU_SIZE*MAX_CU_SIZE];
extern  Int16 tv_resi[MAX_CU_SIZE*MAX_CU_SIZE];
extern  Int16 tv_trans[MAX_CU_SIZE*MAX_CU_SIZE];
extern  Int16 tv_quant[MAX_CU_SIZE*MAX_CU_SIZE];
extern  Int16 tv_iquant[MAX_CU_SIZE*MAX_CU_SIZE];
extern  Int16 tv_itrans[MAX_CU_SIZE*MAX_CU_SIZE];
extern UInt8  tv_rec[MAX_CU_SIZE*MAX_CU_SIZE];
extern UInt32 tv_mostmode[3];
extern UInt32 tv_bestmode;
extern UInt32 tv_sad[35];
extern UInt8 tv_refC[2][MAX_CU_SIZE/2*4+1];
extern UInt8 tv_predC[2][5][MAX_CU_SIZE*MAX_CU_SIZE/4];
extern UInt8 tv_origC[2][MAX_CU_SIZE*MAX_CU_SIZE/4];
extern Int16 tv_resiC[2][5][MAX_CU_SIZE*MAX_CU_SIZE/4];
extern Int16 tv_transC[2][5][MAX_CU_SIZE*MAX_CU_SIZE/4];
extern Int16 tv_quantC[2][5][MAX_CU_SIZE*MAX_CU_SIZE/4];
extern Int16 tv_iquantC[2][5][MAX_CU_SIZE*MAX_CU_SIZE/4];
extern Int16 tv_itransC[2][5][MAX_CU_SIZE*MAX_CU_SIZE/4];
extern UInt8  tv_recC[2][5][MAX_CU_SIZE*MAX_CU_SIZE/4];
extern UInt32 tv_mostmodeC[5];
extern UInt32 tv_dmMode;
extern UInt32 tv_bestmodeC;
extern UInt32 tv_nModeC;
extern UInt32 tv_nIdxC;
extern UInt32 tv_sadC;
int tInitTv( const char *fname );
void tGetVector( );

#endif /* __X265_H__ */

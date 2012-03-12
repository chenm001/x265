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
    UInt8   pcTopModeY[(MAX_WIDTH + MAX_CU_SIZE)    ];
    UInt8   pcTopModeU[(MAX_WIDTH + MAX_CU_SIZE) / 2];
    UInt8   pcTopModeV[(MAX_WIDTH + MAX_CU_SIZE) / 2];
    UInt8   pucLeftPixY[MAX_CU_SIZE    ];
    UInt8   pucLeftPixU[MAX_CU_SIZE / 2];
    UInt8   pucLeftPixV[MAX_CU_SIZE / 2];
    UInt8   pcLeftModeY[(MAX_CU_SIZE + MAX_CU_SIZE)    ];
    UInt8   pcLeftModeU[(MAX_CU_SIZE + MAX_CU_SIZE) / 2];
    UInt8   pcLeftModeV[(MAX_CU_SIZE + MAX_CU_SIZE) / 2];
    UInt8   pucTopLeftY[MAX_PART_NUM    ];
    UInt8   pucTopLeftU[MAX_PART_NUM / 4];
    UInt8   pucTopLeftV[MAX_PART_NUM / 4];

    /// current
    UInt8   pucPixY[MAX_CU_SIZE * MAX_CU_SIZE    ];
    UInt8   pucPixU[MAX_CU_SIZE * MAX_CU_SIZE / 4];
    UInt8   pucPixV[MAX_CU_SIZE * MAX_CU_SIZE / 4];

    /// IntraPred buffer
    UInt8   pucPixRef[2][4*MAX_CU_SIZE+1];          //< 0:ReconPixel, 1:Filtered
    UInt8   pucPredY[MAX_CU_SIZE * MAX_CU_SIZE];
    UInt8   bValid[5];
} X265_Cache;

/// main handle
typedef struct X265_t {
    // Local
    X265_BitStream  bs;
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

// ***************************************************************************
// * Encode.cpp
// ***************************************************************************
void xEncInit( X265_t *h );
Int32 xEncEncode( X265_t *h, X265_Frame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize );
void xEncCahceInit( X265_t *h );
void xEncCahceInitLine( X265_t *h );
void xEncCacheLoadCU( X265_t *h, UInt uiX, UInt uiY );
void xEncIntraLoadRef( X265_t *h, UInt32 uiX, UInt32 uiY, UInt nSize );
UInt xGetTopLeftIndex( UInt32 uiX, UInt32 uiY );
void xEncIntraPredLuma( X265_t *h, UInt nMode, UInt nSize, UInt bLuma );

// ***************************************************************************
// * Pixel.cpp
// ***************************************************************************
typedef UInt32 xSad( Int N, UInt8 *pSrc, UInt nStrideSrc, UInt8 *pRef, UInt nStrideRef );
extern xSad *xSadN[MAX_CU_DEPTH+2];

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
extern UInt32 tv_uiPreds[3];
extern UInt32 tv_bestmode;
int tInitTv( const char *fname );
void tGetVector( );

#endif /* __X265_H__ */

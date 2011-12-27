/*****************************************************************************
 * x265.h: Define struct and macro
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

#ifndef __X265_H__
#define __X265_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "bitstream.h"

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

typedef struct X265_t {
    // Local
    X265_BitStream  bs;
    X265_SliceType  eSliceType;
    Int32           iPoc;
    Int32           iQP;

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

    // Feature
    UInt8   bUseNewRefSetting;
    UInt8   bUseSAO;
    UInt8   bUseLMChroma;
    UInt8   bLoopFilterDisable;
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

#endif /* __X265_H__ */

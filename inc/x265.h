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
#include <assert.h>

#include "config.h"
#include "bitstream.h"

typedef struct X265_t {
    X265_BitStream  bs;

    // Profile
    UInt8   ucProfileIdc;
    UInt8   ucLevelIdc;

    // Params
    UInt8   ucMaxCUWidth;
    UInt8   ucMinCUWidth;
    UInt8   ucMaxCUDepth;
    UInt8   ucQuadtreeTULog2MinSize;
    UInt8   ucQuadtreeTULog2MaxSize;
    UInt8   ucQuadtreeTUMaxDepthInter;
    UInt8   ucQuadtreeTUMaxDepthIntra;
    UInt8   ucMaxNumRefFrames;

    UInt32  uiWidth;
    UInt32  uiHeight;

    // Feature
    UInt8   bUseNewRefSetting;
    UInt8   bUseSAO;
    UInt8   bAMVPMode[MAX_CU_DEPTH];
#if NSQT
    UInt8   bUseNSQT;
#endif
#if AMP
    UInt8   bUseAMP;
#endif
    UInt8   bUseLMChroma;
} X265_t;

#endif /* __X265_H__ */

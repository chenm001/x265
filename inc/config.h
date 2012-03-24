/*****************************************************************************
 * config.h: Common config
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

////////////////////////////
// x265 start
////////////////////////////
#define ENC_DEC_TRACE                       (0)
#define CHECK_TV                            (1)

#define MAX_CU_DEPTH                        (4)
#define MAX_REF_NUM                         (1)
#define MAX_WIDTH                           (2048)
#define MAX_HEIGHT                          (2048)
#define MIN_CU_SIZE                         ( 4)
#define MAX_CU_SIZE                         (64)
#define MAX_PU_XY                           (MAX_CU_SIZE / MIN_CU_SIZE)
#define MAX_PART_NUM                        (MAX_PU_XY * MAX_PU_XY)

#define NUM_INTRA_MODE                      (36)
#define PLANAR_IDX                          ( 0)
#define VER_IDX                             (26)    // index for intra VERTICAL   mode
#define HOR_IDX                             (10)    // index for intra HORIZONTAL mode
#define DC_IDX                              ( 1)
#define DM_CHROMA_IDX                       (36)    // chroma mode index for derived from luma intra mode
#define SHIFT_INV_1ST                       ( 7)    // Shift after first inverse transform stage
#define SHIFT_INV_2ND                       (12)    // Shift after second inverse transform stage
#define MAX_SAD                             ( 1 << 30 )

#define ALWAYS_INLINE                       __inline
////////////////////////////
// x265 end
////////////////////////////

////////////////////////////
// JCT-VC start
////////////////////////////
#define QUANT_IQUANT_SHIFT              20 // Q(QP%6) * IQ(QP%6) = 2^20
#define QUANT_SHIFT                     14 // Q(4) = 2^14
#define SCALE_BITS                      15 // Inherited from TMuC, pressumably for fractional bit estimates in RDOQ
#define MAX_TR_DYNAMIC_RANGE            15 // Maximum transform dynamic range (excluding sign bit)
////////////////////////////
// JCT-VC end
////////////////////////////

////////////////////////////
// JCT-VC H start
////////////////////////////
#define MULTIBITS_DATA_HIDING           1  ///< H0481: multiple sign bit hiding
////////////////////////////
// JCT-VC G end
////////////////////////////

////////////////////////////
// JCT-VC G start
////////////////////////////
#define MRG_MAX_NUM_CANDS_SIGNALED      5           //<G091: value of maxNumMergeCand signaled in slice header 
#define MRG_MAX_NUM_CANDS               5           // MERGE
////////////////////////////
// JCT-VC G end
////////////////////////////


/////////////////////////////////
// AHG SLICES defines section start
/////////////////////////////////

/////////////////////////////////
// AHG SLICES defines section end
/////////////////////////////////
#define H0566_TLA                               1
#define RPS_IN_SPS                              1 // Adopted during discussion of JCTVC-H0423
#define H0567_DPB_PARAMETERS_PER_TEMPORAL_LAYER 1


typedef unsigned char       UInt8;
typedef   signed char        Int8;
typedef unsigned short      UInt16;
typedef          short       Int16;
typedef unsigned int        UInt32;
typedef          int         Int32;
typedef unsigned __int64    UInt64;
typedef          __int64     Int64;

// Fast integer depends on platform
typedef Int32    Int;
typedef UInt32  UInt;

#define FALSE   (0)
#define TRUE    (1)

#endif /* __CONFIG_H__ */

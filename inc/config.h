/*****************************************************************************
 * config.h: Common config
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

////////////////////////////
// x265 start
////////////////////////////
#define ENC_DEC_TRACE                       (0)
#define MAX_CU_DEPTH                        (4)

#define ALWAYS_INLINE                       __inline
////////////////////////////
// x265 end
////////////////////////////

////////////////////////////
// JCT-VC G start
////////////////////////////
#define G1002_RPS                       1
#if  !G1002_RPS
#define REF_SETTING_FOR_LD              1           // reference frame setting for low delay setting (JCTVC-F701)
#else
#define INTER_RPS_PREDICTION            1           // remove this once tested.
#define WRITE_BACK                      1           ///< Enable/disable the encoder to replace the deltaPOC and Used by current from the config file with the values derived by the refIdc parameter.
#define PRINT_RPS_BITS_WRITTEN          0           ///< Enable/disable the printing of bits used to send the RPS.
#endif
#define OL_USE_WPP                      1     // Set to 1 to enable Wavefront Parallel Processing, 0 otherwise
#define G091_SIGNAL_MAX_NUM_MERGE_CANDS 1           //<G091: maxNumMergeCand signaling in slice header
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
#define MRG_MAX_NUM_CANDS_SIGNALED      5           //<G091: value of maxNumMergeCand signaled in slice header 
#endif
#define MRG_MAX_NUM_CANDS               5           // MERGE
////////////////////////////
// JCT-VC G end
////////////////////////////


////////////////////////////
// JCT-VC F start
////////////////////////////
#define DISABLE_4x4_INTER                   1       // Coding one flag into SPS to enable/disable INTER4x4 
#define NSQT                                1       // F410 & F412 : Non-Square Quadtree Transform
#define REF_SETTING_FOR_LD                  1       // reference frame setting for low delay setting (JCTVC-F701)
                                                    // using one nearest frame as reference frame, and the other frames are high quality (POC%4==0) frames (1+X)
                                                    // this should be done with encoder only decision
                                                    // but because of the absence of reference frame management, the related code was hard coded currently
#define AMP                                 1       ///< JCTVC-F379: asymmetric motion partition
#define F747_APS                            1       // F747 : Adaptation Parameter Set (APS)
#define WEIGHT_PRED                         1       ///< F265 & F326: enable weighted prediction
////////////////////////////
// JCT-VC F end
////////////////////////////


////////////////////////////
// JCT-VC E start
////////////////////////////
#define E192_SPS_PCM_BIT_DEPTH_SYNTAX       1       // JCTVC-E192: PCM bit depth
#define E192_SPS_PCM_FILTER_DISABLE_SYNTAX  1       // JCTVC-E192: PCM filter disable flag
#define SAO                                 1       // JCTVC-E049: Sample adaptive offset
////////////////////////////
// JCT-VC E end
////////////////////////////


/////////////////////////////////
// AHG SLICES defines section start
/////////////////////////////////
#define FINE_GRANULARITY_SLICES             1

/////////////////////////////////
// AHG SLICES defines section end
/////////////////////////////////
#define TILES                               1


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

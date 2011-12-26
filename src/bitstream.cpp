/*****************************************************************************
 * bitstream.cpp: Bitstream Functions
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

#include "x265.h"
#include "bitstream.h"
#include "utils.h"

// ***************************************************************************

// ***************************************************************************
#if ENC_DEC_TRACE

#define WRITE_CODE( value, length, name)    xWriteCodeTr ( value, length, name )
#define WRITE_UVLC( value,         name)    xWriteUvlcTr ( value,         name )
#define WRITE_SVLC( value,         name)    xWriteSvlcTr ( value,         name )
#define WRITE_FLAG( value,         name)    xWriteFlagTr ( value,         name )

void  xWriteUvlcTr          ( UInt value,               const Char *pSymbolName);
void  xWriteSvlcTr          ( Int  value,               const Char *pSymbolName);
void  xWriteFlagTr          ( UInt value,               const Char *pSymbolName);

FILE *g_hTrace = NULL;
UInt32 g_nSymbolCounter = 0;

void  xTraceSPSHeader()
{
    fprintf( g_hTrace, "=========== Sequence Parameter Set ID: %d ===========\n", 0 );
}

void  xTracePPSHeader()
{
    fprintf( g_hTrace, "=========== Picture Parameter Set ID: %d ===========\n", 0 );
}

void  xTraceSliceHeader()
{
    fprintf( g_hTrace, "=========== Slice ===========\n");
}


void  xWriteCodeTr (UInt value, UInt  length, const char *pSymbolName)
{
    xWriteCode (value,length);
    fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
    fprintf( g_hTrace, "%-40s u(%d) : %d\n", pSymbolName, length, value ); 
}

void  xWriteUvlcTr (UInt value, const char *pSymbolName)
{
    xWriteUvlc (value);
    fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
    fprintf( g_hTrace, "%-40s u(v) : %d\n", pSymbolName, value ); 
}

void  xWriteSvlcTr (Int value, const char *pSymbolName)
{
    xWriteSvlc(value);
    fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
    fprintf( g_hTrace, "%-40s s(v) : %d\n", pSymbolName, value ); 
}

void  xWriteFlagTr(UInt value, const char *pSymbolName)
{
    xWriteFlag(value);
    fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
    fprintf( g_hTrace, "%-40s u(1) : %d\n", pSymbolName, value ); 
}

#else /* ENC_DEC_TRACE */

#define WRITE_CODE( value, length, name)     xWriteCode ( pBS, value, length )
#define WRITE_UVLC( value,         name)     xWriteUvlc ( pBS, value )
#define WRITE_SVLC( value,         name)     xWriteSvlc ( pBS, value )
#define WRITE_FLAG( value,         name)     xWriteFlag ( pBS, value )

#endif /* ENC_DEC_TRACE */


// ***************************************************************************
void xWriteSPS( X265_t *h )
{
    int i;
    X265_BitStream *pBS = &h->bs;
#if ENC_DEC_TRACE
    xTraceSPSHeader();
#endif

    WRITE_CODE( h->ucProfileIdc,            8,          "profile_idc" );
    WRITE_CODE( 0,                          8,          "reserved_zero_8bits" );
    WRITE_CODE( h->ucLevelIdc,              8,          "level_idc" );
    WRITE_UVLC( 0,                                      "seq_parameter_set_id" );
    WRITE_CODE( 0,                          3,          "max_temporal_layers_minus1" );
    WRITE_CODE( h->usWidth,                16,          "pic_width_in_luma_samples" );
    WRITE_CODE( h->usHeight,               16,          "pic_height_in_luma_samples" );
    
    WRITE_UVLC( 0,    "bit_depth_luma_minus8" );
    WRITE_UVLC( 0,    "bit_depth_chroma_minus8" );
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    WRITE_CODE( 7, 4, "pcm_bit_depth_luma_minus1" );
    WRITE_CODE( 7, 4, "pcm_bit_depth_chroma_minus1" );
#endif
#if G1002_RPS
    WRITE_UVLC( h->ucBitsForPOC-4,                      "log2_max_pic_order_cnt_lsb_minus4" );
    WRITE_UVLC( h->ucMaxNumRefFrames,                   "max_num_ref_pics" ); 
    WRITE_UVLC( 0,                                      "max_num_reorder_pics" ); 
#endif
#if DISABLE_4x4_INTER
    WRITE_FLAG  ( 1,  "DisableInter4x4" );
#endif
    // log2_max_frame_num_minus4
    // pic_order_cnt_type
    // if( pic_order_cnt_type  = =  0 )
    //   log2_max_pic_order_cnt_lsb_minus4
    // else if( pic_order_cnt_type  = =  1 ) {
    //   delta_pic_order_always_zero_flag
    //   offset_for_non_ref_pic
    //   num_ref_frames_in_pic_order_cnt_cycle
    //   for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
    //     offset_for_ref_frame[ i ]
    // }
    // max_num_ref_frames
    // gaps_in_frame_num_value_allowed_flag

    UInt32 MinCUSize = h->ucMaxCUWidth >> (h->ucMaxCUDepth - 1);
    UInt32 log2MinCUSize = xLog2(MinCUSize);

    WRITE_UVLC( log2MinCUSize - 3,                                                    "log2_min_coding_block_size_minus3" );
    WRITE_UVLC( h->ucMaxCUDepth - 1,                                                  "log2_diff_max_min_coding_block_size" );
    WRITE_UVLC( h->ucQuadtreeTULog2MinSize - 2,                                       "log2_min_transform_block_size_minus2" );
    WRITE_UVLC( h->ucQuadtreeTULog2MaxSize - h->ucQuadtreeTULog2MinSize,              "log2_diff_max_min_transform_block_size" );
    WRITE_UVLC( 7 - 3,                                                                "log2_min_pcm_coding_block_size_minus3" );
    WRITE_UVLC( h->ucQuadtreeTUMaxDepthInter - 1,                                     "max_transform_hierarchy_depth_inter" );
    WRITE_UVLC( h->ucQuadtreeTUMaxDepthIntra - 1,                                     "max_transform_hierarchy_depth_intra" );
    WRITE_FLAG( h->bUseLMChroma,                                                      "chroma_pred_from_luma_enabled_flag" );
    WRITE_FLAG( 1,                                                                    "loop_filter_across_slice_flag");
#if SAO
    WRITE_FLAG( h->bUseSAO,                                                           "sample_adaptive_offset_enabled_flag");
#endif
    WRITE_FLAG( 0,                                                                    "adaptive_loop_filter_enabled_flag");
#if E192_SPS_PCM_FILTER_DISABLE_SYNTAX
    WRITE_FLAG( 0,                                                                    "pcm_loop_filter_disable_flag");
#endif
    WRITE_FLAG( 0,                                                                    "cu_qp_delta_enabled_flag" );
    WRITE_FLAG( 0,                                                                    "temporal_id_nesting_flag" );
    
    // !!!KS: Syntax not in WD !!!
    
    WRITE_UVLC  ( 0, "PadX" );
    WRITE_UVLC  ( 0, "PadY" );
    
    // Tools
#if !G1002_RPS
    WRITE_FLAG  ( 1,  "Unknow");
#endif
    WRITE_FLAG  ( 1,  "SOPH/MRG" ); // SOPH:
    
    // AMVP mode for each depth
    for (i = 0; i < h->ucMaxCUDepth; i++)
    {
        WRITE_FLAG( 1, "AMVPMode");
    }
    
#if !G1002_RPS
#if REF_SETTING_FOR_LD
    // these syntax elements should not be sent at SPS when the full reference frame management is supported
    WRITE_FLAG( h->bUseNewRefSetting ? 1 : 0, "UseNewRef" );
    if ( h->bUseNewRefSetting )
    {
        WRITE_UVLC( h->ucMaxNumRefFrames, "MaxNumRefFrames" );
    }
#endif
#endif

#if TILES
    WRITE_FLAG( 0,  "uniform_spacing_idc" );
    WRITE_FLAG( 0,  "tile_boundary_independence_idc" );
    WRITE_UVLC( 0,  "num_tile_columns_minus1" );
    WRITE_UVLC( 0,  "num_tile_rows_minus1" );
#endif
    
    // Software-only flags
#if NSQT
    WRITE_FLAG( 1, "enable_nsqt" );
#endif
#if AMP
    WRITE_FLAG( 1, "enable_amp" );
#endif
    xWriteRBSPTrailingBits(pBS);
}


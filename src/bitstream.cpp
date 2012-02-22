/*****************************************************************************
 * bitstream.cpp: Bitstream Functions
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
    X265_BitStream *pBS = &h->bs;
    int i;
#if ENC_DEC_TRACE
    xTraceSPSHeader();
#endif

    WRITE_CODE( h->ucProfileIdc,            8,          "profile_idc" );
    WRITE_CODE( 0,                          8,          "reserved_zero_8bits" );
    WRITE_CODE( h->ucLevelIdc,              8,          "level_idc" );
    WRITE_UVLC( 0,                                      "seq_parameter_set_id" );
    WRITE_UVLC( CHROMA_420,                             "chroma_format_idc" );
    WRITE_CODE( 0,                          3,          "max_temporal_layers_minus1" );
    WRITE_CODE( h->usWidth,                16,          "pic_width_in_luma_samples" );
    WRITE_CODE( h->usHeight,               16,          "pic_height_in_luma_samples" );
    
    WRITE_UVLC( 0,                                      "bit_depth_luma_minus8" );
    WRITE_UVLC( 0,                                      "bit_depth_chroma_minus8" );

    WRITE_FLAG( 0,                                      "pcm_enabled_flag");

    WRITE_UVLC( h->ucBitsForPOC-4,                      "log2_max_pic_order_cnt_lsb_minus4" );
    WRITE_UVLC( 1,                                      "max_num_ref_pics" ); 
    WRITE_UVLC( 0,                                      "max_num_reorder_pics" ); 
    WRITE_UVLC(0,                                       "max_dec_frame_buffering" );
    WRITE_UVLC(0,                                       "max_latency_increase"    );

    UInt32 MinCUSize = h->ucMaxCUWidth >> (h->ucMaxCUDepth - 1);
    UInt32 log2MinCUSize = xLog2(MinCUSize);

    WRITE_UVLC( log2MinCUSize - 3,                                                    "log2_min_coding_block_size_minus3" );
    WRITE_UVLC( h->ucMaxCUDepth - 1,                                                  "log2_diff_max_min_coding_block_size" );
    WRITE_UVLC( h->ucQuadtreeTULog2MinSize - 2,                                       "log2_min_transform_block_size_minus2" );
    WRITE_UVLC( h->ucQuadtreeTULog2MaxSize - h->ucQuadtreeTULog2MinSize,              "log2_diff_max_min_transform_block_size" );
    if(log2MinCUSize == 3) {
        WRITE_FLAG  ( 1,  "DisableInter4x4" );
    }
    WRITE_UVLC( h->ucQuadtreeTUMaxDepthInter - 1,                                     "max_transform_hierarchy_depth_inter" );
    WRITE_UVLC( h->ucQuadtreeTUMaxDepthIntra - 1,                                     "max_transform_hierarchy_depth_intra" );
    WRITE_FLAG( 0,                                                                    "scaling_list_enabled_flag" ); 
    WRITE_FLAG( 0,                                                                    "chroma_pred_from_luma_enabled_flag" );
    WRITE_FLAG( 0,                                                                    "deblocking_filter_in_aps_enabled_flag");
    WRITE_FLAG( 1,                                                                    "loop_filter_across_slice_flag");
    WRITE_FLAG( 0,                                                                    "sample_adaptive_offset_enabled_flag");
    WRITE_FLAG( 0,                                                                    "adaptive_loop_filter_enabled_flag");
    WRITE_FLAG( 0,                                                                    "temporal_id_nesting_flag" );

    //!!!KS: Syntax not in WD !!!

    WRITE_UVLC  ( 0, "PadX" );
    WRITE_UVLC  ( 0, "PadY" );
    
    // Tools
    WRITE_FLAG  ( 1,  "SOPH/MRG" ); // SOPH:
    
    // AMVP mode for each depth
    for (i = 0; i < h->ucMaxCUDepth; i++) {
        WRITE_FLAG( 1, "AMVPMode");
    }
    
    WRITE_FLAG( 0,  "uniform_spacing_idc" );
    WRITE_UVLC( 0,  "num_tile_columns_minus1" );
    WRITE_UVLC( 0,  "num_tile_rows_minus1" );

    // Software-only flags
    WRITE_FLAG( 1,  "enable_nsqt" );
    WRITE_FLAG( 1,  "enable_amp" );

    xWriteRBSPTrailingBits(pBS);
}

#if G1002_RPS
static void xWriteShortTermRefPicSet( X265_t *h )
{
    X265_BitStream *pBS = &h->bs;
    int i;
#if INTER_RPS_PREDICTION
    WRITE_FLAG( 0, "inter_ref_pic_set_prediction_flag" ); // inter_RPS_prediction_flag
#endif //INTER_RPS_PREDICTION
    // Turn off inter_ref_pic_set_prediction_flag
    {
        WRITE_UVLC( h->ucMaxNumRefFrames,   "num_negative_pics" );
        WRITE_UVLC( 0,                      "num_positive_pics" );
        for( i=0 ; i < h->ucMaxNumRefFrames; i++ ) {
            WRITE_UVLC( (1-i)-1, "delta_poc_s0_minus1" );
            WRITE_FLAG( 1,       "used_by_curr_pic_s0_flag"); 
        }
    }
}
#endif

void xWritePPS( X265_t *h )
{
    X265_BitStream *pBS = &h->bs;
    int i;
#if ENC_DEC_TRACE  
    xTracePPSHeader();
#endif
  
    WRITE_UVLC( 0,  "pic_parameter_set_id" );
    WRITE_UVLC( 0,  "seq_parameter_set_id" );
#if G1002_RPS
  // RPS is put before entropy_coding_mode_flag
  // since entropy_coding_mode_flag will probably be removed from the WD

    WRITE_UVLC(h->ucMaxNumRefFrames, "num_short_term_ref_pic_sets" );
    for( i=0; i < h->ucMaxNumRefFrames; i++ ) {
        xWriteShortTermRefPicSet(h);
    }    
    WRITE_FLAG( 0,  "long_term_ref_pics_present_flag" );
#endif
    // entropy_coding_mode_flag
#if OL_USE_WPP
    // We code the entropy_coding_mode_flag, it's needed for tests.
    WRITE_FLAG( 1,                                          "entropy_coding_mode_flag" );
    WRITE_UVLC( 0,                                          "entropy_coding_synchro" );
    WRITE_FLAG( 0,                                          "cabac_istate_reset" );
#endif
    WRITE_UVLC( 0,                                          "num_temporal_layer_switching_point_flags" );
    //   num_ref_idx_l0_default_active_minus1
    //   num_ref_idx_l1_default_active_minus1
    //   pic_init_qp_minus26  /* relative to 26 */
    WRITE_FLAG( 0,                                          "constrained_intra_pred_flag" );
#if FINE_GRANULARITY_SLICES
    WRITE_CODE( 0, 2,                                       "slice_granularity");
#endif
#if !F747_APS
    WRITE_FLAG( 1,                                          "shared_pps_info_enabled_flag" );
    //   if( shared_pps_info_enabled_flag )
    //     if( adaptive_loop_filter_enabled_flag )
    //       alf_param( )
#endif

#if WEIGHT_PRED
    WRITE_FLAG( 0,    "weighted_pred_flat" );   // Use of Weighting Prediction (P_SLICE)
    WRITE_CODE( 0, 2, "weighted_bipred_idc" );  // Use of Weighting Bi-Prediction (B_SLICE)
#endif

#if TILES
    WRITE_FLAG( 0, "tile_info_present_flag" );
#endif
    xWriteRBSPTrailingBits(pBS);
}

void xWriteSliceHeader( X265_t *h )
{
    X265_BitStream *pBS = &h->bs;
    int i;
#if ENC_DEC_TRACE  
    xTraceSliceHeader();
#endif
    // See NALWrite.cpp :: write(...)
    //xPutBits(pBS, 0, 3); // temporal_id
    //xPutBits(pBS, 1, 1); // output_flag
    //xPutBits(pBS, 1, 4); // reserved_one_4bits
    
    WRITE_FLAG( 0, "lightweight_slice_flag" );
  
    WRITE_UVLC( h->eSliceType,  "slice_type" );
    WRITE_UVLC( 0,              "pic_parameter_set_id" );
#if G1002_RPS
    if ( h->eSliceType == SLICE_I ) {
      WRITE_UVLC( 0, "idr_pic_id" );
      WRITE_FLAG( 0, "no_output_of_prior_pics_flag" );
    }
    else {
      WRITE_CODE( h->iPoc % (1<<h->ucBitsForPOC), h->ucBitsForPOC, "pic_order_cnt_lsb");
      WRITE_FLAG( 1, "short_term_ref_pic_set_pps_flag");
      WRITE_UVLC( 0, "short_term_ref_pic_set_idx" );
    }
#endif

#if !G1002_RPS
    // frame_num
    // if( IdrPicFlag )
    //   idr_pic_id
    // if( pic_order_cnt_type  = =  0 )
    //   pic_order_cnt_lsb  
    WRITE_CODE( h->uiPoc, 10, "pic_order_cnt_lsb" );   //  9 == SPS->Log2MaxFrameNum
    // if( slice_type  = =  P  | |  slice_type  = =  B ) {
    //   num_ref_idx_active_override_flag
    //   if( num_ref_idx_active_override_flag ) {
    //     num_ref_idx_l0_active_minus1
    //     if( slice_type  = =  B )
    //       num_ref_idx_l1_active_minus1
    //   }
    // }
#endif

    // we always set num_ref_idx_active_override_flag equal to one. this might be done in a more intelligent way 
    if ( h->eSliceType != SLICE_I ) {
        WRITE_FLAG( 1 ,                               "num_ref_idx_active_override_flag");
        WRITE_CODE( h->ucMaxNumRefFrames - 1, 3,      "num_ref_idx_l0_active_minus1" );
    }
    else {
    }

#if G1002_RPS
    if ( h->eSliceType != SLICE_I ) {
        WRITE_FLAG(0, "ref_pic_list_modification_flag" );    
    }
    if ( h->eSliceType == SLICE_B ) {
        WRITE_FLAG(0, "ref_pic_list_modification_flag" );
    }
#endif

#if !G1002_RPS
    // ref_pic_list_modification( )
#endif
    // ref_pic_list_combination( )
    // maybe move to own function?
    if ( h->eSliceType == SLICE_B ) {
        WRITE_FLAG(0, "ref_pic_list_combination_flag" );
    }

    //write slice address
    WRITE_FLAG( 1, "first_slice_in_pic_flag" );
  
    //   slice_qp
    WRITE_SVLC( h->iQP, "slice_qp" ); // this should be delta
    //   if( sample_adaptive_offset_enabled_flag )
    //     sao_param()
    //   if( deblocking_filter_control_present_flag ) {
    //     disable_deblocking_filter_idc
    WRITE_FLAG( h->bLoopFilterDisable, "loop_filter_disable");  // should be an IDC

    //     if( disable_deblocking_filter_idc  !=  1 ) {
    //       slice_alpha_c0_offset_div2
    //       slice_beta_offset_div2
    //     }
    //   }
    //   if( slice_type = = B )
    //   collocated_from_l0_flag
    if ( h->eSliceType == SLICE_B ) {
        WRITE_FLAG( 1, "collocated_from_l0_flag" );
        }
    //   if( adaptive_loop_filter_enabled_flag ) {
    //     if( !shared_pps_info_enabled_flag )
    //       alf_param( )
    //     alf_cu_control_param( )
    //   }
    // }
  
  // !!!! sytnax elements not in the WD !!!!
  
    // ????
    WRITE_FLAG(1, "DRBFlag");

#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
    assert( h->ucMaxNumMergeCand <= MRG_MAX_NUM_CANDS_SIGNALED );
    assert( MRG_MAX_NUM_CANDS_SIGNALED <= MRG_MAX_NUM_CANDS );
    WRITE_UVLC(MRG_MAX_NUM_CANDS - h->ucMaxNumMergeCand, "maxNumMergeCand");
#endif
    xWriteAlignOne(pBS);
}

Int32 xPutRBSP(UInt8 *pucDst, UInt8 *pucSrc, UInt32 uiLength)
{
    assert( uiLength > 2 );

    UInt8 *pucDst0 = pucDst;
    int i;
    *pucDst++ = *pucSrc++;
    *pucDst++ = *pucSrc++;

    for( i=2; i < uiLength; i++ ) {
        const Int nLeadZero = (pucSrc[-2] | pucSrc[-1]);
        if ( (nLeadZero == 0) && (pucSrc[0] <= 3) ) {
            *pucDst++ = 0x03;
        }
        *pucDst++ = *pucSrc++;
    }
    return(pucDst - pucDst0);
}

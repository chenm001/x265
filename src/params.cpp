/*****************************************************************************
 * params.cpp: Params config and check
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

void xDefaultParams(X265_t *h)
{
    memset(h, 0, sizeof(*h));

    h->ucProfileIdc                 =  0;
    h->ucLevelIdc                   =  0;

    // Params
    h->usWidth                      =  0;
    h->usHeight                     =  0;
    h->ucMaxCUWidth                 = 64;
    h->ucMinCUWidth                 =  4;
    h->ucMaxCUDepth                 =  0;
    h->ucQuadtreeTULog2MinSize      =  2;
    h->ucQuadtreeTULog2MaxSize      =  5;
    h->ucQuadtreeTUMaxDepthInter    =  3;
    h->ucQuadtreeTUMaxDepthIntra    =  3;
    h->ucMaxNumRefFrames            =  1;
    h->ucBitsForPOC                 =  8;
#if G091_SIGNAL_MAX_NUM_MERGE_CANDS
    h->ucMaxNumMergeCand            = MRG_MAX_NUM_CANDS_SIGNALED;
#endif

    // Feature
    h->bUseNewRefSetting            = FALSE;
    h->bUseSAO                      = FALSE;
    h->bUseLMChroma                 = FALSE;
    h->bLoopFilterDisable           = TRUE;
}

int confirmPara(int bflag, const char* message)
{
    if (!bflag)
        return false;
    
    printf("Error: %s\n",message);
    return true;
}

int xCheckParams( X265_t *h )
{
    int check_failed = false; /* abort if there is a fatal configuration problem */

    h->ucMaxCUDepth = xLog2(h->ucMaxCUWidth) - xLog2(h->ucMinCUWidth);


#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)
    xConfirmPara( (h->ucMaxCUWidth >> h->ucMaxCUDepth) > (1 << h->ucQuadtreeTULog2MinSize), "Assume (QuadtreeTULog2MinSize >= MinCUWidth) fail" );
    xConfirmPara( h->usWidth  % h->ucMaxCUWidth, "Width must be multiple of MaxCUWidth");
    xConfirmPara( h->usHeight % h->ucMaxCUWidth, "Height must be multiple of MaxCUWidth");
    xConfirmPara( h->ucMaxNumRefFrames > MAX_REF_NUM, "Currently, x265 can not support so many reference");

#undef xConfirmPara
    if (check_failed)
    {
        exit(EXIT_FAILURE);
    }
    return 0;
}

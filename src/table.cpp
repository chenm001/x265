/*****************************************************************************
 * table.cpp: Encoder ROM Tables
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

// ====================================================================================================================
// Tables
// ====================================================================================================================

// Table 8-6
const UInt8 g_aucIntraFilterType[5][NUM_INTRA_MODE] = {
    //4x4
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1 }, 
    //8x8
    { 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1 },
    //16x16
    { 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, -1 },
    //32x32
    { 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1 },
    //64x64
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1 },
};

//  Table 8-7
//  Indexing this array with the mode indicated in the bitstream
//  gives a logical index used in the prediction functions.
const UInt8 g_aucIntraPredOrder[NUM_INTRA_MODE] = {
    PLANAR_IDX_ORDER,
        //  ModeOrder LogicalOrderInPredFunctions
        //    Mode  PLANAR  PLANAR
    9,  //       1  VER     DC
    25, //       2  HOR     VER-8 (diagonal from top-left to bottom-right = HOR-8)
    0,  //       3  DC      VER-7
    1,  //       4  VER-8   VER-6
    5,  //       5  VER-4   VER-5
    13, //       6  VER+4   VER-4
    17, //       7  VER+8   VER-3
    21, //       8  HOR-4   VER-2
    29, //       9  HOR+4   VER-1
    33, //      10  HOR+8   VER
    3,  //      11  VER-6   VER+1
    7,  //      12  VER-2   VER+2
    11, //      13  VER+2   VER+3
    15, //      14  VER+6   VER+4
    19, //      15  HOR-6   VER+5
    23, //      16  HOR-2   VER+6
    27, //      17  HOR+2   VER+7
    31, //      18  HOR+6   VER+8
    2,  //      19  VER-7   HOR-7
    4,  //      20  VER-5   HOR-6
    6,  //      21  VER-3   HOR-5
    8,  //      22  VER-1   HOR-4
    10, //      23  VER+1   HOR-3
    12, //      24  VER+3   HOR-2
    14, //      25  VER+5   HOR-1
    16, //      26  VER+7   HOR
    18, //      27  HOR-7   HOR+1
    20, //      28  HOR-5   HOR+2
    22, //      29  HOR-3   HOR+3
    24, //      30  HOR-1   HOR+4
    26, //      31  HOR+1   HOR+5
    28, //      32  HOR+3   HOR+6
    30, //      33  HOR+5   HOR+7
    32, //      34  HOR+7   HOR+8
    -1, //      LM_CHROMA_IDX (Unused in x265)
};

// Table 8-8: index range are [1, 33]
const Int8 g_aucIntraPredAngle[NUM_INTRA_MODE-3+1] = {
    -1, -32, -26, -21, -17, -13, -9, -5,
    -2,   0,   2,   5,   9,  13, 17, 21,
    26,  32, -26, -21, -17, -13, -9, -5,
    -2,   0,   2,   5,   9,  13, 17, 21,
    26,  32,
};

// Table 8-9: index range are [1, 33]
const Int16 g_aucInvAngle[NUM_INTRA_MODE-3+1] = {
      -1, 256,  315,  390, 482, 630, 910, 1638,
    4096,   0, 4096, 1638, 910, 630, 482,  390,
     315, 256,  315,  390, 482, 630, 910, 1638,
    4096,   0, 4096, 1638, 910, 630, 482,  390,
     315, 256,
};


/*****************************************************************************
 * pixel.cpp: Pixel operator functions
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

UInt32 xSad64xN( Int N, UInt8 *pSrc, UInt nStrideSrc, UInt8 *pRef, UInt nStrideRef )
{
    Int x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<64; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

UInt32 xSad32xN( Int N, UInt8 *pSrc, UInt nStrideSrc, UInt8 *pRef, UInt nStrideRef )
{
    Int x, y;
    UInt32 uiSad = 0;

    for( y=0; y<N; y++ ) {
        for( x=0; x<32; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}

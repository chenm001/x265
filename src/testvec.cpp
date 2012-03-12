/*****************************************************************************
 * testvec.cpp: Test Vector generate and automatic verify functions
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

#if (CHECK_TV)

UInt32 tv_size;
UInt8 tv_top[2][MAX_CU_SIZE*2+1];
UInt8 tv_left[2][MAX_CU_SIZE*2];
UInt8 tv_pred[35][MAX_CU_SIZE*MAX_CU_SIZE];
UInt8 tv_orig[MAX_CU_SIZE*MAX_CU_SIZE];
UInt8 tv_bpred[MAX_CU_SIZE*MAX_CU_SIZE];
Int16 tv_resi[MAX_CU_SIZE*MAX_CU_SIZE];
Int16 tv_trans[MAX_CU_SIZE*MAX_CU_SIZE];
Int16 tv_quant[MAX_CU_SIZE*MAX_CU_SIZE];
Int16 tv_iquant[MAX_CU_SIZE*MAX_CU_SIZE];
Int16 tv_itrans[MAX_CU_SIZE*MAX_CU_SIZE];
UInt8  tv_rec[MAX_CU_SIZE*MAX_CU_SIZE];
UInt32 tv_uiPreds[3];
UInt32 tv_bestmode;
static FILE *fp_tv = NULL;

int tInitTv( const char *fname )
{
    tv_size = -1;
    fp_tv = fopen( fname, "r" );

    if (fp_tv == NULL)
        return -1;
    return 0;
}

void tReadLine8( const char *pcBuf, UInt8 *pData, Int iSize )
{
    UInt32 tmp;
    int i;

    for( i=0; i<iSize; i++ ) {
        if ( sscanf( pcBuf, "%02X ", &tmp) > 0 ) {
            pData[i] = tmp;
            pcBuf += 3;
        }
        else
            abort();
    }
}

void tReadLine16( const char *pcBuf, UInt16 *pData, Int iSize )
{
    UInt32 tmp;
    int i;

    for( i=0; i<iSize; i++ ) {
        if ( sscanf( pcBuf, "%04X ", &tmp) > 0 ) {
            pData[i] = tmp;
            pcBuf += 5;
        }
        else
            abort();
    }
}

void tReadMatrix8( FILE *fp, UInt8 *pData, Int iSize )
{
    int i;
    char buf[1024];

    for( i=0; i<iSize; i++ ) {
        fgets( buf, sizeof(buf), fp );
        tReadLine8( buf, &pData[i * MAX_CU_SIZE], iSize );
    }
}

void tReadMatrix16( FILE *fp, UInt16 *pData, Int iSize )
{
    int i;
    char buf[1024];

    for( i=0; i<iSize; i++ ) {
        fgets( buf, sizeof(buf), fp );
        tReadLine16( buf, &pData[i * MAX_CU_SIZE], iSize );
    }
}

void tGetVector( )
{
    assert( fp_tv != NULL );

    tv_size = -1;
    memset( tv_top,     0xCD, sizeof(tv_top)    );
    memset( tv_left,    0xCD, sizeof(tv_left)   );
    memset( tv_pred,    0xCD, sizeof(tv_pred)   );
    memset( tv_orig,    0xCD, sizeof(tv_orig)   );
    memset( tv_bpred,   0xCD, sizeof(tv_bpred)  );
    memset( tv_resi,    0xCD, sizeof(tv_resi)   );
    memset( tv_trans,   0xCD, sizeof(tv_trans)  );
    memset( tv_quant,   0xCD, sizeof(tv_quant)  );
    memset( tv_iquant,  0xCD, sizeof(tv_iquant) );
    memset( tv_itrans,  0xCD, sizeof(tv_itrans) );
    memset( tv_rec,     0xCD, sizeof(tv_rec)    );
    memset( tv_uiPreds, 0xCD, sizeof(tv_uiPreds));
    tv_bestmode = MODE_INVALID;

    char buf[1024];
    char *pcBuf;
    int i, j;
    UInt32 dummy0, dummy1, dummy2, dummy3;

    // Skip first blank
    fgets( buf, sizeof(buf), fp_tv );

    // Read CU info
    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "========== [%4d](%2d): Size=%2d, Avail=(%d,%d), Cand=(%2d,%2d,%2d)\n",
            &dummy0, &dummy1,
            &tv_size,
            &dummy0, &dummy1,
            &tv_uiPreds[0], &tv_uiPreds[1], &tv_uiPreds[2]
    );
    assert( (tv_size >= 4) && (tv_size <= 64) );

    // Read Reference Pixel Info
    fgets( buf, sizeof(buf), fp_tv );
    tReadLine8( buf, tv_top[0], 2*tv_size+1 );

    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "%02X ", &dummy0 );
    tv_left[0][0] = dummy0;

    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "%02X ", &dummy0 );
    tv_left[0][1] = dummy0;
    tReadLine8( buf+3, tv_top[1], 2*tv_size+1 );

    for( i=0; i<2*tv_size-2; i++ ) {
        fgets( buf, sizeof(buf), fp_tv );
        sscanf( buf, "%02X    %02X", &dummy0, &dummy1 );
        tv_left[0][2+i] = dummy0;
        tv_left[1][  i] = dummy1;
    }
    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "%02X ", &dummy0 );
    tv_left[1][2*tv_size-2] = dummy0;
    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "%02X ", &dummy0 );
    tv_left[1][2*tv_size-1] = dummy0;

    // Read Mode Info
    for( i=0; i<35; i++ ) {
        fgets( buf, sizeof(buf), fp_tv );
        sscanf( buf, "*** Mode=%2d, bFilter=%d, Sad=%6d", &dummy0, &dummy1, &dummy2 );
        assert( dummy0 == i );

        for( j=0; j<tv_size; j++ ) {
            fgets( buf, sizeof(buf), fp_tv );
            tReadLine8( buf, &tv_pred[i][j * MAX_CU_SIZE], tv_size );
        }
    }

    // Skip first blank
    fgets( buf, sizeof(buf), fp_tv );

    // Read Best Mode Number
    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "### BestMode=%d", &tv_bestmode );

    // Read Orig
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix8( fp_tv, tv_orig, tv_size );

    // Read Pred
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix8( fp_tv, tv_bpred, tv_size );

    // Read Residual
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_resi, tv_size );

    // Read Transfrom
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_trans, tv_size );

    // Read Quant
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_quant, tv_size );

    // Read IQuant
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_iquant, tv_size );

    // Read ITransform
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_itrans, tv_size );

    // Read Reconst
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix8( fp_tv, tv_rec, tv_size );
}

#endif

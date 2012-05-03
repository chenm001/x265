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
UInt32 tv_sizeC;
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
UInt32 tv_mostmode[3];
UInt32 tv_bestmode;
UInt32 tv_sad[35];

// Chroma
UInt8 tv_refC[2][MAX_CU_SIZE/2*4+1];
UInt8 tv_refLM[2*MAX_CU_SIZE/2];
UInt8 tv_predC[2][NUM_CHROMA_MODE][MAX_CU_SIZE*MAX_CU_SIZE/4];
UInt8 tv_origC[2][MAX_CU_SIZE*MAX_CU_SIZE/4];
Int16 tv_resiC[2][NUM_CHROMA_MODE][MAX_CU_SIZE*MAX_CU_SIZE/4];
Int16 tv_transC[2][NUM_CHROMA_MODE][MAX_CU_SIZE*MAX_CU_SIZE/4];
Int16 tv_quantC[2][NUM_CHROMA_MODE][MAX_CU_SIZE*MAX_CU_SIZE/4];
Int16 tv_iquantC[2][NUM_CHROMA_MODE][MAX_CU_SIZE*MAX_CU_SIZE/4];
Int16 tv_itransC[2][NUM_CHROMA_MODE][MAX_CU_SIZE*MAX_CU_SIZE/4];
UInt8  tv_recC[2][NUM_CHROMA_MODE][MAX_CU_SIZE*MAX_CU_SIZE/4];
UInt32 tv_mostmodeC[NUM_CHROMA_MODE];
UInt32 tv_dmMode;
UInt32 tv_bestmodeC;
UInt32 tv_nModeC;
UInt32 tv_nIdxC;
UInt32 tv_BestSadC;
UInt32 tv_sadC[2][NUM_CHROMA_MODE];

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

void tReadMatrix8( FILE *fp, UInt8 *pData, Int iSize, UInt uiStride )
{
    int i;
    char buf[1024];

    for( i=0; i<iSize; i++ ) {
        fgets( buf, sizeof(buf), fp );
        tReadLine8( buf, &pData[i * uiStride], iSize );
    }
}

void tReadMatrix16( FILE *fp, UInt16 *pData, Int iSize, UInt uiStride )
{
    int i;
    char buf[1024];

    for( i=0; i<iSize; i++ ) {
        fgets( buf, sizeof(buf), fp );
        tReadLine16( buf, &pData[i * uiStride], iSize );
    }
}

void tGetVector( )
{
    assert( fp_tv != NULL );

    tv_size = -1;
    tv_sizeC = -1;
    tv_dmMode = MODE_INVALID;
    tv_bestmode = MODE_INVALID;
    tv_bestmodeC = MODE_INVALID;
    memset( tv_top,         0xCD, sizeof(tv_top)     );
    memset( tv_left,        0xCD, sizeof(tv_left)    );
    memset( tv_refC,        0xCD, sizeof(tv_refC)    );
    memset( tv_refLM,       0xCD, sizeof(tv_refLM)   );
    memset( tv_pred,        0xCD, sizeof(tv_pred)    );
    memset( tv_predC,       0xCD, sizeof(tv_predC)   );
    memset( tv_orig,        0xCD, sizeof(tv_orig)    );
    memset( tv_origC,       0xCD, sizeof(tv_origC)   );
    memset( tv_bpred,       0xCD, sizeof(tv_bpred)   );
    memset( tv_resi,        0xCD, sizeof(tv_resi)    );
    memset( tv_trans,       0xCD, sizeof(tv_trans)   );
    memset( tv_quant,       0xCD, sizeof(tv_quant)   );
    memset( tv_iquant,      0xCD, sizeof(tv_iquant)  );
    memset( tv_itrans,      0xCD, sizeof(tv_itrans)  );
    memset( tv_rec,         0xCD, sizeof(tv_rec)     );
    memset( tv_mostmode,    0xCD, sizeof(tv_mostmode ));
    memset( tv_mostmodeC,   0xCD, sizeof(tv_mostmodeC));

    char buf[1024];
    char *pcBuf;
    int i, j;
    UInt32 dummy0, dummy1, dummy2, dummy3, dummy4;

    // Skip first blank
    fgets( buf, sizeof(buf), fp_tv );

    // Read CU info
    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "========== [%4d](%2d): Size=%2d, Avail=(%d,%d), Cand=(%2d,%2d,%2d)\n",
            &dummy0, &dummy1,
            &tv_size,
            &dummy0, &dummy1,
            &tv_mostmode[0], &tv_mostmode[1], &tv_mostmode[2]
    );
    // TODO: We don't support 64x64 and automatic check, and HM said the CU can't less than 16.
    assert( (tv_size >= 16) && (tv_size <= 32) );
    tv_sizeC = tv_size >> 1;

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

    for( i=0; i<2*(int)tv_size-2; i++ ) {
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

        tv_sad[i] = dummy2;
        for( j=0; j<tv_size; j++ ) {
            fgets( buf, sizeof(buf), fp_tv );
            tReadLine8( buf, &tv_pred[i][j * MAX_CU_SIZE], tv_size );
        }
    }

    // Skip empty line
    fgets( buf, sizeof(buf), fp_tv );

    // Read Best Mode Number
    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "### BestMode=%d", &tv_bestmode );

    // Read Orig
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix8( fp_tv, tv_orig, tv_size, MAX_CU_SIZE );

    // Read Pred
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix8( fp_tv, tv_bpred, tv_size, MAX_CU_SIZE );

    // Read Residual
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_resi, tv_size, MAX_CU_SIZE );

    // Read Transfrom
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_trans, tv_size, MAX_CU_SIZE );

    // Read Quant
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_quant, tv_size, MAX_CU_SIZE );

    // Read IQuant
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_iquant, tv_size, MAX_CU_SIZE );

    // Read ITransform
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix16( fp_tv, (UInt16*)tv_itrans, tv_size, MAX_CU_SIZE );

    // Read Reconst
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix8( fp_tv, tv_rec, tv_size, MAX_CU_SIZE );

    // Skip empty line
    fgets( buf, sizeof(buf), fp_tv );

    // Read MostModeC
    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "@@@ ChromaMode=(%d,%d,%d,%d,%d,%d)", &tv_mostmodeC[0], &tv_mostmodeC[1], &tv_mostmodeC[2], &tv_mostmodeC[3], &tv_mostmodeC[4], &tv_mostmodeC[5] );

    // Read origU
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix8( fp_tv, tv_origC[0], tv_sizeC, MAX_CU_SIZE/2 );

    // Read origV
    fgets( buf, sizeof(buf), fp_tv );
    tReadMatrix8( fp_tv, tv_origC[1], tv_sizeC, MAX_CU_SIZE/2 );

    // Read Current ModeC and Data
    for( i=0; i<NUM_CHROMA_MODE; i++ ) {
        // skip LM_Chroma
        if ( i == NUM_CHROMA_MODE-2 )
            continue;

        // read ModeC
        fgets( buf, sizeof(buf), fp_tv );
        sscanf( buf, "    ChrMode=%d", &dummy0 );
        if ( i != (NUM_CHROMA_MODE-1) )
            assert( dummy0 == tv_mostmodeC[i] );
        else
            tv_dmMode = dummy0;

        for( j=0; j<2; j++ ) {
            if ( i == 0 ) {
                // Read ReferenceC
                fgets( buf, sizeof(buf), fp_tv );
                assert( strstr(buf, "]") != NULL );
                tReadLine8( buf + strlen("    Reference[0]=["), tv_refC[j], tv_sizeC * 4 + 1 );
            }
            if ( (i == NUM_CHROMA_MODE-2) && (j == 0) ) {
                // Read ReferenceLM
                fgets( buf, sizeof(buf), fp_tv );
                assert( strstr(buf, "]") != NULL );
                tReadLine8( buf + strlen("    ReferenceLM=["), tv_refLM, tv_sizeC * 2 );
            }

            // Read Sad
            fgets( buf, sizeof(buf), fp_tv );
            sscanf( buf, "    Sad=%d", &tv_sadC[j][i] );

            // Read Pred
            fgets( buf, sizeof(buf), fp_tv );
            tReadMatrix8( fp_tv, tv_predC[j][i], tv_sizeC, MAX_CU_SIZE/2 );

            // Read Residual
            fgets( buf, sizeof(buf), fp_tv );
            tReadMatrix16( fp_tv, (UInt16*)tv_resiC[j][i], tv_sizeC, MAX_CU_SIZE/2 );

            // Read Transfrom
            fgets( buf, sizeof(buf), fp_tv );
            tReadMatrix16( fp_tv, (UInt16*)tv_transC[j][i], tv_sizeC, MAX_CU_SIZE/2 );

            // Read Quant
            fgets( buf, sizeof(buf), fp_tv );
            assert( strstr(buf, "Quant") != NULL );
            tReadMatrix16( fp_tv, (UInt16*)tv_quantC[j][i], tv_sizeC, MAX_CU_SIZE/2 );

            // Read IQuant
            fgets( buf, sizeof(buf), fp_tv );
            tReadMatrix16( fp_tv, (UInt16*)tv_iquantC[j][i], tv_sizeC, MAX_CU_SIZE/2 );

            // Read ITransform
            fgets( buf, sizeof(buf), fp_tv );
            tReadMatrix16( fp_tv, (UInt16*)tv_itransC[j][i], tv_sizeC, MAX_CU_SIZE/2 );

            // Read Reconst
            fgets( buf, sizeof(buf), fp_tv );
            tReadMatrix8( fp_tv, tv_recC[j][i], tv_sizeC, MAX_CU_SIZE/2 );
        }
    }

    // Skip empty line
    fgets( buf, sizeof(buf), fp_tv );

    // Read bestModeC and SadC
    fgets( buf, sizeof(buf), fp_tv );
    sscanf( buf, "@@@ BestChromaMode=%d, Sad=%d", &tv_bestmodeC, &tv_BestSadC );
}

#endif

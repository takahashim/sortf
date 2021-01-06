#include <stdio.h>
/* mbjp.c : Multi-byte locale support : Japanese
 * (c) Masayuki TOYOSHIMA, 1994. All rights reserved.
 * feel free to copy and distribute under the GNU Public Licence.
 * send bug reports and suggestions to :
 *     mtoyo@Lit.hokudai.ac.jp, MHA00720@niftyserve.or.jp
 */
 
 static char *copyright=" mbjp.c:copyright (C) TOYOSHIMA, Masayuki, 1991. ";
 static char *version= " v00-02 ";
/*
  *Last_updated= "$Header: /home/mtoyo/sortf/sortf.10b/mbjp.c,v 1.1 1994/08/21 13:26:25 mtoyo Exp mtoyo $";
*/
 
/* ================================================================
 *  Nota bene
 * ----------------------------------------------------------------
 * This file contains Multi-byted Japanese Kanji characters.
 *  o   It can be compiled only by Multi-byte-sensitive compilers.
 *  o   It may cause an unwanted effect on non Multi-byte-sensitive
 *      displays / printers.
 */
 
#include "mtconfig.h"
#include <assert.h>
#define isin_mbjp 1
#include "mbjp.h"
 
    char    *mbjp_isMB_table= (0);
    static int mbjp_count_MB_1st_byte= (0);
 
int mbopen()
{
    BUSY int code;
    BUSY unsigned int byte, beg_1block, beg_2block, end_1block, end_2block;
    BUSY unsigned int beg_KANJI, end_KANJI;
 
    if (mbjp_isMB_table) return mbjp_count_MB_1st_byte;
 
    code= mbtelcd();
    if ((code % 10) == KCODE_SJIS) {
        beg_1block= 0x81; end_1block= 0xa0;
        beg_2block= 0xe0; end_2block= 0xef;
        beg_KANJI=  0x88; end_KANJI=  0xeb;
    } else if ((code % 10) == KCODE_EUC) {
        beg_1block= 0xa1; end_1block= 0xff;
        beg_2block= end_2block= 0;
        beg_KANJI=  0xb0; end_KANJI=  0xf4;
    } else if ((code % 10) == KCODE_JIS_GL) {
        beg_1block= 0x21; end_1block= 0x7f;
        beg_2block= end_2block= 0;
        beg_KANJI=  0x30; end_KANJI=  0x74;
    } else {
        fprintf(stderr," Bad installation:Kanji code unrecognizable%c",
            C_LF);
        assert((code % 10) == KCODE_EUC);
        return EOF;
    }
 
    if (mbjp_isMB_table) return mbjp_count_MB_1st_byte;
    if (!(mbjp_isMB_table= (char*)calloc(sizeof(char),UCHAR_MAX+1)))
        return EOF;
    for(byte= 0; byte < (UCHAR_MAX +1);  byte++) {
        if (((byte >= beg_1block) && (byte < end_1block))
        ||  ((byte >= beg_2block) && (byte < end_2block))) {
            *(mbjp_isMB_table + byte)= 1;
            if ((byte >= beg_KANJI) && (byte < end_KANJI)) {
                *(mbjp_isMB_table + byte)+= 1;
            }
            mbjp_count_MB_1st_byte++;
        }
    }
    return mbjp_count_MB_1st_byte;
}
 
int mbclose()
{
    if (mbjp_isMB_table) free(mbjp_isMB_table);
    mbjp_isMB_table= (char*)0;
    mbjp_count_MB_1st_byte= 0;
    return 0;
}
 
int mbtelcd()
{
    unsigned short int n= 0x0102;
    unsigned char c_low, c_high;
    int endian= 0;
    c_low=  *((unsigned char*)(&n) + sizeof(unsigned short int) - 2);
    c_high= *((unsigned char*)(&n) + sizeof(unsigned short int) - 1);
    endian= ((c_low == 0x02) && (c_high == 0x01)) ? ENDIAN_LITTLE:ENDIAN_BIG;
 
    switch(*((unsigned char *)"‚O")) {  /* 0xa3b0 in EUC,  824f in SJIS */
        case (unsigned)0xa3 : return (endian * 10) + KCODE_EUC;
        case (unsigned)0x82 : return (endian * 10) + KCODE_SJIS;
        case (unsigned)0x23 : return (endian * 10) + KCODE_JIS_GL;
    }
    return 0;
}
 
#if defined(DO_TEST)
 
int main()
{
    int i= mbtelcd();
    char *endian, *code;
 
    switch(i/10) {
        case ENDIAN_LITTLE  :
            endian= "little";   break;
        case ENDIAN_BIG     :
            endian= "big";  break;
        default :
            endian= "unknown";
    }
 
    switch(i%10) {
        case KCODE_EUC      :
            code= "euc";    break;
        case KCODE_SJIS     :
            code= "sjis";   break;
        case KCODE_JIS_GL   :
            code= "jis";    break;
        default :
            code= "unknown";
    }
    printf(" %s endian/%s code\n",endian,code);
}
 
#endif

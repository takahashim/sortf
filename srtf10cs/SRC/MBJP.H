#ifndef mbjp_h
#define mbjp_h          1
 
#define KCODE_EUC       1
#define KCODE_SJIS      2
#define KCODE_JIS_GL    3
#define ENDIAN_LITTLE   1
#define ENDIAN_BIG      2
 
#ifndef isin_mbjp
extern char *mbjp_isMB_table;
#endif
 
#define is_MBYTE(c)     (*(mbjp_isMB_table + (c)))
#define is_KANJI(c)     ((*(mbjp_isMB_table + (c)))>1)
 
#if defined(HAS_FUNCTION_PROTOTYPE)
int mbtelcd(void);
int mbopen(void);
int mbclose(void);
#else
int mbtelcd();
int mbopen();
int mbclose();
#endif
 
#endif

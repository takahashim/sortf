#ifndef sortex_h
#define sortex_h 1
 
#if (defined(HAS_FUNCTION_PROTOTYPE))
int sortex(int (*in_func)(char *),int (*out_func)(const char *, size_t),
    int (*cmp_func)(const UNIV *,const UNIV *, size_t, size_t),
    int memory_KB, int bytes_max, int count_workfl, char *name_workfl);
char *sorterr(int code);
#else
int   sortex();
char *sorterr();
#endif
 
/* error severities */
#define LV_INFO 1   /* informational */
#define LV_WARN 2   /* warnings */
#define LV_SEVR 4   /* severe */
#define LV_FATL 6   /* fatal : must stop at once */
#define LV_INST 8   /* bad install */
#define LV_BUG  9   /* bug */
 
/* error mnemonics */
 
/* general */
#define E_MEMOUT        LV_FATL+10 /* out of memory */
#define E_MEMLARGE      LV_FATL+20 /* too large amount of memory */
#define E_WRITE         LV_FATL+30 /* write eror */
 
#define E_SRSZSMALL     LV_SEVR+3000/*too small memory size for sort*/
#define E_SRSZLARGE     LV_SEVR+3010/*too large memory size for sort*/
#define E_SRWKFEW       LV_SEVR+3020/*too few work files for sort*/
#define E_SRWKMANY      LV_SEVR+3030/*too many work files for sort*/
#define E_SRWKNAME      LV_WARN+3040/*too long work file name cut off*/
#define E_SRNOPROC      LV_FATL+3050/*sort in/out/compare func not given*/
#define E_SROWKOPEN     LV_FATL+3060/*cannot write sort work file */
#define E_SRIWKOPEN     LV_FATL+3070/*cannot read sort work file */
#define E_SRWKWRITE     LV_FATL+3080/*write error to sort work file*/
#define E_SRWKREAD      LV_FATL+3090/*read error from sort work file*/
#define E_SRBWKILL      LV_BUG+3110/*bug : destroying work file */
#define E_SRBWFROTN     LV_BUG+3120/*bug : work file collapse */
#define E_SRBPTRNUL     LV_BUG+3130/*bug : core pointer NULL*/
#define E_SRRCLONG      LV_SEVR+3140/*too long input rec*/
#define E_SRRCMANY      LV_WARN+3150/*too many input recs*/
#define E_SRBWCORE      LV_BUG+3170/*write to not read core */
#define E_SRBWEOF       LV_BUG+3180/*bug:EOF of work file not checked*/
#define E_SRBWDESTROY   LV_BUG+3190/*bug:cannot create exstant work file*/
#define E_SRBWNREC      LV_BUG+3200/*bug:scratch file record count mismatch*/

#endif

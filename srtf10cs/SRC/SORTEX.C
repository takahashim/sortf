#include <stdio.h>
 
#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif
#ifndef RECL_MAX
#define RECL_MAX 1024
#endif
 /* sortex.c : sort using disk IO.
  * copyright (C) Masayuki TOYOSHIMA,1988. All rights reserved.
  * feel free to copy and distribute under the GNU Public Licence.
  * send bug reports and suggestions to :
  *     mtoyo@Lit.hokudai.ac.jp, MHA00720@niftyserve.or.jp
  */
 
static char *copyright= " sortex.c copyright (C) Masayuki TOYOSHIMA,1988. ";
static char *version=   " v03-23 ";
static char *Last_updated=
    " $Header: /home/mtoyo/sortf/sortf.10c/sortex.c,v 1.1 1994/08/27 08:01:02 mtoyo Exp mtoyo $ ";
 
#include <assert.h>
#include "mtconfig.h"
#include "sortex.h"
 
#if 0
 ========= Suggestions for compilation ===========
 
    a. under UNIX and GNU c compiler or UNIX pcc
        1. gcc/cc -c sortex.c
    b. under MS-DOS and Visual C++ compiler
        1. cl -AC -Oielt -Ob2 -W3 -Zl -YX -FPa -c sortex.c  [-FPa is optional]
    c. under MS-DOS and Borland c++ compiler
        1. bcc -mc -O -N -d -c sortex.c
    d. under Macintosh OS and Symantec c++
        1. just make a project and compile it :-)
    e. under Human68k and gcc (no support from the author)
        1. rename mtconfig.68k as mtconfig.h
        2. make -f makefile.68k sortex.o
    f. other cases
        God helps those who help themselves.
#endif
 
    /*--------------------------------------*
     *                                      *
     *      OS dependent informations       *
     *                                      *
     *--------------------------------------*
     */
 
    /* 1. MS-DOS  for i80x86 */
#if (!defined(sort_cfg) && defined(OS_MSDOS))
#define sort_cfg        1
#define WK_F_MAX        16          /* max count of work files */
#define WK_CORE_MAX     (1000/64)   /* max count of core buffers
                                     * 1mb / 1 segment (for i80x86) */
#define WK_FNAME_MAX    68          /* max length of file name */
 
#if (defined(CPP_Microsoft))
                    /* get available memory */
#include <dos.h>
#define f_memsz(V)      get_max_memorysize()
                        DCL_PRIVATE unsigned long get_max_memorysize(void);
#endif
 
#if (defined(CPP_Borland))
#if 0
#include <alloc.h>
#endif
#define f_memsz(V)      coreleft()
                        extern unsigned long coreleft(void);
#endif
 
#if (!defined(f_memsz))
#define DFLT_CORE_KB    220 /*  total core size in KB, effective only
                                when available memory size is unknown */
#endif
 
#define DFLT_N_WORK     8   /* count of work_files */
#define CORE1_MAX       ((size_t)USHRT_MAX - 400)
                        /* max portion of core obtainable at 1 time.
                         * NB : this is less than USHRT_MAX in MS-DOS,
                         * because MCB itself needs some control area.
                         * Turbo c 1.5/2.0's library bug comes from here.
                         * Their malloc(USHRT_MAX) gives not reliable pointer.
                         * cf. MSC4.0/5.1/DLC/ZTC returns NULL.
                         */
#define bin_fopen(F,S)  ((*(S)=='w')?fopen((F),"wb"):fopen((F),"rb"))
#define f_unlink(S)     unlink(S)       /* delete file by name */
 
#if (defined(CPP_Microsoft))
#define f_tmpnam(S)     tempnam("","")  /* looks for env. variable "TMP" */
#else
#define f_tmpnam(S)     tmpnam(S)
#endif
 
typedef unsigned short  LENGTH; /* for bytes, not necessarily signed type */
#define LENGTH_MAX      (size_t)USHRT_MAX
typedef unsigned long   REC_COUNT; /* not necessarily signed type */
#define REC_CN_MAX      ((REC_COUNT)ULONG_MAX)
#endif
 
 
    /* 2. OS/2 */
#if (!defined(sort_cfg) && defined(OS_OS2))
#define sort_cfg        1
#define WK_F_MAX        16      /* max count of work files */
#define WK_CORE_MAX     1       /* no core partitions */
#define WK_FNAME_MAX    256     /* max length of file name */
 
#define DFLT_CORE_KB    800     /*  total core size in KB, too much ? */
 
#define DFLT_N_WORK     8   /* count of work_files */
#define CORE1_MAX       ((size_t)SIZE_T_MAX - 8000)
                        /* max portion of core obtainable at 1 time */
#define bin_fopen(F,S)  ((*(S)=='w')?fopen((F),"wb"):fopen((F),"rb"))
#define f_unlink(S)     unlink(S)       /* delete file by name */
 
#if (defined(CPP_Microsoft)||defined(CPP_IBM))
#define f_tmpnam(S)     tempnam("","")  /* looks for env. variable "TMP" */
#else
#define f_tmpnam(S)     tmpnam(S)
#endif
 
typedef unsigned short  LENGTH; /* for bytes, not necessarily signed type */
#define LENGTH_MAX      (size_t)USHRT_MAX
typedef unsigned long   REC_COUNT; /* not necessarily signed type */
#define REC_CN_MAX      ((REC_COUNT)ULONG_MAX)
#endif
 
    /* 3. UNIX 4.2/3 bsd or system V */
#if (!defined(sort_cfg) && defined(OS_UNIX))
#define sort_cfg        1
#define WK_F_MAX        12
#define WK_CORE_MAX     1       /* no core partitions */
#define WK_FNAME_MAX    128
        /* no f_memsz() in virtual memory OS */
#define DFLT_CORE_KB    800     /* too much to ask ? */
#define DFLT_N_WORK     8
#define CORE1_MAX       ((size_t)SIZE_T_MAX - 8000)
#define bin_fopen(F,S)  fopen((F),(S))
#define f_unlink(S)     unlink(S)
#if (defined(OS_NeXT))
        static char     mktemp_prefix[]= "/tmp/sortex.wXXXXXX";
        extern char     *mktemp();
#define f_tmpnam(S)     mktemp(mktemp_prefix)
        /* tmpnam() of gcc installed in NeXT-station does not check if
         * generated file name coincides with existing ones (!).
         * Thus we use mktemp() here instead of tmpnam().
         */
#else
#define f_tmpnam(S)     tmpnam(S)
#endif
typedef size_t          LENGTH;     /* fix 92/02/19 */
#define LENGTH_MAX      ((size_t)SIZE_T_MAX)
typedef unsigned long   REC_COUNT;
#define REC_CN_MAX      ((REC_COUNT)ULONG_MAX)
#endif
 
   /* 4. Macintosh OS/Think c/Symantec c++ */
#if (!defined(sort_cfg) && defined(OS_MAC))
#define sort_cfg        1
#define WK_F_MAX        8
#define WK_CORE_MAX     1
#define WK_FNAME_MAX    128
#define f_memsz(V)      get_max_memorysize()
                        DCL_PRIVATE unsigned long get_max_memorysize(void);
#define DFLT_N_WORK     6
#define CORE1_MAX       ((size_t)SIZE_T_MAX - 1 - 2000)
#define DFLT_CORE_KB    ((CORE1_MAX)/1000)
#define f_unlink(S)     unlink(S)
#define f_tmpnam(S)     tmpnam(S)
#define bin_fopen(F,S)  ((*(S)=='w')?fopen((F),"wb"):fopen((F),"rb"))
typedef size_t          LENGTH;         /* fix 92/02/19 */
#define LENGTH_MAX      ((size_t)SIZE_T_MAX)
typedef unsigned long   REC_COUNT;
#define REC_CN_MAX      ((REC_COUNT)ULONG_MAX)
#endif
 
    /* 5. Human68k: with GCC  (*/
#if (!defined(sort_cfg) && defined(OS_HUMAN))
#define sort_cfg        1
#define WK_F_MAX        16
#define WK_CORE_MAX     1
#define WK_FNAME_MAX    128
#define f_memsz(V)      (min(sizmem()*4L, 128*1024)) /* at most 128K */
                        extern long sizmem(void); /* sizmem() returns in words */
#define DFLT_N_WORK     8
#define CORE1_MAX       ((size_t)SIZE_T_MAX - 1 - 2000)
#define bin_fopen(F,S)  ((*(S)=='w')?fopen((F),"wb"):fopen((F),"rb"))
#define f_unlink(S)     unlink(S)
                        extern int unlink(const char *);
#define f_tmpnam(S)     Human_tmpnam(S)
                        DCL_PRIVATE char *Human_tmpnam(char *);
typedef unsigned int    LENGTH;
#define LENGTH_MAX      (size_t)UINT_MAX
typedef unsigned long   REC_COUNT;
#define REC_CN_MAX      ((REC_COUNT)ULONG_MAX)
#endif
 
    /* 6. HITAC VOS3 (no support) */
#if (!defined(sort_cfg) && defined(OS_VOS3))
#define sort_cfg        1
#define WK_F_MAX        12
#define WK_CORE_MAX     1
#define WK_FNAME_MAX    68
#if (defined(CPP_C370))
#define f_memsz(V)  (unsigned)(malloc(V))
        /* in C370/VOS3 c compiler, malloc(0) returns available memory size.
         * this is contradicting ANSI, but convenient ^^;
         */
#else
#define bin_fopen(F,S)  ((*(S)=='w')?fopen((F),"wb"):fopen((F),"rb"))
#define DFLT_CORE_KB    800
#endif
#define DFLT_NAME_WORK  "\srwk" /* NB. in VOS3 back_slash is $, not \ */
#define DFLT_N_WORK     6
#define CORE1_MAX       ((size_t)SIZE_T_MAX - 1 - 8000)
    /* no f_unlink() :  work files are kept as is */
    /* no f_tmpnam() :  work files must be allocated before */
typedef int             LENGTH;
#define LENGTH_MAX      ((size_t)INT_MAX)
    /* CAUTION!
     * C370 compiler has bug in 'register' declaration of 'short' types.
     * NEVER use '[unsigned] short int' types.
     */
typedef unsigned long   REC_COUNT;
#define REC_CN_MAX      ((REC_COUNT)ULONG_MAX)
#endif
 
    /* 7. other OS's : keep fingers crossed */
#if (!defined(sort_cfg))
#define WK_F_MAX        8
#define WK_CORE_MAX     8
#define WK_FNAME_MAX    128
#define DFLT_CORE_KB    300
#define DFLT_NAME_WORK  "srwk"
#define DFLT_N_WORK     4
#define CORE1_MAX       ((size_t)SIZE_T_MAX - 1 - 4000)
        /* no f_unlink() */
        /* no f_tmpnam() */
        /* no f_memsz() */
typedef size_t          LENGTH;
#define LENGTH_MAX      ((size_t)SIZE_T_MAX)
typedef long int        REC_COUNT;
#define REC_CN_MAX      ((REC_COUNT)LONG_MAX)
#endif
 
#if 0
#if (DO_DEBUG && defined(f_unlink))
#undef f_unlink
#endif
#endif
 
#if (!defined(SRT_GVUP))
#define SRT_GVUP    26      /* if size < SRT_GVUP giveup quick-sort */
#endif
 
    /*--------------------------------------*
     *                                      *
     *      struct/type definitions         *
     *                                      *
     *--------------------------------------*
     */
 
    typedef struct {
        int          fl_strata;             /* count strata of sorted runs */
        REC_COUNT    fl_recs;               /* recs of current strarum */
        FILE        *fl_file;               /* NULL when closed */
        UCHAR       *fl_buf;                /* buffer for input rec  */
        char         fl_name[WK_FNAME_MAX +1];/* file real name : +1 for EOS */
        char         fl_mode;               /* open mode:'r'/'w'/NULL */
    } file_type;
 
    typedef struct {
        size_t        cr_recs;  /* written recs */
        char          cr_open;  /* 1 when opened, 0 else */
        UCHAR       **cr_head,  /* points head of pointers to strings */
                    **cr_next;  /* next point to read */
    } core_type;
 
    typedef struct {
        UCHAR       **sr_buf[WK_CORE_MAX];  /* internal sort buffer */
        file_type    sr_file[WK_F_MAX];     /* sort work files */
        core_type    sr_core[WK_CORE_MAX];  /* cores */
        int          sr_n_file,             /* number of work files */
                     sr_n_core,             /* number of cores */
                     sr_bytes_max;          /* max 1 rec length */
        size_t       sr_sz_pointer;         /* max usable pointers in 1 core */
#if (defined(HAS_FUNCTION_PROTOTYPE))
        int         (*sr_cmp_func)(const UNIV *,const UNIV*,size_t,size_t);
                                            /* compare function */
#else
        int         (*sr_cmp_func)();
#endif
        int          sr_errno;              /* severe error number */
        char        *sr_orig_fname;         /* user defined work file name */
    } sort_type;
 
    typedef struct {
        UCHAR       *mg_str;            /* next string to merge */
#if (defined(HAS_FUNCTION_PROTOTYPE))
        UCHAR       *(*mg_f)(int, sort_type *); /* input function */
#else
        UCHAR       *(*mg_f)();
#endif
        int         mg_numb;            /* input stream number */
    } merge_type;
 
#if (defined(HAS_FUNCTION_PROTOTYPE))
    DCL_PRIVATE UCHAR  *next_core(int, sort_type *);
    DCL_PRIVATE UCHAR  *next_file(int, sort_type *);
    DCL_PRIVATE int     sort_core(int, int (*)(char *),sort_type*);
    DCL_PRIVATE int     put_1_core(int, sort_type *);
    DCL_PRIVATE int     merge_recs(int, int, REC_COUNT,
                        int (*)(const char*,size_t), merge_type*, sort_type *);
    DCL_PRIVATE void    re_heap(merge_type *, int,
                            int (*)(const UNIV*,const UNIV*, size_t, size_t));
    DCL_PRIVATE int     make_1st_heap(merge_type *,int,
                            int (*)(const UNIV*,const UNIV*, size_t, size_t));
    DCL_PRIVATE int     merge_core(int, sort_type *);
    DCL_PRIVATE int     merge_file(int, sort_type *);
    DCL_PRIVATE int     final_merge(int (*)(const char*,size_t), sort_type *);
    DCL_PRIVATE int     core_ropen(int, sort_type *);
    DCL_PRIVATE int     file_ropen(int, sort_type *);
    DCL_PRIVATE FILE   *fopen_work(file_type *, char*, int, sort_type*);
    DCL_PRIVATE int     fclose_work(file_type *, int, sort_type*);
    DCL_PRIVATE int     fclose_all_work(sort_type *, int);
    DCL_PRIVATE int     put_header(FILE *, REC_COUNT, sort_type*);
    DCL_PRIVATE void    do_qsort(UCHAR **,UCHAR **,
                            int (*)(const UNIV*,const UNIV*, size_t, size_t));
    DCL_PRIVATE int     sort_prologue(sort_type *,int,int,int,char*);
    DCL_PRIVATE int     sort_epilogue(sort_type *);
    DCL_PRIVATE void    do_tournament(UCHAR **, UCHAR **,
                            int (*)(const UNIV*,const UNIV*, size_t, size_t));
    DCL_PRIVATE int     get_err_level(int);
#else
    DCL_PRIVATE UCHAR   *next_core(), *next_file();
    DCL_PRIVATE int      sort_core(), put_1_core(), put_header(),
                         merge_core(), merge_file(),
                         merge_recs(), final_merge();
    DCL_PRIVATE int      sort_prologue(), core_ropen(), file_ropen(),
                         sort_epilogue();
    DCL_PRIVATE FILE    *fopen_work();
    DCL_PRIVATE int      fclose_work(), fclose_all_work();
    DCL_PRIVATE int      make_1st_heap();
    DCL_PRIVATE void     re_heap();
    DCL_PRIVATE void     do_qsort(), do_tournament();
    DCL_PRIVATE int      get_err_level();
#endif
 
#ifndef bin_fopen
#if (defined(HAS_FUNCTION_PROTOTYPE))
    DCL_PRIVATE FILE    *bin_fopen(FILE *,char *);
#else
    DCL_PRIVATE FILE    *bin_fopen();
#endif
#endif
 
#define as_len(S) (size_t)(*((LENGTH*)(S)))
#ifndef max
#define max(a,b)    (((a)<(b))?(b):(a))
#endif
#ifndef min
#define min(a,b)    (((a)>(b))?(b):(a))
#endif
 
    /* size of record length descriptor : must be padded to alignment size */
#if (defined(SZ_ALIGN))
#define SZ_HEADER   max(sizeof(LENGTH),SZ_ALIGN)
#else
#define SZ_HEADER   sizeof(LENGTH)  /* no alignment necessary */
#endif
 
 
int sortex(fc_in,fc_out,fc_cmp,sz_KB,bytes_max,file_max,name_file)
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int (*fc_in)(char *),
        (*fc_out)(const char *,size_t),
        (*fc_cmp)(const UNIV *, const UNIV *, size_t, size_t);
#else
    int (*fc_in)(),(*fc_out)(),(*fc_cmp)();
#endif
    int     sz_KB;
    int     bytes_max;
    int     file_max;
    char    *name_file;
{
    sort_type    info; /* sort informations on stack : sortex() is re-entrant */
 
    BUSY int     err= 0, n_out, n_run, n_turn, n_core;
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int         (*put_core)(int, sort_type*);
#else
    int         (*put_core)();
#endif
 
#if (DO_DEBUG)
    fprintf(stderr,"%c This is sortex() [%s] %s%c",C_LF,copyright,version,C_LF);
    fprintf(stderr," compiled on %s (%s)%c%c",__DATE__,__TIME__,C_LF,C_LF);
#endif
 
    if (!fc_in || !fc_out || !fc_cmp)               return E_SRNOPROC;
    if (err=sort_prologue(&info,sz_KB,bytes_max,file_max,name_file)) goto done;
 
    info.sr_cmp_func= fc_cmp;
    file_max= info.sr_n_file; n_core= info.sr_n_core;
    put_core= (n_core > 1)? merge_core : put_1_core;
 
        /* 1. put sorted runs into even-number work files */
    for(n_run= 0; ; n_run++) {
        for(n_out= 0; n_out < file_max; n_out+= 2) {
            BUSY int i;
            for(i= 0; i < n_core; i++) {
                if ((err= sort_core(i,fc_in,&info)) < 0)    goto to_and_fro;
                else if (err)                               goto done;
            }
#if (DO_DEBUG)
            fprintf(stderr," :writing work#%d-%d(%s)...%c",
                n_out,n_run,info.sr_file[n_out].fl_name,C_LF);
#endif
            if (err= (*put_core)(n_out,&info))              goto done;
#if (DO_DEBUG)
            fprintf(stderr," :writing work#%d-%d(%s) done.%c",
                n_out,n_run,info.sr_file[n_out].fl_name,C_LF);
#endif
        }
    }
        /* 2. merge work files to & fro */
  to_and_fro:
    n_turn= 0;
    if (get_err_level(info.sr_errno)>= LV_SEVR)             goto done;
    if (err= fclose_all_work(&info,0))                      goto done;
#if (DO_DEBUG)
    if (n_run < 1) fprintf(stderr," :bypass merge phase...%c",C_LF);
#endif
    while(n_run > 0) {
        for(n_run= 0; ; n_run++) {
            for(n_out= (n_turn + 1) % 2; n_out < file_max; n_out+= 2) {
                if (err= file_ropen(n_turn,&info))  goto merge_end;
                if (err= merge_file(n_out,&info))   goto merge_end;
#if (DO_DEBUG)
                fprintf(stderr," : merge out(%s) done. strata=%d.%c",
                    info.sr_file[n_out].fl_name,
                    info.sr_file[n_out].fl_strata,C_LF);
#endif
            }
        }
     merge_end:
        if (err > 0)    /* severe error */          goto done;
            /* clear EOF of err */
        if (err= fclose_all_work(&info,0))          goto done;
        n_turn= (n_turn + 1) % 2;
    }
  done: /* 3. work files merged */
    if ((err < 1) && (get_err_level(info.sr_errno) < LV_SEVR)) {
#if (DO_DEBUG)
        fprintf(stderr," :final merge...%c",C_LF);
#endif
        err= final_merge(fc_out,&info);
#if (DO_DEBUG)
        fprintf(stderr," :final merge done.%c",C_LF);
#endif
    }
#if (DO_DEBUG)
    else {
        if (get_err_level(err) > get_err_level(info.sr_errno))
            info.sr_errno= err;
        fprintf(stderr," : quitting sortex() due to severe errors...%c",C_LF);
    }
#endif
    fclose_all_work(&info,1);
#if (DO_DEBUG)
    fprintf(stderr," :work files closed.%c",C_LF);
#endif
    err= sort_epilogue(&info);
#if (DO_DEBUG)
    fprintf(stderr,":epilogue done err=%d.%c",err,C_LF);
#endif
    return err;
}
 
PRIVATE
int sort_core(core_out,fc_in,info) int core_out;
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int (*fc_in)(char *);
#else
    int (*fc_in)();
#endif
    sort_type *info;
{
    BUSY core_type  *core;
    BUSY UCHAR      **rec_head;
    BUSY int         len, bytes_max;
    BUSY size_t      max_possible;
    int              ret_val= 0;
#if (DO_DEBUG)
    long int         recs_per_core= 0L;
#endif
 
        /* 1. set internal buffer */
    if (!(rec_head= info->sr_buf[core_out])) {
        fprintf(stderr,"%cbug:sortex:sr_buf[%d] is NULL ptr.%c",
            C_LF,core_out,C_LF);
#if 0
        for(i=0; i<=core_out; i++) {
            fprintf(stderr,"sr_buf[%d]=(%p)%c",i,(info->sr_buf[i]),C_LF);
        }
#endif
        return info->sr_errno= E_SRBPTRNUL;
    }
    bytes_max= info->sr_bytes_max;
    core= info->sr_core + core_out;
    if (core->cr_open) {
        fprintf(stderr,"%cbug:sortex:sort_core:write to input core #%d.%c",
            C_LF,core_out,C_LF);
                                        return info->sr_errno= E_SRBWCORE;
    }
    core->cr_head= core->cr_next= rec_head + info->sr_sz_pointer -1;
    core->cr_recs= (size_t)0; core->cr_open= 1;
#if (DO_DEBUG)
#if defined(OS_HUMAN)
    fprintf(stderr," :core sort: top(%06x)-bottom(%06x)=%ld ptrs(%u bytes).%c",
#else
    fprintf(stderr," :core sort: top(%p)-bottom(%p)=%ld ptrs(%u bytes).%c",
#endif
        core->cr_next +1,rec_head,(long int)((core->cr_next+1) - rec_head),
        (unsigned)((char*)(core->cr_next+1) - (char*)rec_head),C_LF);
#endif
 
        /* 2. fill into buffer :
         *    string from left, pointer from right, while both may not cross.
         */
    max_possible= (size_t)bytes_max + SZ_HEADER;
 
    while((char*)rec_head < ((char*)core->cr_next - max_possible)) {
            /* bug of pointer wrap-around fixed on 92/08/09 */
#if (DO_DEBUG)
        if ((DO_DEBUG > 1) && ((recs_per_core % DO_DEBUG)==1)) {
            fprintf(stderr," #%ld head(%p)+possible(%u)=%p, cr_next=%p.\n",
            recs_per_core,rec_head,(unsigned)max_possible,
            (char*)rec_head+max_possible,core->cr_next);
        }
#endif
        if ((len= (*fc_in)((char*)rec_head+ SZ_HEADER))< 0){/* input EOF*/
            if (len == EOF) ret_val= EOF;
            else    /* fix 94/04/25 */      return info->sr_errno= E_SRRCLONG;
            break;
        }
        if (len >= bytes_max)               return info->sr_errno= E_SRRCLONG;
        *((LENGTH*)((UNIV*)rec_head))= (LENGTH)len;
        *(core->cr_next)= (UCHAR*)rec_head; (core->cr_next)--;
        rec_head = (UCHAR**)add_ALIGN(rec_head,len + SZ_HEADER);
                /* align word boundary : fix : 91/02/27 */
#if (DO_DEBUG)
        recs_per_core++;
#endif
    }
#if (DO_DEBUG)
    fprintf(stderr," :fill core#%d done:%ld.%c",core_out,recs_per_core,C_LF);
#endif
        /* 3. sort buffer */
    if (core->cr_head > core->cr_next) {
        core->cr_recs= (size_t)(core->cr_head - core->cr_next);
#if (DO_DEBUG)
        if (recs_per_core != (long)(core->cr_recs)) {
          fprintf(stderr,"%c?bug?: core#%d:recs_per_core(%ld)!=cr_recs(%u)%c",
            core_out,recs_per_core,core->cr_recs,C_LF);
        }
#endif
        do_qsort(core->cr_next + 1,core->cr_head,info->sr_cmp_func);
    }
    core->cr_open= 0;
    return ret_val;
}
 
PRIVATE
int put_1_core(n_out,info) int n_out; BUSY sort_type *info;
{                   /* when only 1 core exists : bypass core merge */
    BUSY UCHAR  *s_min;
    BUSY FILE   *fout;
    file_type   *fl_out;
    int         err;
 
    if (err= core_ropen(info->sr_n_core,info))  return err;
    fl_out= info->sr_file + n_out;
    if (!(fout= fl_out->fl_file) && !(fout= fopen_work(fl_out,"w",n_out,info)))
                                        return info->sr_errno;
    else if (err= put_header(fout,(REC_COUNT)(info->sr_core->cr_recs),info))
                                                                return err;
    if (fl_out->fl_strata >= INT_MAX) { /* very very large file */
                                        return info->sr_errno= E_SRRCMANY;
    } else fl_out->fl_strata+= 1;
    while(s_min= next_core(0,info)) {
        BUSY size_t n_elem= (size_t)(*((LENGTH*)s_min)) + SZ_HEADER;
        if (fwrite((UNIV*)s_min,sizeof(UCHAR),n_elem,fout)< n_elem) {
            /* work file write error */
                                        return info->sr_errno= E_SRWKWRITE;
        }
    }
    return info->sr_errno;
}
 
PRIVATE
int merge_recs(n_out,n_source,n_recs,func_out,merge,info)
    BUSY int n_out; BUSY int n_source;  REC_COUNT n_recs;
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int (*func_out)(const char*,size_t);
#else
    int (*func_out)();
#endif
    merge_type *merge; sort_type *info;
{
    BUSY FILE       *fout= 0;
    BUSY file_type  *fl_out= 0;
    BUSY merge_type *mg;
    BUSY int         i;
    REC_COUNT        n_written= 0;
 
        /* 1. get 1st records & make initial heap */
    for(mg= merge, i= 0; i < n_source; i++, mg++)
        mg->mg_str= (*(mg->mg_f))(mg->mg_numb,info);
    n_source= make_1st_heap(merge,n_source,info->sr_cmp_func);
    if (n_source < 1)       /* nothing to merge */              return 0;
        /* 2. open output file */
    if (n_out>= 0) {
        BUSY int err;
        fl_out= info->sr_file + n_out;
        if (!(fout= fopen_work(fl_out,"w",n_out,info))) return info->sr_errno;
        if (err= put_header(fout,n_recs,info))          return err;
    }
#if (DO_DEBUG)
    fprintf(stderr," : merging into #%d(%s) %ld recs.%c",n_out,
        (n_out>=0)?info->sr_file[n_out].fl_name:"final",(long)n_recs,C_LF);
#endif
        /* 3. merge body */
    while(n_source > 0) {           /* while sources exist */
#define s_min   (merge->mg_str)                 /* smallest str at heap top */
#define len_s   ((size_t)(*((LENGTH*)s_min)))   /* length of s_min */
#define len_elem (len_s  + SZ_HEADER)
        if (n_out >= 0) {                       /* write work file */
                    /* write smallest source */
            if (fwrite((UNIV*)s_min,sizeof(UCHAR),len_elem,fout)< len_elem) {
                    /* work file write error */
                info->sr_errno= E_SRWKWRITE;               break;
            }
            n_written++;
        } else  {   /* final merge : give pointer to string itself */
            if ((*func_out)((char*)s_min+SZ_HEADER,len_s)==EOF)return EOF;
        }
            /* 4. read from that smallest source */
        if (!(merge->mg_str= (*(merge->mg_f))(merge->mg_numb,info))){ /* EOF */
            if (info->sr_errno > 0) /* error in work file */        break;
            if (--n_source > 0) /* temp. assume bottom of heap as heap top */
                memcpy(merge,merge + n_source,sizeof(merge_type));
        }
        re_heap(merge,n_source,info->sr_cmp_func);
    }
    if (fl_out) fl_out->fl_strata+= 1;
    if ((n_out>= 0)&&(n_written != n_recs)&&(info->sr_errno != E_SRWKWRITE)){
        fprintf(stderr,"%cbug:sortex():merge_recs:work output dif%c",C_LF,C_LF);
        fprintf(stderr," I expected to write %ld, but have written %ld,%c",
            (long)n_recs,(long)n_written,C_LF);
        info->sr_errno= E_SRBWNREC;
    }
    return info->sr_errno;
#undef s_min
#undef len_s
#undef len_elem
}
 
PRIVATE
void re_heap(merge,n_merge,f_cmp) merge_type *merge; int n_merge;
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int (*f_cmp)(const UNIV *,const UNIV *,size_t,size_t);
#else
    int (*f_cmp)();
#endif
{
    BUSY int child, mother;
 
    for(mother= 0; (child= (mother * 2) + 1) < n_merge; mother= child) {
        BUSY merge_type *mg_c, *mg_m;
        mg_c= merge + child;
        if ((child + 1) < n_merge) {    /* find smaller child */
            if ((*f_cmp)(mg_c->mg_str+SZ_HEADER,
                    (mg_c + 1)->mg_str + SZ_HEADER,
                    as_len(mg_c->mg_str),as_len((mg_c+1)->mg_str)) > 0) {
                child++; mg_c++;        /* child at right is smaller */
            }
        }
        mg_m= merge + mother;
        if ((*f_cmp)(mg_m->mg_str+SZ_HEADER,
                mg_c->mg_str+SZ_HEADER,
                as_len(mg_m->mg_str),as_len(mg_c->mg_str)) <= 0) {
            break;                      /* in proper place */
        } else {    /* swap */
            merge_type tmp;
            memcpy(&tmp,mg_c,sizeof(merge_type));
            memcpy(mg_c,mg_m,sizeof(merge_type));
            memcpy(mg_m,&tmp,sizeof(merge_type));
        }
    }
}
 
PRIVATE
int make_1st_heap(merge,n_merge,f_cmp) BUSY merge_type *merge; int n_merge;
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int (*f_cmp)(const UNIV *,const UNIV *,size_t,size_t);
#else
    int (*f_cmp)();
#endif
{
    BUSY merge_type *mg;
    BUSY int         i, n_buf= 0;
 
        /* squeeze NULL leaves */
    for(i= 0, mg= merge; i < n_merge; i++, mg++) {
        if (mg->mg_str) {
            if (i > n_buf) memcpy(merge + n_buf,mg,sizeof(merge_type));
            n_buf++;
        }
    }
    if (n_buf > 0) {
            /* sort leaves */
        for(i= 0; (i + 1) < n_buf; i++) {
            BUSY UCHAR *s_min;
            BUSY int    j, n_min;
 
            n_min= i; s_min= (merge + n_min)->mg_str;
            for(j= i + i; j < n_buf; j++) {
                if ((*f_cmp)(s_min + SZ_HEADER,
                        (merge + j)->mg_str + SZ_HEADER,
                        as_len(s_min),as_len((merge + j)->mg_str)) > 0) {
                    n_min= j; s_min= (merge + n_min)->mg_str;
                }
            }
            /* swap */
            if (n_min != i) {
                merge_type  tmp;
                memcpy(&tmp,merge + n_min,sizeof(merge_type));
                memcpy(merge + n_min,merge + i,sizeof(merge_type));
                memcpy(merge + i,&tmp,sizeof(merge_type));
            }
        }
    }
    return n_buf;
}
 
PRIVATE
int merge_core(n_out,info) int n_out; sort_type *info;
{
    BUSY merge_type *mg;
    BUSY int         i, n_core, err;
    REC_COUNT        n_recs= 0;
    merge_type       merge_info[WK_CORE_MAX];
 
    n_core= info->sr_n_core;
    if (err= core_ropen(n_core,info)) return err;
    for(i= 0, mg= merge_info; i< n_core; i++, mg++) {
        BUSY REC_COUNT n_add;
        mg->mg_numb= i;
        n_add= (REC_COUNT)((info->sr_core + i)->cr_recs);
        if ((REC_CN_MAX - n_recs) < n_add) {    /* very large file */
            return info->sr_errno= E_SRRCMANY;
        }
        n_recs+= n_add;
        mg->mg_str= (UCHAR*)0;
        mg->mg_f= next_core;
    }
    return merge_recs(n_out,n_core,n_recs,(int (*)())0,merge_info,info);
}
 
PRIVATE
int merge_file(n_out,info) int n_out; sort_type *info;
{
    BUSY merge_type *mg;
    BUSY int         i, n_file;
    REC_COUNT        n_recs= 0;
    merge_type       merge_info[WK_F_MAX];
 
    n_file= info->sr_n_file;
    for(i= 0, mg= merge_info; i< n_file; i++, mg++) {
        BUSY REC_COUNT n_add;
        mg->mg_numb= i;
        n_add= (REC_COUNT)((info->sr_file + i)->fl_recs);
        if ((REC_CN_MAX - n_recs) < n_add) {    /* very large file */
            return info->sr_errno= E_SRRCMANY;
        }
        n_recs+= n_add;
        mg->mg_str= (UCHAR*)0;
        mg->mg_f= next_file;
    }
    return merge_recs(n_out,n_file,n_recs,(int (*)())0,merge_info,info);
}
 
PRIVATE
int final_merge(func_out,info)
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int (*func_out)(const char *,size_t);
#else
    int (*func_out)();
#endif
sort_type *info;
{
    BUSY merge_type *mg;
    BUSY int         n_core, n_file, i, err;
    REC_COUNT        n_recs= 0;
    merge_type       merge_info[WK_CORE_MAX + WK_F_MAX];
 
    n_core= info->sr_n_core; n_file= info->sr_n_file;
    if (err= core_ropen(n_core,info))   return err;
    if ((err= file_ropen(0,info)) > 0)  return err;
    if ((err= file_ropen(1,info)) > 0)  return err;
    for(i= 0, mg= merge_info; i< n_core; i++, mg++) {
        mg->mg_numb= i;
        n_recs+= (REC_COUNT)((info->sr_core + i)->cr_recs);
        mg->mg_str= (UCHAR*)0;
        mg->mg_f= next_core;
    }
    for(i= 0; i < n_file; i++, mg++) {
        mg->mg_numb= i;
        n_recs+= (info->sr_file + i)->fl_recs;
        mg->mg_str= (UCHAR*)0;
        mg->mg_f= next_file;
    }
    return merge_recs(-1,n_core + n_file,n_recs,func_out,merge_info,info);
}
 
PRIVATE
UCHAR *next_core(n_core,info) BUSY int n_core; BUSY sort_type *info;
{
    BUSY core_type *core= info->sr_core + n_core;
    if (core->cr_recs < 1) {    /* EOF */
        core->cr_open= 0;   return (UCHAR*)0;
    }
    (core->cr_recs)--;
    return *((core->cr_next)--);
}
 
PRIVATE
UCHAR *next_file(n_file,info) BUSY int n_file; sort_type *info;
{
#define ERR_RETURN  ((UCHAR*)0)
    BUSY file_type  *fl;
    BUSY size_t      len;
    BUSY UCHAR      *s;
    BUSY FILE       *f;
 
    fl= info->sr_file + n_file;
    if (fl->fl_mode != 'r') return (UCHAR*)0;
    if (fl->fl_recs < 1) {
        if (fl->fl_file) {
            if (fl->fl_strata < 1) {
                BUSY int err;
#if (DO_DEBUG)
                fprintf(stderr," :work#%d(%s) EOF%c",n_file,fl->fl_name,C_LF);
#endif
                if (err= fclose_work(fl,1,info)) {
                    /* delete work file for input when EOF */
                    info->sr_errno= err;                    return ERR_RETURN;
                }
            }
#if (DO_DEBUG)
            else {
                fprintf(stderr," :work#%d(%s) at end of stratum %d%c",
                        n_file,fl->fl_name,fl->fl_strata,C_LF);
            }
#endif
        }
        return ERR_RETURN;
    }
    s= fl->fl_buf;
    f= fl->fl_file;
    if (fread((UNIV*)s,SZ_HEADER,1,f)< 1) {        /* unexpected EOF */
        fprintf(stderr,"%c?bug:sortex:next_file:file collapse:EOF %s(%ld)%c",
                    C_LF,fl->fl_name,(long)(fl->fl_recs),C_LF);
        info->sr_errno= E_SRBWFROTN;
        fclose_work(fl,0,info);                             return ERR_RETURN;
    }
    len= (size_t)(*(LENGTH*)s);
    if (fread((UNIV*)(s+SZ_HEADER),sizeof(UCHAR),len,f)< len){ /* short */
        fprintf(stderr,"%c?bug:sortex:next_file:file collapse:length %s(%ld)%c",
                C_LF,fl->fl_name,(long)(fl->fl_recs),C_LF);
        info->sr_errno= E_SRBWFROTN;
        fclose_work(fl,0,info);                             return ERR_RETURN;
    }
    (fl->fl_recs)--;
    return s;
#undef ERR_RETURN
}
 
PRIVATE
int core_ropen(n_core,info) BUSY int n_core; sort_type *info;
{
    BUSY core_type *core= info->sr_core;
    for( ;--n_core >= 0; core++) {
        if (core->cr_recs > 0) {
            core->cr_next= core->cr_head; core->cr_open= 1;
        }
    }
    return 0;
}
 
PRIVATE
int file_ropen(even_odd,info) int even_odd; sort_type *info;
{
    BUSY file_type *fl;
    BUSY int        i, n_file= info->sr_n_file, n_opened= 0;
 
    for(i=even_odd, fl=info->sr_file+i; i< n_file; i+=2, fl+=2) {
        if (fl->fl_file= fopen_work(fl,"r",i,info)) {
            n_opened++;
        } else if (info->sr_errno) break;
    }
    return (info->sr_errno) ? info->sr_errno : (n_opened > 0) ? 0 : EOF;
}
 
PRIVATE
FILE *fopen_work(fl,mode,id_file,info)
    BUSY file_type *fl; char *mode; int id_file; sort_type *info;
    /* id_file is not reffered if compiler has tmpnam() */
{
#define ERR_RETURN (FILE*)0
    BUSY FILE *f;
    fl->fl_recs= 0;
    if ((fl->fl_strata<1) && (*mode=='r')) { /* not written file */
        /* fix 90/01/22 */                                  return ERR_RETURN;
    }
    if (!(fl->fl_file)) {
        if ((*mode == 'w') && !*(fl->fl_name)){ /* make non-extant file name */
            if (info->sr_orig_fname) {
                sprintf(fl->fl_name,"%s%02d",info->sr_orig_fname,id_file);
            } else {
#if (defined(f_tmpnam))
                BUSY char   *tmp_s;
                BUSY size_t  tmp_len;
                if (!(tmp_s= f_tmpnam(fl->fl_name))) {
                    fprintf(stderr," DEBUG:cannot open tmpnam by (F(%p)->%p)[%s]\n",(void*)fl,(void*)(fl->fl_name),fl->fl_name);
                    info->sr_errno= E_SROWKOPEN;           return ERR_RETURN;
                }
 
                if   ((tmp_len= strlen(tmp_s)) > WK_FNAME_MAX) {
 
                    fprintf(stderr," DEBUG:tmp_s length(%d) > fname_max(%d)\n",tmp_len, WK_FNAME_MAX);
 
                    info->sr_errno= E_SROWKOPEN;           return ERR_RETURN;
                }
                memcpy(fl->fl_name,tmp_s,tmp_len +1); /* +1 for EOS:91/07/03*/
#else
                fprintf(stderr,"%cbug:sortex:fopen_work:sr_orig_fname NULL%c",
                    C_LF,C_LF);
                    info->sr_errno= E_SRBPTRNUL;           return ERR_RETURN;
#endif
            }
        }
#if (!defined(OS_VOS3) && !defined(OS_FACOM))
        /* check if work-scratch file already exists.
         * NB. in IBM 370 type OSs, work files must pre-allocated,
         * and thus this code must be ignored.
         */
        if ((*mode == 'w') && (f= fopen(fl->fl_name,"r"))) {
          /* that scratch file already exists. either because
           * 1. f_tmpnam() returned EXISTING file name, cannot depend on it.
           *    This REALLY happenes in gcc on NeXT-station (unbelievable).
           *    We must use mktemp() for these compilers.
           * 2. user designated queer (eg. very long) file prefix, and
           *    OS could not distinguish between them.
           * anyway, we must quit here for data protection. 91/11/20
           */
          fclose(f);
          fprintf(stderr,"%c?bug?:sortex():temp scratch file #%d(%s) exists.%c",
              C_LF,id_file,fl->fl_name,C_LF);
#if (defined(OS_MSDOS) || defined(OS_OS2))
          fprintf(stderr,"?: maybe too long temp-file name specified.%c",C_LF);
#endif
          *(fl->fl_name)= (char)0;  /* so as not to be unlinked */
          info->sr_errno= E_SRBWDESTROY;                   return ERR_RETURN;
        }
#endif
        if (!(fl->fl_file= bin_fopen(fl->fl_name,mode))) {
#if (1 || DO_DEBUG)
            fprintf(stderr,"%c :: problem in opening work file (%s)[%d]%c",
                C_LF,fl->fl_name,fl->fl_mode,C_LF);
#endif
            info->sr_errno= (*mode=='w')?E_SROWKOPEN:E_SRIWKOPEN;
                                                            return ERR_RETURN;
        }
    }
    if (*mode=='r') {           /* get file header (record counter) */
        REC_COUNT head[2];
        if (fl->fl_strata >= 1) {
            fl->fl_strata -= 1;
        } else {
                /* this cannot happen, because EOF is checked in next_file() */
            fprintf(stderr,"%cbug:sortex:fopen_work #%d(%s)strata(%d)<1.%c",
                C_LF,id_file,fl->fl_name,fl->fl_strata,C_LF);
            info->sr_errno= E_SRBWEOF;
            fclose_work(fl,0,info);                         return ERR_RETURN;
        }
        if ((fread(head,sizeof(REC_COUNT),2,fl->fl_file) <2) || (*head >0)) {
                    /* top of header must be 0 : header unrecognizable */
            fprintf(stderr,"%c?bug:sortex:fopen_work:header collapse %s.%c",
                    C_LF,fl->fl_name,C_LF);
            info->sr_errno= E_SRBWFROTN;
            fclose_work(fl,0,info);                         return ERR_RETURN;
        } else {            /* decode header */
            fl->fl_recs= head[1];
#if (DO_DEBUG)
            fprintf(stderr," :reading work (%s) : strata=%d, recs=%ld.%c",
                 fl->fl_name,fl->fl_strata,(long)(fl->fl_recs),C_LF);
#endif
        }
    }
    fl->fl_mode= *mode;
    return fl->fl_file;
#undef ERR_RETURN
}
 
PRIVATE
int fclose_work(fl,if_del,info)
    BUSY file_type *fl; BUSY int if_del; sort_type *info;
{
    if (fl->fl_file) fclose(fl->fl_file);
#if (DO_DEBUG)
    if (fl->fl_file) fprintf(stderr," : closing work [%c](%s)%c",
            fl->fl_mode,fl->fl_name,C_LF);
#endif
#if (defined(f_unlink))
    if (if_del && *(fl->fl_name)) {
        if ((fl->fl_strata) && (get_err_level(info->sr_errno) < LV_SEVR)) {
            fprintf(stderr,"%c?bug:sortex:delete unread work(%s)[strata %d]%c",
                    C_LF,fl->fl_name,fl->fl_strata,C_LF);
            return info->sr_errno= E_SRBWKILL;
        }
        f_unlink(fl->fl_name);
#if (DO_DEBUG)
        fprintf(stderr," :work[%c](%s)removed%c",fl->fl_mode,fl->fl_name,C_LF);
#endif
        *(fl->fl_name)= (char)0;
    }
#endif
    fl->fl_file= (FILE*)0; fl->fl_recs= (REC_COUNT)0; fl->fl_mode= (char)0;
    return 0;
}
 
PRIVATE
int fclose_all_work(info,if_del) sort_type *info; BUSY int if_del;
{
    BUSY file_type *fl;
    BUSY int        i, n_file= info->sr_n_file;
 
    for(i= 0, fl= info->sr_file; i< n_file; i++, fl++) {
        BUSY int err;
        if (err= fclose_work(fl,if_del,info)) {
            info->sr_errno= err;
            return err;
        }
    }
    return 0;
}
 
PRIVATE
int put_header(fout,recs,info) FILE *fout; REC_COUNT recs; sort_type *info;
{
    REC_COUNT buf[2];
 
    buf[0]= 0;      /* sign of file header */
    buf[1]= recs;   /* number of recs */
    if (fwrite((UNIV*)buf,sizeof(REC_COUNT),2,fout) < 2) { /* write error */
        info->sr_errno= E_SRWKWRITE;               return info->sr_errno;
    }
    return 0;
}
 
 /*                         ACKNOWLEDGEMENT
  * This do_qsort() routine is an adaptation of qsort(), appeared on p. 169
  * of ISHIHATA,Kiyoshi 1989.3 "Algorithm and data structure" (Iwanami shoten).
  * The original program is written as pascal-like pseud-code, from which
  * I wrote the c code below.
  */
 
#define as_str(S)   ((UCHAR*)(S) + SZ_HEADER)
#define do_cmp(S,T) (*f_cmp)(as_str(*(S)),as_str(T),as_len(*(S)),as_len(T))
 
PRIVATE
void do_qsort(left_most, right_most,f_cmp) UCHAR **left_most, **right_most;
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int (*f_cmp)(const UNIV *,const UNIV *,size_t,size_t);
#else
    int (*f_cmp)();
#endif
{
    if (left_most < right_most) {
        BUSY UCHAR *pivot, **left, **right;
        if ((right_most - left_most) <= SRT_GVUP) {
            do_tournament(left_most,right_most,f_cmp);  return;
        }
            /* 1. select middle value for pivot */
        pivot= *(left_most + (size_t)((right_most - left_most)/2)); /* mean */
            /* 2. compare given field */
        left= left_most; right= right_most;
        do {
            while(do_cmp(left, pivot) > 0) left++;  /* DESCENDING order */
            while(do_cmp(right,pivot) < 0) right--;
            if (left <= right) {
                BUSY UCHAR *swap;
                swap= *left; *left= *right; *right= swap;
                left++; right--;
            }
        } while(left <= right);
            /* 3. sort divided fields */
        do_qsort(left_most,right,f_cmp);
        do_qsort(left,right_most,f_cmp);
    }
}
#undef do_cmp
#undef as_str
 
#define as_s(S)         ((UCHAR*)((UCHAR*)(*(S)) + SZ_HEADER))
#define do_scmp(S,T)    ((*f_cmp)(as_s(S),as_s(T),as_len(*(S)),as_len(*(T))))
 
PRIVATE
void do_tournament(left,right,f_cmp)
    UCHAR **left, **right;
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int (*f_cmp)(const UNIV *,const UNIV *,size_t,size_t);
#else
    int (*f_cmp)();
#endif
{
    BUSY size_t n_items, sum_items;
 
    if (right > left) {
        BUSY UCHAR **now, **current_min;
        sum_items= (size_t)(right - left);
        for(n_items= 1; n_items <= sum_items; n_items++) {
            current_min= right - n_items + 1;
            for(now= right - n_items; now >= left; now--) {
                if (do_scmp(now,current_min) < 0) {
                    BUSY UCHAR *swap;
                    swap= *current_min;
                    *current_min= *now;
                    *now= swap;
                }
            }
        }
    }
}
#undef do_scmp
#undef as_s
 
PRIVATE
int sort_prologue(info,sz_KB,bytes_max,file_max,name_file)
    sort_type   *info;
    int         sz_KB;
    BUSY int    bytes_max;
    int         file_max;
    char        *name_file;
{
#define VL_min(a,b) ((VERYLONG)(((VERYLONG)(b)<(VERYLONG)(a))?(b):(a)))
    BUSY file_type  *fl;
    BUSY size_t      core_KB, sz_bytes, sz_ptr, nc, split_core;
    int              i;
    VERYLONG         core1_bytes;
 
    assert(SZ_HEADER >= sizeof(LENGTH));
    memset(info,0,sizeof(sort_type));  /* clear info */
 
        /* 1. set max rec size */
    if (bytes_max < 1)  bytes_max= (int)RECL_MAX;
    if ((VERYLONG)bytes_max >= VL_min(INT_MAX,VL_min(LENGTH_MAX,CORE1_MAX)))
                                            return info->sr_errno= E_SRRCLONG;
    info->sr_bytes_max= bytes_max;
 
 
        /* 2. set count of work files */
    if      (file_max < 1) file_max= DFLT_N_WORK;
    if      (file_max < 4)                  return info->sr_errno=E_SRWKFEW;
    else if (file_max > WK_F_MAX)           return info->sr_errno=E_SRWKMANY;
    info->sr_n_file= file_max;
 
        /* 3. set name(prefix) of work files */
    if (name_file && ((strlen(name_file) +2) > WK_FNAME_MAX))
                                            return info->sr_errno=E_SRWKNAME;
    if (name_file && *name_file) {
        info->sr_orig_fname= name_file;
    } else {
#if (defined(f_tmpnam))
        info->sr_orig_fname= name_file= (char*)0;
#else
        info->sr_orig_fname= name_file= DFLT_NAME_WORK;
#endif
    }
        /* 4. alloc memory for rec buffer for each work file */
#if (DO_DEBUG)
    fprintf(stderr," :work files=%d, buf each=%u, total=%lu bytes.%c",
      file_max,(unsigned)bytes_max,(unsigned long)bytes_max * file_max,C_LF);
#endif
    fl= info->sr_file;
    for(i= 0; i < file_max; i++, fl++) {
        if (!(fl->fl_buf= (UCHAR*)calloc((size_t)bytes_max,sizeof(UCHAR)))) {
            BUSY file_type  *fp;
            BUSY int        i_free;
#if (DO_DEBUG)
            fprintf(stderr," :fail calloc(%u,1) for file #%d.%c",
                (unsigned)bytes_max,file_max,C_LF);
#endif
            for(i_free= 0, fp= info->sr_file; i_free < i; i_free++, fp++) {
                if (fp->fl_buf) {
                    free((UNIV*)fp->fl_buf); fp->fl_buf= (UCHAR*)0;
                }
            }
            return info->sr_errno= E_MEMOUT;
        }
    }
 
        /* 5. get memory size for core sort */
#if (DO_DEBUG && defined(f_memsz))
        fprintf(stderr," :after rec buf alloc (for %d files), memory=%lu.%c",
            file_max,(unsigned long)f_memsz(0),C_LF);
#endif
    if (sz_KB < 1) {        /* user not specified memory size */
#ifdef f_memsz
        core_KB= (size_t)((VERYLONG)f_memsz(0) / (VERYLONG)1000);
#else
        core_KB= (size_t)DFLT_CORE_KB;  /* no information from OS */
#endif
    } else core_KB= (size_t)sz_KB;
 
        /* 6. check if must devide memory for smaller portions (eg. segment) */
    split_core= (size_t)(((VERYLONG)core_KB * 1000) / (VERYLONG)CORE1_MAX);
#if (DO_DEBUG)
    fprintf(stderr," :core to alloc in KB=%u, must split core into >=%u.%c",
        (unsigned)core_KB,(unsigned)split_core,C_LF);
#endif
    if (split_core < 1) { /* can be allocated at once : split not necessary */
        split_core= 1;
        core1_bytes= (VERYLONG)core_KB * 1000;
    } else if ((VERYLONG)split_core < VL_min(WK_CORE_MAX,INT_MAX)) {
        core1_bytes= (VERYLONG)CORE1_MAX;
    } else                                  return info->sr_errno= E_MEMLARGE;
#if (DO_DEBUG)
    fprintf(stderr," :memory per(core1_bytes)=%u, cores=%u, total=%lu.%c",
        (unsigned)core1_bytes,(unsigned)split_core,
        (unsigned long)core1_bytes * (unsigned long)split_core, C_LF);
#endif
 
        /* 7. cut down memory portion size to word boundary */
    if (core1_bytes > (VERYLONG)SIZE_T_MAX) return info->sr_errno= E_MEMLARGE;
    sz_ptr= (size_t)(core1_bytes / (VERYLONG)sizeof(UCHAR *));
#if (defined(OS_MAC) || defined(OS_VOS3) || defined(OS_HUMAN))
    sz_ptr= (size_t)(((VERYLONG)sz_ptr * 9) / (VERYLONG)10);
        /* cut size down to 9/10 : must make room for memory control area */
#endif
    sz_bytes= sz_ptr * sizeof(UCHAR *);
#if (DO_DEBUG)
    fprintf(stderr," :max recs per core=%u, memory of 1 core(sz_bytes)=%u.%c",
        (unsigned)sz_ptr,(unsigned)sz_bytes,C_LF);
#endif
    if ((VERYLONG)sz_bytes <= (((VERYLONG)bytes_max + SZ_HEADER) * 2)) {
#if (DO_DEBUG)
    fprintf(stderr," :cannot sort 2 recs (sz_bytes(%u) < bytes_max(%u)*2)%c",
        (unsigned)sz_bytes,(unsigned)bytes_max,C_LF);
#endif
        /* cannot sort even 2 recs */       return info->sr_errno=E_SRSZSMALL;
    }
 
        /* 8. alloc memory portions */
    for(nc= 0; nc < split_core; nc++) {
        if (!(info->sr_buf[nc]= (UCHAR **)calloc(sz_ptr,sizeof(UCHAR *)))) {
#if (DO_DEBUG)
          fprintf(stderr," :fail calloc(%u,%u) for core #%u.%c",
            (unsigned)sz_ptr,(unsigned)(sizeof(UCHAR *)),(unsigned)nc,C_LF);
#endif
            if ((nc < 2) || (sz_KB > 0))    return info->sr_errno= E_MEMOUT;
        }
    }
    info->sr_n_core= (int)nc; /* nc <=split_core < INT_MAX is certified above*/
    info->sr_sz_pointer= sz_ptr;
 
#if (DO_DEBUG)
    fprintf(stderr," :cores=%d, ptrs each=%u, total=%lu, memory left=%lu.%c",
      info->sr_n_core, (unsigned)(info->sr_sz_pointer),
      (VERYLONG)(info->sr_sz_pointer) * (VERYLONG)(info->sr_n_core)
      * (VERYLONG)sizeof(UCHAR*),
#if (defined(f_memsz))
        (unsigned long)f_memsz(0),
#else
        (unsigned long)0,
#endif
        C_LF);
#endif
    return 0;
#undef VL_min
}
 
PRIVATE
int sort_epilogue(info) BUSY sort_type *info;
{
    BUSY file_type  *fl;
    BUSY int         i;
 
    for(i= 0; i < WK_CORE_MAX; i++) {
        if (info->sr_buf[i]) free((UNIV*)(info->sr_buf[i]));
    }
    for(i =0, fl= info->sr_file; i < WK_F_MAX; i++, fl++) {
        if (fl->fl_buf)     free((UNIV*)(fl->fl_buf));
        if (fl->fl_file)    fclose(fl->fl_file);
    }
    return info->sr_errno;
}
 
#ifndef bin_fopen
 /* binary fopen() : C370 portable compiler style */
PRIVATE
FILE *bin_fopen(name,mode) BUSY char *name, *mode;
{
    char fname_buf[RECL_MAX];
    extern FILE *fopen();
 
    sprintf(fname_buf,"%s/BINARY",name);
    return fopen(fname_buf,(*mode=='w')?"w":"r");
}
#endif
 
#if (defined(CPP_Microsoft) && defined(f_memsz))
PRIVATE
unsigned long get_max_memorysize(void)
{
    unsigned sz_paragraph;
 
    _dos_allocmem(UINT_MAX -1,&sz_paragraph);
    return (unsigned long)sz_paragraph * 16;
}
#endif
 
#if (defined(OS_MAC) && defined(f_memsz))
PRIVATE
unsigned long get_max_memorysize(void)
{
#define POSSIBLE_MAX (WK_CORE_MAX * CORE1_MAX)
    long dummy= 8000000L;
    BUSY unsigned long got= MaxMem(&dummy);
    return (got > POSSIBLE_MAX)? POSSIBLE_MAX : got;
#undef POSSIBLE_MAX
}
#endif
 
#if (defined(f_tmpnam) && defined(OS_HUMAN))
PRIVATE
char *Human_tmpnam(char *fname)
{
#define ERROR_VAL ((char *)0)
  static char tmp[WK_FNAME_MAX+1], template[] = "soXXXXXX";
  extern char *mktemp(char *);
  const char *tmp_dir;
  int tmp_len;
  (void)((tmp_dir = getenv("temp")) || /* use env. var. first found */
       (tmp_dir = getenv("TMP"))  ||
       (tmp_dir = getenv("TEMP")) ||
       (tmp_dir = "/"));
  tmp_len = strlen(tmp_dir);
  if (tmp_len >= WK_FNAME_MAX - sizeof template)
    return ERROR_VAL;
  /* Do not use strcpy(), strcat() to avoid memory runout. */
  memcpy(tmp, tmp_dir, tmp_len);
  if (tmp_len > 0 && tmp[tmp_len-1] != '/')
    tmp[tmp_len++] = '/';
  memcpy(tmp + tmp_len, template, sizeof template); /* together with EOS */
  return mktemp(tmp);
#undef ERROR_VAL
}
#endif
 
PRIVATE
int get_err_level(code) BUSY int code;
{
    return code%10;
}
 
char *sorterr(code) BUSY int code;
{
#define WHEN break;case
 BUSY const char *s="", *level="";
 static  char    buf[80];
 
 switch(get_err_level(code))    {
    case LV_INFO    : level="inform";
    WHEN LV_WARN    : level="warn";
    WHEN LV_SEVR    : level="severe";
    WHEN LV_FATL    : level="fatal";
    WHEN LV_INST    : level="bad install";
    WHEN LV_BUG     : level="bug";
 }
 switch(code) {
    case E_MEMOUT       :s="out of memory";
    WHEN E_MEMLARGE     :s="too large amount of memory";
    WHEN E_WRITE        :s="write eror";
    WHEN E_SRSZSMALL    :s="too small memory size for sort";
    WHEN E_SRSZLARGE    :s="too large memory size for sort";
    WHEN E_SRWKFEW      :s="too few work files for sort";
    WHEN E_SRWKMANY     :s="too many work files for sort";
    WHEN E_SRWKNAME     :s="too long work file name cut off";
    WHEN E_SRNOPROC     :s="sort in/out/compare func not given";
    WHEN E_SROWKOPEN    :s="cannot write sort work file ";
    WHEN E_SRIWKOPEN    :s="cannot read sort work file ";
    WHEN E_SRWKWRITE    :s="write error to sort work file";
    WHEN E_SRWKREAD     :s="read error from sort work file";
    WHEN E_SRBWKILL     :s="bug: destroying work file";
    WHEN E_SRBWFROTN    :s="bug: work file collapse";
    WHEN E_SRBPTRNUL    :s="bug: core pointer NULL";
    WHEN E_SRRCLONG     :s="too long input rec";
    WHEN E_SRRCMANY     :s="too many input recs";
    WHEN E_SRBWCORE     :s="write to not read core";
    WHEN E_SRBWEOF      :s="bug:EOF of work file not checked";
    WHEN E_SRBWDESTROY  :s="bug:cannot create exstant work file";
    WHEN E_SRBWNREC     :s="bug:scratch file record count mismatch";
 }
 sprintf(buf,"(%s):%s",level,s);
 return buf;
#undef WHEN
}
 
#if 0
 /* history
  * v03-00 88/12/10-11  completely rewritten.
  * v03-01 88/12/17     sortd decorated.
  * v03-04 89/06/04-07,13 informations made on stack (thus sortex() made
  *                     re-entrant), etc. almost rewritten.
  * v03-05 89/09/18-24,28, 89/10/01, 89/10/15 qsort() replaced by do_qsort()
  *                     thanks to LSI-C86 lib, balanced 2-way merge,
  *                     merge by heap, etc.
  * v03-06 89/12/13     bug when too long rec appers fixed
  * v03-07 90/01/22,25  read-open only written work files
  * v03-08 90/03/14     do_qsort() rewritten
  * v03-09 90/03/24     some static variables moved into struct.
  *                     made really re-entrant :-)
  * v03-10 90/08/07     malloc() -> calloc()
  * v03-11 91/02/01-06  alignment on word boundaries,
  *                     get max available memory size from OS(MS-DOS),
  *                     port to Mac-OS, etc.
  * v03-12 91/02/26     alignment fix
  * v03-13 91/03/18     full prototype
  * v03-14 91/04/28     do_qsort() refined with SRT_GVUP
  * v03-15 91/06/22-25
  *     sortex() interface change. "bytes_max" made size_t,
  *     E_SRRCMANY made effective in merge_core/file(),
  *     get max available memory in Mac-OS, sort_prologue() rewritten,
  *     bug in usage of fwrite()/fread() in Mac-OS fixed.
  * v03-15b 91/07/03  bug in fopen_work() fixed.
  * v03-15c 91/08/10  for LSIC86 v3.30
  * v03-15c bis : 91/10/19 Human68k version thanks to Kaoru MAEDA (pcs30367)
  * v03-16  91/11/18-23
  *     check if named work file exists or not, if it does, returns error.
  *     This is mainly because some tmpnam() [ie. gcc on NeXT] does not check
  *     file existance (!), and some DOS users prone to specify too long names.
  * v03-17  92/02/19    fix "size_t" usage, in case it is signed int (eg. SUN)
  * v03-18  92/03/14
  *     sortex() interface changed. "bytes_max" now restored to int.
  *     This was done because bytes of each record is reported to sortex()
  *     by input function "int fc_in()", and this function MUST return INT,
  *     because it may return EOF. Thus, defining maximum record length as
  *     size_t is, eventually, nonsense.
  *     In effect, under 16-bit-int compilers, we have problems in obtaining
  *     large memory at once (eg. MS-DOS), and we just cannot afford large
  *     record buffer anyway; in 32-bit-int compilers, it is hard to believe
  *     there be any problem in defining record length by int.
  *     One might ask, then why 'len's in fc_out() or fc_cmp() are size_t.
  *     This is mainly because memcmp() expects size_t for its length, and
  *     fc_cmp() must be compatible with it.
  *     This is undoubtedly awkward, and any suggestion for amelioration is
  *     wellcome. Author : mtoyo@lit.hokudai.ac.jp (internet)
  * v03-19  92/03/28
  *     changed to stop at once when read-open of work files fail.
  *     this rarely happens when work files are deliberately removed.
  * v03-20  92/08/09-14
  *     bug in sort_core() fixed: (before v03-19, pointer for text record
  *     could wrap-around, when very long records appear).
  *     This bug was reported by roku@mix.
  * v03-21  93/09/29
  *     bug in alignment after length indicator of each rec fixed.
  * v03-22  93/10/15
  *     alignment for rec length header changed to a macro
  * v03-22a  93/10/24
  *     XC_MS7
  * v03-22b  94/04/25
  *     sets error when fc_in() gives ERROR_FROM_INPUT
  * v03-23   94/04/30 - 94/05/02,13
  *     update for sortf.c v00-10
  */
#endif

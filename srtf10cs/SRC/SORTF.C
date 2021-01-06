#include <stdio.h>
 /* sortf.c : sort with fields. using sortex().
  * copyright (C) TOYOSHIMA, Masayuki, 1989.All rights reserved.
  * feel free to copy and distribute under the GNU Public Licence.
  * send bug reports and/or suggestions to
  *     mtoyo@Lit.hokuda.ac.jp, MHA00720@niftyserve.or.jp
  */
 
 static char *copyright=" sortf.c : copyright (C) TOYOSHIMA, Masayuki, 1989. ";
 static char *version=  "v00-10c";
 static char
    *Last_updated= "$Header: /home/mtoyo/sortf/sortf.10c/sortf.c,v 1.3 1994/08/27 10:57:51 mtoyo Exp mtoyo $";
 
 
#ifndef RECL_MAX
#define RECL_MAX    1024
#endif
#ifndef DO_DEBUG
#define DO_DEBUG    0
#endif
 
#include "mtconfig.h"
#include <assert.h>
 
/*
 *===================================================
 *              compilation switchs
 *===================================================
 */
 
#if (!defined(DO_REAL))
#define DO_REAL     1       /* enable real numbers for -n option */
                            /* otherwise, only integers are recognized */
#endif
 
#if (!defined(DO_MBCHR))
#define DO_MBCHR    1       /* enable Multi-byte characters in data */
                            /* otherwise, MB chars treated as single-byted */
#endif
 
#if (!defined(DO_JPCOL))
#if (DO_MBCHR)
#define DO_JPCOL    1       /* enable Japanese locale collation by -J */
                            /* otherwise, no JP locale collation */
#else
#define DO_JPCOL    0
#endif
#endif
 
#if (!defined(DO_UNIQ))
#define DO_UNIQ     1       /* allow -u for `uniq'-like output selection */
#endif
 
#if (!defined(DO_SQWZ))
#define DO_SQWZ     1       /* allow blank line suppression by -s */
#endif
 
#if (!defined(N_MAX_FIELDS))
#define N_MAX_FIELDS    128 /* max number of fields */
#endif
 
    /*  JPCOL(Japanese locale) assumes MBCHR (Multi-byte char) */
#if ((!defined(DO_MBCHR) || !DO_MBCHR)&&(defined(DO_JPCOL) && DO_JPCOL))
#define DO_MBCHR     0
#define DO_JPCOL     0
#endif
 
#if ((defined(DO_JPCOL) && DO_JPCOL) && (!defined(DO_MBCHR)||!DO_MBCHR))
#ifdef DO_MBCHR
#undef DO_MBCHR
#endif
#define DO_MBCHR     1
#endif
 
 /*===================================================
  *
  *          from here, do not edit
  *
  *===================================================
  */
 
#include "sortex.h"
 
#if 0
 =========  Suggestions for compilation ===========
 
    a. under UNIX and GNU c compiler
        2. make -f makefile.unx
    a2.under Sun OS 4
        2. make -f makefile.unx UCOPT=-DFORCE_UC=1
    b. under UNIX-BSD cc
        2. make -f makefile.unx
    c. under MS-DOS and Visual c++ compiler
        2. make -f makefile.dos (use imake.exe by k16[Ishino,Koichiro])
    d. under MS-DOS and Borland c++ compiler
        2. make -f makefile.dos (use imake.exe by k16[Ishino,Koichiro])
    e. under HITAC VOS3 and Hitachi COPT (optimizing c copiler)
        sorry no longer supported
    f. under HITAC VOS3 and C370 pcc
        sorry no longer supported
    g. under Macintosh OS and Think c v5.0
        1. make project file containg *.c and ANSI && UNIX && MacTraps library
           be sure to use ANSI (not ANSI-small)
        2. bring project up to date and make application
        3. open Get Info dialog and set "Application Memory Size" over 600 KB
    h. under Human68k and gcc
        [without support from the author]
        1. mtconfig.68k as mtconfig.h
        2. make -f makefile.68k : thanks to Kaoru MAEDA (pcs30367).
    i. other cases
        God helps those who help themselves :-)
 
------------------------------------------------------------------------------
 
    structure of 1 sort-output record (handed to sortex()) is as follows :
 
------------------------------------------------------------------------------
 1st KEY POINTER | 2nd KEY PTR | ...| NUMERIC KEYs  | ... |  RECORD BODY    |
 sizeof(key_type)|             |    |sizeof(NUMERIC)|     | (size varies)   |
------------------------------------------------------------------------------
    |                   |        |          ^                   ^     ^
    |___________________|________|__________|___________________|_____|
                        |        |          |                   |
                        |________|__________|___________________|
                                 |          |
                                 |__________|
 
  1. KEY POINTERS are at head of each rec. Count of KEY POINTERS are number
     of keys + 1. (Thus, at leat 1 KEY POINTER exists). KEY POINTERS are of
     type 'key_type'.
     length of KEY_POINTERS is "sr_sz_keys" (same length for all records)
  2. NUMERIC KEYS (for '-n' specified keys, if any) are next to KEY POINTERS.
     length of KEY_POINTERS and NUMERIC KEYS is "sr_sz_suffix" (same for all)
  3. Body of INPUT RECORD comes next. Nothing (like EOS) terminates it.
     INPUT RECORD is stored asis, ie. with no conversions or modifications.
  4. NO padding for word boundaries exist between NUMERIC KEYS && RECORD BODY.
  Total length of 1 record is managed by sortex(), not written in each record.
 
  o For non-numeric (ie. '-n' not specified) keys, 'key_offset' of each KEY
    POINTER points corresponding place in INPUT RECORD. Character conversions
    such as '-f', '-i' are done during comparison, with some overhead, yes.
 
  o For numeric keys, 'key_offset' of KEY POINTERS points NUMERIC KEY itself,
    which is converted to 'NUMERIC' type on input by strtoNUM() function,
    thus avoiding overhead.
 
  o 'character skip's (eg. +1.3) and 'blank skips' ('-b') are done in computing
    'key_offset'.
 
  o No 'special' character (like EOS) marks the end of 1 sort-output record.
 
  <Multi-byte comparisons> when '-J' specified : see collJP.c
 
===============================================================================
#endif
 
/*
 *===================================================
 *              minor configrations
 *===================================================
 */
 
#if (!defined(DO_JPCOL) && (defined(OS_MSDOS)||defined(OS_OS2)||defined(OS_MAC)||defined(OS_UNIX)||defined(OS_HUMAN)))
#define DO_JPCOL    1
#endif
 
#if (defined(DO_JPCOL) && DO_JPCOL && (!defined(DO_MBCHR)))
#define DO_MBCHR     1
#endif
 
#if (!defined(DO_JPCOL))
#define DO_JPCOL    0
#endif
 
#if (!defined(DO_MBCHR))
#define DO_MBCHR    0
#endif
 
#if (!defined(N_MAX_KEYS))
#define N_MAX_KEYS      N_MAX_FIELDS
#endif
 
    /* attribute symbols */
 
#define AT_CUT_BLANK    ((ATTRIB)0x01)
#define AT_LOWER        ((ATTRIB)0x02)
#define AT_PRINT        ((ATTRIB)0x04)
#define AT_ALNUM        ((ATTRIB)0x08)
#define AT_NUMERIC      ((ATTRIB)0x10)
#define AT_REVERSE      ((ATTRIB)0x20)  /* 90/08/15 */
#define AT_JPLOCALE     ((ATTRIB)0x40)  /* 91/03/17 */
#define AT_EMPTY_HIGH   ((ATTRIB)0x80)  /* 93/09/20 */
 
#define DE_FIELD        ((DELMTR)0x01)  /* field delimiter : 91/05/15 */
#define DE_PRGRF        ((DELMTR)0x02)  /* paragraph delimiter */
 
 
#if (!defined(C_SIGN_VACANT_REC))
#define C_SIGN_VACANT_REC   ((int)(UCHAR_MAX) + 1)  /* larger than UCHAR_MAX*/
#endif
 
#define SZ_DELM_TBL         (C_SIGN_VACANT_REC + 1)
 
#if (defined(OS_MAC) && defined(CPP_Symantec))
#include <console.h>    /* Think c : unix-like command line */
#endif
 
#if (defined(CPP_Borland))
    extern unsigned _stklen=16000;
#endif
 
/*
 *===================================================
 *              for wicked systems :-)
 *===================================================
 */
 
/*  1. BAD_STRTOD  : has wicked strtod() function
 *
 *  MS-c v.6/7/Visual c++ has a serious deficiencies in strtod() function.
 *  1)  it cannot take a string longer than 99 bytes.
 *      (This is stated in the manual).
 *  2)  it cannot analyse digits ending with 'd', 'e', 'D', 'E'.
 *      eg. strtod("12E",(char**)0) returns 0.0 (!)
 *      This is contradicting ANSI 4.10.1.4.
 *  To get around this 2nd bug, atof_simple() is prepared below.
 *
 *  NB. This simple_atof() accepts simple constants only such as "-12.34".
 *      It does not accept exponents such as "12.34e5" (treated as 12.34).
 *      It does not recognize -0.0 from 0.0, though MS-c v.6 strtod() does.
 */
#if ((defined(CPP_Microsoft)||defined(CPP_Symantec))&&!defined(BAD_STRTOD))
#define BAD_STRTOD  1
#endif
 
/*  2. FORCE_UC : to force unsigned comparison of memcmp().
 *
 *  signed comparison of memcmp() is contradicting ANSI 4.11.4,
 *  but some system still have standard libraries with signed memcmp().
 *
 *  NB. defining FORCE_UC has drawbacks : it's slow.
 *  FORCE_UC calls unsig_memcmp(), which is a very naive code, not
 *  calling for machine-native fast string comparison codes.
 */
#if (defined(FORCE_UC) && FORCE_UC)
#define UC_memcmp(S,T,L) unsig_memcmp((const UCHAR*)(S),(const UCHAR*)(T),(L))
#if defined(HAS_FUNCTION_PROTOTYPE)
        static int   unsig_memcmp(const UCHAR*,const UCHAR*, size_t);
#else
        static int   unsig_memcmp();
#endif
 
#else   /* system memcmp() does  unsigned comparison */
 
#if (!defined(FORCE_UC))
#define FORCE_UC    0
#endif
#define UC_memcmp(S,T,L) memcmp((S),(T),(L))
#endif
 
/*
 *===================================================
 *              type definitions
 *===================================================
 */
 
 typedef unsigned int   ATTRIB;
 typedef UCHAR          DELMTR;
 
#if (DO_REAL)              /* 91/09/21 */
    typedef double      NUMERIC;
#if (defined(CPP_Microsoft)||defined(CPP_Symantec))
#define strtoNUM(s)     atof_simple((char*)(s))
    DCL_PRIVATE double  atof_simple(char *s);
#else
#define strtoNUM(s)     atof((char*)(s))
#endif
#else
 typedef long int       NUMERIC;
#define strtoNUM(s)     strtol((char*)(s),(char**)0,0)
#endif
 
    typedef struct {
        int     key_offset,     /* distance of key from head of rec */
                key_len;        /* bytes of that key */
    } key_type;
 
    typedef struct {
        int     def_n_from,     /* 'from' key : field number */
                def_n_to,       /* 'to'   key */
                def_order,      /* > 0 in normal order, < 0 in reverse order */
                def_asis,       /* non 0 if keep original sequence */
                def_left,       /* < 0 normal, > 0 under empty_as_high mode */
                def_right;      /* > 0 normal, < 0 under empty_as_high mode */
        ATTRIB  def_attrib,     /* field pair attrib */
                def_to_attrib;  /* 'to' field attrib : only -b is effective */
        int     def_from_skip,  /* 'from' key : len to skip from head */
                def_to_skip;    /* 'to'   key */
        UCHAR  *def_table;      /* char conv table */
#if defined(HAS_FUNCTION_PROTOTYPE)
        int    (*def_f_cmp)(const UCHAR *,const UCHAR *,int,int,const UCHAR *);
#else
        int    (*def_f_cmp)();
#endif
    } def_type;
 
    typedef struct {
        const UCHAR *fld_str;   /* begin  of this field */
        int          fld_len,   /* length of this field : effective length
                                 * (ie. without right spaces when blank split)
                                 */
                     fld_tail;  /* non-effective trailing chars of last field*/
    } field_type;
 
    typedef struct {
        DELMTR   delm_attr,     /* 1 byted char   attribute */
                *delm_2byte;    /* Multi-byte char 2nd byte attributes */
    } delm_type;        /* 91/05/15 */
 
  typedef struct {
      int     numf_number,    /* field number */
              numf_skip;      /* skip at top of field */
      ATTRIB  numf_attr;      /* field attribute */
      char    numf_optchar;   /* char to begin field option */
  } numf_type;
 
#define COPT_BEGF   '+'     /* option character for begining field */
#define COPT_ENDF   '-'     /* option character for ending field */
#define COPT_THISF  '='     /* option character for begin/end (only 1) field */
#define COPT_STDIN  COPT_ENDF   /* option character to specify stdin as input
                                 * must be same as COPT_ENDF */
 
/*
 *===================================================
 *              global variables
 *===================================================
 */
 
    static def_type *sr_defs, sr_whole;
    static int      sr_n_keys= 0,       /* count of key field pairs */
                    sr_max_n_field= 0;  /* max field number */
    static int      sr_sz_keys= 0,      /* output sort key info lengths */
                    sr_sz_prefix= 0;    /* key infos && numeric key lenghts */
    static delm_type *delm_tbl;
    static UCHAR    *keep_old;  /* old string for comparison in 'unique' */
    static int      sr_mem_KB= 0, sr_files= 0;
    static int      sr_length= (0), /* max input rec length (without keys) */
                    sr_user_specification_is_longer= 0;
    static int      sr_bytes= (0);  /* max rec length with keys */
                                    /* sr_length/sr_bytes diveded 92/08/09 */
    static char     *sr_temp= (0);
    static int      sr_verbose= 0, sr_uniq= 0, sr_report= 0, sr_opt_report= 0;
#if (DO_DEBUG)
    static int      sr_give_why= 0;  /* give explanation of why sorting so */
#endif
#if (DO_JPCOL)
    static int      sr_do_mbyte_compare= 0;
    static UNIV     *sr_table_mbyte= (0); /* to store return of opncoll() */
    static int      sr_warned_JP= 0;    /* if warned contradicting options */
#endif
 
    static UCHAR    sr_rec_separator[SZ_MBCHR];
    static int      sr_sz_rec_separator= 0;
 
    static char     *sr_fout_name= (0), *sr_fin_name= "stdin";
    static int      sr_n_fin= 0;
    static int      sr_error= 0, sr_exit_code= 0;
    static FILE     *fin, *fout;
    static int      cpy_argc;
    static char     **cpy_argv;
    static long int recs_in= 0L, recs_out= 0L, recs_report= 0L, recs_this= 0L;
 
/*
 *===================================================
 *    function declarations
 *
 *    DCL_PRIVATE : for static functions
 *    static      : for static pointers to functions
 *
 *      You may wonder why these distinctions, but,
 *      believe it or not, there exist compilers that
 *      can't accept static declaration for functions.
 *===================================================
 */
 
#if defined(HAS_FUNCTION_PROTOTYPE)
    DCL_PRIVATE int  sr_in(char*);
    DCL_PRIVATE int  sr_cmp(const UNIV*, const UNIV*, size_t, size_t);
    static int      (*rec_next)(UCHAR*,int *,const int,FILE *, const char *);
    DCL_PRIVATE int  get_rec(UCHAR*,int *,const int,FILE *,const char *),
                     get_paragraph(UCHAR*,int *,const int,FILE *,const char *);
    static      int  (*get_c_paragraph)(FILE*);
    DCL_PRIVATE int  get_c_vacant(FILE*);
    DCL_PRIVATE int  cmp_numeric(const UCHAR*, const UCHAR*,
                                int, int, const UCHAR *);
    DCL_PRIVATE int  cmp_by_table(const UCHAR*, const UCHAR*,
                                int, int, const UCHAR *);
    DCL_PRIVATE void make_key(UCHAR *, field_type *, const int,
                            const UCHAR *, const int);
    DCL_PRIVATE void make_string_key(key_type *, field_type *, def_type *,
                            int, const UCHAR *, const UCHAR *, const int);
    DCL_PRIVATE void make_numeric_key(key_type *, field_type *, def_type *,
                            NUMERIC *, const UCHAR*);
    DCL_PRIVATE void make_sequence_key(key_type *, field_type *, def_type *,
                            NUMERIC *, const UCHAR*);
    static int   (*sr_split_field)(field_type *,const UCHAR *, const int, int);
    DCL_PRIVATE int  split_blanks (field_type *,const UCHAR *, const int, int);
    DCL_PRIVATE int  split_chars  (field_type *,const UCHAR *, const int, int);
    DCL_PRIVATE int  sr_out(const char*, size_t);
    DCL_PRIVATE int  uniq_out(const char*, size_t);
    DCL_PRIVATE FILE *f_next(void);
    DCL_PRIVATE int  get_options(int, char **);
    DCL_PRIVATE int  anal_field(char *, char *);
    DCL_PRIVATE int  num_field(numf_type *, char *);
    DCL_PRIVATE int  regist_field(numf_type *, numf_type *);
    DCL_PRIVATE int  set_delm_table(const char *, const int);
    DCL_PRIVATE int  set_pattr(const int, const int);
    DCL_PRIVATE int  makeup_keydefs(def_type *, int);
    DCL_PRIVATE int  makeup_delm_tbl(void);
    DCL_PRIVATE int  do_prologue(int argc, char **argv);
    DCL_PRIVATE int  do_epilogue(int local_error, int sorex_err);
    DCL_PRIVATE void report_version(void), report_option_status(void);
    DCL_PRIVATE UCHAR *get_single_char(UCHAR *buf,size_t *len, UCHAR *s);
    DCL_PRIVATE UCHAR *decode_hex(UCHAR *buf, size_t *len, UCHAR *s);
    DCL_PRIVATE const UCHAR *start_new_field(field_type *field_out,
            const UCHAR *s_left, size_t len_of_delimiter_at_left,
            int set_length_of_rest_of_rec_as_tentative_field_length);
 
#if (defined(fgetc))
    /* unexpected implementation : fgetc is defined as macro.
     * (we expect fgetc() to be a pointer to function).
     * surely compiler will complain during compilation.
     * it it does, hack manually (it depends on the fgetc definition) :-<
     * eg. you can setup a (real) function calling fgetc() macro.
     *
     * You may ask why not to use #error directive,
     * but some compilers even don't know #error !
     */
     HACK MANUALLY !
#else
    extern      int  fgetc(FILE*);
#endif
#if (DO_DEBUG)
    DCL_PRIVATE char *cut_lf(char *);
#endif
#else
    static int      (*sr_split_field)(), (*rec_next)(), (*get_c_paragraph)();
    DCL_PRIVATE int get_rec(), get_paragraph(), get_c_vacant();
    DCL_PRIVATE int sr_in(), sr_cmp(), cmp_numeric(),cmp_by_table(),
                    split_blanks(), split_chars(), sr_out(), uniq_out();
    DCL_PRIVATE int makeup_keydefs(), get_options(),
                    anal_field(), regist_field(), num_field(),
                    set_delm_table(), set_pattr(), makeup_delm_tbl(),
                    do_prologue(), do_epilogue();
    DCL_PRIVATE void make_key(), make_string_key(), make_numeric_key(),
                     make_sequence_key(),
                     report_version(), report_option_status();
    DCL_PRIVATE FILE *f_next();
    DCL_PRIVATE UCHAR *get_single_char(), *decode_hex();
    DCL_PRIVATE const UCHAR *start_new_field();
 
#if (defined(fgetc))
    /* unexpected implementation : fgetc is defined as macro */
    HACK MANUALLY !
#else
    extern      int  fgetc();
#endif
#if (DO_DEBUG)
    DCL_PRIVATE char *cut_lf();
#endif
#endif
 
#ifndef max
#define max(a,b) (((a)<(b))?(b):(a))
#endif
#ifndef min
#define min(a,b) (((a)>(b))?(b):(a))
#endif
 
#ifndef do_fclose
#define do_fclose(F) (((F) && ((F) !=stdin) && ((F) !=stdout) && ((F) !=stderr))?fclose(F):0)
#endif
 
/*
 *===================================================
 *          additional variables & functions
 *          for special configurations
 *===================================================
 */
 
    /* 1. Multi-Byte JP-locale compare */
 
#include "mbjp.h"
#if (!defined(SZ_MBCHR))
#define SZ_MBCHR    2       /* bytes of Multi-Byte char */
#endif
 
#if (DO_JPCOL)
#include "colljp.h"
#if defined(HAS_FUNCTION_PROTOTYPE)
    DCL_PRIVATE int cmp_JP_locale(const UCHAR*,const UCHAR*,int,int,const UCHAR*);
#else
    DCL_PRIVATE int cmp_JP_locale();
#endif
#endif
 
    /* 2. squeeze : suppress blank lines */
 
#if (DO_SQWZ)
    static int      sr_squeeze= 0, sr_kill_vacant_lines= 0; /* 93/09/16 */
#if defined(HAS_FUNCTION_PROTOTYPE)
    DCL_PRIVATE int entire_space(const UCHAR *,int);
#else
    DCL_PRIVATE int entire_space();
#endif
#endif
 
    /* 3. unique  */
 
#if (DO_UNIQ)
    static ATTRIB       sr_uniq_attrib= 0;  /* attrib of -u suboptions */
#define UAT_DUP         ((ATTRIB)0x01)      /* if that field duplicated */
#define UAT_NUMBERED    ((ATTRIB)0x02)      /* -un : implies 'normal' unique */
#define UAT_DUP_SEL     ((ATTRIB)0x10)      /* -ud || -uu */
#endif
 
 
/*
 *===================================================
 *          message strings
 *===================================================
 */
 
#if (DO_UNIQ)
    static char *sr_opt2= "[-u[UD][N]] [fields] [input...] >output";
#else
    static char *sr_opt2= "[fields] [input...] >output";
#endif
 
 
#if (DO_JPCOL)
    static char *sr_opt1= "sortf [-abdfhinprstuoJTLMVWZ]";
#else
    static char *sr_opt1= "sortf [-abdfhinprstuoTLMVWZ]";
#endif
 
    static char *sr_opt_field= "[+=]N.M[bdfinr] -N.M";
 
/*
 *===================================================
 *          errors
 *===================================================
 */
 
#define E_INCOMPLETE_LAST_LINE      (LV_INFO + 10)
#define E_SOME_INPUTFILE_NOT_FOUND  (LV_INFO + 20)
#define E_TOO_LONG_INPUT_REC        (LV_SEVR + 30)
#define E_LONG_INPUT_REC_TRUNCATED  (LV_INFO + 40)
#define E_FAIL_OUTPUT_OPEN          (LV_SEVR + 50)
#define E_FATAL_IN_SORT             (LV_FATL + 60)
#define E_BUG_IN_SKIPPING_TOO_LONG_INPUT_REC (LV_BUG+70)
 
#define ERROR_DURING_INPUT ((EOF<=INT_MIN)?(EOF +1):(EOF -1))
 
#define add_error(E)    ((((E)%10) > (sr_error%10))?sr_error=(E):0)
#define is_severe(E)    ((((E)%10) >= LV_SEVR)?1:0)
 
 /*
 *===================================================
 *          main
 *===================================================
 */
 
int main(argc,argv) int argc; char **argv;
{
    int err= 0;
 
#if (defined(CPP_Symantec))
    MaxApplZone();          /* get maximum application heap */
    argc= ccommand(&argv);  /* unix-like command line */
#endif
#if (defined(__EMX__))
    _wildcard(&argc,&argv); /* OS/2 wild-card expansion : emx native */
#endif
 
    cpy_argc= argc; cpy_argv= argv;
    if (do_prologue(argc,argv)) goto done;
    if (sr_uniq) {
        err= sortex(sr_in,uniq_out,sr_cmp,sr_mem_KB,sr_bytes,sr_files,sr_temp);
        if (!err) err= uniq_out((char*)0,(size_t)0); /* flush buffer */
    } else {
        err= sortex(sr_in,sr_out,sr_cmp,sr_mem_KB,sr_bytes,sr_files,sr_temp);
    }
    do_fclose(fout);
  done:
    sr_exit_code= do_epilogue(sr_error,err);
    if (sr_verbose || err || sr_error) {
        fprintf(stderr," %s%ld in, %ld out%c",
           ((sr_exit_code%10) > LV_INFO)? "": "done: ",recs_in,recs_out,C_LF);
    }
    exit(sr_exit_code);
}
 
/*
 *===================================================
 *          compare functions
 *===================================================
 */
 
#define as_num(p)   *((NUMERIC*)(p))
 
PRIVATE
int sr_cmp(s, t, s_bytes, t_bytes) const UNIV *s, *t;
        size_t s_bytes, t_bytes; /* s_bytes/t_bytes not reffered */
{
    BUSY key_type *s_key, *t_key;
    const def_type *def;
    int                 i;
 
    s_key= (key_type*)s;
    t_key= (key_type*)t;
 
#define s_len (s_key->key_len)
#define t_len (t_key->key_len)
#define s_str ((const UCHAR*)s + s_off)
#define t_str ((const UCHAR*)t + t_off)
    for(def= sr_defs, i= 0; i < sr_n_keys; def++, s_key++, t_key++, i++) {
        BUSY int    s_off, t_off, diff;
#if defined(HAS_FUNCTION_PROTOTYPE)
        int    (*f)(const UCHAR *, const UCHAR *, int, int, const UCHAR *);
#else
        int    (*f)();
#endif
        s_off= s_key->key_offset; t_off= t_key->key_offset;
        if (!s_off || !s_len) {       /* if s_key field not exist */
            if (t_off && t_len)         return def->def_left;
            else                        continue;   /* no diff in this key */
        } else if (!t_off || !t_len)    return def->def_right;
        if (f= def->def_f_cmp) {
#if (DO_DEBUG)
                if (sr_give_why) {
                    UCHAR buf_left[RECL_MAX+1], buf_right[RECL_MAX+1];
                    diff= (*f)(s_str,t_str,s_len,t_len,def->def_table);
                    memcpy(buf_left, s_str,min(s_len,RECL_MAX));
                    memcpy(buf_right,t_str,min(t_len,RECL_MAX));
                    *(buf_left  + min(s_len,RECL_MAX))= 0;
                    *(buf_right + min(t_len,RECL_MAX))= 0;
                    fprintf(stderr,":>LEFT(%s) %s RIGHT(%s)%c",
                    cut_lf(buf_left),
                    ((diff < 0) ? "<": ((diff)? ">":"==")),
                    cut_lf(buf_right),C_LF);
                }
#endif
            if (diff= (*f)(s_str,t_str,s_len,t_len,def->def_table)) {
                                        return diff * def->def_order;
            }
        } else {
#if (DO_DEBUG)
            if (sr_give_why) {
                UCHAR buf_left[RECL_MAX+1], buf_right[RECL_MAX+1];
                diff= UC_memcmp(s_str,t_str,(size_t)min(s_len,t_len));
                if (!diff) diff= s_len - t_len;
                memcpy(buf_left, s_str,min(s_len,RECL_MAX));
                memcpy(buf_right,t_str,min(t_len,RECL_MAX));
                *(buf_left  + min(s_len,RECL_MAX))= 0;
                *(buf_right + min(t_len,RECL_MAX))= 0;
                fprintf(stderr,":>LEFT(%u:%s) %s RIGHT(%u:%s)%c",
                s_len,cut_lf(buf_left),
                ((diff < 0) ? "<": ((diff)? ">":"==")),
                t_len,cut_lf(buf_right),C_LF);
            }
#endif
            if ((diff= UC_memcmp(s_str,t_str,(size_t)min(s_len,t_len)))
            ||  (diff= s_len - t_len))
                                        return diff * def->def_order;
        }
        /* else no diff in this key : try next */
    }
#undef  s_str
#undef  t_str
#undef  s_len
#undef  t_len
    return 0;
}
 
PRIVATE
int cmp_numeric(s,t,s_len,t_len,table)
    BUSY const UCHAR        *s, *t;
    int                     s_len, t_len;   /* s_len, t_len not reffered */
    const UCHAR             *table;         /* table        not reffered */
{
    BUSY NUMERIC diff;
    diff= as_num(s) - as_num(t);
    if      (diff > 0)      return 1;
    else if (diff < 0)      return -1;
    else                    return 0;
}
 
PRIVATE
int cmp_by_table(s,t,s_len,t_len,table)
    BUSY const UCHAR    *s, *t;
    BUSY int            s_len, t_len;
    BUSY const UCHAR    *table;
{
        /* bytes of multi-byted chars are treated as follows :
         * 1st byte    : according to the table, ie. just like 1-byted chars,
         * other bytes : according to the table only if 1st byte was valid.
         */
 
#if (DO_MBCHR)
    UCHAR   s_mb_store[SZ_MBCHR], t_mb_store[SZ_MBCHR];
    int     s_next_mb, t_next_mb;
    s_next_mb= t_next_mb= 0;
#if (SZ_MBCHR>2)
#define MB_store(T,S)   memcpy((T),(S),((SZ_MBCHR) - 1))
#else
#define MB_store(T,S)   (*(T)= *(S))
#endif
#endif
 
    while((s_len > 0) && (t_len > 0)) {
        BUSY UCHAR  s_byte, t_byte;
            /* get valid byte to compare for s */
        while(s_len > 0) {
#if (DO_MBCHR)
            if (s_next_mb) {    /* valid multi-byte char fractions exist */
                s_byte= s_mb_store[s_next_mb - 1];
                    /* here, there is no need to check validness, because,
                     * 1st byte of the fraction has been already tested for
                     * validness, and other bytes should simply be compared.
                     */
                if (++s_next_mb >= SZ_MBCHR) s_next_mb= 0;
                break;
            } else if (is_MBYTE(*s)) {
                if (s_byte= table[*s]) {        /* it's valid 1st byte */
                    MB_store(s_mb_store,s+1);   /* store other bytes */
                    s_next_mb= 1;               /* start from 2nd byte */
                    break;
                } else {    /* it's invalid : ignore this multi-byte char */
                    s_next_mb= 0;
                    s+= SZ_MBCHR; /* maybe we ran out of s, but no harm */
                    s_len -= SZ_MBCHR;
                    continue;
                }
            } else
#endif
            if (s_byte= table[*s]) break;   /* it's valid char */
            /* otherwise, non valid chars */
            s++; s_len--;
        }
 
            /* just the same thing for t */
        while(t_len > 0) {
#if (DO_MBCHR)
            if (t_next_mb) {
                t_byte= t_mb_store[t_next_mb - 1];
                if (++t_next_mb >= SZ_MBCHR) t_next_mb= 0;
                break;
            } else if (is_MBYTE(*t)) {
                if (t_byte= table[*t]) {
                    MB_store(t_mb_store,t+1);
                    t_next_mb= 1;
                    break;
                } else {
                    t_next_mb= 0;
                    t+= SZ_MBCHR;
                    t_len -= SZ_MBCHR;
                    continue;
                }
            } else
#endif
            if (t_byte= table[*t]) break;   /* it's valid char */
            /* otherwise, non valid chars */
            t++; t_len--;
        }
 
        /* now we have `s_byte' and `t_byte' to compare */
 
        if ((s_len > 0) && (t_len > 0)) {
            BUSY int diff;
            if (diff= ((int)s_byte - (int)t_byte))        return diff;
            /* else try next byte */
        } else break;   /* no diff in this key */
        s++; t++; s_len--; t_len--;
    }
    return s_len - t_len;
#if (DO_MBCHR)
#undef MB_store
#endif
}
 
#if (DO_JPCOL)
PRIVATE
int cmp_JP_locale(s,t,s_len,t_len,table) /* multi byte chars sensitive */
    BUSY const UCHAR    *s, *t;
    BUSY int            s_len, t_len;
    BUSY const UCHAR    *table;     /* not reffered */
{
    BUSY int diff;
    if ((diff= colljp(s,t,s_len,t_len,sr_table_mbyte))
    ||  (diff= s_len - t_len))      /* 1-byted/2-byted equivalance */
        return diff;
    return memcmp(s,t,min(s_len,t_len)); /* last resort */
}
#endif
 
/*
 *===================================================
 *          input functions
 *===================================================
 */
 
PRIVATE
int sr_in(s_out) BUSY char *s_out;
{
    BUSY UCHAR  *s_text;
    auto int    len_text;
    BUSY int    len_delm, n_fields;
    field_type  out_field[N_MAX_FIELDS + 1];
 
    s_text= (UCHAR*)s_out + sr_sz_prefix;
#if (DO_SQWZ)
  do {
#endif
    while((len_delm= (*rec_next)(s_text,&len_text,sr_length,fin,sr_fin_name))< 0){
            /* current input file EOF */
        do_fclose(fin);
        if (sr_verbose) {
            fprintf(stderr," %5ld(%ld: %s)%c",recs_in,recs_this,sr_fin_name,C_LF);
        }
        if (!(fin =f_next())) return EOF;   /* no more input files */
        recs_this= 0L;
    }
    recs_in++;      /* do I need to worry for wrap-around here ? :-) */
    recs_this++;
    if (len_text < 0) {     /* sign of too long input rec */
        int skip_len= (0);
        if (sr_user_specification_is_longer) {  /* use specified -Lxxx */
               /* user expected long records, but not expected so long ones.
                * in this case, we cut the record at tail, and go on.
                * this seems to be better than just giving everything up.
                */
            fprintf(stderr," %s(%ld): too long (>=%d) record truncated%c",
                sr_fin_name,recs_this,sr_length,C_LF);
            *(s_text + sr_length - 1)= (UCHAR)0;
            sr_error= add_error(E_LONG_INPUT_REC_TRUNCATED);
            do { /* skip the rest of this record */
                UCHAR buf[RECL_MAX +1];
                len_delm= (*rec_next)(buf,&skip_len,RECL_MAX,fin,sr_fin_name);
                if (len_delm < 0) {
                    fprintf(stderr," bug:%s(%ld):EOF in skipping long rec.%c",
                            sr_fin_name,recs_this,C_LF);
                    fprintf(stderr," bug in sortf.c:cannot happen.%c",C_LF);
                    do_fclose(fin);
                    sr_error= add_error(E_BUG_IN_SKIPPING_TOO_LONG_INPUT_REC);
                    return ERROR_DURING_INPUT;  /* notify sortex() to quit */
                }
            } while(skip_len < 0);
                /* terminate this record with record delimiter */
            len_delm= sr_sz_rec_separator;
            len_text= sr_length - 1 - len_delm;
            memcpy(s_text + len_text,sr_rec_separator,(size_t)len_delm);
        } else {
                /* user did not expect long records.
                 * this may lead to some disastrous result,
                 * so we must quit here.
                 */
            fprintf(stderr," %s(%ld): rec too long(>=%d bytes). use -L%c",
                sr_fin_name,recs_this,sr_length,C_LF);
            do_fclose(fin);
            sr_error= add_error(E_TOO_LONG_INPUT_REC);
            return ERROR_DURING_INPUT;  /* notify sortex() to quit */
        }
    }
    if ((recs_report > 0) && ((recs_in % recs_report)==0))
        fprintf(stderr," %5ld in...%c",recs_in,C_LF);
#if (DO_SQWZ)
  } while(sr_squeeze && entire_space(s_text,len_text));
#endif
        /* split input rec into fields */
    if (sr_max_n_field > 0) {
        n_fields= (*sr_split_field)(out_field,s_text,sr_max_n_field,len_text);
#if (DO_DEBUG)
        if (sr_give_why) {
          int       n;
          field_type *fld;
          for(n= 0; n< n_fields; n++) {
              UCHAR buf[RECL_MAX];
              fld= out_field + n;
              memcpy(buf,fld->fld_str,(size_t)min(fld->fld_len,sizeof(buf)-1));
              buf[min(fld->fld_len,sizeof(buf)-1)]= (UCHAR)0;
              fprintf(stderr,":: field#%d[%u:%u]<%s>%c",n,
                  (unsigned)(fld->fld_str - s_text),
                  (unsigned)(fld->fld_len),cut_lf(buf),C_LF);
          }
        }
#endif
    } else {    /* no field split necessary */
        out_field->fld_len= len_text; out_field->fld_str= s_text;
        n_fields= 1;
    }
    make_key((UCHAR*)s_out,out_field,n_fields,s_text,len_text);
    return (int)(sr_sz_prefix + len_text + len_delm);
}
 
/*--------------------------------------------------------------------------
 *  function specifications of get_next() routines:
 *  get_rec()       : records delimited by new-line
 *  get_paragraph() : records delimited by user-specified delimiter
 *
 *  each record shall be terminate by Record Separator (new-line/delimiter)
 *
 *      abcdefghijklmnopqrstuvwvLF
 *          ^                   ^
 *          |__ record body     |__ record separator(not necessarily LF)
 *
 *  get_rec()/get_paragraph()
 *      1. sets bytes of record body to len_text
 *      2. returns bytes of record_separator, or EOF
 *      3. adds new-line/[1st]Record Separator at end of file, if non-existant.
 *         NB. in this case, size of output file exceeds that of input.
 *      4. if too long rec comes, set EOF to len_text, and return 0
 *--------------------------------------------------------------------------
 */
 
PRIVATE
int get_rec(s,len_text,len_max,f,f_name) BUSY UCHAR *s; int *len_text;
    BUSY const int len_max; FILE *f; const char *f_name;
{
    BUSY int    c;
    BUSY int    len;
 
    *len_text= len= 0;
    if (feof(f))                        return EOF;
    while((c= getc(f)) != EOF)  {
        if (++len >= len_max)   {
            *len_text= EOF;             return 0;
        }
        *s++= (UCHAR)c;
        if (c == C_LF)  break;
    }
    if  (!len)                          return EOF;
    if  (c == EOF) {
        fprintf(stderr," :(notice)incomplete last line: %s%c",f_name,C_LF);
        sr_error= add_error(E_INCOMPLETE_LAST_LINE);
        memcpy(s,sr_rec_separator,(size_t)sr_sz_rec_separator);
        len+= sr_sz_rec_separator; /* here, size of output exceeds input */
        s+= sr_sz_rec_separator;
    }
    *len_text= len - 1; /* -1 for LF */
    return 1;   /* length of record separator : NOT record length */
}
 
PRIVATE
int get_paragraph(s,len_text,len_max,f,f_name) BUSY UCHAR *s; int *len_text;
    const int len_max; FILE *f; const char *f_name; /* f_name not referred */
{
    BUSY int    c, len, len_delm;
    *len_text= len= len_delm= 0;
    if (feof(f))                            return EOF;
    while((c= (*get_c_paragraph)(f)) != EOF) {
#if (DO_MBCHR)
        BUSY int    c2= 0;
        if (is_MBYTE(c) && ((c2= get_c_paragraph(f)) == EOF)) {
            /* its fractional MB_CHR 1st byte*/
            break;
        }
#endif
        if ((len + 1) >= len_max) { /* too long input rec */
            *len_text= EOF;                 return 0;
        } else {
            if (c != C_SIGN_VACANT_REC) {
                *s++= (UCHAR)c; len++;
            }
#if (DO_MBCHR)
            if (c2) {   /* delm_tbl[c].delm_2byte != NULL is certified */
                if (++len >= len_max) { /* too long input rec */
                    *len_text= EOF;         return 0;
                } else *s++= (UCHAR)c2;
                if (((delm_tbl[c].delm_2byte)[c2]) & DE_PRGRF) {
                    /* it's Multi-byte char delimiter */
                    len_delm= SZ_MBCHR;
                    break;
                }
                c2= 0;
            } else
#endif
            if (((delm_tbl[c]).delm_attr) & DE_PRGRF){ /* 1 byted delimiters */
                len_delm= 1;
                break;
            }
        }
    }
    if  (!len)                                              return EOF;
    if  (c == EOF) {
        fprintf(stderr," :(notice)incomplete last line: %s%c",f_name,C_LF);
        memcpy(s,sr_rec_separator,(size_t)sr_sz_rec_separator);
        len_delm= sr_sz_rec_separator;
        len+= len_delm; /* here, size of output exceeds input */
        s+= len_delm;
    }
    *len_text= len - len_delm;
    return len_delm;
}
 
PRIVATE
int get_c_vacant(f) FILE *f;
{
    BUSY    int c;
    static  int n_LF= 0;
    static  int n_LF_atmost= 0;
    static  int c_save= 0;
 
    if (n_LF > 0) {
        n_LF--;                 return C_LF;
    } else if (n_LF_atmost > 1) {
        n_LF_atmost= n_LF= 0;   return C_SIGN_VACANT_REC;
    } else if (c_save) {
        c= c_save; c_save= 0;   return c;
    }
    while ((c= getc(f)) != EOF) {
        if (c == C_LF) {
            n_LF++;
        } else if (n_LF > 0) {
            c_save= c;
            n_LF_atmost= n_LF;
#if (DO_SQWZ)
            if (sr_kill_vacant_lines) n_LF= min(2,n_LF);
#endif
            n_LF--;
            return C_LF;
        } else {
            return c;
        }
    }
    n_LF_atmost= n_LF;
#if (DO_SQWZ)
    if (sr_kill_vacant_lines) n_LF= min(2,n_LF);
#endif
    return (n_LF-- > 0)? C_LF : EOF;
}
 
PRIVATE
FILE *f_next()
{
    static int  used_arg= 1;
    static int  n_stdin_opened= 0;
 
    while(used_arg < cpy_argc) {
        BUSY char *s;
        BUSY FILE *f;
 
        if (s= *(cpy_argv + used_arg++)) {
            if ((*s == COPT_BEGF) || (*s == COPT_THISF)) continue;
            if (*s == COPT_STDIN) {    /* 93/09/19 */
                if (!(*(s + 1)) && (n_stdin_opened++ < 1)) {
                        /* only 1st effective */
                    sr_fin_name= "stdin"; sr_n_fin++;
                    return stdin;
                }
                continue; /* else ordinary options : ignore */
            }
            sr_n_fin++;
            if (f= fopen(s,"r")) {
                sr_fin_name= s;
                return f;
            } else {
                fprintf(stderr," :skip (not found) %s%c",s,C_LF);
                sr_error= add_error(E_SOME_INPUTFILE_NOT_FOUND);
            }
        }
    }
    return (FILE*)0;
}
 
/*
 *===================================================
 *          key setup/recognition functions
 *===================================================
 */
 
PRIVATE
void make_key(s_out,in_field,n_fields,s_in,len_text)
    UCHAR *s_out; field_type *in_field;
    BUSY const int n_fields; const UCHAR *s_in; const int len_text;
{
    BUSY def_type   *def;
    BUSY key_type   *out_key;
    BUSY NUMERIC    *s_num_key;
    int             i;
 
    out_key=  (key_type*)s_out;
    s_num_key= (NUMERIC*)(s_out + sr_sz_keys);
 
    for(def= sr_defs, i= 0; i < sr_n_keys; def++, out_key++, i++) {
        if (def->def_n_from >= n_fields) {      /* no key fields in this rec */
            out_key->key_offset= out_key->key_len= (size_t)0;
        } else if (def->def_asis) {
            make_sequence_key(out_key,in_field,def,s_num_key,s_out);
            s_num_key++;
        } else if ((def->def_attrib) & AT_NUMERIC) {
            make_numeric_key(out_key,in_field,def,s_num_key,s_out);
            s_num_key++;
        } else {
            make_string_key(out_key,in_field,def,n_fields,s_out,s_in,len_text);
        }
#if (DO_DEBUG)
        if (sr_give_why) {
          UCHAR buf[RECL_MAX];
          if ((def->def_attrib) & AT_NUMERIC) {
            field_type *fld= in_field + def->def_n_from;
            strcpy(buf,fld->fld_str + def->def_from_skip);
            fprintf(stderr,"::key[%d]=[%u:NUM]=%.2f((%p)%s)%c",
                i,out_key->key_offset,
                (double)(as_num(s_num_key -1)),
                  fld->fld_str + def->def_from_skip,cut_lf(buf),C_LF);
          } else {
            if (out_key->key_len) {
                memcpy(buf,s_out + out_key->key_offset,out_key->key_len);
                *(buf + out_key->key_len)= 0;
            }
            fprintf(stderr,"::key[%d]=[%d:%d]=(%p)%s%c",
                i,out_key->key_offset,out_key->key_len,
                (out_key->key_len)?(char*)(s_out+out_key->key_offset):(char*)0,
                (out_key->key_len)?(char*)cut_lf(buf):"NONE", C_LF);
          }
        }
#endif
    }
}
 
PRIVATE
void make_string_key(out_key,in_field,def,n_fields,s_out,s_in,len_text)
    key_type *out_key; field_type *in_field; def_type *def;
    int n_fields; const UCHAR *s_out, *s_in; const int len_text;
{
    BUSY int len;
    BUSY const UCHAR  *s= (in_field + def->def_n_from)->fld_str;
 
    if ((def->def_n_to || def->def_to_skip) && (def->def_n_to < n_fields)) {
        /* ^ 93/11/27 : when 'to' field not specified, assume rest of rec */
        /*             ^ 94/04/23 : eg. +0.2 -0.5, must look for to_skip */
        BUSY field_type  *field_to = in_field + def->def_n_to;
        BUSY const UCHAR *s_to= field_to->fld_str;
 
        if (def->def_to_skip)   {   /* when specifed as -M.N */
            BUSY int len_to= field_to->fld_len;
            if (def->def_to_attrib & AT_CUT_BLANK) { /* -M.Nb */
                    /* must ignore spaces at top of right : 90/07/26 */
                while(isspace(*s_to) && (len_to > 0)) {
                    s_to++;     /* trim right field head */
                    len_to--;
                }
            }
            if ((s_to + min(def->def_to_skip,len_to)) > s) {
                len= (int)((s_to+ min(def->def_to_skip,len_to))- s);
            } else {
                len= 0;
            }
        } else {    /* specified as -M (no fractional part) */
            if ((s_to - field_to->fld_tail) > s) {
                len= (int)((s_to - field_to->fld_tail) - s);
            } else {
                len= 0;
            }
        }
    } else {        /* right not limited : rest of whole rec */
        if ((s_in + len_text) > s)  len= (int)(s_in + len_text - s);
        else                        len= 0;
    }
    if ((def->def_attrib) & AT_CUT_BLANK) { /* trim left field head */
        while((len > 0) && isspace(*s)) {
            s++; len--;
        }
    }
    {
        BUSY int len_skip= min(def->def_from_skip,len);
        s+= len_skip; len-= len_skip;
    }
    out_key->key_offset= (int)(s - s_out);
    out_key->key_len=    len;
}
 
PRIVATE
void make_numeric_key(out_key,in_field,def,s_num_key,s_out)
  key_type *out_key; field_type *in_field; def_type *def;
  NUMERIC *s_num_key; const UCHAR *s_out;
{
    BUSY field_type   *field= in_field + def->def_n_from;
    BUSY const UCHAR  *s= field->fld_str;
    BUSY int len_field= field->fld_len, len_skip= def->def_from_skip;
            /* add numeric key at tail of rec */
    if (len_field > len_skip)   as_num(s_num_key)= strtoNUM(s + len_skip);
    else                        as_num(s_num_key)= (NUMERIC)0;
    out_key->key_len=           (int)(sizeof(NUMERIC));
    out_key->key_offset=        (int)((UCHAR*)s_num_key -s_out);
}
 
PRIVATE
void make_sequence_key(out_key,in_field,def,s_num_key,s_out)
    key_type *out_key;
    field_type *in_field; def_type *def; /* these 2 vars are not reffered */
    NUMERIC *s_num_key; const UCHAR *s_out;
{
        /* add input record number as numeric key at tail of rec */
    as_num(s_num_key)=      (NUMERIC)recs_in;
    out_key->key_len=       (int)(sizeof(NUMERIC));
    out_key->key_offset=    (int)((UCHAR*)s_num_key -s_out);
}
 
/*
 *===================================================
 *          field split functions
 *===================================================
 */
 
PRIVATE
int split_blanks(field_out,s,max_field,len_text)
    BUSY field_type *field_out; BUSY const UCHAR *s;
    const int max_field; BUSY int len_text;
{
    BUSY const UCHAR *s_start;
    int               n_field= 0;
 
/* split_blanks() splits s into fields by blanks, and returns count of fields.
 * Blanks (white spaces, including Multi-byte char spaces) are treated idiosyncratically.
 * 1. 1 or more subsequent white spaces are taken to be only 1 field separator.
 * 2. Spaces at top of record not function as field separator.
 *    But, they are *retained* as is (NOT ignored).     [changed 92/03/14]
 * eg. (^ head of record, $ end of record)
 *      ^abc de f  $     : field #1 /abc/,   #2 /de/, #3 /f/, return value == 3
 *      ^abc        de f$: same as above
 *      ^  abc  de  f  $ : field #1 /  abc/, #2 /de/, #3 /f/, return value == 3
 *                                   ^ NB
 */
    s_start= start_new_field(field_out,s,0,len_text);
    n_field++; /* ie. every record has at least 1 field */
 
    while((n_field <= max_field) && (len_text > 0)) {
        BUSY int len= 0;
        while(isspace(*s) && (len_text > 0)) {  /* skip spaces at top */
            s++; len++; /* initial spaces are part of 1st field : 92/03/14 */
            len_text--;
        }
            /* now 's' points 1st non-space character of the field : or EOLN */
        while(len_text > 0) {   /* check EOLN */
            if (isspace(*s)) {  /* 's' points spaces at tail of this field */
                s++;
                len_text--;     /* do not count up 'len' */
                while(isspace(*s) && (len_text > 0)) {
                    s++;        /* eat up spaces at tail */
                    len_text--;
                }
                break;
            }
#if (DO_MBCHR)
            if (is_MBYTE(*s) && (len_text >= SZ_MBCHR)) {
                s+= SZ_MBCHR; len+= SZ_MBCHR; len_text -= SZ_MBCHR;
            } else
#endif
            {
                s++; len++; len_text--; /* ordinary 1 byted chars */
            }
        }
        /* we are here, because field found or reached end of rec.
         * in either case, we must terminate current field.
         */
        field_out->fld_len= len;
        s_start=start_new_field(++field_out,s,(int)(s -s_start) -len,len_text);
        n_field++;
    }
    return n_field;
}
 
PRIVATE
int split_chars(field_out,s,max_field,len_text)
    field_type *field_out; BUSY const UCHAR *s; const int max_field;
    BUSY int len_text;
{
    BUSY const UCHAR *s_start;
    int               n_field= 0;
 
    s_start= start_new_field(field_out,s,0,len_text);
    n_field++; /* ie. every record has at least 1 field */
 
    while((n_field <= max_field) && (len_text > 0)) {
        BUSY int            sz_delm= 0;
        while(len_text > 0) {
            BUSY delm_type  *delm_now= delm_tbl + *s;
#if (DO_MBCHR)
            BUSY DELMTR     *delm2= delm_now->delm_2byte;
            if (delm2)  {   /* it's MB_CHR */
                if (len_text < SZ_MBCHR)        {   /* incomplete Multi-byte char */
                    s+= len_text; len_text= 0;      /* skip this chunk */
                } else if (delm2[*(s + 1)] &DE_FIELD) {
                    sz_delm= SZ_MBCHR;      /* delimiter found */
                    break;
                } else {
                    s+= SZ_MBCHR; len_text-= SZ_MBCHR;  /* ordinary Multi-byte char*/
                }
            } else
#endif
            {               /* it's 1 byted char */
                BUSY DELMTR attr= delm_now->delm_attr;
                if (attr & DE_FIELD) {
                    sz_delm= 1; break;  /* delimiter found */
                } else  {
                    s++; len_text--;    /* ordinary 1 byted char */
                }
            }
        }
        /* we are here, because of delimiter found or reached end of rec.
         * in either case, we must terminate current field.
         */
        field_out->fld_len= (int)(s - s_start); /* determine the length */
        if (len_text >= sz_delm) {
                s+= sz_delm; len_text-= sz_delm;
        } else  len_text= 0;
 
        if (sz_delm) {  /* we came here because a delimiter found */
            s_start= start_new_field(++field_out,s,sz_delm,len_text);
            n_field++;
            sz_delm= 0;
        }
    }
    return n_field;
}
 
PRIVATE
const UCHAR *start_new_field(field_out,s_left,len_of_delimiter_left,
        set_length_of_rest_of_rec_as_tentative_field_length)
    field_type *field_out;
    BUSY const UCHAR *s_left;
    size_t  len_of_delimiter_left;
    int set_length_of_rest_of_rec_as_tentative_field_length;
{
    field_out->fld_tail= len_of_delimiter_left;
    field_out->fld_len=  set_length_of_rest_of_rec_as_tentative_field_length;
    return field_out->fld_str= s_left;
}
 
/*
 *===================================================
 *          output functions
 *===================================================
 */
 
 
PRIVATE
int sr_out(s,len) BUSY const char *s; BUSY size_t len;
{
    if (recs_out < 1) {
        if (is_severe(sr_error)) return EOF;   /* maybe error during input */
        if (!fout && !(fout= fopen(sr_fout_name,"w"))) {
            fprintf(stderr," cannot open output %s%c",sr_fout_name,C_LF);
            sr_error= add_error(E_FAIL_OUTPUT_OPEN);
            return EOF;
        }
    }
#define len_out     ((size_t)((size_t)len - (size_t)sr_sz_prefix))
    if (fwrite(s + sr_sz_prefix,sizeof(UCHAR),len_out,fout) != len_out) {
        fprintf(stderr," write error #%ld%c",recs_out+1,C_LF);
        return EOF;
    }
#undef len_out
    recs_out++;
    if ((recs_report > 0) && ((recs_out % recs_report)==0))
        fprintf(stderr," %5ld in, %5ld out...%c",recs_in,recs_out,C_LF);
    return 0;
}
 
PRIVATE
int uniq_out(s,len) BUSY const char *s; BUSY size_t len;
{
    static size_t   len_keep_old;
    static long int recs_dup= 0L;
 
    if ((recs_out < 1) && (recs_dup < 1L)) {
        if (is_severe(sr_error)) return EOF;   /* maybe error during input */
        if (!fout && !(fout= fopen(sr_fout_name,"w"))) {
            fprintf(stderr," cannot open output %s%c",sr_fout_name,C_LF);
            sr_error= add_error(E_FAIL_OUTPUT_OPEN);
            return EOF;
        }
        if (sr_n_keys > 1) sr_n_keys--; /* kill whole rec key : strict match */
        memcpy(keep_old,s,len); len_keep_old= len;
        recs_dup= 1L;
        return 0;
    }
    if (s && !sr_cmp((UCHAR*)s,keep_old,len,len_keep_old)) { /* == old one */
        recs_dup++;
    } else {    /* new rec : output old one */
#if (DO_UNIQ)
      BUSY ATTRIB is_dup= 0;
      if (sr_uniq_attrib & UAT_DUP_SEL) {           /* -uu or -ud */
          if (recs_dup > 1L) is_dup |= UAT_DUP;     /* not single uniq */
          if ((sr_uniq_attrib ^ is_dup) & UAT_DUP) {    /* if opt not match */
              memcpy(keep_old,s,len); len_keep_old= len;
              recs_dup= 1L; return 0;               /* no output */
          }
      }
      if (sr_uniq_attrib & UAT_NUMBERED) {
          BUSY size_t   len_buf;
          char          buf[80];
          sprintf(buf,"%ld%c",recs_dup,C_TAB);
          len_buf= strlen(buf);
          if (fwrite(buf,sizeof(UCHAR),len_buf,fout) != len_buf) {
              fprintf(stderr," write error #%ld%c",recs_out+1,C_LF);
              return EOF;
          }
      }
#endif
#define len_out     ((size_t)(len_keep_old - (size_t)sr_sz_prefix))
      if (fwrite(keep_old +sr_sz_prefix,sizeof(UCHAR),len_out,fout) !=len_out){
          fprintf(stderr," write error #%ld%c",recs_out+1,C_LF);
          return EOF;
      }
#undef len_out
      recs_out++;
      if ((recs_report>0) && ((recs_out % recs_report)==0))
          fprintf(stderr," %5ld in, %5ld out...%c",recs_in,recs_out,C_LF);
      memcpy(keep_old,s,len); len_keep_old= len;
      recs_dup= 1L;
    }
    return 0;
}
 
 
/*
 *===================================================
 *          prologue / epilogue
 *===================================================
 */
 
 
PRIVATE
int do_prologue(argc,argv) int argc; char **argv;
{
    int quit_now= 0;
 
    assert(EOF < 0);
    assert(ERROR_DURING_INPUT < 0);
    assert(isspace(C_LF));  /* needed in split_field() under paragraph mode */
    assert(COPT_ENDF == COPT_STDIN);  /* needed in f_next() */
    assert(N_MAX_FIELDS < INT_MAX);  /* needed in anal_field() */
 
    if (!(sr_defs= (def_type*)calloc((size_t)N_MAX_KEYS,sizeof(def_type)))
    ||  !(delm_tbl=(delm_type*)calloc((size_t)(SZ_DELM_TBL),sizeof(delm_type)))
       ) {
        fprintf(stderr," out of memory%c",C_LF);        return EOF;
    }
    if (makeup_delm_tbl()) {
        fprintf(stderr," out of memory%c",C_LF);        return EOF;
    }
    if ((quit_now= get_options(argc,argv)) < 0)         return EOF;
    if (sr_report) report_version();
    if (sr_uniq) {
        if (!(keep_old= (UCHAR*)calloc((size_t)sr_bytes,sizeof(UCHAR)))) {
            fprintf(stderr," out of memory%c",C_LF);    return EOF;
        }
    }
#if (DO_MBCHR)
    mbopen();
#if (DO_JPCOL)
    if (sr_do_mbyte_compare && !(sr_table_mbyte= opncoll())) {
       fprintf(stderr," out of memory%c",C_LF);         return EOF;
    }
#endif
#endif
    if (makeup_keydefs(sr_defs,sr_n_keys))              return EOF;
    if (sr_opt_report) report_option_status();
    if (quit_now)                                       return EOF;
    if (!(fin= f_next())) {
        if (sr_n_fin < 1)   fin= stdin;
        else    /* specified input file not found */    return EOF;
    }
    if (sr_sz_rec_separator < 1) {
        *sr_rec_separator= (UCHAR)C_LF;
        *(sr_rec_separator + 1)= (UCHAR)0;
        sr_sz_rec_separator= 1;
    }
    return 0;
}
 
PRIVATE
int do_epilogue(local_error, sortex_error) int local_error, sortex_error;
{
    char *msg_sortex_error;
#if (DO_MBCHR)
#if (DO_JPCOL)
    if (sr_table_mbyte)             clscoll(sr_table_mbyte);
#endif
    mbclose();
#endif
    if (keep_old)                   free((UNIV*)keep_old);
    if (delm_tbl) {
        if (delm_tbl->delm_2byte)   free((UNIV*)(delm_tbl->delm_2byte));
        free((UNIV*)delm_tbl);
    }
    if (sr_defs) {
        if (sr_defs->def_table)     free((UNIV*)(sr_defs->def_table));
        free((UNIV*)sr_defs);
    }
    if (is_severe(sortex_error)) sr_error= add_error(E_FATAL_IN_SORT);
    msg_sortex_error= sorterr(sortex_error);
    if (memcmp(msg_sortex_error,"():",3)) {
        fprintf(stderr,"%c error %s%c",C_LF,msg_sortex_error,C_LF);
    }
    return max((sortex_error%10),(local_error%10));
}
 
PRIVATE
int makeup_keydefs(defs,n_defs) BUSY def_type *defs; int n_defs;
{
    BUSY UCHAR *tbl;
    int         i;
 
        /* 0. add global attrib as pivot field (for complete match) */
    if (sr_whole.def_asis) sr_whole.def_attrib &=(~(AT_REVERSE|AT_EMPTY_HIGH));
    memcpy(defs + n_defs -1,&sr_whole,sizeof(def_type));
 
        /* 1. alloc char conv table */
    tbl= (UCHAR*)calloc((size_t)n_defs,(size_t)(UCHAR_MAX+1)*sizeof(UCHAR));
    if (!tbl) {
        fprintf(stderr," out of memory%c",C_LF); return EOF;
    }
        /* 2. set each key attrib */
    for(i= 0; i< n_defs; defs++, i++) {
        BUSY int    c;
        BUSY ATTRIB attr;
        defs->def_table= tbl; attr= defs->def_attrib;
        for(c= 0; c <= (int)UCHAR_MAX; c++, tbl++) {
            *tbl= (UCHAR)c;
            if (attr& AT_PRINT) *tbl=(UCHAR)((isprint(c)||isspace(c))? c: 0);
            if (attr& AT_ALNUM) *tbl=(UCHAR)((isalnum(c)||isspace(c))? *tbl:0);
            if (attr& AT_LOWER) *tbl=(UCHAR)((isupper(c))? tolower(c): *tbl);
                                                        /* ^ fix 91/09/21 */
        }
        defs->def_order= (attr & AT_REVERSE) ? (-1) : 1; /* 90/08/15 */
 
            /* set compare functions : 92/03/11 */
        if ((attr & AT_NUMERIC) || defs->def_asis)
            defs->def_f_cmp= cmp_numeric;
#if (DO_JPCOL)
        else if (attr & AT_JPLOCALE)        {
            if ((attr & (AT_LOWER|AT_PRINT|AT_ALNUM)) && !sr_warned_JP) {
                fprintf(stderr," (warn) -J overriding -f/-d/-i%c",C_LF);
                sr_warned_JP++;
            }
            defs->def_f_cmp= cmp_JP_locale;
        }
#endif
        else if (attr & (AT_LOWER|AT_PRINT|AT_ALNUM))
            defs->def_f_cmp= cmp_by_table;
 
        if (attr & AT_EMPTY_HIGH) { /* 93/09/29 */
            defs->def_left=   1 * defs->def_order;
            defs->def_right= -1 * defs->def_order;
        } else {
            defs->def_left=  -1 * defs->def_order;
            defs->def_right=  1 * defs->def_order;
        }
    }
      /* 3. report status */
#if (DO_DEBUG)
  if (sr_give_why) sr_opt_report++;
#endif
    return 0;
}
 
PRIVATE
int makeup_delm_tbl()
{
    BUSY DELMTR *buf;
    int         n_mbyte, byte;
 
    if ((n_mbyte= mbopen()) < 0) return EOF;
    buf= (DELMTR*)calloc((size_t)(n_mbyte * (UCHAR_MAX+1)),sizeof(DELMTR));
    if (!buf)  return EOF;
    delm_tbl->delm_2byte= buf; /* mark [0] for ease of free() */
    for(byte= 0; byte <= UCHAR_MAX; byte++) {
        if (is_MBYTE(byte)) {
            (delm_tbl + byte)->delm_2byte= buf;
            buf+= (UCHAR_MAX + 1);
        }
    }
    return 0;
}
 
/*
 *===================================================
 *          command-line options
 *===================================================
 */
 
PRIVATE
int get_options(argc,argv) int argc; char **argv;
{
  int   n_num_keys= 0, delm_field= 0, delm_paragraph= 0, n_argc;
  int   quit_now= 0;
#if defined(HAS_FUNCTION_PROTOTYPE)
  int   (*new_paragraph)(FILE*);
#else
  int   (*new_paragraph)();
#endif
  sr_length= (int)RECL_MAX;
 
  for(n_argc= 1; n_argc < argc; n_argc++) {
    BUSY char *s_opt= argv[n_argc];
    char    old_p[RECL_MAX];
    if ((*s_opt== COPT_BEGF)||(*s_opt== COPT_ENDF)||(*s_opt== COPT_THISF)) {
      BUSY char *s;
      for(s= s_opt + 1; *s; s++) {
        switch(*s) {
          case 't' : s++;
              if (*s) {
                   if (set_delm_table(s,DE_FIELD))                  return EOF;
              } else {
                   fprintf(stderr," specify char as -tC or -t0x43%c",C_LF);
                   return EOF;
              }
              delm_field++;
              goto next;
          case 'p' : s++;
              if (*s) {
                  if (set_delm_table(s,DE_PRGRF))                   return EOF;
                  new_paragraph= fgetc;
              } else {  /* 2 or more vacant lines to be separator */
                  if (set_pattr(C_SIGN_VACANT_REC,DE_PRGRF))        return EOF;
                  new_paragraph= get_c_vacant;
              }
              if (get_c_paragraph && (get_c_paragraph != new_paragraph)) {
                 fprintf(stderr," (warn) -p%s overriding -p%s%c",s,old_p,C_LF);
              }
              strcpy(old_p,s);
              get_c_paragraph= new_paragraph;
              delm_paragraph++;
              goto next;
          case 'o' : sr_fout_name=   s + 1;                         goto next;
              /* key attributes */
          case 'a' : sr_whole.def_asis= 1;                          break;
          case 'b' : sr_whole.def_attrib |= AT_CUT_BLANK;           break;
          case 'd' : sr_whole.def_attrib |= AT_ALNUM;               break;
          case 'f' : sr_whole.def_attrib |= AT_LOWER;               break;
          case 'h' : sr_whole.def_attrib |= AT_EMPTY_HIGH;          break;
          case 'i' : sr_whole.def_attrib |= AT_PRINT;               break;
#if (DO_JPCOL)
          case 'k' :
              fprintf(stderr,"(note) -k shall be obsolete. use -J%c",C_LF);
              /* go thru -J */
          case 'J' : sr_whole.def_attrib |= AT_JPLOCALE;
                     sr_do_mbyte_compare=1;                         break;
#endif
          case 'n' : sr_whole.def_attrib |= AT_NUMERIC;             break;
          case 'r' : sr_whole.def_attrib |= AT_REVERSE;             break;
                                  /* ^ bug fix : 90/08/15 thanks to t.k. */
#if (DO_SQWZ)
          case 's' : sr_squeeze++; sr_kill_vacant_lines++;          break;
#endif
#if (DO_UNIQ)
          case 'u' : sr_uniq= 1;
              sr_uniq_attrib |= UAT_DUP;
              sr_uniq_attrib &= UAT_DUP_SEL;
              while(*(++s)){
                switch(*s) {  /* options changed to upper-case 91/03/21 */
                  case 'U' : sr_uniq_attrib &= ~UAT_DUP;
                             sr_uniq_attrib |=  UAT_DUP_SEL;        break;
                  case 'D' : sr_uniq_attrib |=  UAT_DUP;
                             sr_uniq_attrib |=  UAT_DUP_SEL;        break;
                  case 'N' : sr_uniq_attrib |=  UAT_NUMBERED;       break;
                  default  :
                      fprintf(stderr," bad suboption %c to -u%c",*s,C_LF);
                      return EOF;
                }
              }
              goto next;
#else
          case 'u' : sr_uniq= 1;                                    break;
#endif
          case 'T' : sr_temp= s + 1;                                goto next;
          case 'l' :
          case 'L' :
            {
              BUSY long int ls= atol(s + 1);
              if ((ls < 1) || ((VERYLONG)(ls + 1) >= (VERYLONG)INT_MAX)) {
               fprintf(stderr," sorry : -L must be <%d%c",INT_MAX-1,C_LF);
                                                                    return EOF;
              }
              if ((int)ls > sr_length) sr_user_specification_is_longer++;
              sr_length= (int)ls;
            }
                                                                    goto next;
          case 'M' : sr_mem_KB=   atoi(s + 1);                      goto next;
          case 'W' : sr_files=    atoi(s + 1);                      goto next;
          case 'z' :
          case 'Z' : sr_verbose= 1;
                     if (isdigit(*(s + 1))) {
                        recs_report= atol(s + 1);                   goto next;
                     }
                     break;
          case 'v' : sr_report= sr_opt_report= 1;
#if (DO_DEBUG)
                     sr_give_why= 1;
#endif
                     break;
          case 'V' : sr_report= quit_now= 1;
                     break;
              /* sort key fields */
          case '0' : case '1' : case '2' : case '3' : case '4' :
          case '5' : case '6' : case '7' : case '8' : case '9' :
            if (s <= (s_opt + 1)) { /* if 0-9 appeared at head of option */
                BUSY char *next_opt= (char*)0;
                BUSY int to_add;
                if ((n_argc+ 1) < argc) {
                  next_opt= argv[n_argc + 1];
                  if ((*next_opt != COPT_ENDF)|| !isdigit(*(next_opt +1))){
                    next_opt= (char*)0;
                  }
                }
                if (((*s_opt != COPT_BEGF) && (*s_opt != COPT_THISF))
                ||  ((to_add= anal_field(s_opt,next_opt)) < 0)) {
                    fprintf(stderr," field format? (%s %s):expect %s%c",
                        s_opt,(next_opt)?next_opt : "",sr_opt_field,C_LF);
                                                                    return EOF;
                }
                n_num_keys+= to_add;
                if (next_opt) n_argc++;                             goto next;
            } /* else error : break thru default */
          default  :
            fprintf(stderr," %s %s%c",sr_opt1,sr_opt2,C_LF);
            fprintf(stderr," bad option %c in %s%c",*s,s_opt,C_LF);
            return EOF;
        }
      }
      next :
      ;
    }
  }
  if (sr_whole.def_attrib & AT_NUMERIC)   n_num_keys++;
  if (sr_whole.def_asis)                  n_num_keys++;
  sr_n_keys++; /* +1 for the last whole-rec key */
  sr_sz_keys=     force_ALIGN(sizeof(key_type) * sr_n_keys);
#if (DO_DEBUG)
  if (sr_give_why) {
    fprintf(stderr," :setup:keys(%d), sizeof numeric(%u),aligned keyloc(%d)%c",
      n_num_keys,(unsigned)sizeof(NUMERIC),sr_sz_keys,C_LF);
  }
#endif
  sr_sz_prefix=   sr_sz_keys + (int)(sizeof(NUMERIC) * n_num_keys);
  if (((VERYLONG)(sr_length + 1) + (VERYLONG)sr_sz_prefix) >=
              (VERYLONG)(INT_MAX - RECL_MAX)) {
      fprintf(stderr," sorry : -L too long, max possible will be < ca %d.%c",
              (int)(INT_MAX - RECL_MAX - sr_sz_prefix -1),C_LF);
      return EOF;
  } else sr_bytes= (sr_length + 1) + sr_sz_prefix; /* 92/08/09 */
  fout= (sr_fout_name && *sr_fout_name) ? (FILE*)0      : stdout;
  sr_split_field= (delm_field)        ? split_chars     : split_blanks;
  rec_next=       (delm_paragraph)    ? get_paragraph   : get_rec;
  return quit_now;
}
 
#define ERR_RETURN (-1)
PRIVATE
int anal_field(opt_left,opt_right) BUSY char *opt_left, *opt_right;
{
  auto numf_type  left_numf, right_numf;
  int max_n_field, n_num_keys;
 
  memset(&left_numf,  0, sizeof(numf_type)); /* clear */
  memset(&right_numf, 0, sizeof(numf_type));
  if (num_field(&left_numf,opt_left))     return ERR_RETURN;
  if (num_field(&right_numf,opt_right))   return ERR_RETURN;
  if ((n_num_keys=
      regist_field(&left_numf,(opt_right)? &right_numf:(numf_type*)0)) < 0) {
      return ERR_RETURN;
  }
  sr_n_keys++;
  max_n_field= max(left_numf.numf_number,right_numf.numf_number);
  if (max_n_field > sr_max_n_field) sr_max_n_field= max_n_field;
  return n_num_keys;
}
 
PRIVATE
int num_field(out_numf, s) BUSY numf_type *out_numf; BUSY char *s;
{
    BUSY ATTRIB     attr= 0;
    BUSY int        len_to_skip= 0;
    BUSY long int   number_of_field= 0;
    char            *s_next, *s_original= s;
 
    if (!s) return 0;
    out_numf->numf_optchar= *s++;
    number_of_field= strtol(s,&s_next,10);
    if ((number_of_field < 0) || (number_of_field >= (long int)N_MAX_FIELDS)) {
          fprintf(stderr," field number(%s) must be <%d%c",s,N_MAX_FIELDS,C_LF);
          return ERR_RETURN;
    }
    if (*s_next == '.') {
      BUSY long int tmp_long= strtol(s_next + 1,&s_next,10);
        if ((tmp_long < 0) || (tmp_long >= (long int)INT_MAX)) {
            fprintf(stderr," field skip(%s) must be <%d%c",s,INT_MAX,C_LF);
            return ERR_RETURN;
        }
      len_to_skip= (int)tmp_long;
    }
    for(; *s_next; s_next++) {
        switch(*s_next) {
#if 0
          case 'a' : assert(!"cannot reach here");  break;
          /* notice, 'asis' is nonsense except for the last default key.
           * thus it is commented out here.
           */
#endif
          case 'b' : attr |= AT_CUT_BLANK;          break;
          case 'd' : attr |= AT_ALNUM;              break;
          case 'f' : attr |= AT_LOWER;              break;
          case 'h' : attr |= AT_EMPTY_HIGH;         break;
          case 'i' : attr |= AT_PRINT;              break;
#if (DO_JPCOL)
          case 'k' :
            fprintf(stderr,"(note) -k shall be obsolete. use -J%c",C_LF);
                /* go thru -J */
          case 'J' : attr |= AT_JPLOCALE;
                     sr_do_mbyte_compare=1;         break;
#endif
          case 'n' : attr |= AT_NUMERIC;            break;
          case 'r' : attr |= AT_REVERSE;            break;
          default  :
            fprintf(stderr," %s %s%c",sr_opt1,sr_opt2,C_LF);
            fprintf(stderr," bad option %c in %s%c",*s_next,s_original,C_LF);
            return ERR_RETURN;
        }
    }
    out_numf->numf_number=  (int)number_of_field;
    out_numf->numf_skip=    (int)len_to_skip;
    out_numf->numf_attr=    attr;
    return 0;
}
 
PRIVATE
int regist_field(left,right) BUSY numf_type *left, *right;
{
  BUSY def_type   *def= sr_defs + sr_n_keys;
  BUSY char       c_head;
  int             n_num_keys= 0;
 
      /* 1. register LEFT */
  assert(left);
  c_head= left->numf_optchar;
  assert((c_head == COPT_BEGF) || (c_head == COPT_THISF));
  if ((sr_n_keys+ 1) >= N_MAX_KEYS) {     /* +1 for whole-rec key */
      fprintf(stderr," too many key defs: must <%d%c",N_MAX_KEYS,C_LF);
      return ERR_RETURN;
  }
  def->def_n_from=      left->numf_number;
  def->def_from_skip=   left->numf_skip;
  def->def_attrib=      (left->numf_attr)?left->numf_attr:sr_whole.def_attrib;
  def->def_n_to=        (c_head == COPT_THISF)? def->def_n_from + 1: 0;
  def->def_to_skip=   0;              /* default */
  def->def_to_attrib= (ATTRIB)0;      /* default */
        if (((def->def_attrib) & AT_NUMERIC) || def->def_asis) n_num_keys++;
  if (def->def_n_from > sr_max_n_field) sr_max_n_field= def->def_n_from;
  if (def->def_n_to   > sr_max_n_field) sr_max_n_field= def->def_n_to;
  if (!right) return n_num_keys;
 
      /* 2. register RIGHT */
  if (c_head == COPT_THISF) return ERR_RETURN;
  c_head= right->numf_optchar;
  assert(c_head == COPT_ENDF);
  def->def_n_to=      right->numf_number;
  def->def_to_skip=   right->numf_skip;
  def->def_to_attrib= right->numf_attr;
 
  if ((def->def_n_from > def->def_n_to)
  || ((def->def_n_from == def->def_n_to)
  &&  (def->def_from_skip > def->def_to_skip))) { /* left > right */
        fprintf(stderr," field from(+%d.%u) > to(-%d.%u)%c",
          def->def_n_from,(unsigned int)(def->def_from_skip),
        def->def_n_to,  (unsigned int)(def->def_to_skip),C_LF);
        return ERR_RETURN;
  }
  if (def->def_n_to > sr_max_n_field) sr_max_n_field= def->def_n_to;
  return n_num_keys;
}
#undef ERR_RETURN
 
PRIVATE
int set_pattr(v,attr_set) const int v; const int attr_set;
{
  if (v) {
      (delm_tbl + v)->delm_attr |= (DELMTR)attr_set;
      if ((attr_set & DE_PRGRF) && (v == C_SIGN_VACANT_REC)) {
          *sr_rec_separator= (UCHAR)C_LF;  /* can be 2 or more */
          sr_sz_rec_separator= 1;
      }
  }
  return 0;
}
 
PRIVATE
int set_delm_table(ss,attr_set) const char *ss; const int attr_set;
{
    BUSY UCHAR *s= (UCHAR*)ss;
    if (!s) return EOF;
    while(*s) {
        BUSY delm_type *delm;
        auto size_t len= (0);
        UCHAR buf[SZ_MBCHR + 1];
        if (!(s= get_single_char(buf,&len,s))) return EOF;
        delm= delm_tbl + *buf;
#if (DO_MBCHR)
        if (len > 1) {
            *(delm->delm_2byte + *(buf+1)) |= (DELMTR)attr_set;
        } else
#endif
        {
            (delm->delm_attr) |= (DELMTR)attr_set;
        }
        if (attr_set & DE_PRGRF) memcpy(sr_rec_separator,buf,len);
        sr_sz_rec_separator= len;
    }
    return 0;
}
 
PRIVATE
UCHAR *get_single_char(buf,len,s) UCHAR *buf; size_t *len; BUSY UCHAR *s;
{
#define ERR_RETURN  ((UCHAR*)0)
#if (DO_MBCHR)
    if ((delm_tbl + *s)->delm_2byte) { /* it's Multi-byte char 1st byte */
        if (!(*(s + 1))) {
            fprintf(stderr," fragmental Multi-byte char (%02x%02x)%c",
                *s,*(s + 1),C_LF);                          return ERR_RETURN;
        }
        memcpy(buf,s,(size_t)SZ_MBCHR); *len= (size_t)SZ_MBCHR; s+= SZ_MBCHR;
    } else
#endif
    {   /* it's octal/hexadecimal or normal 1-byted char */
        if (*s == '0' && *(s + 1)) { /* assume hexadecimal or octal */
            if (!(s= decode_hex(buf,len,s)))                return ERR_RETURN;
        } else {    /* normal 1-byted chars */
          *buf= *s++; *len= 1;
        }
    }
    return s;   /* next decode point */
#undef ERR_RETURN
}
 
PRIVATE
UCHAR *decode_hex(buf, len, s) UCHAR *buf; size_t *len; UCHAR *s;
{
#define ERR_RETURN  ((UCHAR*)0)
    auto char *s_next;
    long int value= strtol((char*)s,&s_next,0);
    *buf= 0;
    if (*s_next) {
        fprintf(stderr," bad octal/hexadecimal %s%c",s,C_LF);
                                                            return ERR_RETURN;
    }
#if (DO_MBCHR)
    if ((value > (long int)USHRT_MAX) || (value < 1))
#else
    if ((value > (long int)UCHAR_MAX) || (value < 1))
#endif
    {
        fprintf(stderr,
          " sorry : such a char (%s) cannot be delimiter%c",s,C_LF);
                                                            return ERR_RETURN;
#if (DO_MBCHR)
    } else if (value > UCHAR_MAX) { /* multi-byte hex */
        BUSY unsigned int value_shift, byte_lower, byte_higher;
        value_shift= (unsigned)value; /* (value < USHRT_MAX) is cheked above */
        byte_lower=  value_shift % (UCHAR_MAX + 1);
        byte_higher= value_shift / (UCHAR_MAX + 1);
        assert(byte_higher <= UCHAR_MAX);
        assert(byte_lower  <= UCHAR_MAX);
        buf[0]= byte_higher; buf[1]= byte_lower;
        *len= SZ_MBCHR;
#endif
    } else {
        assert(value <= UCHAR_MAX);
        *buf= (unsigned)value; *len= 1;
    }
#if (DO_MBCHR)
    if ((value <= UCHAR_MAX) && is_MBYTE(value)) {
        fprintf(stderr," sorry: 0x%02x cannot be delimiter :Multi-byte area%c",
            (unsigned)value,C_LF);
                                                            return ERR_RETURN;
    } else if ((value > UCHAR_MAX) && !is_MBYTE(*buf)) {
        fprintf(stderr," sorry: 0x%04x is not valid Multi-byte character%c",
            (unsigned)value,C_LF);
                                                            return ERR_RETURN;
    }
#endif
    return (UCHAR*)s_next;
#undef ERR_RETURN
}
 
PRIVATE
void report_version ()
{
    char buf[RECL_MAX];
    strcpy(buf,version);
#if (DO_JPCOL)
    strcat(buf,"J");        /* with Multi-byte Japanese locale lexical sort */
#endif
#if (!DO_UNIQ)
    strcat(buf,"N");        /* no UNIQ */
#endif
    fprintf(stderr," %s(%s)%c",copyright,buf,C_LF);
}
 
PRIVATE
void report_option_status ()
{
    BUSY int n, code= mbtelcd();
    fprintf(stderr,"  Last updated : %s%c",Last_updated,C_LF);
#if (defined(__DATE__) && defined(__TIME__))
    fprintf(stderr,"  compiled "
#if (defined(OS_KNOWN) && defined(CP_KNOWN))
    "under " OS_KNOWN "OS by " CP_KNOWN "compiler "
#endif
    "on : %s %s%c",__DATE__,__TIME__,C_LF);
#endif
 
#if (DO_MBCHR)
    fprintf(stderr,"  Environment  : %s endian, Kanji code is %s%c",
        (code/10)?(((code/10)==ENDIAN_BIG)?"big":"little"):"unknown",
        (code%10)?(((code%10)==KCODE_SJIS)?"Shift-JIS":"EUC"):"unknown",C_LF);
#endif
    fprintf(stderr,"%c :setup informations%c",C_LF,C_LF);
    fprintf(stderr,"  %d (%s) work files to prefix %s.%c",
        sr_files,((sr_files<1)?"default":"specified"),
        (sr_temp)?sr_temp:"(default)",C_LF);
    fprintf(stderr,"  write to %s%c",(sr_fout_name)?sr_fout_name:"stdout",C_LF);
    fprintf(stderr,"  split fields by %s, records by %s%c",
        (sr_split_field==split_chars)? "specified delimiter":"white spaces",
        (rec_next==get_paragraph)? "paragraphs" : "LFs", C_LF);
 
    fprintf(stderr,"  keys=%d, max field number=%d%c",
        sr_n_keys,sr_max_n_field,C_LF);
    for(n= 0; n < sr_n_keys; n++) {
        BUSY def_type *key= sr_defs + n;
        BUSY ATTRIB  attr= key->def_attrib;
        char optbuf[RECL_MAX], field_buf[80];
        *optbuf= (char)0;
        if (attr & AT_CUT_BLANK)    strcat(optbuf," -b");
        if (attr & AT_LOWER)        strcat(optbuf," -f");
        if (attr & AT_PRINT)        strcat(optbuf," -i");
        if (attr & AT_ALNUM)        strcat(optbuf," -d");
        if (attr & AT_REVERSE)      strcat(optbuf," -r");
        if (attr & AT_NUMERIC)      strcat(optbuf," -n");
        if (attr & AT_JPLOCALE)     strcat(optbuf," -J");
        if (attr & AT_EMPTY_HIGH)   strcat(optbuf," -h");
        if (key->def_asis)          strcat(optbuf," -a");
        sprintf(field_buf,"%d.%d", key->def_n_to, key->def_to_skip);
        if (!strcmp(field_buf,"0.0")) strcpy(field_buf,"End of rec");
        fprintf(stderr,"  key#%d (%d.%d to %s) %s%c",
            n, key->def_n_from, key->def_from_skip, field_buf, optbuf, C_LF);
    }
    fprintf(stderr,"%c",C_LF);
}
 
 
/*
 *===================================================
 *      functions needed for specified options
 *===================================================
 */
 
#if (DO_SQWZ)
PRIVATE
int entire_space(s,len) BUSY const UCHAR *s; BUSY int len;
{
    while((len > 0) && isspace(*s)) {
        s++; len--;
    }
    return (len > 0) ? 0 : 1;   /* NB. NULL recs are treated as spaces */
}
#endif
 
#if (FORCE_UC)
PRIVATE
int unsig_memcmp(s,t,len) BUSY const UCHAR *s, *t; BUSY size_t len;
{
    while(len-- > 0) {
        BUSY int diff;
        if (diff = (int)(*s++) - (int)(*t++)) return diff;
    }
    return 0;
}
#endif
 
#if (defined(BAD_STRTOD))
#include <float.h>
#include <math.h>
 
PRIVATE
double atof_simple(s) BUSY char *s;
{
    BUSY double     prefix, suffix;
#define VERY_LARGE  ((double)FLT_MAX)
    BUSY double     div_by_10;
    BUSY int        sign= 1;
 
    while(isspace(*s)) {
        s++;
    }
    if (*s == '-') {
        sign= -1; s++;
    } else if (*s == '+') {
        s++;
    }
    prefix= suffix= (double)0; div_by_10= (double)1;
    while(isdigit(*s)) {
        if (prefix < (VERY_LARGE / (double)10))
                prefix= (prefix * (double)10) + (double)(*s - '0');
        else    prefix= VERY_LARGE;
        s++;
    }
    if (*s == '.') {
        s++;
        while(isdigit(*s)) {
            if (suffix < (VERY_LARGE / (double)10)) {
                    suffix= (suffix * (double)10) + (double)(*s - '0');
                    div_by_10*= (double)10;
            } else {
                    suffix= VERY_LARGE;
            }
            s++;
        }
    }
    if (prefix >= VERY_LARGE)   return (double)sign * (double)(VERY_LARGE);
    if (!prefix && !suffix)     return (double)0;
    return (double)sign * ((double)prefix + ((double)suffix / div_by_10));
#undef VERY_LARGE
}
#endif
 
#if (DO_DEBUG)
PRIVATE char *cut_lf(s) char *s;
{
    BUSY char *s_lf;
    for(s_lf= s; *s_lf; s_lf++) {
        if (*s_lf == C_LF) break;
    }
    *s_lf= 0;
    return s;
}
#endif
 
#if 0
 history
 v00-00     89/12/15
 v00-01     90/03/12-14
 v00-02     90/03/17 bug in set_keydefs() fixed.
 v00-03     90/03/21 abandon execution when "too long rec" error.
 v00-04a    90/06/07 -u (unique), as requested by Gen,TAKAGI.
 v00-04b    90/06/12 -bdinf bug fix, detailed spec defined.
 v00-04c    90/07/26 bug in -b of 'to' field fixed.
 v00-04d    90/07/31 -U deleted(substituted by +0 -u)
 v00-04e    90/08/15 bug in -r fixed. thanks to t.k.(AQE27688@PC-VAN)
            -r made ATTRIB, then def_order is set to 1/-1.
 v00-04f    90/08/25 bug in -n && -u is fiexed. uniq_out() rewritten.
            SPEC_UNCOMMON : -uc, -ud, -uu -s(squeeze) are added.
            -uc : requested by t.k.(AQE27688@PC-VAN),
            -s  : requested by TAKAHASHI,Hangyo(GDJ85261@PC-VAN). Thanks.
 v00-05a    91/02/06 alignment on word boundaries, port to Mac-OS
 v00-05b    91/02/26 alignment fix
 v00-05bK   91/03/17-19 KANA lexical sort (with bugs, lamentably :-<)
 v00-05c    91/03/21 bug in cmp_modify fix,
            SPEC_UNCOMMON made default (sub-options changed to uppercase),
            collJP() enlarged,
 v00-05d    91/04/25 bug in cmp_modify fix (thanks to hortense@mix)
            bug>when return of collJP() == 0, gave up comparison there
 v00-05e    91/04/29 - 91/05/10 bugs in make_key()/split_fields fixed.
            new feature -p (paragraph) requested by Hangyo(GDJ85261@PC-VAN).
 v00-05f    91/05/12 - 18
            bug in get_paragraph() fixed thanks to Hangyo(GDJ85261@PC-VAN).
            bug in collJP() (q.v.) fixed.
            KANJI enabled as delimiters, KANJI spaces recognized as blanks.
 v00-06     91/06/22 - 24
            max record length (-L) enlarged to SIZE_T_MAX.
            sortex() v03-15
 v00-06a    91/07/07 version format changed, requested by TAKAHASI,Hangyo
 v00-06b    91/09/21 bugs in -dfi fixed.
            1. -f was ineffective when -f and/or -i specified together.
            2. KANJIs were treated as 1 byted and thus gave unreliable results.
 v00-06b    bis : 91/10/19 Human68k version thanks to Kaoru MAEDA (pcs30367)
 v00-06c    91/11/12
            1. in sr_cmp() NUMERIC fields tightly cut off at right
            2. 2 byted spaces are treated as space or not by +-B option switch
 v00-06d    92/01/05 - 12
            1. strtod() replaced by atof_simple() because of bug in MS-c 6
            2. bug in split_spaces() fixed. (spaces at top of rec miscounted)
 v00-06e    92/02/19 - 29
            1. data spec clarified : record be terminated with Record-Separator.
            this affects get_rec()/get_paragraph(), and final field of 1 rec.
            2. bugs in make_key() (calculation of field lengths) fixed.
            3. reject nonsence field specification (eg. sortf +2 -1).
            4. -s (squeeze) made default.
 v00-06f    92/03/10 - 15
            1. keep NUL(0x00) in input record as is. Because of this change,
            1-1. fwrite() replaced fputs() in sr_out()/uniq_out(),
            1-2. end of rec tested by length in split_blanks()/split_chars().
            2. split_blanks() revised to RETAIN initial spaces.
            This is done so as to be in accordance with UNIX sort commands.
            3. -+B option (2 byted spaces) abolished.
            4. sr_cmp() routine replaced by functon-pointer calls.
            5. sort record (handled to sortex()) reconfigured, so that record body.
            comes last (and numeric keys comes before record body).
            6. unsig_memcmp() added for some wicked (&& ANSI-contradicting)
            signed-compare libraries (ie. Sun)
 v00-07 92/03/28
            bug in 'sr_sz_keys' fixed. (v00-06f only '+n's, not '-n's, counted).
            This bug was reported by T.Ueki(NBD00327@Nifty), aMI(KUH95291@PC-VAN).
 v00-07a    92/08/09
            with revision of sortex().
            To avoid possible memory overwrite, sr_length & sr_bytes distinguished.
            This bug was reported by roku@mix.
 v00-08 93/09/04
            new option -a : add original sequence as the last key
 v00-08a, 08b 93/09/15-19,25,28
            modify -p :  to recognize 2 or more vacant lines as paragraph delimiter
            bugs fixed thanks to aMI(KUH95291@PC-VAN), Hangyo(GDJ85261@PC-VAN),
            modify -s :  to ignore multiple vacant lines in -p
            this feature is thanks to 5G(EHF66843@PC-VAN).
            -  : to positively specify stdin (requested by aMI(KUH95291@PC-VAN))
            =m : for +m -n (n== m+1) (requested by Hangyo(GDJ85261@PC-VAN))
 v00-08c    93/10/15
            sortex() revision.
            bug in =m.i -n (wrongly assumed +m.i -n.i) fixed (reported by Hangyo)
 v00-08d    93/11/27,28
            bug in -tSOMETHING is fixed (reported by Kuusan(NLF06372@PC-VAN))
            bug in make_string_key() fixed
            -v to check field configurations
 V00-09a    94/04/23
            Xdef/Ylib changed
            bug in make_string_key() fixed (reported by MASA(LBB53627@PC-VAN))
 V00-10     94/04/30 - 94/05/02
            collJP() replaces YcmpKlc() for Multi-byte comparison,
            much more closer to tentative JIS collation draft (hopefully)
 V00-10a(bis) 94/05/09
            bug with -t (Multibyte) fixed
            bug with fclose(NULL) (-oFILE and no input) fixed
 V00-10b      94/08/20
            bug in specification of split_char() / split_blank() fixed.
            now, every record has at least 1 field.
            in old versions, records terminated by delimiters lost that very
            last field.
            This bug was reported by MIZUHA(RRB94102@pc-van.or.jp).
#endif

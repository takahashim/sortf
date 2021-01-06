/* mtconfig.ms : OS/compiler dependent features : X68000/Human-K
 * copyright (C) Masayuki TOYOSHIMA,1988. All rights reserved.
 * v01-00 88/06/17 newly replacing old "COMPILER.H"
 * v02-00 94/04/28 newly replacing old "Xsysdef.h"
 * MS-DOS,OS/2,Mac-OS,UNIX generic version 94/05/01
 * not yet tested on X68000 / no support from the author
 */
#ifndef mtconfg_h
#define mtconfg_h   1
 
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
 
#define OS_HUMAN        1
#define OS_KNOWN		"Human "
#if (defined(__GNUC__))
#define CPP_GNU
#define CP_KNOWN				"GNU "
#define HAS_SIZE_T
#define HAS_FUNCTION_PROTOTYPE  1
#else
#define CPP_HUMAN
    /* no function prototype */
#endif
 
/****************************
 *  1. types                *
 ****************************
 */
 
    /* size_t  :
	 *
  	 * if the environment/compiler typedef's size_t, and not #define's _SIZE_T,
	 * you must #define _SIZE_T here.
	 *
	 */

#if (defined(__sys_stdtypes_h) || defined(__stdtypes_h))
	/* assume size_t is defined in /usr/include/sys/stdtypes.h */
#ifndef HAS_SIZE_T	
#define HAS_SIZE_T		
#endif
#endif

#ifndef HAS_SIZE_T
#ifndef _SIZE_T
#ifndef _SIZE_T_
#ifndef _size_t
#ifndef _size_t_
#ifndef __SIZE_T
#ifndef __SIZE_T_
#ifndef __size_t
#ifndef __size_t_
#ifndef _SIZE_T_DEFINED
#ifndef __SIZE_T_DEFINED
#ifndef _size_t_defined
#ifndef __size_t_defined
    typedef unsigned int size_t;
#define __size_t_defined
#define _size_t_defined
#define __SIZE_T_DEFINED
#define _SIZE_T_DEFINED
#define __size_t_
#define __size_t
#define __SIZE_T_
#define __SIZE_T
#define _size_t_
#define _size_t
#define _SIZE_T_
#define _SIZE_T
#define HAS_SIZE_T
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
 
#ifndef SIZE_T_MAX
#if (defined(CPP_UNIXPCC))
#define SIZE_T_MAX      INT_MAX
#else
#define SIZE_T_MAX      UINT_MAX
#endif
#endif

#ifndef SZ_MBCHR
#define SZ_MBCHR        2   /* bytes of multi-byte char */
#endif
 
    /* alignment for memory allocation for word-machines */
#if (defined(OS_MSDOS)||defined(OS_OS2))
    /* no alignment necessary in i80x86 family */
#define SZ_ALIGN        (1)             /* no alignment */
#define force_ALIGN(n)  (n)             /* no alignment */
#define add_ALIGN(p,n)  ((char*)(p) + force_ALIGN(n))
#else
    /* else assume word-bounded machines */
#define SZ_ALIGN        (sizeof(double))
#define force_ALIGN(n)  (((n)%SZ_ALIGN)?((n) + SZ_ALIGN - ((n)%SZ_ALIGN)):(n))
#define add_ALIGN(p,n)  ((char*)(p) + force_ALIGN(n))
#endif
 
#define BUSY            register
#ifndef PRIVATE
#define PRIVATE         static
#endif
#ifndef DCL_PRIVATE
#define DCL_PRIVATE     PRIVATE
#endif
 
#if (defined(HAS_FUNCTION_PROTOTYPE))
    /* assume void pointer, const, volatile, func(arg,arg,...) */
#define UNIV            void
#include <stdarg.h>
#else
    /* old-fashoned ones */
#define UNIV            char
#define const
#define volatile
#endif
 
typedef unsigned char   UCHAR;
typedef unsigned long   VERYLONG;
 
/****************************************
 *  2. special char/string notations    *
 ****************************************
 */
#define C_NULL          ('\0')
#define S_NULL          ("\0")
#define UC_NULL         ((UCHAR)'\0')
#define C_SPACE         (' ')
#define S_SPACE         (" ")
#define C_LF            ('\n')
#define S_LF            ("\n")
#define C_BACKSLASH     ('\\')
#define S_BACKSLASH     ("\\")
#define C_YEN           ('\\')
#define S_YEN           ("\\")
#define C_DOLLAR        ('$')
#define S_DOLLAR        ("$")
#define C_DQUOTE        ('\"')
#define S_DQUOTE        ("\"")
#define C_SQUOTE        ('\'')
#define S_SQUOTE        ("\'")
#define C_TAB           ('\t')
#define S_TAB           ("\t")
 
#ifndef RECL_MAX
#define RECL_MAX    512
#endif
 
#endif

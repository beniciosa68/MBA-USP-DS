/* src/util/machines.h $Revision: 2008.3 $ */

/*! \file
 * Include file for all Molpro C files
 *
 * This include file should always be included by all Molpro C files.
 *
 * \b FORT_Extern(subroutine_name,SUBROUTINE_NAME)
 * 
 * Description: C Macro that converts the name of a C subroutine
 * into an external name that can be called by FORTRAN 77/90/95.
 * 
 * This is commonly used for subroutines that need to be called
 * from both FORTRAN and C.  The subroutine is written in C and
 * a FORTRAN wrapper function is created to call the C code.
 *
 * FORTRAN externals are machine dependent.  Subroutine objects
 * names are generally augmented with 0, 1 or 2 underscores.
 * 
 * _UNDERSCORES should be defined with the number of underscores
 * requred by a particular machines FORTRAN. If this is unknown,
 * compile a simple FORTRAN subroutine with the -c option and
 * use 'nm' to look at the object file.
*/

/* MOLPRO header file for C programs:
 * include files
 * default definition values, override with -Dname=[definition] */

/* cpp flag
 * _AIX		AIX
 * __APPLE__	Darwin
 * __CYGWIN__	Cygwin
 * __hpux	HP-UX
 * sgi		IRIX64
 * linux	Linux
 * sun		SunOS
 * SX		SUPER-UX
 * __uxp__	Fujitsu
 * _WIN32	Windows
 */

#ifndef __MACHINES_H__
#define __MACHINES_H__

#if defined(__linux) && !defined(linux) /* needed for Linux/ppc64 */
#define linux
#endif

#if defined(linux) && !defined(NOLARGEFILES)
#if defined(__x86_64__) || defined(__ia64)
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
/* #define _USE_LARGEFILE64  Probably this is needed too, but it seems to work!? */
#endif
#else
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#endif
#endif

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <sys/param.h>
#endif
#include <sys/stat.h>
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#include <sys/times.h>
#endif
#include <sys/types.h>
#ifdef _WIN32
#define ssize_t int
#define mode_t unsigned long
#include <io.h>
#define F_OK 0
#else
#include <unistd.h>
#ifndef _AIX
#include <sys/unistd.h>
#endif
#endif

#ifdef SX
#include <stdint.h>
#endif
#if defined(_WIN32) || defined(__uxp__)
typedef char int8_t;
typedef short int16_t;
#endif

#if defined(_WIN32) || defined(__uxp__)
typedef int int32_t;
#endif

#ifdef _AIX
#define REVERSE 10000
#define OPEN open
#ifdef AIX42
#undef OPEN
#define OPEN open64
#define LSEEK lseek64
#define TRUNCATE ftruncate64
#define OFFSET          off64_t
#endif
#ifndef INT64
#ifndef AIX_SHM /* this is probably dead now */
#define AIX_SHM 16777216 /* threshold for using shared memory segments */
#endif
#ifndef AIX_SHM_SEG_MAX
#define AIX_SHM_SEG_MAX 10
#endif
#endif
#include <memory.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <sys/mode.h>
#define FORTCL_END
#ifdef PAM6000
#define CLSEG clseg_
#endif
#ifndef RESERVE
#define RESERVE 10000
#endif
#endif

/* stuff above should be sorted */

#if defined(_AIX) || defined(__APPLE__) || defined( __CYGWIN__) || defined(__hpux) || defined(linux) || defined(sgi) || defined(sun) || defined(SX) || defined(__uxp__) || defined(_WIN32)
#define FORTCL_END
#endif

#if defined(_AIX) || defined(__APPLE__) || defined( __CYGWIN__) || defined(__hpux) || defined(linux) || defined(sgi) || defined(sun)  || defined(SX) || defined(__uxp__)
#define HAS_UTSNAME
#endif

#if defined(SX)
#define KEEPTEMP
#endif

#ifdef __uxp__
#define _LLTYPES
#endif

#if defined(__hpux)
#define	MVBITs MVBITS
#elif defined(__uxp__) || defined(sun)
#define MVBITs mvbits_
#endif

#if defined(NAME_LU) || defined(_AIX) || defined(__APPLE__) || defined( __CYGWIN__) || defined(__hpux) || defined(linux) || defined(sgi) || defined(sun) || defined(SX) || defined(_WIN32)
#define _UNDERSCORES 1
#endif
#if defined(NAME_L)
#define _UNDERSCORES 0
#endif

#if defined(__hpux) || defined(sun) || defined(__uxp__)
#define NOGETWD
#endif

#if defined(_WIN32) || defined (SX)
#define NOGLOB
#endif

#if defined(__uxp__)
#define NOTRAP
#endif

#if defined(__APPLE__) || defined( __CYGWIN__) || defined(linux)
#define REVERSE_BYTE
#endif

#ifdef __uxp__
#define TRACEBACK errtra_()
#endif

#ifndef FORTINT
#ifdef INT64
#if defined(_AIX) || defined( __CYGWIN__) || defined(__hpux) || defined(linux) || defined(sgi)
#define FORTINT long long
/* set macro EXT_INT64 and EXT_INT in order to keep consistent with GA */
#define EXT_INT64
#define EXT_INT
#endif
#else
#if defined(sgi) || defined(__APPLE__) || defined(SX) || defined(__uxp__) || ( defined(__x86_64__) && defined(linux) )
#define FORTINT int
#endif
#endif
#endif
#ifndef FORTINT
#define FORTINT	long
#define EXT_INT
#endif
typedef FORTINT fortint ; /* fortran integer type */

#ifndef FORTINTC
#ifdef __uxp__
#define FORTINTC int
#endif
#ifdef INT64
#if defined(_AIX) || defined( __CYGWIN__) || defined(__hpux) || defined(linux) || defined(sgi)
#define FORTINTC long
#endif
#else
#if defined(sgi)
#define FORTINTC long
#endif
#endif
#endif
#ifndef FORTINTC
#define FORTINTC FORTINT
#endif
typedef FORTINTC fortintc ; /* fortran character string length type */

#ifndef NOMALLOCH
#if defined(__APPLE__)
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#endif

#ifdef __uxp__ /* sysv/ucb nonsense */
#define index	strchr
#define rindex	strrchr
#endif

#ifndef TIMER_TICK
#if defined(__APPLE__)
#define TIMER_TICK	CLK_TCK
#elif defined(_SC_CLK_TCK)
#define TIMER_TICK      sysconf(_SC_CLK_TCK)
#elif defined(HZ)
#define TIMER_TICK      HZ
#else
#define TIMER_TICK      60
#endif
#endif

#if defined(FORTCL_END) || defined(FORTCL_NEXT)
#define FORTCL
#endif

#ifdef USE_FCDLEN
#include <fortran.h>
#endif

#define Getcwd(a,b)	getcwd(a,b);

#ifndef MEMALLOC
#if defined(__ia64) && defined(linux)
#define MEMALLOC(n)	calloc(n,sizeof(double))
#endif
#endif
#ifndef MEMALLOC
#define MEMALLOC(n)	malloc(n*sizeof(double))
#endif

#ifndef LSEEK
#if (defined(__hpux) && !defined(INT64)) || (defined(sgi) && (_MIPS_ISA > 2)) || defined(sun) || defined(_USE_LARGEFILE64)
#define LSEEK 		lseek64
#else
#define LSEEK		lseek
#endif
#endif

#ifndef OFFSET
#ifdef _USE_LARGEFILE64
#define	OFFSET		__off64_t
#elif (defined(__hpux) && !defined(INT64)) || (defined(sgi) && (_MIPS_ISA > 2)) || defined(sun)
#define OFFSET 		off64_t
#else
#define OFFSET		off_t
#endif
#endif

#ifndef OPENFLAGS
#ifdef _USE_LARGEFILE64
#define OPENFLAGS	O_LARGEFILE
#else
#define OPENFLAGS	0
#endif
#endif

#ifndef OPEN
#if (defined(__hpux) && !defined(INT64)) || defined(sun) || defined(_USE_LARGEFILE64)
#define OPEN 		open64
#else
#define OPEN 		open
#endif
#endif

#ifndef TRUNCATE
#ifdef _WIN32
#define TRUNCATE        chsize
#endif
#endif
#ifndef TRUNCATE
#if (defined(__hpux) && !defined(INT64)) || (defined(sgi) && (_MIPS_ISA > 2)) || defined(sun) || defined(_USE_LARGEFILE64)
#define TRUNCATE 	ftruncate64
#else
#define TRUNCATE	ftruncate
#endif
#endif

#ifndef SEEK
#define SEEK		0.001		/* average seektime in seconds */
#endif

#ifndef SPEED
#ifdef sgi
#define SPEED		3000000
#endif
#endif
#ifndef SPEED
#define SPEED		200000		/* speed in words per second */
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN	1024
#endif

#if defined _UNDERSCORES

#ifdef FORT_UPPERCASE

#if _UNDERSCORES == 0
#define FORT_Extern(funct,FUNCT) FUNCT
#elif _UNDERSCORES == 1
#define FORT_Extern(funct,FUNCT) FUNCT ## _
#elif _UNDERSCORES == 2
#define FORT_Extern(funct,FUNCT) FUNCT ## __
#else
#error "_UNDERSCORES not properly defined."
#endif

#else

#if _UNDERSCORES == 0
#define FORT_Extern(funct,FUNCT) funct
#elif _UNDERSCORES == 1
#define FORT_Extern(funct,FUNCT) funct ## _
#elif _UNDERSCORES == 2
#define FORT_Extern(funct,FUNCT) funct ## __
#else
#error "_UNDERSCORES not properly defined."
#endif

#endif

#else
#error "_UNDERSCORES not defined."
#endif

#endif /*  __MACHINES_H__ */

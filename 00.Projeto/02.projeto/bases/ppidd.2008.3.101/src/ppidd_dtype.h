/* src/mpp/ppidd_dtype.h $Revision: 2008.3 $ */

/*! \file
  Data type for PPIDD
  prerequisite: include machines.h
*/

/*! use fortlogical instead of logical, since GA has already defined logical */
#ifndef __PPIDD_DTYPE_H__
#define __PPIDD_DTYPE_H__

 typedef fortint fortlogical;
 typedef fortint Fortlogical;

 #ifdef CRAY_YMP
 #define FALSE _btol(0)
 #define TRUE  _btol(1)
 #else
 #define FALSE (fortlogical) 0
 #define TRUE  (fortlogical) 1
 #endif

#endif

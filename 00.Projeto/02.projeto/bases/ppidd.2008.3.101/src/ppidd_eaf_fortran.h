/* src/mpp/ppidd_eaf_fortran.h $Revision: 2008.3 $ */

/* ------------------------------------------------------------- *\
   FORTRAN Interface of PPIDD Exclusive Access File Library
   ( External PPIDD EAF routines which are used to be called by FORTRAN routines)
   =====================================================================

   All PPIDD subroutines are written in C.  PPIDD subroutines called
   from FORTRAN go through a C wrapper.  All FORTRAN wrapper 
   subroutines are prefixed with PPIDD_.
   
   The following C definitions substitute the FORTRAN wrapper
   subroutine name with the correct machine dependent FORTRAN
   external name.
   
   Note: FORTRAN externals are generally all lowercase, but may
   be uppercase.  See machines.f for details.
   
\* ------------------------------------------------------------- */

#ifndef __PPIDD_EAF_FORTRAN_H__
#define __PPIDD_EAF_FORTRAN_H__

 # include "machines.h"

 #     define PPIDD_Eaf_open                 FORT_Extern(ppidd_eaf_open,PPIDD_EAF_OPEN)
 #     define PPIDD_Eaf_write                FORT_Extern(ppidd_eaf_write,PPIDD_EAF_WRITE)
 #     define PPIDD_Eaf_awrite               FORT_Extern(ppidd_eaf_awrite,PPIDD_EAF_AWRITE)
 #     define PPIDD_Eaf_read                 FORT_Extern(ppidd_eaf_read,PPIDD_EAF_READ)
 #     define PPIDD_Eaf_aread                FORT_Extern(ppidd_eaf_aread,PPIDD_EAF_AREAD)
 #     define PPIDD_Eaf_wait                 FORT_Extern(ppidd_eaf_wait,PPIDD_EAF_WAIT)
 #     define PPIDD_Eaf_waitall              FORT_Extern(ppidd_eaf_waitall,PPIDD_EAF_WAITALL)
 #     define PPIDD_Eaf_probe                FORT_Extern(ppidd_eaf_probe,PPIDD_EAF_PROBE)
 #     define PPIDD_Eaf_close                FORT_Extern(ppidd_eaf_close,PPIDD_EAF_CLOSE)
 #     define PPIDD_Eaf_delete               FORT_Extern(ppidd_eaf_delete,PPIDD_EAF_DELETE)
 #     define PPIDD_Eaf_length               FORT_Extern(ppidd_eaf_length,PPIDD_EAF_LENGTH)
 #     define PPIDD_Eaf_truncate             FORT_Extern(ppidd_eaf_truncate,PPIDD_EAF_TRUNCATE)
 #     define PPIDD_Eaf_errmsg               FORT_Extern(ppidd_eaf_errmsg,PPIDD_EAF_ERRMSG)

#endif

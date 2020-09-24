/* src/mpp/ppidd_sf_fortran.h $Revision: 2008.3 $ */

/* ------------------------------------------------------------- *\
   FORTRAN Interface of PPIDD Shared Files Library
   ( External PPIDD SF routines which are used to be called by FORTRAN routines)
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

#ifndef __PPIDD_SF_FORTRAN_H__
#define __PPIDD_SF_FORTRAN_H__

 # include "machines.h"

 #     define PPIDD_Sf_create               FORT_Extern(ppidd_sf_create,PPIDD_SF_CREATE)
 #     define PPIDD_Sf_write                FORT_Extern(ppidd_sf_write,PPIDD_SF_WRITE)
 #     define PPIDD_Sf_read                 FORT_Extern(ppidd_sf_read,PPIDD_SF_READ)
 #     define PPIDD_Sf_wait                 FORT_Extern(ppidd_sf_wait,PPIDD_SF_WAIT)
 #     define PPIDD_Sf_waitall              FORT_Extern(ppidd_sf_waitall,PPIDD_SF_WAITALL)
 #     define PPIDD_Sf_destroy              FORT_Extern(ppidd_sf_destroy,PPIDD_SF_DESTROY)
 #     define PPIDD_Sf_errmsg               FORT_Extern(ppidd_sf_errmsg,PPIDD_SF_ERRMSG)

#endif

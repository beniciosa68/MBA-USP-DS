/* src/mpp/ppidd_sf_c.h $Revision: 2008.3 Patch(2008.3): ppidd_sf_c $ */

/* ------------------------------------------------------------- *\
   C Interface of PPIDD Shared Files Library
   ( External PPIDD SF routines which are used to be called by C routines)
   =====================================================================

   All PPIDD subroutines are written in C.  The subroutines in this file 
   named PPIDD_XXXXX can be only called by C program directly.  Any calling
   by Fortran progam should refer to the routines in the ppidd_sf_fortran.h file.
   
\* ------------------------------------------------------------- */

#ifndef __PPIDD_SF_C_H__
#define __PPIDD_SF_C_H__

   #include "machines.h"
   extern void PPIDD_Sf_create(char *fname, double *size_hard_limit, double *size_soft_limit, double *req_size, fortint *handle, fortint *ierr);
   extern void PPIDD_Sf_write(fortint *handle,double *byte_offset,double *byte_length, double *buff,fortint *request_id,fortint *ierr);
   extern void PPIDD_Sf_read(fortint *handle,double *byte_offset,double *byte_length, double *buff,fortint *request_id,fortint *ierr);
   extern void PPIDD_Sf_wait(fortint *request_id,fortint *ierr);
   extern void PPIDD_Sf_waitall(fortint *list, fortint *num,fortint *ierr);
   extern void PPIDD_Sf_destroy(fortint *handle,fortint *ierr);
   extern void PPIDD_Sf_errmsg(fortint *ierr,char *message);

#endif

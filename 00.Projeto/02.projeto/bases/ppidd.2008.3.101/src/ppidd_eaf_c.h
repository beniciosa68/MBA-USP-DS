/* src/mpp/ppidd_eaf_c.h $Revision: 2008.3 Patch(2008.3): ppidd_eaf_c $ */

/* ------------------------------------------------------------- *\
   C Interface of PPIDD Exclusive Access File Library
   ( External PPIDD EAF routines which are used to be called by C routines)
   =====================================================================

   All PPIDD subroutines are written in C.  The subroutines in this file 
   named PPIDD_XXXXX can be only called by C program directly.  Any calling
   by Fortran progam should refer to the routines in the ppidd_eaf_fortran.h file.
   
\* ------------------------------------------------------------- */

#ifndef __PPIDD_EAF_C_H__
#define __PPIDD_EAF_C_H__

   #include "machines.h"
   extern void PPIDD_Eaf_open(char *fname, fortint *type, fortint *handle, fortint *ierr);
   extern void PPIDD_Eaf_write(fortint *handle,double *byte_offset,void *buff,fortint *byte_length,fortint *ierr);
   extern void PPIDD_Eaf_awrite(fortint *handle,double *byte_offset,void *buff,fortint *byte_length,fortint *request_id,fortint *ierr);
   extern void PPIDD_Eaf_read(fortint *handle,double *byte_offset,void *buff,fortint *byte_length,fortint *ierr);
   extern void PPIDD_Eaf_aread(fortint *handle,double *byte_offset,void *buff,fortint *byte_length,fortint *request_id,fortint *ierr);
   extern void PPIDD_Eaf_wait(fortint *handle,fortint *request_id,fortint *ierr);
   extern void PPIDD_Eaf_waitall(fortint *list, fortint *num,fortint *ierr);
   extern void PPIDD_Eaf_probe(fortint *request_id,fortint *status,fortint *ierr);
   extern void PPIDD_Eaf_close(fortint *handle,fortint *ierr);
   extern void PPIDD_Eaf_delete(char *fname, fortint *ierr);
   extern void PPIDD_Eaf_length(fortint *handle,double *fsize,fortint *ierr);
   extern void PPIDD_Eaf_truncate(fortint *handle,double *offset,fortint *ierr);
   extern void PPIDD_Eaf_errmsg(fortint *code,char *message);

#endif

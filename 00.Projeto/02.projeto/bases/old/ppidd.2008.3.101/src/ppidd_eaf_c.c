/* src/mpp/ppidd_eaf_c.c $Revision: 2008.3 Patch(2008.3): ppidd_eaf_c ppidd_rank $ */

/* ====================================================================== *\
 *                    Exclusive Access File Library                       *
 *                    =============================                       *
 *  (c) Copyright 2008 by the authors of MOLPRO. All rights reserved.     *
 * ------------------------------------------------------------------------------------------ *
 * An exclusive access file is a file which is generated and/or read by a single process of a *
 * distributed parallel application. Files are not shared between different processes. The    *
 * library is an abstract high-performance file system which provides a common interface for  *
 * a variety of architecture specific parallel storage systems.  The library also makes       *
 * available features like asynchronous input and output to Fortran.  EAF's syntax is similar *
 * to the standard Unix C file operations, differences indicate new semantics or extended     *
 * features available through EAF.                                                            *
 *    The last argument of all subroutines returns an integer error code with the value zero  *
 * implying success, non-zero implying some error condition.  Offsets are doubles and an      *
 * offset with a fractional component generates an error.                                     *
 *--------------------------------------------------------------------------------------------*
 * C interface.  The subroutines in this file named PPIDD_XXXXX can be    *
 * only called by C program directly.  Any calling by Fortran progam      *
 * should refer to the routines in the ppidd_eaf_fortran.c file.          *
 *                                                                        *
 * Written by: Manhui Wang                                                *
 * Date:       15/07/2008                                                 *
\* ====================================================================== */


#include "ppidd_eaf_c.h"   /* include machines.h */

#ifdef FORTCL_NEXT
#undef FORTCL_NEXT
#endif

#ifdef FORTCL_END
#undef FORTCL_END
#endif

#ifdef FORTINTC_DIVIDE
#undef FORTINTC_DIVIDE
#endif

/* The following code should be the same as those in ppidd_eaf_fortran.c (except ppidd_eaf_rank). One should make it consistent once code in ppidd_eaf_fortran.c is changed. */

#ifdef MPI2
 #include <mpi.h>
 #define   MPI_EAF_RW -1
 #define   MPI_EAF_W  -2
 #define   MPI_EAF_R  -3
 extern MPI_Comm MPIGA_WORK_COMM;
#endif

#ifdef GA_TOOLS
 #include "ga.h"
 #include "eaf.h"
#endif

 extern int ppidd_eaf_rank(void);
 static int MPI_Debug=0;


/* ************************************************************************ *\
   FORTRAN Wrapper for PPIDD_Eaf_create
   Creates shared file using name and path specified in fname as a template. 
   req_size specifies size of a typical request (-1. means "don't know").
   It is a collective operation.
\* ************************************************************************ */
   void PPIDD_Eaf_open(char *fname
#if defined(FORTCL_NEXT)
	       ,fortintc lx
#endif
	       ,fortint *type, fortint *handle, fortint *ierr
#if defined(FORTCL_END)
	       ,fortintc lx
#endif
)  {
#ifdef MPI2
      MPI_File mpi_fh;
      MPI_Fint mpifhandle;
      MPI_Comm mpicomm=MPIGA_WORK_COMM;
      int amode;
      int mpierr;
#endif
#ifdef GA_TOOLS
      int gaerr;
      int gahandle;
#endif
      int modetype=(int)*type;
      int i;
      char *name2;
      int lxi;

      
      if(MPI_Debug)printf("%4d: In ppidd_eaf_open: begin.\n",ppidd_eaf_rank());
#if defined(FORTCL_NEXT) || defined(FORTCL_END)
      lxi=(int)lx;
#ifdef FORTINTC_DIVIDE
      lxi=(int)lx/FORTINTC_DIVIDE;
#endif
#else
      lxi=strlen(fname);
#endif
      strncpy((name2=(char *)malloc(lxi+1)),fname,lxi);
      name2[lxi]=(char)0;
      for(i=lxi-1; (i>=0 && name2[i]==' '); i--) name2[i]=(char)0;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_open: sizeof(fortint)=%d,sizeof(fortintc)=%d,lxi=%d,filename=%s\n",ppidd_eaf_rank(),(int)sizeof(fortint),(int)sizeof(fortintc),lxi,name2);
   
#ifdef MPI2
      switch(modetype){
        case MPI_EAF_RW: amode = MPI_MODE_RDWR|MPI_MODE_CREATE ;
                         break;
        case MPI_EAF_W:  amode = MPI_MODE_WRONLY|MPI_MODE_CREATE ;
                         break;
        case MPI_EAF_R:  amode = MPI_MODE_RDONLY ;
                         break;
        default:      
                         MPI_Abort(mpicomm,911);
      }
/*      MPI_MODE_RDWR|MPI_MODE_CREATE|MPI_MODE_DELETE_ON_CLOSE|MPI_MODE_UNIQUE_OPEN;*/
      
      mpierr=MPI_File_open(MPI_COMM_SELF,name2,amode,MPI_INFO_NULL,&mpi_fh);
      mpifhandle = MPI_File_c2f( mpi_fh );
      *handle=(fortint)mpifhandle;
      *ierr=(fortint)mpierr;
#endif
#ifdef GA_TOOLS
      gaerr=eaf_open(name2, modetype, &gahandle);
      *handle=(fortint)gahandle;
      *ierr=(fortint)gaerr;
#endif
      free(name2);
      if(MPI_Debug)printf("%4d: In ppidd_eaf_open: end. handle=%d,ierr=%d\n",ppidd_eaf_rank(),(int)*handle,(int)*ierr);
   }
	
/* ******************************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Eaf_write
   Asynchronous write operation. 
   Writes number of bytes to the file identified by handle at location offset.
   Operation is guaranteed to be complete when eaf_wait called with request_id argument returns. 
\* ******************************************************************************************** */
   void PPIDD_Eaf_write(fortint *handle,double *byte_offset,void *buff,fortint *byte_length,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      MPI_Offset offset;
      int count;
      int mpierr;
      MPI_Datatype datatype;
      MPI_Status status;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_write : begin. handle=%d,byte_offset=%f,byte_length=%ld\n",ppidd_eaf_rank(),(int)mpifhandle,*byte_offset,(long)*byte_length);
      mpi_fh = MPI_File_f2c(mpifhandle);
      offset=(MPI_Offset)(*byte_offset);
      count=(int)(*byte_length/8);
      datatype=MPI_DOUBLE;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_write : before MPI_File_write_at. handle=%d,offset=%ld,count=%d\n",ppidd_eaf_rank(),(int)mpifhandle,offset,count);
      mpierr=MPI_File_write_at(mpi_fh,offset,buff,count,datatype,&status);
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_write : end. handle=%d,ierr=%d\n",ppidd_eaf_rank(),(int)mpifhandle,(int)*ierr);
#endif
#ifdef GA_TOOLS
      int gahandle=(int)*handle;
      eaf_off_t offset=(eaf_off_t)*byte_offset;
      size_t bytes=(size_t)*byte_length;
      int gaerr;

      gaerr=eaf_write(gahandle,offset,buff,bytes);
      *ierr=(fortint)gaerr;
#endif
   }

/* ******************************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Eaf_awrite
   Asynchronous write operation. 
   Writes number of bytes to the file identified by handle at location offset.
   Operation is guaranteed to be complete when eaf_wait called with request_id argument returns. 
\* ******************************************************************************************** */
   void PPIDD_Eaf_awrite(fortint *handle,double *byte_offset,void *buff,fortint *byte_length,fortint *request_id,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      MPI_Offset offset;
      int count;
      int mpierr;
      MPI_Datatype datatype;
#ifdef MPIO_USES_MPI_REQUEST
      MPI_Request request;
#else
      MPIO_Request request;
#endif
      if(MPI_Debug)printf("%4d: In ppidd_eaf_awrite : begin. handle=%d,byte_offset=%f,byte_length=%ld\n",ppidd_eaf_rank(),(int)mpifhandle,*byte_offset,(long)*byte_length);
      mpi_fh = MPI_File_f2c(mpifhandle);
      offset=(MPI_Offset)(*byte_offset);
      count=(int)(*byte_length/8);
      datatype=MPI_DOUBLE;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_awrite : before MPI_File_iwrite_at. handle=%d,offset=%ld,count=%d\n",ppidd_eaf_rank(),(int)mpifhandle,offset,count);
      mpierr=MPI_File_iwrite_at(mpi_fh,offset,buff,count,datatype,&request);
      *request_id=(fortint)request;
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_awrite : end. handle=%d,ierr=%d,request_id=%d,request=%d\n",ppidd_eaf_rank(),(int)mpifhandle,(int)*ierr,(int)*request_id,(int)request);
#endif
#ifdef GA_TOOLS
      int gahandle=(int)*handle;
      eaf_off_t offset=(eaf_off_t)*byte_offset;
      size_t bytes=(size_t)*byte_length;
      int request;
      int gaerr;

      gaerr=eaf_awrite(gahandle,offset,buff,bytes,&request);
      *request_id=(fortint)request;
      *ierr=(fortint)gaerr;
#endif
   }


/* ******************************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Eaf_read 
   Asynchronous read operation. 
   Reads number of bytes to the file identified by handle at location offset.
   Operation is guaranteed to be complete when eaf_wait called with request_id argument returns.
\* ******************************************************************************************** */
   void PPIDD_Eaf_read(fortint *handle,double *byte_offset,void *buff,fortint *byte_length,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      MPI_Offset offset;
      int count;
      int mpierr;
      MPI_Datatype datatype;
      MPI_Status status;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_read  : begin. handle=%d,byte_offset=%f,byte_length=%ld\n",ppidd_eaf_rank(),(int)mpifhandle,*byte_offset,(long)*byte_length);
      mpi_fh = MPI_File_f2c(mpifhandle);
      offset=(MPI_Offset)(*byte_offset);
      count=(int)(*byte_length/8);
      datatype=MPI_DOUBLE;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_read  : before MPI_File_read_at. handle=%d,offset=%ld,count=%d\n",ppidd_eaf_rank(),(int)mpifhandle,offset,count);
      mpierr=MPI_File_read_at(mpi_fh,offset,buff,count,datatype,&status);
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_read  : end. handle=%d,ierr=%d\n",ppidd_eaf_rank(),(int)mpifhandle,(int)*ierr);
#endif
#ifdef GA_TOOLS
      int gahandle=(int)*handle;
      eaf_off_t offset=(eaf_off_t)*byte_offset;
      size_t bytes=(size_t)*byte_length;
      int gaerr;

      gaerr=eaf_read(gahandle,offset,buff,bytes);
      *ierr=(fortint)gaerr;
#endif
   }


/* ******************************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Eaf_aread 
   Asynchronous read operation. 
   Reads number of bytes to the file identified by handle at location offset.
   Operation is guaranteed to be complete when eaf_wait called with request_id argument returns.
\* ******************************************************************************************** */
   void PPIDD_Eaf_aread(fortint *handle,double *byte_offset,void *buff,fortint *byte_length,fortint *request_id,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      MPI_Offset offset;
      int count;
      int mpierr;
      MPI_Datatype datatype;
#ifdef MPIO_USES_MPI_REQUEST
      MPI_Request request;
#else
      MPIO_Request request;
#endif
      if(MPI_Debug)printf("%4d: In ppidd_eaf_aread  : begin. handle=%d,byte_offset=%f,byte_length=%ld\n",ppidd_eaf_rank(),(int)mpifhandle,*byte_offset,(long)*byte_length);
      mpi_fh = MPI_File_f2c(mpifhandle);
      offset=(MPI_Offset)(*byte_offset);
      count=(int)(*byte_length/8);
      datatype=MPI_DOUBLE;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_aread  : before MPI_File_iread_at. handle=%d,offset=%ld,count=%d\n",ppidd_eaf_rank(),(int)mpifhandle,offset,count);
      mpierr=MPI_File_iread_at(mpi_fh,offset,buff,count,datatype,&request);
      *request_id=(fortint)request;
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_aread  : end. handle=%d,ierr=%d,request_id=%d,request=%d\n",ppidd_eaf_rank(),(int)mpifhandle,(int)*ierr,(int)*request_id,(int)request);
#endif
#ifdef GA_TOOLS
      int gahandle=(int)*handle;
      eaf_off_t offset=(eaf_off_t)*byte_offset;
      size_t bytes=(size_t)*byte_length;
      int request;
      int gaerr;

      gaerr=eaf_aread(gahandle,offset,buff,bytes,&request);
      *request_id=(fortint)request;
      *ierr=(fortint)gaerr;
#endif
   }


/* ************************************************************************************ *\
   FORTRAN Wrapper for PPIDD_Eaf_wait 
   Wait for the completion of the asynchronous request associated with request_id.
   request_id is invalidated.
   integer request_id   --[in]  Handle of asynchronous request.
   integer ierr         --[out] Error code. 0 if it is able to wait for completion, 
                          else returns error code.
\* ************************************************************************************ */
   void PPIDD_Eaf_wait(fortint *handle,fortint *request_id,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      int mpierr;
#ifdef MPIO_USES_MPI_REQUEST
      MPI_Request request=(MPI_Request)(*request_id);
#else
      MPIO_Request request=(MPIO_Request)(*request_id);
#endif
      MPI_Status status;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_wait  : begin. handle=%d,request_id=%d,request=%d\n",ppidd_eaf_rank(),(int)mpifhandle,(int)*request_id,(int)request);
      
#ifdef MPIO_USES_MPI_REQUEST
      mpierr=MPI_Wait( &request, &status );
#else
      mpierr=MPIO_Wait(&request, &status);
#endif
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_wait  : end. ierr=%d\n",ppidd_eaf_rank(),(int)*ierr);
#endif
#ifdef GA_TOOLS
      int gahandle=(int)*handle;
      int request=(int)*request_id;
      int gaerr;

      gaerr=eaf_wait(gahandle,request);
      *ierr=(fortint)gaerr;
#endif
   }


/* ********************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Eaf_waitall 
   Blocks the calling process until all of the num I/O operations associated with ids
   specified in list complete. Invalidates (modifies) ids on the list.
\* ********************************************************************************** */
   void PPIDD_Eaf_waitall(fortint *list, fortint *num,fortint *ierr) {
#ifdef MPI2
      int i;
      int mpierr,mpierrsub;
      int count=(int)(*num);
      MPI_Status *array_of_statuses;
#ifdef MPIO_USES_MPI_REQUEST
      MPI_Request *array_of_requests;
      array_of_requests=(MPI_Request*)malloc(count*sizeof(MPI_Request));
#else
      MPIO_Request *array_of_requests;
      array_of_requests=(MPIO_Request*)malloc(count*sizeof(MPIO_Request));
#endif
     
      array_of_statuses=(MPI_Status*)malloc(count*sizeof(MPI_Status));
      for(i=0;i<count;i++) array_of_requests[i]=(MPI_Request)list[i];

#ifdef MPIO_USES_MPI_REQUEST
      mpierr=MPI_Waitall(count,array_of_requests,array_of_statuses);
#else
      for(i=0,mpierr=0;i<count;i++) {
        mpierrsub=MPIO_Wait(&array_of_requests[i], &array_of_statuses[i]);
        mpierr=mpierr+mpierrsub;
      }
#endif
      *ierr=(fortint)mpierr;
#endif
   }

/* ************************************************************************************ *\
   FORTRAN Wrapper for PPIDD_Eaf_probe 
   Determine if an asynchronous request has completed or is pending.
   integer request_id   --[in]  Handle of asynchronous request.
   integer status       --[out] Pending or completed status argument.
                          status returns 0 if the asynchronous operation is complete, or 1 otherwise.
			  If the asynchronous request is complete, id is invalidated.
   integer ierr         --[out] Error code. 0 if probe succeeded, else returns error code. 
\* ************************************************************************************ */
   void PPIDD_Eaf_probe(fortint *request_id,fortint *status,fortint *ierr) {
#ifdef MPI2
      int flag;
      int mpierr;
#ifdef MPIO_USES_MPI_REQUEST
      MPI_Request request=(MPI_Request)(*request_id);
#else
      MPIO_Request request=(MPIO_Request)(*request_id);
#endif
      MPI_Status mpistatus;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_probe  : begin. request_id=%d,request=%d\n",ppidd_eaf_rank(),(int)*request_id,(int)request);
      
#ifdef MPIO_USES_MPI_REQUEST
      mpierr=MPI_Test(&request, &flag, &mpistatus);
#else
      mpierr=MPIO_Test(&request, &flag, &mpistatus);
#endif
      if(flag) *status=(fortint)0;
      else *status=(fortint)1;
      
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_probe  : end. ierr=%d\n",ppidd_eaf_rank(),(int)*ierr);
#endif
#ifdef GA_TOOLS
      int garequest=(int)*request_id;
      int gastatus;
      int gaerr;

      gaerr=eaf_probe(garequest, &gastatus);
      *status=(fortint)gastatus;
      *ierr=(fortint)gaerr;
#endif
   }


/* ************************************************************************************ *\
   FORTRAN Wrapper for PPIDD_Eaf_close 
   Close a eaf file.
   integer handle  --[in]  File Handle.
   integer ierr    --[out] Error code. 0 if the file was closed, else returns error code.
\* ************************************************************************************ */
   void PPIDD_Eaf_close(fortint *handle,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      int mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_close: begin. handle=%d\n",ppidd_eaf_rank(),(int)mpifhandle);
      mpi_fh = MPI_File_f2c(mpifhandle);
      mpierr=MPI_File_close( &mpi_fh );
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_close: end. handle=%d,ierr=%d\n",ppidd_eaf_rank(),(int)mpifhandle,(int)*ierr);
#endif
#ifdef GA_TOOLS
      int gahandle=(int)*handle;
      int gaerr;

      gaerr=eaf_close(gahandle);
      *ierr=(fortint)gaerr;
#endif
   }


/* ********************************************************************************************* *\
   FORTRAN Wrapper for PPIDD_Eaf_delete
   Delete a file
   character*(*) fname   -- [in]  File name.
   integer ierr          -- [out] Error code. 0 if the file was deleted, else returns error code.
\* ********************************************************************************************* */
   void PPIDD_Eaf_delete(char *fname
#if defined(FORTCL_NEXT)
	       ,fortintc lx
#endif
	       ,fortint *ierr
#if defined(FORTCL_END)
	       ,fortintc lx
#endif
)  {
      int i,pierr=0;
      char *name2;
      int lxi;
      
      if(MPI_Debug)printf("%4d: In ppidd_eaf_delete: begin.\n",ppidd_eaf_rank());
#if defined(FORTCL_NEXT) || defined(FORTCL_END)
      lxi=(int)lx;
#ifdef FORTINTC_DIVIDE
      lxi=(int)lx/FORTINTC_DIVIDE;
#endif
#else
      lxi=strlen(fname);
#endif
      strncpy((name2=(char *)malloc(lxi+1)),fname,lxi);
      name2[lxi]=(char)0;
      for(i=lxi-1; (i>=0 && name2[i]==' '); i--) name2[i]=(char)0;
      
#ifdef MPI2
      pierr=MPI_File_delete(name2,MPI_INFO_NULL);
#endif
#ifdef GA_TOOLS
      pierr=eaf_delete(name2);
#endif

      if(MPI_Debug)printf("%4d: In ppidd_eaf_delete: mid. fname=%s,pierr=%d\n",ppidd_eaf_rank(),name2,pierr);
      free(name2);
      *ierr=(fortint)pierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_delete: end. ierr=%d\n",ppidd_eaf_rank(),(int)*ierr);
   }


/* ************************************************************************************ *\
   FORTRAN Wrapper for PPIDD_Eaf_length 
   Determine the length (in bytes) of a file.
   integer handle    --[in]  File Handle.
   double fsize      --[out] File length in bytes.
\* ************************************************************************************ */
   void PPIDD_Eaf_length(fortint *handle,double *fsize,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      MPI_Offset size;
      int mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_length: begin. handle=%d\n",ppidd_eaf_rank(),(int)mpifhandle);
      mpi_fh = MPI_File_f2c(mpifhandle);
      mpierr=MPI_File_get_size(mpi_fh, &size);
      *fsize=(double)size;
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_length: end. handle=%d,fsize=%f\n",ppidd_eaf_rank(),(int)mpifhandle,*fsize);
#endif
#ifdef GA_TOOLS
      int gahandle=(int)*handle;
      eaf_off_t length;
      int gaerr;

      gaerr=eaf_length(gahandle,&length);
      *fsize=(double)length;
      *ierr=(fortint)gaerr;
#endif
   }

/* *************************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Eaf_truncate 
   Truncate a file at specified offset (in bytes).
   integer handle --[in]  File Handle.
   double offset  --[in]  Offset in bytes.
   integer ierr   --[out] Error code. 0 if the file was truncated, else returns error code.
\* *************************************************************************************** */
   void PPIDD_Eaf_truncate(fortint *handle,double *offset,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      MPI_Offset size=(MPI_Offset)(*offset);
      int mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_truncate: begin. handle=%d\n",ppidd_eaf_rank(),(int)mpifhandle);
      mpi_fh = MPI_File_f2c(mpifhandle);
      mpierr=MPI_File_set_size(mpi_fh,size);
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_eaf_truncate: end. handle=%d,size=%ld\n",ppidd_eaf_rank(),(int)mpifhandle,size);
#endif
#ifdef GA_TOOLS
      int gahandle=(int)*handle;
      eaf_off_t length=(eaf_off_t)*offset;
      int gaerr;

      gaerr=eaf_truncate(gahandle,length);
      *ierr=(fortint)gaerr;
#endif
   }
	

/* ********************************************************************************* *\
   FORTRAN Wrapper for PPIDD_Eaf_errmsg 
   Returns a string interpretation of the error code, or an empty string 
   (Fortran all blanks, C null first character) if the error code is not recognized.
        code             -- [in]  error code returned by a previous call to EAF
        message          -- [out] character string where the corresponding message
\* ********************************************************************************* */
   void PPIDD_Eaf_errmsg(fortint *code,char *message
#if defined(FORTCL_NEXT) || defined(FORTCL_END)
	       ,fortintc lx
#endif
)  {
#ifdef MPI2
      int eclass, len;
      char estring[MPI_MAX_ERROR_STRING],estring2[MPI_MAX_ERROR_STRING];
#endif
      int perrcode=(int)*code;
      int i;
      int lxi;
      
#if defined(FORTCL_NEXT) || defined(FORTCL_END)
      lxi=(int)lx;
#ifdef FORTINTC_DIVIDE
      lxi=(int)lx/FORTINTC_DIVIDE;
#endif
#else
      lxi=strlen(message);
#endif
      
      if(MPI_Debug)printf("%4d: In ppidd_eaf_errmsg: begin. perrcode=%d\n",ppidd_eaf_rank(),perrcode);
#ifdef MPI2
      MPI_Error_class(perrcode, &eclass);
      MPI_Error_string(perrcode, estring, &len);
      sprintf(estring2," Error %d: %s", eclass, estring);
      strcpy(message,estring2);
#endif
#ifdef GA_TOOLS
      eaf_errmsg(perrcode, message);
#endif
      if(MPI_Debug)printf("%4d: In ppidd_eaf_errmsg: middle. message=%s\n",ppidd_eaf_rank(),message);
      for(i=strlen(message);i<lxi;i++) message[i]=' ';
      if(MPI_Debug)printf("%4d: In ppidd_eaf_errmsg: end. perrcode=%d\n",ppidd_eaf_rank(),perrcode);
   }

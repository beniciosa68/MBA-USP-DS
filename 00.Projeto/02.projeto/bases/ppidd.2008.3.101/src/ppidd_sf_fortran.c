/* src/mpp/ppidd_sf_fortran.c $Revision: 2008.3 Patch(2008.3): ppidd_sf_c ppidd_rank $ */

/* ====================================================================== *\
 *                    PPIDD Shared Files Library                          *
 *                    ==========================                          *
 *  (c) Copyright 2008 by the authors of MOLPRO. All rights reserved.     *
 * ------------------------------------------------------------------------------------------ *
 * The Shared Files (SF) library implements logically-shared temporary files for parallel     *
 * SPMD (single-program-multiple-data) applications. The main features are listed as follows: *
 * -- Shared files are non-persistent (temporary)                                             *
 * -- Shared files resemble one-dimensional arrays in main memory                             *
 * -- Each process can independently read/write to any location in the file                   *
 * -- All routines return error code: "0" means success.                                      *
 * -- sf_create and sf_destroy are collective                                                 *
 * -- file, request sizes, and offset (all in bytes) are DOUBLE PRECISION arguments,          *
 *    all the other arguments are INTEGERS                                                    *
 * -- read/writes are asynchronous                                                            *
 *--------------------------------------------------------------------------------------------*     
 * FORTRAN interface.  The subroutines in this file named PPIDD_XXXXX are *
 * converted to the proper FORTRAN external by the FORT_Extern macro and  *
 * the definitions in the ppidd_sf_fortran.h header file.                 *
 *                                                                        *
 * Written by: Manhui Wang                                                *
 * Date:       15/07/2008                                                 *
\* ====================================================================== */


#include "ppidd_sf_fortran.h"   /* include machines.h */

#ifdef MPI2
 #include <mpi.h>
 extern MPI_Comm MPIGA_WORK_COMM;
#endif

#ifdef GA_TOOLS
 #include "ga.h"
 #include "sf.h"
 extern Integer sf_create(char *fname, SFsize_t *size_hard_limit, SFsize_t *size_soft_limit, SFsize_t *req_size, Integer *handle);
 extern Integer FATR sf_write_(Integer *s_a, SFsize_t *offset, SFsize_t *bytes, char *buffer, Integer *req_id);
 extern Integer FATR sf_read_(Integer *s_a, SFsize_t *offset, SFsize_t *bytes, char *buffer, Integer *req_id);
 extern Integer FATR sf_wait_(Integer *req_id);
 extern Integer FATR sf_waitall_(Integer *list, Integer *num);
 extern Integer FATR sf_destroy_(Integer *s_a);
 extern void sf_errmsg(int code, char *msg);
#endif

 static int MPI_Debug=0;


/* ************************************************************************ *\
   Get calling process id in the work communicator. This function is only 
   used in operations related to sf. 
\* ************************************************************************ */
   int ppidd_sf_rank(void) {
      int myid=0;
#ifdef MPI2
      MPI_Comm mpicomm=MPIGA_WORK_COMM;

      MPI_Comm_rank(mpicomm,&myid);
#endif
#ifdef GA_TOOLS
      myid=GA_Nodeid();
#endif
      return(myid);
   }

/* ************************************************************************ *\
   FORTRAN Wrapper for PPIDD_Sf_create
   Creates shared file using name and path specified in fname as a template. 
   req_size specifies size of a typical request (-1. means "don't know").
   It is a collective operation.
\* ************************************************************************ */
   void PPIDD_Sf_create(char *fname
#if defined(FORTCL_NEXT)
	       ,fortintc lx
#endif
	       ,double *size_hard_limit, double *size_soft_limit, double *req_size, fortint *handle, fortint *ierr
#if defined(FORTCL_END)
	       ,fortintc lx
#endif
)  {
#ifdef MPI2
      MPI_Comm mpicomm=MPIGA_WORK_COMM;
      MPI_File mpi_fh;
      MPI_Fint mpifhandle;
      int mpierr;
#endif
      int i;
      char *name2;
      int lxi;
      
      if(MPI_Debug)printf("%4d: In ppidd_sf_create: begin.\n",ppidd_sf_rank());
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
      if(MPI_Debug)printf("%4d: In ppidd_sf_create: midlle.sizeof(fortint)=%d,sizeof(fortintc)=%d,lxi=%d,filename=%s\n",ppidd_sf_rank(),(int)sizeof(fortint),(int)sizeof(fortintc),lxi,name2);

#ifdef MPI2
      mpierr=MPI_File_open(mpicomm,name2,MPI_MODE_RDWR|MPI_MODE_CREATE|MPI_MODE_DELETE_ON_CLOSE|MPI_MODE_UNIQUE_OPEN,MPI_INFO_NULL,&mpi_fh);
      mpifhandle = MPI_File_c2f( mpi_fh );
      *handle=(fortint)mpifhandle;
      *ierr=(fortint)mpierr;
#endif
#ifdef GA_TOOLS
      if(MPI_Debug) {
         printf("%4d: PPIDD_Sf_create: sizeof(double) =%d, sizeof(SFsize_t)=%d\n",ppidd_sf_rank(),(int)sizeof(double),(int)sizeof(SFsize_t));
         printf("%4d: PPIDD_Sf_create: sizeof(fortint)=%d, sizeof(Integer) =%d\n",ppidd_sf_rank(),(int)sizeof(fortint),(int)sizeof(Integer));
      }
      if ( sizeof(double) != sizeof(SFsize_t) ) {
         printf("%4d: PPIDD_Sf_create: sizeof(double) =%d, sizeof(SFsize_t)=%d\n",ppidd_sf_rank(),(int)sizeof(double),(int)sizeof(SFsize_t));
         GA_Error(" PPIDD_Sf_create: Data types do not match between [double] and [SFsize_t]",0); 
      }
      if ( sizeof(fortint) != sizeof(Integer) ) {
         printf("%4d: PPIDD_Sf_create: sizeof(fortint)=%d, sizeof(Integer) =%d\n",ppidd_sf_rank(),(int)sizeof(fortint),(int)sizeof(Integer));
         GA_Error(" PPIDD_Sf_create: Data types do not match between [fortint] and [Integer]",0); 
      }

      *ierr=(fortint)sf_create(name2, size_hard_limit, size_soft_limit, req_size, handle);
#endif

      free(name2);
      if(MPI_Debug)printf("%4d: In ppidd_sf_create: end. handle=%d,ierr=%d\n",ppidd_sf_rank(),(int)*handle,(int)*ierr);
   }
	

/* ******************************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Sf_write
   Asynchronous write operation. 
   Writes number of bytes to the file identified by handle at location offset.
   Operation is guaranteed to be complete when sf_wait called with request_id argument returns. 
\* ******************************************************************************************** */
   void PPIDD_Sf_write(fortint *handle,double *byte_offset,double *byte_length, double *buff,fortint *request_id,fortint *ierr) {
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
      if(MPI_Debug)printf("%4d: In ppidd_sf_write : begin. handle=%d,byte_offset=%f,byte_length=%f\n",ppidd_sf_rank(),(int)mpifhandle,*byte_offset,*byte_length);
      mpi_fh = MPI_File_f2c(mpifhandle);
      offset=(MPI_Offset)(*byte_offset);
      count=(int)((*byte_length)/sizeof(double));
      datatype=MPI_DOUBLE;
      if(MPI_Debug)printf("%4d: In ppidd_sf_write : before MPI_File_iwrite_at. handle=%d,offset=%ld,count=%d\n",ppidd_sf_rank(),(int)mpifhandle,offset,count);
      mpierr=MPI_File_iwrite_at(mpi_fh,offset,buff,count,MPI_DOUBLE,&request);
      *request_id=(fortint)request;
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_sf_write : end. handle=%d,ierr=%d,request_id=%d,request=%d\n",ppidd_sf_rank(),(int)mpifhandle,(int)*ierr,(int)*request_id,(int)request);
#endif
#ifdef GA_TOOLS
      char *buffer;

      buffer=(char *)buff;
      *ierr=(fortint)sf_write_(handle, byte_offset, byte_length, buffer, request_id);
#endif
   }

/* ******************************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Sf_read 
   Asynchronous read operation. 
   Reads number of bytes to the file identified by handle at location offset.
   Operation is guaranteed to be complete when sf_wait called with request_id argument returns.
\* ******************************************************************************************** */
   void PPIDD_Sf_read(fortint *handle,double *byte_offset,double *byte_length, double *buff,fortint *request_id,fortint *ierr) {
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
      if(MPI_Debug) printf("%4d: In ppidd_sf_read  : begin. handle=%d\n",ppidd_sf_rank(),(int)mpifhandle);
      mpi_fh = MPI_File_f2c(mpifhandle);
      offset=(MPI_Offset)(*byte_offset);
      count=(int)((*byte_length)/sizeof(double));
      datatype=MPI_DOUBLE;
      if(MPI_Debug)printf("%4d: In ppidd_sf_read  : before MPI_File_iread_at. handle=%d,offset=%ld,count=%d\n",ppidd_sf_rank(),(int)mpifhandle,offset,count);
      mpierr=MPI_File_iread_at(mpi_fh,offset,buff,count,MPI_DOUBLE,&request);
      *request_id=(fortint)request;
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_sf_read  : end. handle=%d,ierr=%d,request_id=%d,request=%d\n",ppidd_sf_rank(),(int)mpifhandle,(int)*ierr,(int)*request_id,(int)request);
#endif
#ifdef GA_TOOLS
      char *buffer;

      buffer=(char *)buff;
      *ierr=(fortint)sf_read_(handle, byte_offset, byte_length, buffer, request_id);
#endif
   }


/* ************************************************************************************ *\
   FORTRAN Wrapper for PPIDD_Sf_wait 
   Blocks the calling process until I/O operation associated with request_id completes. 
   Invalidates request_id. 
\* ************************************************************************************ */
   void PPIDD_Sf_wait(fortint *request_id,fortint *ierr) {
#ifdef MPI2
      int mpierr;
#ifdef MPIO_USES_MPI_REQUEST
      MPI_Request request=(MPI_Request)(*request_id);
#else
      MPIO_Request request=(MPIO_Request)(*request_id);
#endif
      MPI_Status status;
      if(MPI_Debug)printf("%4d: In ppidd_sf_wait  : begin. request_id=%d,request=%d\n",ppidd_sf_rank(),(int)*request_id,(int)request);
#ifdef MPIO_USES_MPI_REQUEST
      mpierr=MPI_Wait( &request, &status );
#else
      mpierr=MPIO_Wait(&request, &status);
#endif
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_sf_wait  : end. ierr=%d\n",ppidd_sf_rank(),(int)*ierr);
#endif
#ifdef GA_TOOLS
      *ierr=(fortint)sf_wait_(request_id);
#endif
   }


/* ********************************************************************************** *\
   FORTRAN Wrapper for PPIDD_Sf_waitall 
   Blocks the calling process until all of the num I/O operations associated with ids
   specified in list complete. Invalidates (modifies) ids on the list.
\* ********************************************************************************** */
   void PPIDD_Sf_waitall(fortint *list, fortint *num,fortint *ierr) {
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
#ifdef GA_TOOLS
      *ierr=(fortint)sf_waitall_(list, num);
#endif
   }


/* ************************************************ *\
   FORTRAN Wrapper for PPIDD_Sf_destroy 
   Destroys the shared file associated with handle.
   It is a collective operation.
\* ************************************************ */
   void PPIDD_Sf_destroy(fortint *handle,fortint *ierr) {
#ifdef MPI2
      MPI_Fint mpifhandle=(MPI_Fint)*handle;
      MPI_File mpi_fh;
      int mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_sf_destroy: begin. handle=%d\n",ppidd_sf_rank(),(int)mpifhandle);
      mpi_fh = MPI_File_f2c(mpifhandle);
      mpierr=MPI_File_close( &mpi_fh );
      *ierr=(fortint)mpierr;
      if(MPI_Debug)printf("%4d: In ppidd_sf_destroy: end. handle=%d,ierr=%d\n",ppidd_sf_rank(),(int)mpifhandle,(int)*ierr);
#endif
#ifdef GA_TOOLS
      *ierr=(fortint)sf_destroy_(handle);
#endif
   }


/* ********************************************************************************* *\
   FORTRAN Wrapper for PPIDD_Sf_errmsg 
   Returns a string interpretation of the error code, or an empty string 
   (Fortran all blanks, C null first character) if the error code is not recognized.
        ierr             -- error code returned by a previous call to SF [in]
        message          -- character string where the corresponding message
                            is written [out]
\* ********************************************************************************* */
   void PPIDD_Sf_errmsg(fortint *ierr,char *message
#if defined(FORTCL_NEXT)
	       ,fortintc lx
#endif
#if defined(FORTCL_END)
	       ,fortintc lx
#endif
)  {
#ifdef MPI2
      int eclass, len;
      char estring[MPI_MAX_ERROR_STRING],estring2[MPI_MAX_ERROR_STRING];
#endif
      int perrcode=(int)*ierr;
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
      
      if(MPI_Debug)printf("%4d: In ppidd_sf_errmsg: begin. perrcode=%d\n",ppidd_sf_rank(),perrcode);
#ifdef MPI2
      MPI_Error_class(perrcode, &eclass);
      MPI_Error_string(perrcode, estring, &len);
      sprintf(estring2," Error %d: %s", eclass, estring);
      strcpy(message,estring2);
#endif
#ifdef GA_TOOLS
      sf_errmsg(perrcode, message);
#endif
      if(MPI_Debug)printf("%4d: In ppidd_sf_errmsg: middle. message=%s\n",ppidd_sf_rank(),message);
      for(i=strlen(message);i<lxi;i++) message[i]=' ';
      if(MPI_Debug)printf("%4d: In ppidd_sf_errmsg: end. perrcode=%d\n",ppidd_sf_rank(),perrcode);
   }

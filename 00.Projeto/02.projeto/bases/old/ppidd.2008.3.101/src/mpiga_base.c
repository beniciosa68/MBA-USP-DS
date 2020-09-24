/* src/mpp/mpiga_base.c $Revision: 2008.3 Patch(2008.3): ppidd_sgi $ */

/* ====================================================================== *\
 *                    MPI Version of Global Arrays                        *
 *                    ============================                        *
 *  (c) Copyright 2008 by the authors of MOLPRO. All rights reserved.     *
 * ---------------------------------------------------------------------- *
 * C sorce code of MPI Version of Global Arrays. The subroutines can be   *
 * called directly by c code, while the corresponding fortran wrappers    *
 * (which can be called by fortran code) are in file mpiga_fortran.c.     *
 *                                                                        *
 * Written by: Manhui Wang                                                *
 * Date:       02/04/2008                                                 *
\* ====================================================================== */

#ifdef MPI2

#include "mpi_utils.h"
#include "mpiga_base.h"
#include "mpi_nxtval.h"

mpiglobal_array_t *mpiga_main_data_structure=NULL, *MPIGAIndex;
mpimutex_t_index  *mpiga_mutex_data_struc=NULL, *mpiga_mutexindex;
int *mpigv(map)=NULL,*mpigv(proclist)=NULL;
static int MPIGAinitialized = 0;
static int MPIGA_memory_limited = 1;
static int MPIGA_Debug = 0;

int SR_parallel;
int NUM_TOTAL_NNODES;
MPI_Comm MPIGA_WORK_COMM;


long  mpigv(curmem),mpigv(maxmem),mpigv(totmem);  /* current and maximum memory of MPIGA */
int  mpigv(nga),mpigv(nmutex); 
int mpigv(nprocs), mpigv(myproc); /* Number of processes and rank of the calling process in the group of MPI comm */
MPI_Comm  mpigv(Compute_comm);


int mpiga_initialize(int *argcmain, char ***argvmain)
{ 
    int flag,size,rank,i;
    int mpierr,mpierr1,mpierr2,mpierr3;
    int size_all,rank_all;
    if (MPIGA_Debug) printf("In mpiga_initilize begin.\n");
    if(MPIGAinitialized) return 0; 
    flag = 0;
    mpierr=MPI_Initialized(&flag);
    if (!flag)    {
       mpierr=MPI_Init(argcmain,argvmain);
       if (mpierr!=MPI_SUCCESS) {
	  fprintf(stderr,"ERROR in mpiga_initialize. MPI_Init failed. mpierr=%d\n",mpierr);
	  return 1;
       }
    }
    else {
	fprintf(stderr,"WARNING in mpiga_initialize! MPI_Init has been called before MPI_Init.\n");
    }

/* Split off one process to hold the counter; remaining processes are used to construct work/compute communicator.*/

    mpierr1=MPI_Comm_size(MPI_COMM_WORLD, &size_all);
    mpierr2=MPI_Comm_rank(MPI_COMM_WORLD, &rank_all);
    SR_parallel = size_all > 1 ? 1 : 0;
    NUM_TOTAL_NNODES = NNodes_Total(MPI_COMM_WORLD);
 
    make_worker_comm( MPI_COMM_WORLD, &MPIGA_WORK_COMM);
    MPI_Barrier(MPI_COMM_WORLD);

/* Install global helpmutex data structure on all processes */
    install_helpmutexes();
/*   Install nxtval server on the last process, including initialize_helpga and initialize_nxtval_helpmutex on all processes  */
    install_nxtval();

/* Initialization in work communicator*/
    if ( MPIGA_WORK_COMM != MPI_COMM_NULL) {

    /* zero in pointers in MPIGA array */
    mpiga_main_data_structure
       = (mpiglobal_array_t *)malloc(sizeof(mpiglobal_array_t)*MAX_MPI_ARRAYS);
    if(!mpiga_main_data_structure){
      fprintf(stderr,"ERROR in mpiga_initialize: failed to malloc mpiga \n");
      return 1;
    }
    MPIGAIndex = mpiga_main_data_structure;
    for(i=0;i<MAX_MPI_ARRAYS; i++) {
/*       MPIGAIndex[i].ptr  = (MPIGA)0;*/
       MPIGAIndex[i].ptr  = NULL;
       MPIGAIndex[i].actv = 0;
    }

/*    mpierr1=MPI_Comm_dup(MPIGA_WORK_COMM,&mpigv(Compute_comm));*/
    mpigv(Compute_comm)=(MPI_Comm)MPIGA_WORK_COMM;
    mpierr2=MPI_Comm_size(mpigv(Compute_comm), &size); 
    mpierr3=MPI_Comm_rank(mpigv(Compute_comm), &rank);
    if (mpierr1!=MPI_SUCCESS || mpierr2!=MPI_SUCCESS || mpierr3!=MPI_SUCCESS) {
       fprintf(stderr,"ERROR in mpiga_initialize. MPI_Comm_dup/size/rank failed. %d %d %d\n",mpierr1,mpierr2,mpierr3);
       return 1;
    }
    mpigv(nprocs) = size;
    mpigv(myproc) = rank;

    mpigv(map)=(int *)malloc(2*size*sizeof(int));        /* initialize list of lower and upper indices */
    mpigv(proclist)=(int *)malloc(size*sizeof(int));     /* initialize list of processors              */

    mpigv(curmem) = 0;
    mpigv(totmem) = 0;
    mpigv(nga)=0;         /* initialize MPIGA number */  
    mpigv(nmutex)=0;      /* initialize MPIGA mutex number */  
    
    MPIGAinitialized = 1;  /* initialization finished. */

   }
    
    if (MPIGA_Debug) printf("%4d: In mpiga_initilize end. size_all=%d\n",rank_all,size_all);

    return 0;
}

int mpiga_terminate(void)
{ 
    int i,flag;
    int size,rank;
    int handle;
    
    if (MPIGA_Debug) printf("%4d: In mpiga_terminate begin: mpigv(nga)= %d\n",mpigv(myproc),mpigv(nga));
    if (mpigv(nga)>0) {
       for(i=0;i<MAX_MPI_ARRAYS; i++) {
         if ( MPIGAIndex[i].actv == 1) { handle=i+MPI_GA_OFFSET; mpiga_free(handle); }
       }
    }
    if(mpiga_main_data_structure) {
       if (MPIGA_Debug) printf("%4d: In mpiga_terminate:  free mpiga_main_data_structure.\n",mpigv(myproc));
       free(mpiga_main_data_structure);
    }

    if(mpigv(map)) free(mpigv(map));      /* free the memory for list of lower and upper indices */
    if(mpigv(proclist)) free(mpigv(proclist)); /* free the memory for list of list of processors */

    if (MPIGA_Debug) printf("%4d: In mpiga_terminate:  free the memory for mpigv(map) and mpigv(proclist).\n",mpigv(myproc));

     MPI_Barrier(mpigv(Compute_comm));

     if ( mpigv(Compute_comm) != MPI_COMM_NULL) { 
#ifdef NXTVAL_SERVER
       int zero=0,junk;
       if( SR_parallel )  junk=NXTVAL(&zero);
       if (MPIGA_Debug) printf("%4d: In mpiga_terminate:  after NXTVAL(&zero).\n",mpigv(myproc));
       MPI_Barrier(MPI_COMM_WORLD);
#endif
    if ( mpigv(Compute_comm) != MPI_COMM_NULL) MPI_Comm_free( &mpigv(Compute_comm) );
/*    if ( MPIGA_WORK_COMM   != MPI_COMM_NULL) MPI_Comm_free( &MPIGA_WORK_COMM );*/
    
/* clean up nxtval resources on computation processes, include finalize_helpga and finalize_nxtval_helpmutex */
    finalize_nxtval();
    finalize_helpmutexes(); /* clean up helpmutex resources on computation processes */

    if (MPIGA_Debug) printf("%4d: In mpiga_terminate:  before MPI_Finalized.\n",mpigv(myproc));
    MPI_Finalized(&flag);
    if (!flag) MPI_Finalize();
    else printf("MPI_Finalize has been called before MPI_Finalize.\n");
   }

    MPIGAinitialized = 0;
    
    if (MPIGA_Debug) printf("%4d: In mpiga_terminate end: mpigv(nga)= %d\n",mpigv(myproc),mpigv(nga));

    return 0;

}


int mpiga_create_irreg(char *name, int *lenin, int nchunk, MPI_Datatype dtype, int *handle) 
{ 
    MPIGA       new_ga; 
    int      size, sizeoftype;
    long     sizetot; 
    MPI_Aint local_size; 
    int      rank,homerank;
    int      *len;
    int      lentot,i,mpierr;
    int      handle_orig;
    char     *mpiganame;
    mpimutex_t mutex;
    
    if (MPIGA_Debug) printf("%4d: In mpiga_create_irreg begin: nchunk= %d\n",mpigv(myproc),nchunk);
    
    /* Get a new structure */ 
    new_ga = (MPIGA)malloc( sizeof(struct STRUC_MPIGA) );   
    if (!new_ga) return 1; 
 
    /* Determine size of MPIGA memory */ 
    MPI_Comm_size( mpigv(Compute_comm), &size ); 
    MPI_Comm_rank( mpigv(Compute_comm), &rank );
    
    len= (int *)malloc( size *sizeof( int) );  
    if (nchunk<0 || nchunk>size) { 
       printf("ERROR in mpiga_create_irreg : nchunk(%4d)larger than process number(%4d).\n", nchunk,size);
       return 1;
    } 
    lentot=0;
    for(i=0;i<nchunk;i++) {len[i]=lenin[i];lentot=lentot+lenin[i];}
    for(i=nchunk;i<size;i++) len[i]=0;
    
    MPI_Type_size( dtype, &sizeoftype ); 

    local_size = len[rank] * sizeoftype; 
 
    /* Allocate memory my ga_win and create window */ 
    if ( local_size==(MPI_Aint)0 ) new_ga->win_ptr=NULL;
    else  MPI_Alloc_mem( local_size, MPI_INFO_NULL, &new_ga->win_ptr ); 
     
    MPI_Win_create( new_ga->win_ptr, local_size, sizeoftype,  
                    MPI_INFO_NULL, mpigv(Compute_comm), &new_ga->ga_win ); 
 
/*    printf("In mpiga_create_irreg 1: size= %d \n", size); */
    MPI_Barrier(mpigv(Compute_comm));
    strcpy(mpiganame=(char *)malloc(strlen(name)+1),name); 
    /* Save other data and return */ 
    new_ga->mutex_p    = NULL; 
    new_ga->name       = mpiganame; 
    new_ga->dtype      = dtype; 
    new_ga->dtype_size = sizeoftype; 
    new_ga->lentot     = lentot; 
    new_ga->nchunk     = nchunk; 
    new_ga->len        = len; 

/*    printf("In mpiga_create_irreg middle 2: nchunk= %d \n", nchunk); */
    
    sizetot=(long)sizeoftype*(long)lentot;
    mpigv(nga)++;
   /* ----------------------------------------------------------------- *\
      Check to ensure the maximum number of arrays hasn't been reached.
   \* ----------------------------------------------------------------- */
      if( mpigv(nga) > MAX_MPI_ARRAYS ) {
        if(rank == 0) {
           fprintf(stderr," MPI Error:  The maximum number of global arrays [%i] has been reached.\n",MAX_MPI_ARRAYS);
           fprintf(stderr," Information:  The maximum number of global arrays is a MPI compile-time option.\n");
        }
        return 1;
      }
    for(i=0;i<MAX_MPI_ARRAYS;i++){
       if ( MPIGAIndex[i].actv == 0 ) {
          handle_orig=i;  /* original sequence number of mpiga */
	  break;
       }
    }
    
#ifdef NXTVAL_SERVER
/*  use mutex stored on helper process */
    mpierr=NXTVAL_alloc_helpmutex_orig(handle_orig);
#else
/*  use mutex stored on compute processes */
/*    mpierr=mpi_alloc_helpmutex_orig(handle_orig);*/
    /* Create critical section window */ 
    homerank=0;
    mpierr = MPIMUTEX_Create(homerank, mpigv(Compute_comm), &mutex);
    if (mpierr != MPI_SUCCESS) MPI_Abort(mpigv(Compute_comm), MPI_ERR_UNKNOWN);
    new_ga->mutex_p    = mutex; 
#endif

    MPIGAIndex[handle_orig].ptr=new_ga;
    MPIGAIndex[handle_orig].actv=1;
    MPIGAIndex[handle_orig].size=(long)local_size;
    *handle=handle_orig+MPI_GA_OFFSET;  /* adjusted sequence number of mpiga */
    
    mpigv(curmem) += sizetot;
         
    if (MPIGA_Debug) printf("%4d: In mpiga_create_irreg end: handle=%d, ga_win=%12d\n",mpigv(myproc),*handle,new_ga->ga_win);
 
    return 0; 
   
} 


int mpiga_create( char *name, int lentot, MPI_Datatype datatype, int *handle ) 
{ 
    int      size,sizeoflen,nchunk; 
    int      *len;
    int      minlen,nbiglen,i;
    
 
    if (MPIGA_Debug) printf("%4d: In mpiga_create begin.\n",mpigv(myproc)); 
    /* Determine size of MPIGA memory */ 

    MPI_Comm_size( mpigv(Compute_comm), &size ); 
    
    minlen = lentot / size;
    nbiglen= lentot % size;
    
    if (minlen==0) sizeoflen=lentot;
    else sizeoflen=size;
    
    len= (int *)malloc( sizeoflen *sizeof( int) );   
    
    for(i=0; i<sizeoflen; i++) {
       if(i<nbiglen) len[i]=minlen+1;
       else len[i]=minlen;
    }

    mpiga_create_irreg(name, len, sizeoflen, datatype, handle); 
 
    free(len);

    if (MPIGA_Debug) printf("%4d: In mpiga_create end. handle=%d, nprocs=%d\n",mpigv(myproc),*handle,size); 
    
    return 0; 
} 


int mpiga_free( int handle ) 
{ 
    int flag,mpierr;
    long sizetot; 
    int handle_orig;
    MPIGA ga; 

    if (MPIGA_Debug) printf("%4d: In mpiga_free: begin. handle=%d\n",mpigv(myproc),handle); 

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;
    if (ga) { 
      if (ga->ga_win != MPI_WIN_NULL) mpierr=MPI_Win_free( &ga->ga_win ); 
#ifdef NXTVAL_SERVER
/*  free mutex stored on helper process */
      mpierr=NXTVAL_free_helpmutex_orig(handle_orig);
#else
/*  free mutex stored on compute processes */
/*      mpierr=mpi_free_helpmutex_orig(handle_orig);*/
      mpierr = MPIMUTEX_Free(&ga->mutex_p);
#endif
      if (mpierr != 0) MPI_Abort(MPI_COMM_WORLD, MPI_ERR_UNKNOWN);
      if (ga->win_ptr) mpierr=MPI_Free_mem( ga->win_ptr ); 
      if (mpierr != MPI_SUCCESS) printf("In mpiga_free: after MPI_Free_mem, mpierr=%d\n",mpierr); 
      sizetot=(long)(ga->lentot)* (long)(ga->dtype_size);
      if (ga->name) free(ga->name);
      if (ga->len) free(ga->len);
      free(ga);
      MPIGAIndex[handle_orig].ptr=NULL;
      MPIGAIndex[handle_orig].actv = 0;     
    }
   mpigv(nga)--;
   if(MPIGA_memory_limited) mpigv(totmem) += MPIGAIndex[handle_orig].size;
   mpigv(curmem) -= sizetot;
   if (MPIGA_Debug) printf("%4d: In mpiga_free: end. handle=%d\n",mpigv(myproc),handle);
   return 0;
} 


/* RETURNS AMOUNT OF MEMORY on each processor IN ACTIVE MPI GLOBAL ARRAYS */
long  mpiga_localmem(void)
{
    int i;
    long sum=0;
    for(i=0; i<MAX_MPI_ARRAYS; i++) 
        if(MPIGAIndex[i].actv) sum += (long)MPIGAIndex[i].size; 
    return(sum);
}


/* get the original handle of mpiga, and check whether the handle is out of range or active */
int mpiga_handle_orig( int handle ) 
{ 
    int handle_orig;

    if (MPIGA_Debug) printf("%4d: In mpiga_handle_orig: begin. handle=%d\n",mpigv(myproc),handle);

    /* check whether mpiga handle is out of range, and check whether it is active */
    handle_orig=handle-MPI_GA_OFFSET;
    if(handle_orig < 0 || handle_orig >= MAX_MPI_ARRAYS){
       fprintf(stderr,"%4d: mpiga_handle_orig ERROR:  invalid handle [%d]. Should be [ %d -- %d ].\n",
                       mpigv(myproc),handle,MPI_GA_OFFSET,(MPI_GA_OFFSET+MAX_MPI_ARRAYS));
       exit (1);
    }
    if(MPIGAIndex[handle_orig].actv==0){
       fprintf(stderr,"%4d: mpiga_handle_orig ERROR:  mpiga with handle=%d is not active.\n",mpigv(myproc),handle);
       exit (1);
    }
    if (MPIGA_Debug) printf("%4d: In mpiga_handle_orig: end. handle=%d\n",mpigv(myproc),handle);
    return (handle_orig);
} 


/* get the name of mpiga */
int mpiga_inquire_name( int handle, char **name ) 
{ 
    int handle_orig;
    MPIGA ga; 

    if (MPIGA_Debug) printf("%4d: In mpiga_inquire_name: begin. handle=%d\n",mpigv(myproc),handle); 

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;

    *name=ga->name;

   if (MPIGA_Debug) printf("%4d: In mpiga_inquire_name: end. name=%s.\n",mpigv(myproc),*name);
   return 0;
} 


/* get the storetype of mpiga */
/*     storetype: number of helper processes for storing this global array
 *  - \c storetype=0 : Normal distributed array stored across the distributed processes
 *  - \c storetype>=1: Low-latency array stored on one or more helper processes (effective only when helper process is enabled).
 */
int mpiga_inquire_storetype( int handle ) 
{ 
    int storetype;

    if (MPIGA_Debug) printf("%4d: In mpiga_inquire_storetype: begin. handle=%d\n",mpigv(myproc),handle); 

    if (handle >= MPI_GA_OFFSET  && handle < (MPI_GA_OFFSET+MAX_MPI_ARRAYS) ) storetype=0;
    else storetype=1; 

   if (MPIGA_Debug) printf("%4d: In mpiga_inquire_storetype: end. handle=%d\n",mpigv(myproc),handle);
   return (storetype);
} 


int mpiga_distribution( int handle, int iproc, int *ilo, int *ihigh)
{ 
    int i; 
    int size;
    int lenleft;
    int handle_orig;
    MPIGA ga; 

    if (MPIGA_Debug) printf("%4d: In mpiga_distribution begin: handle=%d\n",mpigv(myproc),handle); 

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;
    
    size=mpigv(nprocs);

    if ( iproc < 0 || iproc >=size ) {
      fprintf(stderr,"ERROR in mpiga_distribution: iproc= %d over range!!\n",iproc);
      MPI_Abort(MPI_COMM_WORLD,MPI_ERR_OTHER);
    }
    
    for (lenleft=0,i=0;i<iproc;i++) lenleft=lenleft + ga->len[i];

    if (ga->len[iproc] !=0 ) {
       *ilo=lenleft+1;
       *ihigh=lenleft+ga->len[iproc];
    }
    else {
       *ilo=0;
       *ihigh=-1;
    }
	    
    if (MPIGA_Debug) printf("%4d: In mpiga_distribution end: handle=%d, ga_win=%12d\n",mpigv(myproc),handle,ga->ga_win); 

    return 0;
}


      
int mpiga_location( int handle, int ilo, int ihigh, int *map, int *proclist, int *np)
{ 
    int ifirst, ilast, i, rank; 
    int size;
    int lenleft,iproclow,iprochigh,iilow,iihig,offset;
    int handle_orig;
    MPIGA ga; 

    if (MPIGA_Debug) printf("%4d: In mpiga_location: begin. handle=%d\n",mpigv(myproc),handle); 

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;
    
    size=mpigv(nprocs);

    if ( ilo < 1 || ilo >ihigh ||ihigh > ga->lentot) {
      fprintf(stderr,"ERROR in mpiga_location: over range! ilo=%d,ihigh=%d\n",ilo,ihigh);
      MPI_Abort(MPI_COMM_WORLD,MPI_ERR_OTHER);
    }
    for (lenleft=0,i=0;i<size;i++){
       if (ilo   >=lenleft+1 && ilo   <= lenleft+ga->len[i]) iproclow=i;
       if (ihigh >=lenleft+1 && ihigh <= lenleft+ga->len[i]) { iprochigh=i; break; }
       lenleft=lenleft + ga->len[i];
    }

    *np=iprochigh-iproclow+1;
    
    for (lenleft=0,i=0;i<iproclow;i++) lenleft=lenleft + ga->len[i];

    for (rank=iproclow;rank<=iprochigh;rank++) {
       i=rank-iproclow;
       proclist[i]=rank;
       iilow=lenleft+1;
       iihig=lenleft + ga->len[rank];
       offset=2*i;
       map[offset]  = iilow < ilo   ? ilo:   iilow;
       map[offset+1]= iihig > ihigh ? ihigh: iihig;
       lenleft=lenleft + ga->len[rank];
/*    printf("In mpiga_location: i=%d,proclist[i]=%d,map=%d %d\n",i,proclist[i],map[offset],map[offset+1]);*/ 
    }
    if (MPIGA_Debug) printf("%4d: In mpiga_location end. handle=%d, ga_win=%12d\n",mpigv(myproc),handle,ga->ga_win); 

     return 0;
} 

/* global version of MPI_Put
  For MPI_put in most MPI implementations, if the target and origin ranks are the same, 
  then copy the data from the origin buffer to the target buffer.
*/
int mpiga_put( int handle, int ilo, int ihigh, void *buf ) 
{ 
    int ifirst, ilast, i, rank; 
    MPI_Aint disp; 
    int np,lenleft,ilen;
    mpimutex_t mutex;
    int mpierr;
    int handle_orig;
    MPIGA ga; 

    if (MPIGA_Debug) printf("%4d: In mpiga_put: begin. handle=%d,ilo=%d,ihi=%d\n",mpigv(myproc),handle,ilo,ihigh); 

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;

    mpiga_location(handle, ilo, ihigh, mpigv(map), mpigv(proclist), &np);
/*    for (i=0;i<np;i++)  printf("In mpiga_put: i=%d,proclist[i]=%d,map=%d %d\n",i,proclist[i],map[2*i],map[2*i+1]); */

    for (lenleft=0,i=0;i<mpigv(proclist)[0];i++) lenleft=lenleft + ga->len[i];

/* put the data to distributed location of remote processes */
    for(i=0;i<np;i++) { 
       rank   = mpigv(proclist)[i];
       ifirst = mpigv(map)[2*i]; 
       ilast  = mpigv(map)[2*i+1]; 
       disp   = ifirst-lenleft - 1; 
       ilen   = ilast - ifirst + 1;
       
       /* For multiple nodes, use mutex to avoid overwriting the current window; without mutex, it will fail to run. */ 
       /* Using MPI_LOCK_SHARED allows get accesses to proceed */
       /* Using MPI_LOCK_EXCLUSIVE to exclude other accesses to the current window to proceed */ 
       if ( NUM_TOTAL_NNODES > 1 ) {
#ifdef NXTVAL_SERVER
          mpierr = NXTVAL_lock_helpmutex_orig(handle_orig);
#else
          mutex=ga->mutex_p; 
          mpierr = MPIMUTEX_Lock(mutex);
/*    mpierr = mpi_lock_helpmutex_orig(handle_orig);*/
#endif
          MPI_Win_lock( MPI_LOCK_SHARED, rank, MPI_MODE_NOCHECK, ga->ga_win );
       }
       else {
          MPI_Win_lock( MPI_LOCK_EXCLUSIVE, rank, 0, ga->ga_win );

       }
       MPI_Put( buf, ilen, ga->dtype, rank, disp, ilen, ga->dtype, ga->ga_win ); 
       MPI_Win_unlock( rank, ga->ga_win ); 
       if ( NUM_TOTAL_NNODES > 1 ) {
#ifdef NXTVAL_SERVER
          mpierr = NXTVAL_unlock_helpmutex_orig(handle_orig);
#else
          mpierr = MPIMUTEX_Unlock(mutex);
/*    mpierr = mpi_unlock_helpmutex_orig(handle_orig);*/
#endif
       }

       lenleft=lenleft + ga->len[rank];
       buf = (void *)( ((char *)buf) + ilen * ga->dtype_size ); 
    } 
    if (MPIGA_Debug) printf("%4d: In mpiga_put end. handle=%d, ga_win=%12d\n",mpigv(myproc),handle,ga->ga_win);
    return 0;
} 

/* global version of MPI_Get. 
  For MPI_get in most MPI implementations, if the target and origin ranks are the same, 
  then copy the data from  the target buffer to the origin buffer.
 */
int mpiga_get( int handle, int ilo, int ihigh, void *buf ) 
{ 
    int ifirst,ilast,i,rank; 
    MPI_Aint disp; 
    int np,lenleft,ilen;
    int mpierr;
    int handle_orig;
    MPIGA ga; 

    if (MPIGA_Debug) printf("%4d: In mpiga_get: begin. handle=%d, ilo=%d,ihigh=%d\n",mpigv(myproc),handle,ilo,ihigh);

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;

    mpiga_location(handle, ilo, ihigh, mpigv(map), mpigv(proclist), &np);

    for (lenleft=0,i=0;i<mpigv(proclist)[0];i++) lenleft=lenleft + ga->len[i];

/* get the data from distributed location of remote processes */
    for(i=0;i<np;i++) { 
       rank=mpigv(proclist)[i];
       ifirst = mpigv(map)[2*i]; 
       ilast  = mpigv(map)[2*i+1]; 
       disp = ifirst-lenleft-1;
       ilen = ilast-ifirst+1;  

       MPI_Win_lock( MPI_LOCK_SHARED, rank, 0, ga->ga_win ); 
       MPI_Get( buf, ilen, ga->dtype, rank, disp, ilen, ga->dtype, ga->ga_win ); 
       MPI_Win_unlock( rank, ga->ga_win ); 

       lenleft=lenleft + ga->len[rank];
       buf = (void *)( ((char *)buf) + ilen * ga->dtype_size ); 
    } 
    if (MPIGA_Debug) printf("%4d: In mpiga_get end. handle=%d, ga_win=%12d\n",mpigv(myproc),handle,ga->ga_win); 
    return 0;
} 

/* global version of MPI_Accumulate
  For MPI_Accumulate in most MPI implementions, if the target and origin ranks are the same, 
  then copy the data from  the origin memory region to the target memory region, and then accumulate.
*/
int mpiga_acc(int handle, int ilo, int ihigh, void *buf, void *fac) 
{ 
    int ifirst, ilast, i, rank, rank_first, rank_last; 
    MPI_Aint disp; 
    int size;
    int np,lenleft;
    mpimutex_t mutex;
    int mpierr;
    int handle_orig;
    int len,ilen;
    int isint,islong,isllong,isfloat,isdbl,isone;
    void *alphabuf;
    int *ialphabuf,*itempbuf,*ifac;
    long *lalphabuf,*ltempbuf,*lfac;
    long long *llalphabuf,*lltempbuf,*llfac;
    float   *falphabuf,*ftempbuf,*ffac;
    double  *dalphabuf,*dtempbuf,*dfac;
    MPIGA ga; 

    if (MPIGA_Debug) printf("%4d: In mpiga_acc: begin. handle=%d\n",mpigv(myproc),handle); 

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;
    
    size=mpigv(nprocs);
    
    len=ihigh-ilo+1;
    isint=0;
    islong=0;
    isllong=0;
    isfloat=0;
    isdbl=0;
    isone=1;
    if (ga->dtype==MPI_INT) {
       isint=1;
       ifac=(int *)fac;
       if ((*ifac)==(int)1) alphabuf=buf;
       else {
	  isone=0;     
	  itempbuf=(int *)buf;
          ialphabuf=(int *)malloc(len*sizeof(int));
          for(i=0;i<len;i++)ialphabuf[i]=(*ifac)*itempbuf[i];
          alphabuf=(void *)ialphabuf;
       }
    } 
    else if (ga->dtype==MPI_LONG) {
       islong=1;
       lfac=(long *)fac;
       if ((*lfac)==(long)1) alphabuf=buf;
       else {
	  isone=0;     
	  ltempbuf=(long *)buf;
          lalphabuf=(long *)malloc(len*sizeof(long));
          for(i=0;i<len;i++)lalphabuf[i]=(*lfac)*ltempbuf[i];
          alphabuf=(void *)lalphabuf;
       }
    } 
    else if (ga->dtype==MPI_LONG_LONG) {
       isllong=1;
       llfac=(long long *)fac;
       if ((*llfac)==(long long)1) alphabuf=buf;
       else {
	  isone=0;     
	  lltempbuf=(long long *)buf;
          llalphabuf=(long long *)malloc(len*sizeof(long long));
          for(i=0;i<len;i++)llalphabuf[i]=(*llfac)*lltempbuf[i];
          alphabuf=(void *)llalphabuf;
       }
    } 
    else if (ga->dtype==MPI_FLOAT) {
       isfloat=1;
       ffac=(float *)fac;
       if (fabs((*ffac)-1.0e0)<1.0e-6) alphabuf=buf;
       else {
	  isone=0;     
	  ftempbuf=(float *)buf;
          falphabuf=(float *)malloc(len*sizeof(float));
          for(i=0;i<len;i++) falphabuf[i]=(*ffac)*ftempbuf[i];
          alphabuf=(void *)falphabuf;
       }
    }
    else if (ga->dtype==MPI_DOUBLE) {
       isdbl=1;
       dfac=(double *)fac;
       if (fabs((*dfac)-1.0e0)<1.0e-6) alphabuf=buf;
       else {
	  isone=0;     
	  dtempbuf=(double *)buf;
          dalphabuf=(double *)malloc(len*sizeof(double));
          for(i=0;i<len;i++) dalphabuf[i]=(*dfac)*dtempbuf[i];
          alphabuf=(void *)dalphabuf;
       }
    }
    else {
       MPIGA_Error("mpiga_acc: wrong MPI_Datatype ",(int)(ga->dtype));
    }

    mpiga_location(handle, ilo, ihigh, mpigv(map), mpigv(proclist), &np);

    rank_first = mpigv(proclist)[0]; 
    rank_last  = mpigv(proclist)[np-1]; 
    for (lenleft=0,i=0;i<rank_first;i++) lenleft=lenleft + ga->len[i];
    /* In order to ensure that the entire update is atomic, we must 
       first mutex-lock all of the windows that we will access */ 
/*    for (rank = rank_first; rank <= rank_last; rank++) { 
	MPE_Mutex_lock( rank, ga->lock_win ); 
    } 
*/    

#ifdef NXTVAL_SERVER
    mpierr = NXTVAL_lock_helpmutex_orig(handle_orig);
#else
    mutex=ga->mutex_p; 
    mpierr = MPIMUTEX_Lock(mutex);
/*    mpierr = mpi_lock_helpmutex_orig(handle_orig);*/
#endif
    
    for (rank = rank_first; rank <= rank_last; rank++) { 
       i=rank-rank_first;
       ifirst = mpigv(map)[2*i]; 
       ilast  = mpigv(map)[2*i+1]; 
       disp = ifirst-lenleft-1;
       ilen = ilast-ifirst + 1;
 
/*       MPI_Win_lock( MPI_LOCK_SHARED, rank, MPI_MODE_NOCHECK, ga->ga_win ); fail on SGI Altix(IA64) machine */
       MPI_Win_lock( MPI_LOCK_EXCLUSIVE, rank, MPI_MODE_NOCHECK, ga->ga_win ); 
	    
       MPI_Accumulate( alphabuf, ilen, ga->dtype, rank, disp, ilen, ga->dtype, MPI_SUM, ga->ga_win ); 
 
       MPI_Win_unlock( rank, ga->ga_win );
       
       lenleft=lenleft + ga->len[rank];
       alphabuf = (void *)( ((char *)alphabuf) + ilen *  ga->dtype_size );
    } 

#ifdef NXTVAL_SERVER
    mpierr = NXTVAL_unlock_helpmutex_orig(handle_orig);
#else
    mpierr = MPIMUTEX_Unlock(mutex);
/*    mpierr = mpi_unlock_helpmutex_orig(handle_orig);*/
#endif
    
    if(!isone) {
       if(isint)free(ialphabuf);
       else if(islong)free(lalphabuf);
       else if(isllong)free(llalphabuf);
       else if(isfloat)free(falphabuf);
       else free(dalphabuf);
    }
    if (MPIGA_Debug) printf("%4d: In mpiga_acc end. handle=%d, ga_win=%12d\n",mpigv(myproc),handle,ga->ga_win); 
    return 0;
}


int mpiga_read_inc( int handle, int inum, int inc ) 
{ 
    void  *buf, *incval; 
    int returnval; 
    int ibuf, iincval=(int)inc; 
    long lbuf, lincval=(long)inc; 
    long long llbuf, llincval=(long long)inc;
    int isint=0,islong=0,isllong=0; 
    int rank; 
    MPI_Aint disp; 
    int size;
    int np,lenleft;
    int ierr;
    int i;
    mpimutex_t mutex;
    int mpierr;
    int handle_orig;
    MPIGA ga; 

    if (MPIGA_Debug) printf("%4d: In mpiga_read_inc: begin. handle=%d\n",mpigv(myproc),handle); 

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;
    
    size=mpigv(nprocs);

    if (ga->dtype==MPI_INT) {
       incval=&iincval;
       buf=&ibuf;
       isint=1;
       if (MPIGA_Debug) printf("%4d: In mpiga_read_inc: it is an int MPIGA.\n",mpigv(myproc));
    }
    else if (ga->dtype==MPI_LONG) {
       incval=&lincval;
       buf=&lbuf;
       islong=1;
       if (MPIGA_Debug) printf("%4d: In mpiga_read_inc: it is a long int MPIGA.\n",mpigv(myproc));
    }
    else if (ga->dtype==MPI_LONG_LONG) {
       incval=&llincval;
       buf=&llbuf;
       isllong=1;
       if (MPIGA_Debug) printf("%4d: In mpiga_read_inc: it is a long long int MPIGA.\n",mpigv(myproc));
    }
    else {
       fprintf(stderr,"%4d: mpiga_read_inc ERROR:  wrong data type (should be an integer MPIGA). handle=%d\n",mpigv(myproc),handle);
       MPIGA_Error("mpiga_read_inc: wrong MPI_Datatype ",(int)(ga->dtype));
    }
 	
    mpiga_location(handle, inum, inum, mpigv(map), mpigv(proclist), &np);
/*    for (i=0;i<np;i++)  printf("In mpiga_read_inc: i=%d,proclist[i]=%d,map=%d %d\n",i,proclist[i],map[2*i],map[2*i+1]); 
*/
    rank = mpigv(proclist)[0]; 
    for (lenleft=0,i=0;i<rank;i++) lenleft=lenleft + ga->len[i];
   
    /* disp depends on the displacement unit being sizeof(int) */ 
    disp = inum-lenleft - 1; 

    if (MPIGA_Debug) printf("%4d: In mpiga_read_inc before mutex. rank=%d,ga_win=%d,disp=%d\n",mpigv(myproc),rank,ga->ga_win,(int)disp); 

#ifdef NXTVAL_SERVER
    mpierr = NXTVAL_lock_helpmutex_orig(handle_orig);
#else
    mutex=ga->mutex_p; 
    mpierr = MPIMUTEX_Lock(mutex);
/*    mpierr = mpi_lock_helpmutex_orig(handle_orig);*/
#endif

    MPI_Win_lock( MPI_LOCK_SHARED, rank, MPI_MODE_NOCHECK, ga->ga_win );
/*    MPI_Win_lock( MPI_LOCK_SHARED, rank, 0, ga->ga_win ); */
    MPI_Get(buf, 1, ga->dtype,  rank, disp, 1, ga->dtype, ga->ga_win ); 
    MPI_Win_unlock( rank, ga->ga_win ); 

    if (MPIGA_Debug) printf("In mpiga_read_inc. After MPI_Get: buf=%d\n",(int)(*(int*)buf));
 
    MPI_Win_lock( MPI_LOCK_SHARED, rank, MPI_MODE_NOCHECK, ga->ga_win );
/*    MPI_Win_lock( MPI_LOCK_SHARED, rank, 0, ga->ga_win ); */
    MPI_Accumulate(incval, 1, ga->dtype, rank, disp, 1, ga->dtype, MPI_SUM, ga->ga_win ); 
    MPI_Win_unlock( rank, ga->ga_win ); 
    
#ifdef NXTVAL_SERVER
    mpierr = NXTVAL_unlock_helpmutex_orig(handle_orig);
#else
    mpierr = MPIMUTEX_Unlock(mutex);
/*    mpierr = mpi_unlock_helpmutex_orig(handle_orig);*/
#endif
    
    if (MPIGA_Debug) printf("%4d: In mpiga_read_inc after mutex. rank=%d,ga_win=%d,disp=%d\n",mpigv(myproc),rank,ga->ga_win,(int)disp); 

    if (MPIGA_Debug) printf("%4d: In mpiga_read_inc End. handle=%d\n",handle,mpigv(myproc));

    if(isint) returnval=(int)ibuf;
    else if(islong) returnval=(int)lbuf;
    else returnval=(int)llbuf;

    return returnval; 
} 


int mpiga_zero_patch( int handle, int ilo, int ihigh) 
{ 
    int ifirst, ilast, i, j, rank; 
    MPI_Aint disp; 
    int irank;
    int np,lenleft;
    int mpierr;
    int handle_orig;
    MPIGA ga; 
    int len,ilen;
    int  *ibuf;
    long  *lbuf;
    long long *llbuf;
    float   *fbuf;
    double  *dbuf;

    if (MPIGA_Debug)  printf("%4d: In mpiga_zero_patch: begin. handle=%d\n",mpigv(myproc),handle);

    handle_orig=mpiga_handle_orig(handle);

    ga=MPIGAIndex[handle_orig].ptr;
    
    irank=mpigv(myproc);
  
    len=ihigh-ilo+1;
    
    mpiga_location(handle, ilo, ihigh, mpigv(map), mpigv(proclist), &np);
    for (lenleft=0,i=0;i<mpigv(proclist)[0];i++) lenleft=lenleft + ga->len[i];
    for(i=0;i<np;i++) { 
       rank=mpigv(proclist)[i];
       if (rank==irank){
         ifirst = mpigv(map)[2*i]; 
         ilast  = mpigv(map)[2*i+1]; 
         disp = ifirst-lenleft - 1;
	 ilen = ilast -ifirst  + 1;

         if (ga->dtype==MPI_INT) {
            ibuf=(int *)ga->win_ptr;
            for (j=disp;j<disp+ilen;j++) ibuf[j]=(int)0;
         } 
         else if (ga->dtype==MPI_LONG) {
            lbuf=(long *)ga->win_ptr;
            for (j=disp;j<disp+ilen;j++) lbuf[j]=(long)0;
         } 
         else if (ga->dtype==MPI_LONG_LONG) {
            llbuf=(long long *)ga->win_ptr;
            for (j=disp;j<disp+ilen;j++) llbuf[j]=(long long)0;
         } 
         else if (ga->dtype==MPI_FLOAT) {
            fbuf=(float *)ga->win_ptr;
            for (j=disp;j<disp+ilen;j++) fbuf[j]=0.0;
         }
         else if (ga->dtype==MPI_DOUBLE) {
            dbuf=(double *)ga->win_ptr;
            for (j=disp;j<disp+ilen;j++) dbuf[j]=0.0e0;
         }
         else {
            MPIGA_Error("mpiga_zero_patch: wrong MPI_Datatype ",(int)(ga->dtype));
         }
       }
       lenleft=lenleft + ga->len[rank];
    }
    
    MPI_Barrier(mpigv(Compute_comm));
    if (MPIGA_Debug) printf("%4d: In mpiga_zero_patch end. handle=%d, ga_win=%12d\n",mpigv(myproc),handle,ga->ga_win);
    return 0;
} 


int mpiga_zero( int handle) 
{ 
    int handle_orig;
    int len;

    if (MPIGA_Debug) printf("%4d: In mpiga_zero: begin. handle=%d\n",mpigv(myproc),handle); 

    handle_orig=mpiga_handle_orig(handle);

    len=(MPIGAIndex[handle_orig].ptr)->lentot;
    
    mpiga_zero_patch(handle,1,len); 
    
    if (MPIGA_Debug) printf("%4d: In mpiga_zero end. ga_win=%12d\n",mpigv(myproc),(MPIGAIndex[handle_orig].ptr)->ga_win);
    return 0;
} 


/* sum over the buffer according to type and operator */
int MPI_GSum(MPI_Datatype mpidtype, void *buffer,int len, char *op)
{
     MPI_Comm     mpicomm;
     int sizeoftype;
     int mpiop;
     char *op2;
     int size,i;
     int *ialphabuf,*itempbuf;
     long *lalphabuf,*ltempbuf;
     long long *llalphabuf,*lltempbuf;
     float   *falphabuf,*ftempbuf;
     double  *dalphabuf,*dtempbuf;

     op2=(char *)malloc(strlen(op)+1);
     strcpy(op2,op);
     strlower(op2);
     if(strstr(op2,"+")==op2) mpiop=MPI_SUM;
     else if(strstr(op2,"*")==op2) mpiop=MPI_PROD;
     else if(strstr(op2,"max")==op2) mpiop=MPI_MAX;
     else if(strstr(op2,"min")==op2) mpiop=MPI_MIN;
     free(op2);
      
     MPI_Type_size( mpidtype, &sizeoftype ); 
     size = len*sizeoftype;
     mpicomm=mpigv(Compute_comm);
     if (mpidtype==MPI_INT) {
        ialphabuf=(int *)malloc(size);
        MPI_Allreduce(buffer,ialphabuf,len,mpidtype,mpiop,mpicomm);
	itempbuf=(int *)buffer;
	for(i=0;i<len;i++) itempbuf[i]=ialphabuf[i];
	free(ialphabuf);
     } 
     else if ( mpidtype==MPI_LONG) {
        lalphabuf=(long *)malloc(size);
        MPI_Allreduce(buffer,lalphabuf,len,mpidtype,mpiop,mpicomm);
	ltempbuf=(long *)buffer;
	for(i=0;i<len;i++) ltempbuf[i]=lalphabuf[i];
	free(lalphabuf);
     } 
     else if ( mpidtype==MPI_LONG_LONG) {
        llalphabuf=(long long *)malloc(size);
        MPI_Allreduce(buffer,llalphabuf,len,mpidtype,mpiop,mpicomm);
	lltempbuf=(long long *)buffer;
	for(i=0;i<len;i++) lltempbuf[i]=llalphabuf[i];
	free(llalphabuf);
     } 
     else if ( mpidtype==MPI_FLOAT) {
        falphabuf=(float *)malloc(size);
        MPI_Allreduce(buffer,falphabuf,len,mpidtype,mpiop,mpicomm);
	ftempbuf=(float *)buffer;
	for(i=0;i<len;i++) ftempbuf[i]=falphabuf[i];
	free(falphabuf);
     }
     else if ( mpidtype==MPI_DOUBLE) {
        dalphabuf=(double *)malloc(size);
        MPI_Allreduce(buffer,dalphabuf,len,mpidtype,mpiop,mpicomm);
	dtempbuf=(double *)buffer;
	for(i=0;i<len;i++) dtempbuf[i]=dalphabuf[i];
	free(dalphabuf);
     }
     else
        MPIGA_Error("MPI_GSum: wrong MPI_Datatype ",(int)mpidtype);

     return 0; 
}


/* convert characters in a string  into lower case */
char *strlower(char *s)
{
      char *p;
      for (p=s; *p != '\0'; ++p) *p=tolower(*p);
      return s;
}

/* creates a set containing the number of mutexes. Only one set of mutexes can exist at a time. Mutexes can be
created and destroyed as many times as needed. Mutexes are numbered: 0, ..., number-1. */
int mpiga_create_mutexes(int number)
{
    mpimutex_t mutex;
    int  i,mpierr;
    int  rank,homerank;

    if (MPIGA_Debug) printf("%4d: In mpiga_create_mutexes begin:  %d mutexes will be created.\n",mpigv(myproc),number);

    if (!mpiga_mutex_data_struc) {
      mpiga_mutex_data_struc = (mpimutex_t_index *)malloc(sizeof(mpimutex_t_index)*number);
    }
    else {
       fprintf(stderr,"ERROR in mpiga_create_mutex: one set of mutexes has existed.\n");
       return 1;
    }
    if(!mpiga_mutex_data_struc){
       fprintf(stderr,"ERROR in mpiga_create_mutex: malloc mpimutex failed\n");
       return 1;
    }
    mpiga_mutexindex = mpiga_mutex_data_struc;
    mpigv(nmutex)=number;

    for (i=0;i<number;i++) {
       homerank=0;    
       mpierr = MPIMUTEX_Create(homerank, mpigv(Compute_comm), &mutex);
       if (mpierr != MPI_SUCCESS) MPI_Abort(mpigv(Compute_comm), MPI_ERR_UNKNOWN);
       mpiga_mutexindex[i].actv   = 1;
       mpiga_mutexindex[i].lock   = 0;
       mpiga_mutexindex[i].ptr    = mutex;
    }
    
    if (MPIGA_Debug) printf("%4d: In mpiga_create_mutexes end:  %d mutexes has been created.\n",mpigv(myproc),number);

    return 0;
}


/* lock a mutex object identified by the mutex number. It is a fatal error for a process
   to attempt to lock a mutex which has already been locked by this process */
int mpiga_lock_mutex(int inum)
{
    int  mpierr;
    
    if (MPIGA_Debug) printf("%4d: In mpiga_lock_mutex begin: mutex num=%d, total num=%d\n",mpigv(myproc),inum,mpigv(nmutex));

    if (inum <0 || inum >= mpigv(nmutex) ) {
       fprintf(stderr,"ERROR in mpiga_lock_mutex: over range! mutex num=%d, total num=%d\n",inum,mpigv(nmutex));
       return 1;
    }
    else if (mpiga_mutexindex[inum].lock == 1 ) {
       fprintf(stderr,"ERROR in mpiga_lock_mutex: attempt to lock a mutex which has already been locked\n");
       return 1;
    }
    else {
       mpierr = MPIMUTEX_Lock(mpiga_mutexindex[inum].ptr);
       mpiga_mutexindex[inum].lock   = 1;
    }
    
    if (MPIGA_Debug) printf("%4d: In mpiga_lock_mutex: mutex %d has been locked.\n",mpigv(myproc),inum);

    return 0;
}


/* unlock a mutex object identified by the mutex number. It is a fatal error for a process
 * to attempt to unlock a mutex which has not been locked by this process. */
int mpiga_unlock_mutex(int inum)
{
    int  mpierr;
    
    if (MPIGA_Debug) printf("%4d: In mpiga_unlock_mutex begin: mutex=%d\n",mpigv(myproc),inum);

    if (inum <0 || inum >= mpigv(nmutex) ) {
       fprintf(stderr,"ERROR in mpiga_unlock_mutex: over range! mutex=%d\n",inum);
       return 1;
    }
    else if (mpiga_mutexindex[inum].lock == 0 ) {
       fprintf(stderr,"ERROR in mpiga_unlock_mutex: attempt to unlock a mutex which has not been locked\n");
       return 1;
    }
    else {
       mpierr = MPIMUTEX_Unlock(mpiga_mutexindex[inum].ptr);
       mpiga_mutexindex[inum].lock  = 0;
    }
    
    if (MPIGA_Debug) printf("%4d: In mpiga_unlock_mutex: mutex %d has been unlocked.\n",mpigv(myproc),inum);

    return 0;
}

/* destroys the set of mutexes created with ga_create_mutexes.*/
int mpiga_destroy_mutexes(void)
{
    int  i,mpierr;
    
    if (MPIGA_Debug) printf("%4d: In mpiga_destroy_mutexes begin: mpigv(nmutex)=%d\n",mpigv(myproc),mpigv(nmutex));

    if (mpigv(nmutex)<=0) {
       fprintf(stderr,"ERROR in mpiga_destroy_mutexes: no mutex is needed to be destroyed. N mutex=%d\n",mpigv(nmutex));
       return 1;
    }
    for(i=0;i<mpigv(nmutex); i++) {
       if ( mpiga_mutexindex[i].lock ==1 ){
          fprintf(stderr,"WARNING in mpiga_destroy_mutexes: mutex is still locked before destroyed. Now unlocking it...\n");
          mpierr = MPIMUTEX_Unlock(mpiga_mutexindex[i].ptr);
          mpiga_mutexindex[i].lock  = 0;
       }
    }
    for(i=0;i<mpigv(nmutex); i++) {
        mpierr = MPIMUTEX_Free(&mpiga_mutexindex[i].ptr);
        if (MPIGA_Debug) printf("%4d: In mpiga_destroy_mutexes: free mutex=%d,mpierr=%d\n",mpigv(myproc),i,mpierr);
    }

    if(mpiga_mutex_data_struc) {
       if (MPIGA_Debug) printf("%4d: In mpiga_destroy_mutexes: free mpiga_mutex_data_struc.\n",mpigv(myproc));
       free(mpiga_mutex_data_struc);
       mpiga_mutex_data_struc=NULL;
    }

    mpigv(nmutex)= 0;
    
    if (MPIGA_Debug) printf("%4d: In mpiga_destroy_mutexes: mpigv(nmutex)=%d\n",mpigv(myproc),mpigv(nmutex));

    return 0;
}

#else

void mpiga_base_dummy () {
}

#endif /* end of MPI2 */

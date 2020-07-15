/* src/mpp/mpi_helpmutex.c $Revision: 2008.3 $ */

/* ====================================================================== *\
 *                 MPI Version of Mutual Exclusion(Mutex)                 *
 *                 ======================================                 *
 *  (c) Copyright 2008 by the authors of MOLPRO. All rights reserved.     *
 * ---------------------------------------------------------------------- *
 * C sorce code of MPI Version of Mutual exclusion. The subroutines can be*
 * called directly by c code, while the corresponding fortran wrappers    *
 * (which can be called by fortran code) are in file mpiga_fortran.c.     *
 *                                                                        *
 * Written by: Manhui Wang                                                *
 * Date:       30/01/2009                                                 *
\* ====================================================================== */

#ifdef MPI2

#include "mpi_utils.h"
#include "mpiga_base.h"
#include "mpi_nxtval.h"
 
mpimutex_t_index *mpi_helpmutex_data_struc=NULL, *mpi_helpmutex_index;
int mpigv(nhelpmutex); 

static int MPI_Debug = 0;

/* There are two kind of mutexes: one is used in each global array; another is used in other general code.
MAX_MPI_HELPMUTEX_ARRAYS=MAX_MPI_HELPMUTEX_GA (used in GA) + MAX_MPI_HELPMUTEX_NONGA (used in other code) */

/* initialization for helpmutexes -- called in mpiga_initialize */
void install_helpmutexes()
{
    int i,mpierr;
    int homerank;
    mpimutex_t mutex;
    int  server = ServerID();         /* id of server process */

#  ifdef NXTVAL_SERVER
/* zero in pointers in MPI_HELPMUTEX array, initialise mpi_helpmutex data structure */
    if (MPI_Debug) printf("%4d: In install_helpmutexes: sizeof(mpimutex_t_index)=%d, sizeof(mpimutex)=%d, MAX_MPI_HELPMUTEX_ARRAYS=%d\n",
        ProcID(),(int)sizeof(mpimutex_t_index),(int)sizeof(struct mpimutex),MAX_MPI_HELPMUTEX_ARRAYS);
    mpi_helpmutex_data_struc=(mpimutex_t_index *)malloc(sizeof(mpimutex_t_index)*MAX_MPI_HELPMUTEX_ARRAYS);
    if(!mpi_helpmutex_data_struc){
      fprintf(stderr,"ERROR in install_helpmutexes: failed to malloc mpi_helpmutex data structure.\n");
      exit (1);
    }
    mpi_helpmutex_index = mpi_helpmutex_data_struc;
    for(i=0;i<MAX_MPI_HELPMUTEX_ARRAYS; i++) {
       homerank=server;    
       mpierr = MPIMUTEX_Create(homerank, MPI_COMM_WORLD, &mutex);
       if (MPI_Debug) printf("%4d: In install_helpmutexes: after calling MPIMUTEX_Create. i=%d\n",ProcID(),i);
       if (mpierr != MPI_SUCCESS) MPI_Abort(MPI_COMM_WORLD, MPI_ERR_UNKNOWN);
       mpi_helpmutex_index[i].actv = 0;
       mpi_helpmutex_index[i].lock = 0;
       mpi_helpmutex_index[i].ptr  = mutex;
    }
    mpigv(nhelpmutex)=0;
    if (MPI_Debug) printf("%4d: In install_helpmutexes: %d mutexes have been installed.\n",ProcID(),MAX_MPI_HELPMUTEX_ARRAYS);
#  endif
}

void finalize_helpmutexes()
{
    int i,mpierr;
#  ifdef NXTVAL_SERVER
    /* free the memory for global mpi_helpmutex data structure */
    if (MPI_Debug) printf("%4d: In finalize_helpmutexes: begin.\n",mpigv(myproc));

    for(i=0;i<MAX_MPI_HELPMUTEX_ARRAYS; i++) {
       if ( mpi_helpmutex_index[i].ptr != NULL ) mpierr = MPIMUTEX_Free(&mpi_helpmutex_index[i].ptr);
    }
    if(mpi_helpmutex_data_struc) {
       if (MPI_Debug) printf("%4d: In finalize_helpmutexes: free mpi_helpmutex_data_struc.\n",mpigv(myproc));
       free(mpi_helpmutex_data_struc);
       mpi_helpmutex_data_struc=NULL;
    }
    if (MPI_Debug) printf("%4d: In finalize_helpmutexes: %d mutexes have been destroyed.\n",mpigv(myproc),MAX_MPI_HELPMUTEX_ARRAYS);
#  endif
}

/* allocate one mutex object identified by the original mutex number. Check whether it has overranged, activated, or locked */
int mpi_alloc_helpmutex_orig(int inum)
{
    int i,mpierr;

    if (MPI_Debug) printf("%4d: In mpi_alloc_helpmutex_orig begin: original mutex num=%d\n",mpigv(myproc),inum);

    if (inum <0 || inum >= MAX_MPI_HELPMUTEX_ARRAYS ) {
       fprintf(stderr,"ERROR in mpi_alloc_helpmutex_orig: over range! original mutex num=%d, max num=%d\n",inum,MAX_MPI_HELPMUTEX_ARRAYS);
       exit (1);
    }
    else if ( mpi_helpmutex_index[inum].actv==1 ) {
       fprintf(stderr,"ERROR in mpi_alloc_helpmutex_orig: original mutex %d has been used by others.\n",inum);
       exit (1);
    }
    else if ( mpi_helpmutex_index[inum].lock==1 ) {
       fprintf(stderr,"ERROR in mpi_alloc_helpmutex_orig: original mutex %d has been locked before allocating.\n",inum);
       exit (1);
    }
    else {
       mpi_helpmutex_index[inum].actv=1;
    }
    
    if (MPI_Debug) printf("%4d: In mpi_alloc_helpmutex_orig end: original mutex %d has been allocated.\n",mpigv(myproc),inum);

    return 0;
}

/* free one mutex object identified by the original mutex number. Check whether it has overranged, activated, or locked */
int mpi_free_helpmutex_orig(int inum)
{
    int i,mpierr;

    if (MPI_Debug) printf("%4d: In mpi_free_helpmutex_orig begin: original mutex num=%d\n",mpigv(myproc),inum);

    if (inum <0 || inum >= MAX_MPI_HELPMUTEX_ARRAYS ) {
       fprintf(stderr,"ERROR in mpi_free_helpmutex_orig: over range! original mutex num=%d, max num=%d\n",inum,MAX_MPI_HELPMUTEX_ARRAYS);
       exit (1);
    }
    else if ( mpi_helpmutex_index[inum].actv==0 ) {
       fprintf(stderr,"ERROR in mpi_free_helpmutex_orig: original mutex %d has not activated for use.\n",inum);
       exit (1);
    }
    else if ( mpi_helpmutex_index[inum].lock==1 ) {
       fprintf(stderr,"WARNING in mpi_free_helpmutex_orig: original mutex %d is still locked before freed. Now unlocking and freeing it...\n",inum);
       mpierr=mpi_unlock_helpmutex_orig(inum);
       mpi_helpmutex_index[inum].lock = 0;
       mpi_helpmutex_index[inum].actv = 0;
    }
    else {
       mpi_helpmutex_index[inum].actv = 0;
    }
    
    if (MPI_Debug) printf("%4d: In mpi_free_helpmutex_orig end: original mutex %d has been allocated.\n",mpigv(myproc),inum);

    return 0;
}

/* allocates a set containing the number of mutexes. Only one set of mutexes can exist at a time. Mutexes can be
allocated and freed as many times as needed. Mutexes are numbered: 0, ..., number-1. */
int mpi_allocate_helpmutexes(int number)
{
    int i,mpierr;
    int handle_orig;

    if (MPI_Debug) printf("%4d: In mpi_allocate_helpmutexes: begin.\n",mpigv(myproc));
    if ( number <=0 || number > MAX_MPI_HELPMUTEX_NONGA ) {
       fprintf(stderr,"ERROR in mpi_allocate_helpmutexes: over range! number=%d, max num=%d\n",number,MAX_MPI_HELPMUTEX_NONGA);
       return 1;
    }
    for (i=0;i<number;i++) {
       handle_orig=i+MAX_MPI_HELPMUTEX_GA;
       mpierr=mpi_alloc_helpmutex_orig(handle_orig);
    }
    mpigv(nhelpmutex)=number;
    
    if (MPI_Debug) printf("%4d: In mpi_allocate_helpmutexes end:  %d mutexes have been allocated.\n",mpigv(myproc),number);

    return 0;
}

/* frees the set of mutexes allocated with mpi_allocate_helpmutexes.*/
int mpi_free_helpmutexes()
{
    int i,mpierr;
    int handle_orig;
    
    if (MPI_Debug) printf("%4d: In mpi_free_helpmutexes begin: mpigv(nhelpmutex)=%d\n",mpigv(myproc),mpigv(nhelpmutex));

    if (mpigv(nhelpmutex)<=0) {
       fprintf(stderr,"ERROR in mpi_free_helpmutexes: no mutex is needed to be freed. N mutex=%d\n",mpigv(nhelpmutex));
       return 1;
    }
    for(i=0;i<mpigv(nhelpmutex); i++) {
       handle_orig=i+MAX_MPI_HELPMUTEX_GA;
       mpierr=mpi_free_helpmutex_orig(handle_orig);
    }

    mpigv(nhelpmutex)= 0;
    
    if (MPI_Debug) printf("%4d: In mpi_free_helpmutexes end: now mpigv(nhelpmutex)=%d\n",mpigv(myproc),mpigv(nhelpmutex));

    return 0;
}

/* lock a mutex object identified by the original mutex number. It is a fatal error for a process
   to attempt to lock a mutex which has already been locked by this process */
int mpi_lock_helpmutex_orig(int inum)
{
    int  mpierr;
    
    if (MPI_Debug) printf("%4d: In mpi_lock_helpmutex_orig begin: original mutex num=%d\n",mpigv(myproc),inum);

    if (inum <0 || inum >= MAX_MPI_HELPMUTEX_ARRAYS ) {
       fprintf(stderr,"ERROR in mpi_lock_helpmutex_orig: over range! original mutex num=%d, max num=%d\n",inum,MAX_MPI_HELPMUTEX_ARRAYS);
       exit (1);
    }
    else if ( mpi_helpmutex_index[inum].actv==0 ) {
       fprintf(stderr,"ERROR in mpi_lock_helpmutex_orig: mutex has not been activated yet.\n");
       exit (1);
    }
    else if ( mpi_helpmutex_index[inum].lock==1 ) {
       fprintf(stderr,"ERROR in mpi_lock_helpmutex_orig: attempt to lock a mutex which has already been locked.\n");
       exit (1);
    }
    else {
       mpierr = MPIMUTEX_Lock(mpi_helpmutex_index[inum].ptr);
       mpi_helpmutex_index[inum].lock = 1;
    }
    
    if (MPI_Debug) printf("%4d: In mpi_lock_helpmutex_orig: original mutex %d has been locked.\n",mpigv(myproc),inum);

    return 0;
}

/* unlock a mutex object identified by the original mutex number. It is a fatal error for a process
 * to attempt to unlock a mutex which has not been locked by this process. */
int mpi_unlock_helpmutex_orig(int inum)
{
    int  mpierr;
    
    if (MPI_Debug) printf("%4d: In mpi_unlock_helpmutex_orig begin: original mutex num=%d\n",mpigv(myproc),inum);

    if (inum <0 || inum >= MAX_MPI_HELPMUTEX_ARRAYS ) {
       fprintf(stderr,"ERROR in mpi_unlock_helpmutex_orig: over range! original mutex num=%d, max num=%d\n",inum,MAX_MPI_HELPMUTEX_ARRAYS);
       exit (1);
    }
    else if ( mpi_helpmutex_index[inum].actv==0 ) {
       fprintf(stderr,"ERROR in mpi_unlock_helpmutex_orig: mutex has not been activated yet.\n");
       exit (1);
    }
    else if ( mpi_helpmutex_index[inum].lock==0 ) {
       fprintf(stderr,"ERROR in mpi_unlock_helpmutex_orig: attempt to unlock a mutex which has not been locked.\n");
       exit (1);
    }
    else {
       mpierr = MPIMUTEX_Unlock(mpi_helpmutex_index[inum].ptr);
       mpi_helpmutex_index[inum].lock = 0;
    }
    
    if (MPI_Debug) printf("%4d: In mpi_unlock_helpmutex_orig: original mutex %d has been unlocked.\n",mpigv(myproc),inum);

    return 0;
}

/* lock a mutex object identified by the wrapped mutex number. It is a fatal error for a process
   to attempt to lock a mutex which has already been locked by this process */
int mpi_lock_helpmutex(int inum)
{
    int mpierr;
    int inum_orig;
    
    if (MPI_Debug) printf("%4d: In mpi_lock_helpmutex begin: mutex num=%d, total num=%d\n",mpigv(myproc),inum,mpigv(nhelpmutex));

    if (inum <0 || inum >= mpigv(nhelpmutex) ) {
       fprintf(stderr,"ERROR in mpi_lock_mutex: over range! mutex num=%d, total num=%d\n",inum,mpigv(nhelpmutex));
       return 1;
    }
    else {
       inum_orig=inum+MAX_MPI_HELPMUTEX_GA;
       mpierr=mpi_lock_helpmutex_orig(inum_orig);
    }
    
    if (MPI_Debug) printf("%4d: In mpi_lock_helpmutex: mutex %d has been locked.\n",mpigv(myproc),inum);

    return 0;
}


/* unlock a mutex object identified by the wrapped mutex number. It is a fatal error for a process
 * to attempt to unlock a mutex which has not been locked by this process. */
int mpi_unlock_helpmutex(int inum)
{
    int mpierr;
    int inum_orig;
    
    if (MPI_Debug) printf("%4d: In mpi_unlock_helpmutex begin: mutex num=%d, total num=%d\n",mpigv(myproc),inum,mpigv(nhelpmutex));

    if (inum <0 || inum >= mpigv(nhelpmutex) ) {
       fprintf(stderr,"ERROR in mpi_unlock_helpmutex: over range! mutex num=%d, total num=%d\n",inum,mpigv(nhelpmutex));
       return 1;
    }
    else {
       inum_orig=inum+MAX_MPI_HELPMUTEX_GA;
       mpierr=mpi_unlock_helpmutex_orig(inum_orig);
    }
    
    if (MPI_Debug) printf("%4d: In mpi_unlock_helpmutex: mutex %d has been unlocked.\n",mpigv(myproc),inum);

    return 0;
}


#else

void mpi_helpmutex_dummy() { }

#endif /* end of MPI2 */

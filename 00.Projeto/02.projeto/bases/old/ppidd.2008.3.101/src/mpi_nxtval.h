/* src/mpp/mpi_nxtval.h $Revision: 2008.3 $ */

/* ==================================================================== *\
 * MPI Version of nextval
 * ============================
 * 
\* ==================================================================== */

#ifndef __MPI_NXTVAL_H__
#define __MPI_NXTVAL_H__

/* **Include Files*** */
/*  prerequisite: include machines.h and mpi.h, and some basic libraries  */
#include "machines.h"
#include <mpi.h>

/* defined variables */

#define TYPE_NXTVAL 33333
#define TYPE_EXTRA  43333
#define MAX_PROCESS 4096

#if !defined NO_NXTVAL_SERVER
#define NXTVAL_SERVER
#else
#undef NXTVAL_SERVER
#endif

#define LEN 2
#define INCR 1                  /* increment for NXTVAL */
#define NXTFLAG 1555            /* flag of NXTVAL operations: terminate, fetch-and-add, release */
#define COLFLAG 1666            /* flag of collective operations for helpga: create, zeroize, destroy */
#define RMAFLAG 1777            /* flag of remoted memory access operations for helpga: get, fetch-and-add, put */
#define ETRFLAG 1888            /* flag of extra remoted memory access operations for helpga: get the value of elements,  put values to many helpga elements */
#define MUTFLAG 1999            /* flag of NXTVAL helpmutex operations for helpga: lock and unlock mutex */
    
#if !defined MAX_MPI_HELPGA_ARRAYS
#define MAX_MPI_HELPGA_ARRAYS 100
#endif

#define MPI_HELPGA_OFFSET 4000

/* MPI nxtvalmutex maxinum number = (num for GA) + (num for NONGA) */
/* MAX_MPI_NXTVALMUTEX_GA=MAX_MPI_ARRAYS, which is defined in mpiga_base.h */
#if !defined MAX_MPI_NXTVALMUTEX_GA
  #define MAX_MPI_NXTVALMUTEX_GA  512
#endif
#if !defined MAX_MPI_HELPMUTEX_NONGA
  #define MAX_MPI_NXTVALMUTEX_NONGA  0
#endif
#define MAX_MPI_NXTVALMUTEX_ARRAYS (MAX_MPI_NXTVALMUTEX_GA+MAX_MPI_NXTVALMUTEX_NONGA)

/* MPI helpmutex maxinum number = (num for GA) + (num for NONGA) */
#if !defined MAX_MPI_HELPMUTEX_GA
  #define MAX_MPI_HELPMUTEX_GA  0
#endif
#if !defined MAX_MPI_HELPMUTEX_NONGA
  #define MAX_MPI_HELPMUTEX_NONGA  50
#endif
#define MAX_MPI_HELPMUTEX_ARRAYS (MAX_MPI_HELPMUTEX_GA+MAX_MPI_HELPMUTEX_NONGA)
/* MAX_MPI_HELPMUTEX_ARRAYS can't be too large. Otherwise, this error ("too many communicators") 
may happen since MPICH2 runs out of context ids. When you create new communicators, it's best to free them after using. 
Internally in MPICH2, MPI_Cart_create, MPI_Win_create, and some of the MPI-IO functions call MPI_Comm_dup. */


/* derived data structure */

typedef struct {
       int  actv;               /* activity status                      */
       int  nele;               /* number of elements                   */
       void  *ptr;              /* pointer to Fortran integer           */
       char  *name;	        /* name of helpga                       */
       MPI_Datatype dtype;      /* Datatype of helpga                   */              
} mpi_helpga_array_t;


/* ----------------------------------------- *\
   Global Variables -- MPI NXTVAL Data
\* ----------------------------------------- */

    extern mpi_helpga_array_t *mpi_helpga_data_struc, *mpi_helpga_index;
    extern int mpi_helpga_num; /* Number of MPI helpga in use */

   /* NXTVAL helpmutex global lock list */
    extern int *nxtvalmutex_locklist;
    extern int nxtval_helpmutex_num;             /* Number of MPI NXTVAL helpmutex in use */

    extern int SR_parallel;    /* parallel flag */
    extern MPI_Comm MPIGA_WORK_COMM; /* MPI Work/Computation Communicator */


/* =========================== *\
    MPI NXTVAL Function Prototypes
\* =========================== */
   
    extern int NProcs_Work(void);
    extern int ServerID(void);
    extern void make_worker_comm( MPI_Comm , MPI_Comm * );  
    extern void NextValueServer(void);
    extern int NXTVAL(int *);
    extern int NXTVAL_helpga(int , int , int , int , int *, char *, MPI_Datatype );
    extern int NXTVAL_helpga_one(int , int , int , int , int *);
    extern int NXTVAL_helpga_extra(int , int , int , int , int *, void *);
    extern int NXTVAL_helpga_extra_acc(int , int , int , int , int *, void *, void *);
    extern int NXTVAL_helpga_handle_orig( int ); 
    extern MPI_Datatype NXTVAL_helpga_inquire_dtype( int ); 
    extern int NXTVAL_helpga_inquire_name(int , char **); 
    extern int NXTVAL_helpmutex(int , int , int );
    extern int NXTVAL_lock_helpmutex_orig(int );
    extern int NXTVAL_unlock_helpmutex_orig(int );
    extern int NXTVAL_lock_helpmutex(int );
    extern int NXTVAL_unlock_helpmutex(int );
    extern int NXTVAL_alloc_helpmutex_orig(int );
    extern int NXTVAL_free_helpmutex_orig(int );
    extern int NXTVAL_alloc_helpmutexes(int );
    extern int NXTVAL_free_helpmutexes(void);
    extern void initialize_nxtval_helpmutex(void);
    extern void finalize_nxtval_helpmutex(void);
    extern void initialize_helpga(void);
    extern void finalize_helpga(void);
    extern void install_nxtval(void);
    extern void finalize_nxtval(void);

#endif
/* endif of __MPI_NXTVAL_H__ */

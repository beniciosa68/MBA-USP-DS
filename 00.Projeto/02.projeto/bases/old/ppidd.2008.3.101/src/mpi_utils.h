/* src/mpp/mpi_utils.h $Revision: 2008.3 $ */

/* ==================================================================== *\
 * Standalone Utility Tools for MPI
\* ==================================================================== */

/* ------------- *\
***Include Files***
\* ------------- */

#ifndef __MPI_UTILS_H__
#define __MPI_UTILS_H__

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef CRAY_YMP
#define USE_MPI_ABORT
#endif

/* some definitions for mpi_test_status */
#if !defined PPIDD_MIN
  #define PPIDD_MIN(a,b) (((a) <= (b)) ? (a) : (b))
#endif
#define MSG_ERR_STR_LEN 80
#define TEST_ERR_STR_LEN (MSG_ERR_STR_LEN+MPI_MAX_ERROR_STRING)
extern  char  mpi_test_err_string[TEST_ERR_STR_LEN];



/* =========================== *\
    mpi_utils Function Prototypes
\* =========================== */
   
    extern int NNodes_Total(MPI_Comm);
    extern int ProcID(void);
    extern void MPIGA_Error(char *, int);
    extern void mpi_test_status(char *, int);
    extern int mpiga_type_f2c(int , MPI_Datatype *, int *);
    extern int mpiga_type_c2c(int , MPI_Datatype *, int *);

#endif

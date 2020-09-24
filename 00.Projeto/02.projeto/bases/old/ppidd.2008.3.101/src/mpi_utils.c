/* src/mpp/mpi_utils.c $Revision: 2008.3 $ */

/* ====================================================================== *\
 *                    Standalone Utility Tools for MPI                    *
 *                    ================================                    *
 *  (c) Copyright 2008 by the authors of MOLPRO. All rights reserved.     *
 * ---------------------------------------------------------------------- *
 * C sorce code of Standalone Utility Tools for MPI. The subroutines can  *
 * be called directly by c code, while the corresponding fortran wrappers *
 * are unnecessary.                                                       *
 *                                                                        *
 * Written by: Manhui Wang                                                *
 * Date:       11/03/2009                                                 *
\* ====================================================================== */

#if defined(MPI2) || defined(GA_MPI)

#include "machines.h"  /* needed by fortint in mpiga_type_f2c */
#include "mpi_utils.h"

char  mpi_test_err_string[TEST_ERR_STR_LEN];

static int DEBUG_=0;


/* Get the Total node number for specified communicator */
int NNodes_Total(MPI_Comm comm)
{
    int nprocs,rank,nnodes;
    char **nodename;
    int length;
    int max_length=256;
    int i,j,skip;

    MPI_Comm_size(comm,&nprocs); 
    MPI_Comm_rank(comm,&rank); 

    nodename = (char **)malloc(nprocs * sizeof(char *));
    for(i = 0; i < nprocs; i++) {
       nodename[i] = (char *)malloc(max_length * sizeof(char));
    }
    MPI_Get_processor_name(nodename[rank], &length);
    if(DEBUG_) fprintf(stdout,"%4d: In NNodes_Total: procname=%s,strlen=%d\n",rank,nodename[rank],length);

    for(i = 0; i < nprocs; i++){
       MPI_Bcast ( nodename[i], max_length, MPI_CHAR, i, comm );
    }
    
    nnodes=1;
    for(i = 0; i < nprocs; i++){
       skip=0;
       for(j = 0; j < nnodes; j++){
         if(strcmp(nodename[i],nodename[j])==0) skip=1;
       }
       if (skip==0) {nnodes+=1;strcpy(nodename[nnodes-1],nodename[i]);}
    }
    if(DEBUG_) fprintf(stdout,"%4d: In NNodes_Total: nnodes=%d\n",rank,nnodes);
    
    for(i = 0; i < nprocs; i++) free(nodename[i]);
    free(nodename);
        
    return(nnodes);
}

/* Get current calling process id */
int ProcID()
{
    int myid;

    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    return(myid);
}


/* Print the error message and exit */
void MPIGA_Error(char *string, int code)
{
    fprintf(stdout,"%4d: %s %d (%#x).\n", ProcID(), string,code,code);
    fflush(stdout);
    fprintf(stderr,"%4d: %s %d (%#x).\n", ProcID(), string,code,code);

    printf("%4d: In mpi_utils.c [MPIGA_Error]: now exiting...\n",ProcID());
#ifdef USE_MPI_ABORT
    MPI_Abort(MPI_COMM_WORLD,code);
#else
    exit(1);
#endif
}


/* Check the returned status; if status is not failing one, then print the error message and exit */
void mpi_test_status(char *msg_str, int status)
{
    if ( status != MPI_SUCCESS ) {
       int len_err_str, len_msg_str = PPIDD_MIN(MSG_ERR_STR_LEN, strlen(msg_str));
       strncpy(mpi_test_err_string, msg_str, len_msg_str);
       MPI_Error_string(status, mpi_test_err_string + len_msg_str, &len_err_str);
       MPIGA_Error(mpi_test_err_string, status);
    }
}

/* convert Fortran data types to C MPI_Datatypes */
/* type==0 : Fortran Integer and Logical types */
/* type==1 : Fortran Double Precision type */
/* type==others:  not allowed, ERROR  */
int mpiga_type_f2c(int type, MPI_Datatype *dtype, int *sizeofdtype)
{
    int ctype;
    int sizempidtype;
    MPI_Datatype mpidtype=MPI_CHAR;
    switch(type){
    case 0: 
            ctype=sizeof(fortint);
            if (ctype==(int)sizeof(int)) {
               MPI_Type_size(MPI_INT, &sizempidtype);
               if (ctype==sizempidtype) mpidtype=MPI_INT;
            }
            else if (ctype==(int)sizeof(long)) {
               MPI_Type_size(MPI_LONG, &sizempidtype);
               if (ctype==sizempidtype) mpidtype=MPI_LONG;
            }
            else if (ctype==(int)sizeof(long long)) {
               MPI_Type_size(MPI_LONG_LONG, &sizempidtype);
               if (ctype==sizempidtype) mpidtype=MPI_LONG_LONG;
            }
            else {
               MPIGA_Error("mpiga_type_f2c: no matched C type for Fortran Integer ", type);
            }
            if (mpidtype==MPI_CHAR) MPIGA_Error("mpiga_type_f2c: can't assign C MPI_Datatype for Fortran Integer ", type);
            break;
           /* MPI_INTEGER4 and MPI_INTEGER8 are not available in MPICH1 */
           /* if (ctype==4) mpitype=MPI_INTEGER4; */
           /* if (ctype==8) mpitype=MPI_INTEGER8; */
    case 1: 
            ctype=sizeof(double);
            MPI_Type_size(MPI_DOUBLE, &sizempidtype);
            if (ctype==sizempidtype) mpidtype=MPI_DOUBLE;
            else  MPIGA_Error("mpiga_type_f2c: can't assign C MPI_Datatype for Fortran Double Precision ", type);
            break;
           /* MPI_REAL4 is not available in MPICH1 */
           /* if (ctype==4) mpitype=MPI_REAL4; */
    default: 
            MPIGA_Error("mpiga_type_f2c: can't assign C MPI_Datatype ", type);
            break;
    }
    *dtype=mpidtype;
    *sizeofdtype=sizempidtype;
    return 0;
}


/* convert C data types to C MPI_Datatypes */
/* type==0 : C type  int                   */
/* type==1 : C type  double                */
/* type==2 : C type  long                  */
/* type==3 : C type  long long             */
/* type==4 : C type  float                 */
/* type==others:  not allowed, ERROR       */
int mpiga_type_c2c(int type, MPI_Datatype *dtype, int *sizeofdtype)
{
    int ctype;
    int sizempidtype;
    MPI_Datatype mpidtype=MPI_CHAR;
    switch(type){
    case 0: 
            ctype=sizeof(int);
            MPI_Type_size(MPI_INT, &sizempidtype);
            if (ctype==sizempidtype) mpidtype=MPI_INT;
            break;
    case 1: 
            ctype=sizeof(double);
            MPI_Type_size(MPI_DOUBLE, &sizempidtype);
            if (ctype==sizempidtype) mpidtype=MPI_DOUBLE;
            break;
    case 2: 
            ctype=sizeof(long);
            MPI_Type_size(MPI_LONG, &sizempidtype);
            if (ctype==sizempidtype) mpidtype=MPI_LONG;
            break;
    case 3: 
            ctype=sizeof(long long);
            MPI_Type_size(MPI_LONG_LONG, &sizempidtype);
            if (ctype==sizempidtype) mpidtype=MPI_LONG_LONG;
            break;
    case 4: 
            ctype=sizeof(float);
            MPI_Type_size(MPI_FLOAT, &sizempidtype);
            if (ctype==sizempidtype) mpidtype=MPI_FLOAT;
            break;
    default: 
            MPIGA_Error("mpiga_type_c2c: can't assign C MPI_Datatype ", type);
            break;
    }
    if (mpidtype==MPI_CHAR) MPIGA_Error("mpiga_type_c2c: can't assign C MPI_Datatype for C type ", type);
    *dtype=mpidtype;
    *sizeofdtype=sizempidtype;
    return 0;
}

#else

void mpi_utils_dummy () {}

#endif /* end of MPI2 or GA_MPI */

/* src/mpp/mpi_nxtval.c $Revision: 2008.3 $ */

/* ====================================================================== *\
 *                    MPI Version of Shared Counter                       *
 *                    =============================                       *
 *  (c) Copyright 2008 by the authors of MOLPRO. All rights reserved.     *
 * ---------------------------------------------------------------------- *
 * C sorce code of MPI Version of Shared Counter. The subroutines can be  *
 * called directly by c code, while the corresponding fortran wrappers    *
 * (which can be called by fortran code) are in file mpiga_fortran.c.     *
 *                                                                        *
 * Written by: Manhui Wang                                                *
 * Date:       02/11/2008                                                 *
\* ====================================================================== */

#ifdef MPI2

#include "machines.h"
#include "mpi_utils.h"
#include "mpi_nxtval.h"
void finalize_helpmutexes(void);

/* NXTVAL helpga global variables */
mpi_helpga_array_t *mpi_helpga_data_struc=NULL, *mpi_helpga_index;
int mpi_helpga_num=0;
/* NXTVAL helpmutex global variables */
int *nxtvalmutex_locklist=NULL;
int nxtval_helpmutex_num=0; 

static int DEBUG_=0;


/* get the number of work/compute processes */
int NProcs_Work()
{
    int numprocs;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
#  ifdef NXTVAL_SERVER
     if(SR_parallel) return(numprocs-1);
#  else
     if(DEBUG_) fprintf(stdout,"%4d: In NProcs_Work: NXTVAL_SERVER has not been defined, total procs=%d\n",ProcID(),numprocs);
#  endif
    return(numprocs);
}

/* Get the ID of server process */
int ServerID()
{
#  ifdef NXTVAL_SERVER
     int  server = NProcs_Work();
#  else
     int  server = NProcs_Work()-1;
#  endif
    return(server);
}


/* =================================================================================== *\
 *                  Create work or computation communicator.                           *
 *  It is a collective operation for old_comm; it will split off one process of the    *
 *  communicator's group to hold the counter, and return a new work communicator. The  *
 *  work communicator will be used by the remaining processes for the main computation.*
\* =================================================================================== */
void make_worker_comm( MPI_Comm old_comm, MPI_Comm *worker_comm )  
{ 
    int myid, numprocs, server, color; 
 
# ifdef NXTVAL_SERVER
    if( SR_parallel ){   
          /* data server for a single process */
          MPI_Comm_size(old_comm, &numprocs); 
          MPI_Comm_rank(old_comm, &myid); 
          server = numprocs-1;     /*   last proc is server */ 
          if (myid == server) color = MPI_UNDEFINED; 
          else color = 0; 
          MPI_Comm_split( old_comm, color, myid, worker_comm ); 
    }else
# endif
          MPI_Comm_dup( old_comm, worker_comm ); /* duplicate old_com to worker_comm */ 
}


void NextValueServer()
{
  int  cnt     = 0;            /* actual counter */
  void  *buf_helpga=NULL;      /* help ga which is located in help process */
  void  *buf_ielem =NULL;      /* pointer to helpga[ielem-1]               */
  void  *buf_temp=NULL;        /* temporary buffer which are used to store data for accumulation */
  int *ibuf,*ibuf_helpga;
  long *lbuf,*lbuf_helpga;
  long long *llbuf,*llbuf_helpga;
  float  *fbuf,*fbuf_helpga;
  double *dbuf,*dbuf_helpga;
  int  totworkproc;            /* total number of work processes */
  int  ndone   = 0;            /* no. finished for this loop */
  int  type    = TYPE_NXTVAL;  /* message type */
  int  type_extra = TYPE_EXTRA;/* extra RMA message type */
  int  buf[LEN+3];             /* buffer to get values */
  int  localbuf[LEN+3];        /* temporary local buffer */
  int  num_locked;             /* number of locked mutexes */
  int  mproc,submproc;         /* no. of processes running loop */
  int  operflag,suboperflag;   /* NXTFLAG, COLFLAG, RMAFLAG, ETRFLAG, or MUTFLAG */
  int  done_list[MAX_PROCESS]; /* list of processes finished with this loop */
  int  *done_list_rls=NULL,*done_list_zero=NULL;
  int  lenmes, sublenmes, nodefrom;
  int  node, i;
  int  ntermin=0;
  int  ndone_init=0, ndone_rls=0, ndone_zero=0;
  int  ielem_dtype,ielem,nelem_valput,handle,handle_orig,subhandle;
  int  nelem=0;
  int  nelem_helpga=0;
  int  mpiflag;      /* return flag of mpi calling */
  int  flagzero=0;   /* return successful flag 0   */
  int  returnval=0;  /* returned value for helpha RMA opreations  */
  MPI_Status status;
  MPI_Datatype dtype;   /* MPI Datatype */
  int sizeofdtype;      /* Size of MPI Datatype */

  if (DEBUG_) printf("%4d: In mpi_nxtval: NextValueServe begin.\n",ProcID());

  totworkproc=NProcs_Work(); 

  while (1) {

    /* Wait for input from any node */
    
    MPI_Recv(buf, LEN+3, MPI_INT, MPI_ANY_SOURCE, type, MPI_COMM_WORLD, &status); 
    MPI_Get_count(&status, MPI_INT, &lenmes);
    nodefrom = status.MPI_SOURCE;

    mproc = buf[0];      /* number of process */
    operflag = buf[1];   /* Operartion Flag: NXTFLAG (NXTVAL); COLFLAG and RMAFLAG (NXTVAL_HELPGA); ETRFLAG (NXTVAL_HELPGA_EXTRA) */

    if (DEBUG_) printf("%4d: NextValueServer: In the beginning of while loop. from=%d, mproc=%d, operflag=%d\n",ProcID(),nodefrom,mproc,operflag);

    if ( lenmes == LEN && operflag == NXTFLAG ) {  /* NXTVAL */ 
     if (mproc == 0) {

      /* Sending process is about to terminate. Send reply and disable
       * sending to him. If all processes have finished return.
       *
       * All processes block on waiting for message
       * from nxtval server before terminating. nxtval only lets
       * everyone go when all have registered termination.
       */

      if (++ntermin == totworkproc) {
        for (node=0; node<totworkproc; node++) {
          MPI_Send(&cnt, 1, MPI_INT,  node, type, MPI_COMM_WORLD); 
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if ( MPIGA_WORK_COMM != MPI_COMM_NULL) MPI_Comm_free( &MPIGA_WORK_COMM );
/* clean up nxtval resources on server, include finalize_helpga and finalize_nxtval_helpmutex */
        finalize_nxtval();
/* clean up helpmutex resources on server */
        finalize_helpmutexes();
        if (DEBUG_) printf("%4d: In mpi_nxtval: NextValueServer: before MPI_Finalized.\n",ProcID());
        MPI_Finalized(&mpiflag);
        if (!mpiflag) MPI_Finalize();
        else printf("In mpi_nxtval: NextValueServer: MPI_Finalize has been called before MPI_Finalize.\n");
        if (DEBUG_) printf("In mpi_nxtval: NextValueServer: terminate nxtval server.\n");
        exit(0); 
      }
     }
     else if (mproc > 0) {    /* fetch-and-add INCR */
      MPI_Send(&cnt, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD); 
      cnt += INCR;
     }
     else if (mproc < 0) {   /* release and set cnt=0 */
      /* Wait until all -mproc processes have finished before releasing it */
      done_list[ndone++] = nodefrom;
      if (ndone == -mproc) {
	while (ndone--) {
	  nodefrom = done_list[ndone];
          MPI_Send(&cnt, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD); 
	}
	cnt = 0;
	ndone = 0;
      }
     }
    } /* End of NXTVAL */
    else if ( lenmes == LEN+3 && operflag == COLFLAG ) {
      /* HELPGA collective operations: create (mproc=0) , zeroize(mproc>0), destroy (mproc<0) */
     nelem_valput=buf[2]; /* mproc=0: number of elements; mproc>=0: no use */
     ielem_dtype =buf[3];        /* no use here */
     handle=buf[4];      /* adjuested sequence number of helpga */
     handle_orig=handle-MPI_HELPGA_OFFSET; /* original sequence number of helpga */

     if (mproc == 0) { /* create a help GA */

      /* All work processes tell server to create an array with nelem_valput elements.
       * Servers wait until it has received all requests from all work processes.
       * After received all requests, helpga is created in server, and server send 
       * a SUCESS flag to all work processes.
       *
       * All work processes block on waiting for message from nxtval server before return. 
       * If all work processes have received SUCESS flag, then return.
       */

      if (++ndone_init == totworkproc) {
        dtype=(MPI_Datatype)ielem_dtype;
        MPI_Type_size( dtype, &sizeofdtype ); 
        mpi_helpga_index[handle_orig].ptr=malloc(nelem_valput*sizeofdtype);
        mpi_helpga_index[handle_orig].nele=nelem_valput;
        mpi_helpga_index[handle_orig].actv=1;
        mpi_helpga_index[handle_orig].dtype=dtype;

        for (node=0; node<totworkproc; node++) {
          MPI_Send(&flagzero, 1, MPI_INT,  node, type, MPI_COMM_WORLD); 
        }
        mpi_helpga_num++;
        ndone_init=0;
      }
     } /* end of creating help GA */
     else if (mproc > 0) { /* zeroize help GA */
      if (!done_list_zero) done_list_zero=(int *)calloc(totworkproc,sizeof(int));
      done_list_zero[ndone_zero++] = nodefrom;
      if (ndone_zero == mproc) {
	while (ndone_zero--) {
	  nodefrom = done_list_zero[ndone_zero];
          MPI_Send(&flagzero, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD); 
	}

        nelem_helpga=mpi_helpga_index[handle_orig].nele;
        dtype=mpi_helpga_index[handle_orig].dtype;
        if (dtype==MPI_INT) {
           ibuf=(int *)mpi_helpga_index[handle_orig].ptr;
           for (i=0;i<nelem_helpga;i++) ibuf[i]=(int)0;
        } 
        else if (dtype==MPI_LONG) {
           lbuf=(long *)mpi_helpga_index[handle_orig].ptr;
           for (i=0;i<nelem_helpga;i++) lbuf[i]=(long)0;
        } 
        else if (dtype==MPI_LONG_LONG) {
           llbuf=(long long *)mpi_helpga_index[handle_orig].ptr;
           for (i=0;i<nelem_helpga;i++) llbuf[i]=(long long)0;
        } 
        else if (dtype==MPI_FLOAT) {
           fbuf=(float *)mpi_helpga_index[handle_orig].ptr;
           for (i=0;i<nelem_helpga;i++) fbuf[i]=0.0;
        }
        else if (dtype==MPI_DOUBLE) {
           dbuf=(double *)mpi_helpga_index[handle_orig].ptr;
           for (i=0;i<nelem_helpga;i++) dbuf[i]=0.0e0;
        }
        else {
           MPIGA_Error("NextValueServer: helpga_zero: wrong MPI_Datatype ",(int)dtype);
        }

        if (done_list_zero != NULL) {free (done_list_zero); done_list_zero=NULL;}
	ndone_zero = 0;
      }
     } /* end of zeroizing HELP GA */
     else if (mproc < 0) { /* release/destroy HELP GA */
      if (!done_list_rls) done_list_rls=(int *)calloc(totworkproc,sizeof(int));
      done_list_rls[ndone_rls++] = nodefrom;
      if (ndone_rls == -mproc) {
	while (ndone_rls--) {
	  nodefrom = done_list_rls[ndone_rls];
          MPI_Send(&flagzero, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD); 
	}
        if (mpi_helpga_index[handle_orig].ptr!=NULL) free (mpi_helpga_index[handle_orig].ptr);
        mpi_helpga_index[handle_orig].nele=0;
        mpi_helpga_index[handle_orig].actv=0;
        mpi_helpga_num--;
        if (done_list_rls != NULL) { free (done_list_rls); done_list_rls=NULL; }
	ndone_rls = 0;
      }
     } /* release/destroy HELP GA */
    } /* End of HELPGA */
    else if ( lenmes == LEN+3 && operflag == RMAFLAG ) {
      /* HELPGA RMA operations: get (mproc=0) , fetch-and-add(mproc>0), put (mproc<0) */
     nelem_valput=buf[2]; /* mproc=0: no use; mproc<0: value to be put; mproc>0: increment value */
     ielem=buf[3];        /* sequence number of element in helpga: 1,2,...,n      */
     handle=buf[4];      /* adjuested sequence number of helpga */
     handle_orig=handle-MPI_HELPGA_OFFSET; /* original sequence number of helpga */

     dtype=mpi_helpga_index[handle_orig].dtype;
     if (mproc == 0) { /* work process gets a value from help GA, ie server sends an element value to a work process */
       if (dtype==MPI_INT) {
          ibuf=(int *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)ibuf[ielem-1];
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)lbuf[ielem-1];
       } 
       else {
          llbuf=(long long *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)llbuf[ielem-1];
       } 
       MPI_Send(&returnval, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD); 
     }
     else if (mproc > 0) { /* fetch-and-add INCR to help GA */
       if (dtype==MPI_INT) {
          ibuf=(int *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)ibuf[ielem-1];
          ibuf[ielem-1]+=(int)nelem_valput;
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)lbuf[ielem-1];
          lbuf[ielem-1]+=(long)nelem_valput;
       } 
       else {
          llbuf=(long long *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)llbuf[ielem-1];
          llbuf[ielem-1]+=(long long)nelem_valput;
       } 
       MPI_Send(&returnval, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD); 
     }
     else { /* put a value to help GA */
       if (dtype==MPI_INT) {
          ibuf=(int *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)ibuf[ielem-1];
          ibuf[ielem-1]=(int)nelem_valput;
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)lbuf[ielem-1];
          lbuf[ielem-1]=(long)nelem_valput;
       } 
       else {
          llbuf=(long long *)mpi_helpga_index[handle_orig].ptr;
          returnval = (int)llbuf[ielem-1];
          llbuf[ielem-1]=(long long)nelem_valput;
       } 
       MPI_Send(&returnval, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD); 
     }
    } /* End of HELPGA RMA operations */
    else if ( lenmes == LEN+3 && operflag == ETRFLAG ) {
      /* HELPGA extra RMA operations: get(mproc=0), put (mproc<0) */
     nelem_valput=buf[2]; /* mproc<=0: number of elements to be gotten/put; >0 no use;    */
     ielem=buf[3];        /* sequence number of element in helpga: 1,2,...,n      */
     handle=buf[4];       /* adjuested sequence number of helpga */
     handle_orig=handle-MPI_HELPGA_OFFSET; /* original sequence number of helpga */

     buf_helpga=mpi_helpga_index[handle_orig].ptr;
     dtype=mpi_helpga_index[handle_orig].dtype;
     MPI_Type_size( dtype, &sizeofdtype ); 
     buf_ielem = (void *)( (char *)buf_helpga + (ielem-1) * sizeofdtype );

     if (mproc == 0) { /* work process gets a set of values from help GA, ie server sends a set of values to a work process */
      MPI_Send(buf_ielem, nelem_valput, dtype,  nodefrom, type_extra, MPI_COMM_WORLD);  /*send n elements to work process*/
      MPI_Send(&flagzero, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD);  /*send SUCCESS flag to work process*/
     }
     else if (mproc < 0) { /* put a set of values to help GA */
      /*receive n elements from work process*/
      MPI_Recv(buf_ielem, nelem_valput, dtype,  nodefrom, type_extra, MPI_COMM_WORLD, &status); 
      MPI_Get_count(&status, dtype, &nelem);
      if ( nelem != nelem_valput) MPIGA_Error("NextValueServer: for put operation. nelem != nelem_valput", nelem);
      MPI_Send(&flagzero, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD);  /*send SUCCESS flag to work process*/
     }
     else { /* mproc>0, accumulate a set of values to help GA (First receive data, then add it to help GA ) */
      /*receive n elements from work process*/
       if (!buf_temp) buf_temp=malloc(nelem_valput*sizeofdtype);
       MPI_Recv(buf_temp, nelem_valput, dtype,  nodefrom, type_extra, MPI_COMM_WORLD, &status); 
       MPI_Get_count(&status, dtype, &nelem);
       if ( nelem != nelem_valput) MPIGA_Error("NextValueServer: for accumulation. nelem != nelem_valput", nelem);
       MPI_Send(&flagzero, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD);  /*send SUCCESS flag to work process*/
       if (dtype==MPI_INT) {
          ibuf=(int *)buf_temp;
          ibuf_helpga=(int *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) ibuf_helpga[ielem-1+i]+=ibuf[i];   
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)buf_temp;
          lbuf_helpga=(long *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) lbuf_helpga[ielem-1+i]+=lbuf[i];   
       } 
       else if (dtype==MPI_LONG_LONG) {
          llbuf=(long long *)buf_temp;
          llbuf_helpga=(long long *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) llbuf_helpga[ielem-1+i]+=llbuf[i];   
       } 
       else if (dtype==MPI_FLOAT) {
          fbuf=(float *)buf_temp;
          fbuf_helpga=(float *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) fbuf_helpga[ielem-1+i]+=fbuf[i];   
       } 
       else if (dtype==MPI_DOUBLE) {
          dbuf=(double *)buf_temp;
          dbuf_helpga=(double *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) dbuf_helpga[ielem-1+i]+=dbuf[i];   
       } 
       else {
          MPIGA_Error("NextValueServer NXTVAL_helpga_extra: wrong MPI_Datatype ",(int)dtype);
       } 
       if (buf_temp != NULL) { free (buf_temp); buf_temp=NULL; }
     }
    } /* End of HELPGA extra RMA operations */
    else if ( lenmes == LEN+1 && operflag == MUTFLAG ) {
      /* NXTVAL HELPMUTEX lock and unlock operations: lock(mproc>0), unlock(mproc<0),no-use(mproc=0) */
     handle_orig=buf[2]; /* original sequence number of NXTVAL helpmutex */
     num_locked=0;
     if (mproc > 0) { /* lock a NXTVAL helpmutex */
       if (nxtvalmutex_locklist[handle_orig]==0) nxtvalmutex_locklist[handle_orig]=1;
       else MPIGA_Error("NextValueServer: attempt to lock a locked mutex. handle_orig ", handle_orig);
       num_locked+=1;
       MPI_Send(&flagzero, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD);  /*send SUCCESS flag to work process*/
       while ( num_locked != 0 ) {
         MPI_Recv(localbuf, LEN+3, MPI_INT,  nodefrom, type, MPI_COMM_WORLD, &status); 
         MPI_Get_count(&status, MPI_INT, &sublenmes);
         submproc=localbuf[0];     /* lock(>0) or unlock(<0) flag */
         suboperflag=localbuf[1];  /* MUTFLAG */
         subhandle=localbuf[2];    /* original sequence number of NXTVAL helpmutex */
         if ( sublenmes != LEN+1 && operflag != MUTFLAG ) MPIGA_Error("NextValueServer: wrong sublenmes ", sublenmes);
         if (submproc>0) { /* lock another mutex */
           if (nxtvalmutex_locklist[subhandle]==0) nxtvalmutex_locklist[subhandle]=1;
           else MPIGA_Error("NextValueServer: attempt to lock a locked mutex ", subhandle);
           num_locked+=1;
         }
         else if (submproc<0) { /* unlock current mutex */
           if (nxtvalmutex_locklist[subhandle]==1) nxtvalmutex_locklist[subhandle]=0;
           else MPIGA_Error("NextValueServer: attempt to unlock an unlocked mutex ", subhandle);
           num_locked-=1;
         }
         else { /* submproc=0 */
           MPIGA_Error("NextValueServer: wrong submproc", submproc);
         }
         MPI_Send(&flagzero, 1, MPI_INT,  nodefrom, type, MPI_COMM_WORLD);  /*send SUCCESS flag to work process*/
       }    
     }
     else { /* mproc<=0 */
       MPIGA_Error("NextValueServer: helpmutex with wrong mproc ", mproc);
     }
    } /* End of NXTVAL helpmutex lock and unlock operations */
    if (DEBUG_) printf("%4d: NextValueServer: In the end of while loop. from=%d, mproc=%d, operflag=%d\n",ProcID(),nodefrom,mproc,operflag);
  } /* end of while loop */
}


int NXTVAL(int *mproc)
/*
  Get next value of shared counter.

  mproc > 0 ... fetch-and-add operation, returns requested value
  mproc < 0 ... server blocks until abs(mproc) processes are queued
                and returns junk, release cnt
  mproc = 0 ... indicates to server that I am about to terminate

*/
{
  int  buf[2];
  MPI_Status status;
  int  type = TYPE_NXTVAL;
  int  local;
  int  server = ServerID();       /* id of server process */


  if (SR_parallel) {
     buf[0] = *mproc;
     buf[1] = NXTFLAG;

     if (DEBUG_) {
       (void) printf("%4d: NXTVAL: mproc=%d\n",ProcID(), *mproc);
       (void) fflush(stdout);
     }

#    ifdef NXTVAL_SERVER
       MPI_Send(buf, LEN, MPI_INT,  server, type, MPI_COMM_WORLD); 
       MPI_Recv(buf, 1,   MPI_INT,  server, type, MPI_COMM_WORLD, &status); 
       return buf[0];
#    endif
  } 
  else {
     /* Not running in parallel ... just do a simulation */
     static int count = 0;
     if (*mproc == 1)
       local = count++;
     else if (*mproc == -1) {
       count = 0;
       local = 0;
    }
    else
      MPIGA_Error("NXTVAL: sequential version with silly mproc ", *mproc);
  }

  return local;
}


int NXTVAL_helpga(int mproc, int flag, int nelem_valput, int ielem, int *handle, char *name, MPI_Datatype mpidtype)
/*
  Operations for helpga:
____________________________________________________________________________________________________
|                |     COLFLAG       |        RMAFLAG               |          ETRFLAG             |
|  mproc = 0 ... |  create helpga    | get one element from helpga  |get elements from helpga      |
|  mproc > 0 ... |  zeroize helpga   | fetch-and-add to helpga      |accumulate helpga elements    | 
|  mproc < 0 ... |  destroy helpga   | put one value to helpga      |put values to helpga elements |
----------------------------------------------------------------------------------------------------
*/
{
  int  buf[5];
  int  countsent=LEN+3;
  MPI_Status status;
  int  type = TYPE_NXTVAL;
  int  local=0;
  int  operflag=flag;
  int  handle_orig;
  int  i;
  char *helpganame;
  MPI_Datatype dtype;
  int sizeofdtype;
  int ielem_dtype=ielem;
  int  server = ServerID();       /* id of server process */

  if (DEBUG_) printf("%4d: NXTVAL_helpga: begin. mproc=%d\n",ProcID(),mproc);

  if( operflag!=COLFLAG ) { /*check if collective operations */
    fprintf(stderr," NXTVAL_helpga ERROR:  not collective operations.\n");
    return 1;
  }

  if( mproc==0 ) { /*create helpga */
    mpi_helpga_num++; 
   /* Check to ensure the maximum number of arrays hasn't been reached.*/
    if( mpi_helpga_num > MAX_MPI_HELPGA_ARRAYS ) {
      fprintf(stderr," NXTVAL_helpga ERROR:  %d over the maximum number of mpi helpga [%i].\n",mpi_helpga_num,MAX_MPI_HELPGA_ARRAYS);
      return 1;
    }
    for(i=0;i<MAX_MPI_HELPGA_ARRAYS;i++){
       if ( mpi_helpga_index[i].actv == 0 ) {
          handle_orig=i;  /* original sequence number of helpga */
	  break;
       }
    }
    strcpy(helpganame=(char *)malloc(strlen(name)+1),name); 
    mpi_helpga_index[handle_orig].nele=nelem_valput;
    mpi_helpga_index[handle_orig].actv=1;
    mpi_helpga_index[handle_orig].ptr=NULL;
    mpi_helpga_index[handle_orig].name=helpganame;
    mpi_helpga_index[handle_orig].dtype=mpidtype;
    ielem_dtype=(int)mpidtype;

    *handle=handle_orig+MPI_HELPGA_OFFSET;  /* adjusted sequence number of helpga */
  }
  else { 

    handle_orig=NXTVAL_helpga_handle_orig(*handle); 

    if( flag==COLFLAG && mproc<0 ) { /* release helpga */
      mpi_helpga_index[handle_orig].nele=0;
      mpi_helpga_index[handle_orig].actv=0;
      if ( mpi_helpga_index[handle_orig].ptr != NULL ) free (mpi_helpga_index[handle_orig].ptr);
      if ( mpi_helpga_index[handle_orig].name!= NULL ) free (mpi_helpga_index[handle_orig].name);
      mpi_helpga_num--;
    }
  } /* other operations (zeroization and release) for helpga */

  if (DEBUG_) {
    (void) printf("%4d: NXTVAL_helpga: mproc=%d, handle=%d\n",ProcID(),mproc,*handle);
    (void) fflush(stdout);
  }

  if (SR_parallel) {
     buf[0] = mproc; /* COLFLAG: create(=0), zeroize(>0), destroy(<0); RMAFLAG: get(=0), fetch-and-add(>0), put(<0) */
     buf[1] = flag;  /* COLFLAG or RMAFLAG */
     buf[2] = nelem_valput; /* COLFLAG and mproc=0: number of elements; RMAFLAG: value to be put(mproc<0), increment value(mproc>0); others: no use */
     buf[3] = ielem_dtype;  /* COLFLAG and mproc=0: MPI_Datatype; RMAFLAG: sequence number of element (1,2,...,n) */
     buf[4] = *handle;     /* sequence number of helpga */

#    ifdef NXTVAL_SERVER
       MPI_Send(buf, countsent, MPI_INT,  server, type, MPI_COMM_WORLD); 
       MPI_Recv(buf, 1,   MPI_INT,  server, type, MPI_COMM_WORLD, &status); 
       local=buf[0];
#    endif
  } 
  else {
     /* Not running in parallel ... just do a simulation */
    int *ibuf;
    long *lbuf;
    long long *llbuf;
    float  *fbuf;
    double *dbuf;
    int j;
    int nelem_localga=0;
    dtype=mpi_helpga_index[handle_orig].dtype;
    if ( operflag==COLFLAG ) {         /* simulate HELPGA collective operations   */
     if (mproc == 0) {                 /* create a local helpga  */
       MPI_Type_size( dtype, &sizeofdtype ); 
       mpi_helpga_index[handle_orig].ptr=malloc(nelem_valput*sizeofdtype);
       local = 0;
     }
     else if (mproc == 1) {            /* zeroize local helpga */
       nelem_localga=mpi_helpga_index[handle_orig].nele;
       if (dtype==MPI_INT) {
          ibuf=(int *)mpi_helpga_index[handle_orig].ptr;
          for (j=0;j<nelem_localga;j++) ibuf[j]=(int)0;
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)mpi_helpga_index[handle_orig].ptr;
          for (j=0;j<nelem_localga;j++) lbuf[j]=(long)0;
       } 
       else if (dtype==MPI_LONG_LONG) {
          llbuf=(long long *)mpi_helpga_index[handle_orig].ptr;
          for (j=0;j<nelem_localga;j++) llbuf[j]=(long long)0;
       } 
       else if (dtype==MPI_FLOAT) {
          fbuf=(float *)mpi_helpga_index[handle_orig].ptr;
          for (j=0;j<nelem_localga;j++) fbuf[j]=0.0;
       }
       else if (dtype==MPI_DOUBLE) {
          dbuf=(double *)mpi_helpga_index[handle_orig].ptr;
          for (j=0;j<nelem_localga;j++) dbuf[j]=0.0e0;
       }
       else {
          MPIGA_Error("helpga_zero: wrong MPI_Datatype ",(int)dtype);
       }
       local = 0;
     }
     else if (mproc == -1) {           /* destroy local helpga */
 /*      if (mpi_helpga_index[handle_orig].ptr!=NULL) free (mpi_helpga_index[handle_orig].ptr);*/
       local = 0;
     }
     else
       MPIGA_Error("NXTVAL_helpga: sequential version with silly mproc ", mproc);
    }
  } /* end else of big if-else */

  if (DEBUG_) printf("%4d: NXTVAL_helpga: end. handle=%d\n",ProcID(),*handle);

  return local;
}

/* one-element rma operations for helpga */
/* MPI_Datatye can only be integer type (MPI_INT, MPI_LONG, and MPI_LONG_LONG, but not MPI_FLOAT and MPI_DOUBLE) */
int NXTVAL_helpga_one(int mproc, int flag, int nelem_valput, int ielem, int *handle)
/*
  Operations for helpga:
____________________________________________________________________________________________________
|                |     COLFLAG       |        RMAFLAG               |          ETRFLAG             |
|  mproc = 0 ... |  create helpga    | get one element from helpga  |get elements from helpga      |
|  mproc > 0 ... |  zeroize helpga   | fetch-and-add to helpga      |accumulate helpga elements    | 
|  mproc < 0 ... |  destroy helpga   | put one value to helpga      |put values to helpga elements |
----------------------------------------------------------------------------------------------------
*/
{
  int  buf[5];
  int  countsent=LEN+3;
  MPI_Status status;
  int  type = TYPE_NXTVAL;
  int  local=0;
  int  operflag=flag;
  int  handle_orig;
  MPI_Datatype dtype;
  int  server = ServerID();       /* id of server process */

  if (DEBUG_) printf("%4d: NXTVAL_helpga_one: begin. handle=%d\n",ProcID(),*handle);

  if( operflag!=RMAFLAG ) { /*check if one-element rma operations */
    fprintf(stderr," NXTVAL_helpga_one ERROR:  not one-element rma operations.\n");
    return 1;
  }

  handle_orig=NXTVAL_helpga_handle_orig(*handle); 

  dtype=mpi_helpga_index[handle_orig].dtype;
  if (dtype==MPI_INT || dtype==MPI_LONG || dtype==MPI_LONG_LONG ) {
    if (DEBUG_) printf("%4d: NXTVAL_helpga_one: array with handle=%d  is an integer MPIGA.\n",ProcID(),*handle);
  }
  else  MPIGA_Error(" NXTVAL_helpga_one: wrong MPI_Datatype ",(int)dtype);

  if (DEBUG_) {
    (void) printf("%4d: NXTVAL_helpga_one: mproc=%d, handle=%d\n",ProcID(),mproc,*handle);
    (void) fflush(stdout);
  }

  if (SR_parallel) {
     buf[0] = mproc; /* COLFLAG: create(=0), zeroize(>0), destroy(<0); RMAFLAG: get(=0), fetch-and-add(>0), put(<0) */
     buf[1] = flag;  /* COLFLAG or RMAFLAG */
     buf[2] = nelem_valput; /* COLFLAG and mproc=0: number of elements; RMAFLAG: value to be put(mproc<0), increment value(mproc>0); others: no use */
     buf[3] = ielem;        /* COLFLAG: MPI_Datatype; RMAFLAG: sequence number of element (1,2,...,n) */
     buf[4] = *handle;      /* sequence number of helpga */

#    ifdef NXTVAL_SERVER
       MPI_Send(buf, countsent, MPI_INT,  server, type, MPI_COMM_WORLD); 
       MPI_Recv(buf, 1,   MPI_INT,  server, type, MPI_COMM_WORLD, &status); 
       local=buf[0];
#    endif
  } 
  else {
     /* Not running in parallel ... just do a simulation */
    int *ibuf;
    long *lbuf;
    long long *llbuf;
    if ( operflag==RMAFLAG ) {    /* simulate HELPGA one-element RMA operations     */
     if (mproc == 0) {                 /* get a value from local helpga      */
       if (dtype==MPI_INT) {
          ibuf=(int *)mpi_helpga_index[handle_orig].ptr;
          local = (int)ibuf[ielem-1];
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)mpi_helpga_index[handle_orig].ptr;
          local = (int)lbuf[ielem-1];
       } 
       else {
          llbuf=(long long *)mpi_helpga_index[handle_orig].ptr;
          local = (int)llbuf[ielem-1];
       } 
     }
     else if (mproc == 1) {            /* fetch-and-add INCR to local helpga */
       if (dtype==MPI_INT) {
          ibuf=(int *)mpi_helpga_index[handle_orig].ptr;
          local = (int)ibuf[ielem-1];
          ibuf[ielem-1]+=(int)nelem_valput;
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)mpi_helpga_index[handle_orig].ptr;
          local = (int)lbuf[ielem-1];
          lbuf[ielem-1]+=(long)nelem_valput;
       } 
       else {
          llbuf=(long long *)mpi_helpga_index[handle_orig].ptr;
          local = (int)llbuf[ielem-1];
          llbuf[ielem-1]+=(long long)nelem_valput;
       } 
     }
     else if (mproc == -1) {           /* put a value  to local helpga       */
       if (dtype==MPI_INT) {
          ibuf=(int *)mpi_helpga_index[handle_orig].ptr;
          local = (int)ibuf[ielem-1];
          ibuf[ielem-1]=(int)nelem_valput;
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)mpi_helpga_index[handle_orig].ptr;
          local = (int)lbuf[ielem-1];
          lbuf[ielem-1]=(long)nelem_valput;
       } 
       else {
          llbuf=(long long *)mpi_helpga_index[handle_orig].ptr;
          local = (int)llbuf[ielem-1];
          llbuf[ielem-1]=(long long)nelem_valput;
       } 
     }
     else  MPIGA_Error("NXTVAL_helpga_one: sequential version with silly mproc ", mproc);
    }
  } /* end else of big if-else */

  if (DEBUG_) printf("%4d: NXTVAL_helpga_one: end. handle=%d\n",ProcID(),*handle);

  return local;
}


/* many-element rma operations for helpga, MPI_Datatye can be MPI_INT --- MPI_DOUBLE */
int NXTVAL_helpga_extra(int mproc, int flag, int nelem, int ielem, int *handle, void *buff)
/*
  Operations for helpga:
____________________________________________________________________________________________________
|                |     COLFLAG       |        RMAFLAG               |          ETRFLAG             |
|  mproc = 0 ... |  create helpga    | get one element from helpga  |get elements from helpga      |
|  mproc > 0 ... |  zeroize helpga   | fetch-and-add to helpga      |accumulate helpga elements    | 
|  mproc < 0 ... |  destroy helpga   | put one value to helpga      |put values to helpga elements |
----------------------------------------------------------------------------------------------------
*/
{
  int  buf[5];
  int  countsent=LEN+3;
  int  nelem_get=0;
  MPI_Status status;
  int  type = TYPE_NXTVAL;
  int  type_extra = TYPE_EXTRA;
  int  local=0;
  int  handle_orig;
  int  i;
  MPI_Datatype dtype;  /* MPI Datatype for helpga element */
  int  server = ServerID();       /* id of server process */

  if (DEBUG_) printf("%4d: NXTVAL_helpga_extra: begin. handle=%d\n",ProcID(),*handle);

  if( flag!=ETRFLAG ) { /*check if extra rma operations */
    fprintf(stderr," NXTVAL_helpga_extra ERROR:  not extra rma operations.\n");
    return 1;
  }

  handle_orig=NXTVAL_helpga_handle_orig(*handle); 

  dtype=mpi_helpga_index[handle_orig].dtype;

  if (DEBUG_) {
    (void) printf("%4d: NXTVAL_helpga_extra: mproc=%d, handle=%d\n",ProcID(),mproc,*handle);
    (void) fflush(stdout);
  }

  if (SR_parallel) {
     buf[0] = mproc; /* COLFLAG: create(=0), zeroize(>0), destroy(<0); RMAFLAG: get(=0), fetch-and-add(>0), put(<0); ETRFLAG: get n elements(=0), put n elements(<0) */
     buf[1] = flag;  /* COLFLAG, RMAFLAG, or  ETRFLAG. Here should be ETRFLAG. */
     buf[2] = nelem; /* ETRFLAG: number of elements to be gotten/put */
     buf[3] = ielem; /* ETRFLAG: sequence number of element (1,2,...,n) */
     buf[4] = *handle;     /* sequence number of helpga */

#    ifdef NXTVAL_SERVER
       MPI_Send(buf, countsent, MPI_INT,  server, type, MPI_COMM_WORLD); 

       if (mproc == 0) {                 /* get a set of elements from helpga  */
         MPI_Recv(buff, nelem, dtype, server, type_extra, MPI_COMM_WORLD, &status); 
         MPI_Get_count(&status, dtype, &nelem_get);
         if ( nelem_get != nelem ) MPIGA_Error("NXTVAL_helpga_extra: nelem_get != nelem ", nelem_get);
       }
       else if (mproc < 0) {            /* put a set of elements to helpga */
         MPI_Send(buff, nelem, dtype, server, type_extra, MPI_COMM_WORLD); 
       }
       else { /* mproc>0, accumulate a set of elements to helpga */
         MPI_Send(buff, nelem, dtype, server, type_extra, MPI_COMM_WORLD); 
       }

       MPI_Recv(buf, 1,   MPI_INT,  server, type, MPI_COMM_WORLD, &status); 
       local=buf[0];
#    endif
  } 
  else {
     /* Not running in parallel ... just do a simulation */
    int *ibuf,*ibuf_helpga;
    long *lbuf,*lbuf_helpga;
    long long *llbuf,*llbuf_helpga;
    float  *fbuf,*fbuf_helpga;
    double *dbuf,*dbuf_helpga;
    if ( flag==ETRFLAG ) {    /* simulate HELPGA extra RMA operations     */
     if (mproc == 0) {                 /* get a set of values from local helpga      */
       if (dtype==MPI_INT) {
          ibuf=(int *)buff;
          ibuf_helpga=(int *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) ibuf[i]=ibuf_helpga[ielem-1+i];   
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)buff;
          lbuf_helpga=(long *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) lbuf[i]=lbuf_helpga[ielem-1+i];   
       } 
       else if (dtype==MPI_LONG_LONG) {
          llbuf=(long long *)buff;
          llbuf_helpga=(long long *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) llbuf[i]=llbuf_helpga[ielem-1+i];   
       } 
       else if (dtype==MPI_FLOAT) {
          fbuf=(float *)buff;
          fbuf_helpga=(float *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) fbuf[i]=fbuf_helpga[ielem-1+i];   
       } 
       else if (dtype==MPI_DOUBLE) {
          dbuf=(double *)buff;
          dbuf_helpga=(double *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) dbuf[i]=dbuf_helpga[ielem-1+i];   
       } 
       else {
          MPIGA_Error("NXTVAL_helpga_extra: wrong MPI_Datatype ",(int)dtype);
       } 
     }
     else if (mproc == -1) {           /* put a set of values  to local helpga       */
       if (dtype==MPI_INT) {
          ibuf=(int *)buff;
          ibuf_helpga=(int *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) ibuf_helpga[ielem-1+i]=ibuf[i];   
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)buff;
          lbuf_helpga=(long *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) lbuf_helpga[ielem-1+i]=lbuf[i];   
       } 
       else if (dtype==MPI_LONG_LONG) {
          llbuf=(long long *)buff;
          llbuf_helpga=(long long *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) llbuf_helpga[ielem-1+i]=llbuf[i];   
       } 
       else if (dtype==MPI_FLOAT) {
          fbuf=(float *)buff;
          fbuf_helpga=(float *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) fbuf_helpga[ielem-1+i]=fbuf[i];   
       } 
       else if (dtype==MPI_DOUBLE) {
          dbuf=(double *)buff;
          dbuf_helpga=(double *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) dbuf_helpga[ielem-1+i]=dbuf[i];   
       } 
       else {
          MPIGA_Error("NXTVAL_helpga_extra: wrong MPI_Datatype ",(int)dtype);
       } 
     }
     else if (mproc == 1) {  /* mproc>0, accumulate a set of elements to local helpga */
       if (dtype==MPI_INT) {
          ibuf=(int *)buff;
          ibuf_helpga=(int *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) ibuf_helpga[ielem-1+i]+=ibuf[i];   
       } 
       else if (dtype==MPI_LONG) {
          lbuf=(long *)buff;
          lbuf_helpga=(long *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) lbuf_helpga[ielem-1+i]+=lbuf[i];   
       } 
       else if (dtype==MPI_LONG_LONG) {
          llbuf=(long long *)buff;
          llbuf_helpga=(long long *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) llbuf_helpga[ielem-1+i]+=llbuf[i];   
       } 
       else if (dtype==MPI_FLOAT) {
          fbuf=(float *)buff;
          fbuf_helpga=(float *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) fbuf_helpga[ielem-1+i]+=fbuf[i];   
       } 
       else if (dtype==MPI_DOUBLE) {
          dbuf=(double *)buff;
          dbuf_helpga=(double *)mpi_helpga_index[handle_orig].ptr;
          for (i=0;i<nelem;i++) dbuf_helpga[ielem-1+i]+=dbuf[i];   
       } 
       else {
          MPIGA_Error("NXTVAL_helpga_extra: wrong MPI_Datatype ",(int)dtype);
       } 
     }
     else  MPIGA_Error("NXTVAL_helpga_extra: sequential version with silly mproc ", mproc);
     local = 0;
    }
  } /* end else of big if-else */

  if (DEBUG_) printf("%4d: NXTVAL_helpga_extra: end. handle=%d\n",ProcID(),*handle);

  return local;
}

/* many-element accumulation operation for helpga, MPI_Datatye can be MPI_INT --- MPI_DOUBLE */
/* determine if fac==1/1.0d0 */
int NXTVAL_helpga_extra_acc(int mproc, int flag, int nelem, int ielem, int *handle, void *buf, void *fac)
{
    MPI_Datatype dtype;  /* MPI Datatype for helpga element */
    int i,len;
    int isint,islong,isllong,isfloat,isdbl,isone;
    void *alphabuf;
    int *ialphabuf,*itempbuf,*ifac;
    long *lalphabuf,*ltempbuf,*lfac;
    long long *llalphabuf,*lltempbuf,*llfac;
    float   *falphabuf,*ftempbuf,*ffac;
    double  *dalphabuf,*dtempbuf,*dfac;

    if (DEBUG_) printf("%4d: Calling NXTVAL_helpga_extra_acc.\n",ProcID());
    dtype=NXTVAL_helpga_inquire_dtype(*handle);
    len=nelem;
    isint=0;
    islong=0;
    isllong=0;
    isfloat=0;
    isdbl=0;
    isone=1;
    if (dtype==MPI_INT) {
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
    else if (dtype==MPI_LONG) {
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
    else if (dtype==MPI_LONG_LONG) {
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
    else if (dtype==MPI_FLOAT) {
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
    else if (dtype==MPI_DOUBLE) {
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
       MPIGA_Error("NXTVAL_helpga_extra_acc: wrong MPI_Datatype ",(int)(dtype));
    }

    NXTVAL_helpga_extra(mproc, flag, nelem, ielem, handle, alphabuf);

    if(!isone) {
       if(isint)free(ialphabuf);
       else if(islong)free(lalphabuf);
       else if(isllong)free(llalphabuf);
       else if(isfloat)free(falphabuf);
       else free(dalphabuf);
    }

    return 0;
}


/* get the original handle of helpga, and check whether handle is out of range or allocated */
int NXTVAL_helpga_handle_orig( int handle ) 
{ 
    int handle_orig;

    if (DEBUG_) printf("%4d: NXTVAL_helpga_handle_orig: begin. handle=%d\n",ProcID(),handle);

    /* check whether helpga handle is out of range, and check whether it is allocated */
    handle_orig=handle-MPI_HELPGA_OFFSET; 
    if(handle_orig < 0 || handle_orig >= MAX_MPI_HELPGA_ARRAYS){
       fprintf(stderr," NXTVAL_helpga_handle_orig ERROR:  invalid handle [%d]. Should be [ %d -- %d ].\n",
                       handle,MPI_HELPGA_OFFSET,(MPI_HELPGA_OFFSET+MAX_MPI_HELPGA_ARRAYS));
       exit (1);
    }
    if(mpi_helpga_index[handle_orig].actv==0){
       fprintf(stderr," NXTVAL_helpga_handle_orig ERROR:  no memory allocated for this helpga [%d].\n",handle);
       exit (1);
    }
    if (DEBUG_) printf("%4d: NXTVAL_helpga_handle_orig: end. handle=%d\n",ProcID(),handle);
    return (handle_orig);
} 


/* get the MPI_Datatype of a helpga represented by handle */
MPI_Datatype NXTVAL_helpga_inquire_dtype( int handle ) 
{ 
    int handle_orig;
    MPI_Datatype mpidtype;

    if (DEBUG_) printf("%4d: NXTVAL_helpga_inquire_dtype: begin. handle=%d\n",ProcID(),handle);

    handle_orig=NXTVAL_helpga_handle_orig(handle); 

    mpidtype=mpi_helpga_index[handle_orig].dtype;

    if (DEBUG_) printf("%4d: NXTVAL_helpga_inquire_dtype: end. handle=%d\n",ProcID(),handle);

    return (mpidtype);
} 


/* get the name of a helpga represented by handle */
int NXTVAL_helpga_inquire_name( int handle, char **name ) 
{ 
    int handle_orig;

    if (DEBUG_) printf("%4d: NXTVAL_helpga_inquire_name: begin. handle=%d\n",ProcID(),handle);

    handle_orig=NXTVAL_helpga_handle_orig(handle); 

    *name=mpi_helpga_index[handle_orig].name;

    if (DEBUG_) printf("%4d: NXTVAL_helpga_inquire_name: end. handle=%d, name=%s\n",ProcID(),handle,*name);
    return 0;
} 


int NXTVAL_helpmutex(int mproc, int flag, int handle_orig)
/*
  Operations for nxtval_helpmutex:
______________________________________
|                |     MUTFLAG       | 
|  mproc > 0 ... |  lock mutex       |
|  mproc < 0 ... |  unlock mutex     |
--------------------------------------
*/
{
  int  buf[3];
  int countsent=LEN+1;
  MPI_Status status;
  int type = TYPE_NXTVAL;
  int local;
  int i;
  int  server = ServerID();       /* id of server process */


  if (DEBUG_) printf("%4d: Calling NXTVAL_helpmutex: mproc=%d\n",ProcID(),mproc);

  if( flag!=MUTFLAG ) { /*check if nxtval_helpmutex operations */
     fprintf(stderr," NXTVAL_helpmutex ERROR:  not nxtval_helpmutex operations.\n");
     exit (1);
  }
  /* check the handle of nxtval_helpmutex */
  if(handle_orig < 0 || handle_orig >= MAX_MPI_NXTVALMUTEX_ARRAYS){
     fprintf(stderr," NXTVAL_helpmutex ERROR:  invalid original mutex handle [%d].\n",handle_orig);
     exit (1);
  }
  if ( mproc>0 ) { /* lock a nxtval_helpmutex */
     if (nxtvalmutex_locklist[handle_orig]==1) {
        fprintf(stderr," NXTVAL_helpmutex ERROR: attempt to lock a mutex [handle_orig=%d] which has already been locked.\n",handle_orig);
        exit (1);
     }
     else if (nxtvalmutex_locklist[handle_orig]==0) {
        nxtvalmutex_locklist[handle_orig]=1;
     }
  }
  else if ( mproc<0 ) { /* unlock a nxtval_helpmutex */
     if (nxtvalmutex_locklist[handle_orig]==0) {
        fprintf(stderr," NXTVAL_helpmutex ERROR: attempt to unlock a mutex [handle_orig=%d] which has not been locked.\n",handle_orig);
        exit (1);
     }
     else if (nxtvalmutex_locklist[handle_orig]==1) {
        nxtvalmutex_locklist[handle_orig]=0;
     }
  }
  else { /* mproc=0 */
     MPIGA_Error("NXTVAL_helpmutex: wrong mproc ", mproc);
  }
    

  if (SR_parallel) {
     buf[0] = mproc;           /* MUTFLAG: lock(>0), unlock(<0), error(=0); */
     buf[1] = flag;            /* Here it should be MUTFLAG. */
     buf[2] = handle_orig;     /* original sequence number of nxtval_helpmutex */

#    ifdef NXTVAL_SERVER
       MPI_Send(buf, countsent, MPI_INT,  server, type, MPI_COMM_WORLD); 
       MPI_Recv(buf, 1,   MPI_INT,  server, type, MPI_COMM_WORLD, &status); 
       return buf[0];
#    endif
  } 
  else {
     /* Not running in parallel ... just do a simulation */
     if ( abs(mproc) != 1 )  MPIGA_Error("NXTVAL_helpmutex: sequential version with silly mproc ", mproc);
     local = 0;
  } /* end else of big if-else */

  return local;
}

 /* lock a nxtval_helpmutex object identified by the original mutex number. */
int NXTVAL_lock_helpmutex_orig(int handle_orig)
{
    int mproc=NProcs_Work();
    int flag=MUTFLAG;
    int mpierr=0;

    mpierr=NXTVAL_helpmutex(mproc, flag, handle_orig);
    return mpierr;
}

 /* unlock a nxtval_helpmutex object identified by the original mutex number. */
int NXTVAL_unlock_helpmutex_orig(int handle_orig)
{
    int mproc=-NProcs_Work();
    int flag=MUTFLAG;
    int mpierr;

    mpierr=NXTVAL_helpmutex(mproc, flag, handle_orig);
    return mpierr;
}

/* lock a mutex object identified by the wrapped mutex number. It is a fatal error for a process
   to attempt to lock a mutex which has already been locked by this process */
int NXTVAL_lock_helpmutex(int inum)
{
    int mproc=NProcs_Work();
    int flag=MUTFLAG;
    int mpierr;
    int inum_orig;

    if (DEBUG_) printf("%4d: In NXTVAL_lock_helpmutex begin: mutex num=%d, total num=%d\n",ProcID(),inum,nxtval_helpmutex_num);

    if (inum <0 || inum >= nxtval_helpmutex_num ) {
       fprintf(stderr," NXTVAL_lock_helpmutex ERROR: over range! mutex num=%d, total num=%d\n",inum,nxtval_helpmutex_num);
       return 1;
    }
    else {
       inum_orig=inum+MAX_MPI_NXTVALMUTEX_GA;
       mpierr=NXTVAL_helpmutex(mproc, flag, inum_orig);
    }

    if (DEBUG_) printf("%4d: In NXTVAL_lock_helpmutex end: : mutex %d has been locked.\n",ProcID(),inum);

    return 0;
}


/* unlock a mutex object identified by the wrapped mutex number. It is a fatal error for a process
 * to attempt to unlock a mutex which has not been locked by this process. */
int NXTVAL_unlock_helpmutex(int inum)
{
    int mproc=-NProcs_Work();
    int flag=MUTFLAG;
    int mpierr;
    int inum_orig;

    if (DEBUG_) printf("%4d: In NXTVAL_lock_helpmutex begin: mutex num=%d, total num=%d\n",ProcID(),inum,nxtval_helpmutex_num);

    if (inum <0 || inum >= nxtval_helpmutex_num ) {
       fprintf(stderr," NXTVAL_lock_helpmutex ERROR: over range! mutex num=%d, total num=%d\n",inum,nxtval_helpmutex_num);
       return 1;
    }
    else {
       inum_orig=inum+MAX_MPI_NXTVALMUTEX_GA;
       mpierr=NXTVAL_helpmutex(mproc, flag, inum_orig);
    }

    if (DEBUG_) printf("%4d: In NXTVAL_lock_helpmutex end: mutex %d has been unlocked.\n",ProcID(),inum);

    return 0;
}

/* allocate one mutex object identified by the original mutex number. Check whether it has overranged or locked */
int NXTVAL_alloc_helpmutex_orig(int inum_orig)
{
    int i,mpierr;

    if (DEBUG_) printf("%4d: In NXTVAL_alloc_helpmutex_orig begin: original mutex num=%d\n",ProcID(),inum_orig);

    if (inum_orig <0 || inum_orig >= MAX_MPI_NXTVALMUTEX_ARRAYS ) {
       fprintf(stderr,"ERROR in NXTVAL_alloc_helpmutex_orig: over range! original mutex num=%d, max num=%d\n",inum_orig,MAX_MPI_NXTVALMUTEX_ARRAYS);
       exit (1);
    }
    if ( nxtvalmutex_locklist[inum_orig]==1 ) {
       fprintf(stderr,"ERROR in NXTVAL_alloc_helpmutex_orig: original mutex %d has been locked before allocating.\n",inum_orig);
       exit (1);
    }
    
    if (DEBUG_) printf("%4d: In NXTVAL_alloc_helpmutex_orig end: original mutex=%d\n",ProcID(),inum_orig);

    return 0;
}

/* free one mutex object identified by the original mutex number. Check whether it has overranged, or locked */
int NXTVAL_free_helpmutex_orig(int inum_orig)
{
    int i,mpierr;

    if (DEBUG_) printf("%4d: In NXTVAL_free_helpmutex_orig begin: original mutex num=%d\n",ProcID(),inum_orig);

    if (inum_orig <0 || inum_orig >= MAX_MPI_NXTVALMUTEX_ARRAYS ) {
       fprintf(stderr,"ERROR in NXTVAL_free_helpmutex_orig: over range! original mutex num=%d, max num=%d\n",inum_orig,MAX_MPI_NXTVALMUTEX_ARRAYS);
       exit (1);
    }
    if ( nxtvalmutex_locklist[inum_orig]==1 ) {
       fprintf(stderr,"WARNING in NXTVAL_free_helpmutex_orig: original mutex %d is still locked before freed. Now unlocking and freeing it...\n",inum_orig);
       mpierr=NXTVAL_unlock_helpmutex_orig(inum_orig);
    }
    
    if (DEBUG_) printf("%4d: In NXTVAL_free_helpmutex_orig end: original mutex=%d\n",ProcID(),inum_orig);

    return 0;
}

/* allocates a set containing the number of mutexes. Only one set of mutexes can exist at a time. Mutexes can be
allocated and freed as many times as needed. Mutexes are numbered: 0, ..., number-1. */
int NXTVAL_alloc_helpmutexes(int number)
{
    int i,mpierr;
    int handle_orig;

    if (DEBUG_) printf("%4d: In NXTVAL_alloc_helpmutexes: begin.\n",ProcID());
    if ( number <=0 || number > MAX_MPI_NXTVALMUTEX_NONGA ) {
       fprintf(stderr,"ERROR in NXTVAL_alloc_helpmutexes: over range! number=%d, max num=%d\n",number,MAX_MPI_NXTVALMUTEX_NONGA);
       return 1;
    }
    for (i=0;i<number;i++) {
       handle_orig=i+MAX_MPI_NXTVALMUTEX_GA;
       mpierr=NXTVAL_alloc_helpmutex_orig(handle_orig);
    }
    nxtval_helpmutex_num=number;
    
    if (DEBUG_) printf("%4d: In NXTVAL_alloc_helpmutexes end:  %d mutexes have been allocated.\n",ProcID(),number);

    return 0;
}

/* frees the set of mutexes allocated with NXTVAL_alloc_helpmutexes.*/
int NXTVAL_free_helpmutexes()
{
    int i,mpierr;
    int handle_orig;
    
    if (DEBUG_) printf("%4d: In NXTVAL_free_helpmutexes begin: nxtval_helpmutex_num=%d\n",ProcID(),nxtval_helpmutex_num);

    if (nxtval_helpmutex_num<=0) {
       fprintf(stderr,"ERROR in NXTVAL_free_helpmutexes: no mutex is needed to be freed. N mutex=%d\n",nxtval_helpmutex_num);
       return 1;
    }
    for(i=0;i<nxtval_helpmutex_num; i++) {
       handle_orig=i+MAX_MPI_NXTVALMUTEX_GA;
       mpierr=NXTVAL_free_helpmutex_orig(handle_orig);
    }

    nxtval_helpmutex_num=0;
    
    if (DEBUG_) printf("%4d: In NXTVAL_free_helpmutexes end: now nxtval_helpmutex_num=%d\n",ProcID(),nxtval_helpmutex_num);

    return 0;
}


/* initialization for nxtval_helpmutex -- called in mpiga_initialize */
void initialize_nxtval_helpmutex()
{
    int i;
    /* allocate memory and initialize it with zero. */
    nxtvalmutex_locklist=(int *)calloc(MAX_MPI_NXTVALMUTEX_ARRAYS,sizeof(int));
    if(!nxtvalmutex_locklist){
      fprintf(stderr,"In mpi_nxtval: initialize_nxtval_helpmutex ERROR: failed to allocate memory for nxtval_helpmutex locklist.\n");
      exit (1);
    }
    nxtval_helpmutex_num=0;
    if (DEBUG_) printf("%4d: In mpi_nxtval: initialize_nxtval_helpmutex end. maximum mutex=%d\n",ProcID(),MAX_MPI_NXTVALMUTEX_ARRAYS);
}

/* free the memory for global nxtval_helpmutex lock list */
void finalize_nxtval_helpmutex()
{
    if ( nxtvalmutex_locklist != NULL ) free (nxtvalmutex_locklist); 
    if (DEBUG_) printf("%4d: In mpi_nxtval: finalize_nxtval_helpmutex end.\n",ProcID());
}

/* initialise mpi_helpga data structure, zeroize for pointers in MPI_HELPGA array */
void initialize_helpga()
{
    int i;
    mpi_helpga_data_struc=(mpi_helpga_array_t *)malloc(sizeof(mpi_helpga_array_t)*MAX_MPI_HELPGA_ARRAYS);
    if(!mpi_helpga_data_struc){
      fprintf(stderr,"In mpi_nxtval: initialize_helpga ERROR: failed to allocate memory for mpi_helpga_data_struc.\n");
      exit (1);
    }
    mpi_helpga_index = mpi_helpga_data_struc;
    for(i=0;i<MAX_MPI_HELPGA_ARRAYS; i++) {
       mpi_helpga_index[i].nele = 0;
       mpi_helpga_index[i].actv = 0;
       mpi_helpga_index[i].ptr  = NULL;
       mpi_helpga_index[i].name = NULL;
       mpi_helpga_index[i].dtype= MPI_CHAR;
    }
    mpi_helpga_num=0;
    if (DEBUG_) printf("%4d: In mpi_nxtval: initialize_helpga end. Maximum helpga=%d\n",ProcID(),MAX_MPI_HELPGA_ARRAYS);
}

/* free the memory for global mpi_helpga data structure */
void finalize_helpga()
{
    int i;
    if (DEBUG_) printf("%4d: In mpi_nxtval: finalize_helpga begin. mpi_helpga_num=%d\n",ProcID(),mpi_helpga_num);
    if (mpi_helpga_num>0) {
       for(i=0;i<MAX_MPI_HELPGA_ARRAYS; i++) {
         if ( mpi_helpga_index[i].actv == 1 ) {
            if ( mpi_helpga_index[i].ptr != NULL ) free (mpi_helpga_index[i].ptr);
            if ( mpi_helpga_index[i].name!= NULL ) free (mpi_helpga_index[i].name);
         }
       }
    }
    if(mpi_helpga_data_struc) free(mpi_helpga_data_struc);
    if (DEBUG_) printf("%4d: In mpi_nxtval: finalize_helpga end.\n",ProcID());
}

/* initialization for nxtval -- called in mpiga_initialize */
void install_nxtval()
{
    int numprocs, myid;

#  ifdef NXTVAL_SERVER
    initialize_helpga(); /* initialise mpi_helpga data structure  */
    initialize_nxtval_helpmutex(); /* initialise nxtval_helpmutex */
#  endif

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

#  ifdef NXTVAL_SERVER
       /* in this mode one process is hidden from the application */
       if(SR_parallel && myid == numprocs -1) {
         if (DEBUG_) {
           printf("%4d: In mpi_nxtval: install_nxtval info: excluding one process for nxtval server.\n",ProcID());
           fflush(stdout);
         }
         NextValueServer();
         if (DEBUG_) printf("In mpi_nxtval: NextValueServer has been installed/terminated.\n");
       }
#  else
       if(myid == 0) {
         fprintf(stderr,"WARNING: In mpi_nxtval [install_nxtval]: Do not know how to implement nxtval!\n");
         fprintf(stderr,"WARNING: All functions relying on helper process will be unavailable!\n");
       }
/*       exit (1); */
#  endif
}

void finalize_nxtval()
{ 
#  ifdef NXTVAL_SERVER
    finalize_helpga(); /* free global mpi_helpga data structure */
    finalize_nxtval_helpmutex(); /* free global nxtval_helpmutex lock list */
#  endif
}


#else

void mpi_nxtval_dummy () {}

#endif /* end of MPI2 */

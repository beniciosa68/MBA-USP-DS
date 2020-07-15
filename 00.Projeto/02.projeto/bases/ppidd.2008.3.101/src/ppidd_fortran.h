/* src/mpp/ppidd_fortran.h $Revision: 2008.3 $ */

/* ------------------------------------------------------------- *\
   External MPI Functions called by FORTRAN
   ========================================

   All MPI subroutines are written in C.  MPI subroutines called
   from FORTRAN go through a C wrapper.  All MPI subroutines are
   prefixed with PPIDD_ and all FORTRAN wrapper subroutines are
   prefixed with FORT_.
   
   The following C definitions substitute the FORTRAN wrapper
   subroutine name with the correct machine dependent FORTRAN
   external name.
   
   Note: FORTRAN externals are generally all lowercase, but may
   be uppercase.  Define FORT_UPPERCASE if your machine's FORTRAN
   using uppercase externals.
   
\* ------------------------------------------------------------- */

#ifndef __PPIDD_FORTRAN_H__
#define __PPIDD_FORTRAN_H__

 # include "machines.h"

 #     define PPIDD_Initialize              FORT_Extern(ppidd_initialize,PPIDD_INITIALIZE)
 #     define PPIDD_Finalize                FORT_Extern(ppidd_finalize,PPIDD_FINALIZE)
 #     define PPIDD_Uses_ma                 FORT_Extern(ppidd_uses_ma,PPIDD_USES_MA)
 #     define PPIDD_MA_init                 FORT_Extern(ppidd_ma_init,PPIDD_MA_INIT)
 #     define PPIDD_Wtime                   FORT_Extern(ppidd_wtime,PPIDD_WTIME)
 #     define PPIDD_Error                   FORT_Extern(ppidd_error,PPIDD_ERROR)
 #     define PPIDD_Nxtval                  FORT_Extern(ppidd_nxtval,PPIDD_NXTVAL)
 #     define PPIDD_Size_all                FORT_Extern(ppidd_size_all,PPIDD_SIZE_ALL)
 #     define PPIDD_Size                    FORT_Extern(ppidd_size,PPIDD_SIZE)
 #     define PPIDD_Rank                    FORT_Extern(ppidd_rank,PPIDD_RANK)
 #     define PPIDD_Init_fence              FORT_Extern(ppidd_init_fence,PPIDD_INIT_FENCE)
 #     define PPIDD_Fence                   FORT_Extern(ppidd_fence,PPIDD_FENCE)
 #     define PPIDD_Send                    FORT_Extern(ppidd_send,PPIDD_SEND)            
 #     define PPIDD_Recv                    FORT_Extern(ppidd_recv,PPIDD_RECV)
 #     define PPIDD_Wait                    FORT_Extern(ppidd_wait,PPIDD_WAIT)
 #     define PPIDD_Iprobe                  FORT_Extern(ppidd_iprobe,PPIDD_IPROBE)
 #     define PPIDD_BCast                   FORT_Extern(ppidd_bcast,PPIDD_BCAST)
 #     define PPIDD_Barrier                 FORT_Extern(ppidd_barrier,PPIDD_BARRIER)
 #     define PPIDD_Gsum                    FORT_Extern(ppidd_gsum,PPIDD_GSUM)
 #     define PPIDD_Create_irreg            FORT_Extern(ppidd_create_irreg,PPIDD_CREATE_IRREG)
 #     define PPIDD_Create                  FORT_Extern(ppidd_create,PPIDD_CREATE) 
 #     define PPIDD_Destroy                 FORT_Extern(ppidd_destroy,PPIDD_DESTROY)
 #     define PPIDD_Distrib                 FORT_Extern(ppidd_distrib,PPIDD_DISTRIB)
 #     define PPIDD_Location                FORT_Extern(ppidd_location,PPIDD_LOCATION)
 #     define PPIDD_Get                     FORT_Extern(ppidd_get,PPIDD_GET)
 #     define PPIDD_Put                     FORT_Extern(ppidd_put,PPIDD_PUT)
 #     define PPIDD_Acc                     FORT_Extern(ppidd_acc,PPIDD_ACC)
 #     define PPIDD_Read_inc                FORT_Extern(ppidd_read_inc,PPIDD_READ_INC)
 #     define PPIDD_Zero_patch              FORT_Extern(ppidd_zero_patch,PPIDD_ZERO_PATCH)
 #     define PPIDD_Zero                    FORT_Extern(ppidd_zero,PPIDD_ZERO)
 #     define PPIDD_Duplicate               FORT_Extern(ppidd_duplicate,PPIDD_DUPLICATE)
 #     define PPIDD_Inquire_name            FORT_Extern(ppidd_inquire_name,PPIDD_INQUIRE_NAME)
 #     define PPIDD_Inquire_stype           FORT_Extern(ppidd_inquire_stype,PPIDD_INQUIRE_STYPE)
 #     define PPIDD_Inquire_mem             FORT_Extern(ppidd_inquire_mem,PPIDD_INQUIRE_MEM)
 #     define PPIDD_Create_mutexes          FORT_Extern(ppidd_create_mutexes,PPIDD_CREATE_MUTEXES)
 #     define PPIDD_Lock_mutex              FORT_Extern(ppidd_lock_mutex,PPIDD_LOCK_MUTEX)
 #     define PPIDD_Unlock_mutex            FORT_Extern(ppidd_unlock_mutex,PPIDD_UNLOCK_MUTEX)
 #     define PPIDD_Destroy_mutexes         FORT_Extern(ppidd_destroy_mutexes,PPIDD_DESTROY_MUTEXES)
#endif

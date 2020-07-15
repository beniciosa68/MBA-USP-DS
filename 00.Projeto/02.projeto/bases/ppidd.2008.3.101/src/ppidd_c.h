/* src/mpp/ppidd_c.h $Revision: 2008.3 $ */
#ifndef __PPIDD_C_H__
#define __PPIDD_C_H__
   extern void PPIDD_Initialize(int argc, char **argv);
   extern void PPIDD_Finalize(void);
   extern void PPIDD_Uses_ma(int *ok);
   extern void PPIDD_MA_init(int *dtype, int *stack, int *heap, int *ok);
   extern void PPIDD_Wtime(double *ctime);
   extern void PPIDD_Error(char *message,int *code);
   extern void PPIDD_Nxtval(int *numproc, int *val);
   extern void PPIDD_Size_all(int *np);
   extern void PPIDD_Size(int *np);
   extern void PPIDD_Rank(int *me);
   extern void PPIDD_Init_fence(void);
   extern void PPIDD_Fence(void);
   extern void PPIDD_Send(void *buf,int *count,int *dtype,int *dest,int *sync);
   extern void PPIDD_Recv(void *buf,int *count,int *dtype,int *source,int *lenreal,int *sourcereal,int *sync);
   extern void PPIDD_Wait(int *nodesel);
   extern void PPIDD_Iprobe(int *tag,int *source,int *ok);
   extern void PPIDD_BCast(void *buffer,int *count,int *type,int *root);
   extern void PPIDD_Barrier(void);
   extern void PPIDD_Gsum(int *type,void *buffer,int *len, char *op);
   extern void PPIDD_Create_irreg(char *name,int *lenin,int *nchunk,int *datatype,int *handle,int *ok);
   extern void PPIDD_Create(char *name,int *lentot, int *datatype, int *storetype, int *handle, int *ok);
   extern void PPIDD_Destroy(int *handle,int *ok);
   extern void PPIDD_Distrib(int *handle,int *rank,int *ilo,int *ihi,int *ok);
   extern void PPIDD_Location(int *handle,int *ilo,int *ihi,int *map,int *proclist,int *np,int *ok);
   extern void PPIDD_Get(int *handle,int *ilo,int *ihi,void *buff,int *ok);
   extern void PPIDD_Put(int *handle,int *ilo,int *ihi,void *buff,int *ok);
   extern void PPIDD_Acc(int *handle,int *ilo,int *ihi,void *buff,void *fac,int *ok);
   extern void PPIDD_Read_inc(int *ihandle,int *inum,int *incr,int *returnval);
   extern void PPIDD_Zero_patch(int *ihandle,int *ilo,int *ihi);
   extern void PPIDD_Zero(int *handle,int *ok);
   extern void PPIDD_Duplicate(int *handlei, int *handlej, char *name);
   extern void PPIDD_Inquire_name(int *handle, char *name);
   extern void PPIDD_Inquire_stype(int *handle, int *storetype);
   extern void PPIDD_Inquire_mem(int *mem_used);
   extern void PPIDD_Create_mutexes(int *storetype,int *number,int *ok);
   extern void PPIDD_Lock_mutex(int *inum);
   extern void PPIDD_Unlock_mutex(int *inum);
   extern void PPIDD_Destroy_mutexes(int *ok);
#endif

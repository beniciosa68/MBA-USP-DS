/* src/mpp/ppidd_ctest.c $Revision: 2008.3 $ */
/*! PPIDD standalone c test suites */
#include "machines.h"
#include "ppidd_c.h"

#ifdef MOLPRO
    int ppidd_ctest(void)
#else
    int main(int argc, char **argv)
#endif
{
    int me, nproc;

#ifndef MOLPRO
     /* initialize PPIDD*/
    PPIDD_Initialize(argc, argv);
#endif

    PPIDD_Size(&nproc);
    PPIDD_Rank(&me);

    if(me==0) {
       printf(" PPIDD initialized\n");
       printf(" Nprocs= %d    My proc= %d\n",nproc,me);
       printf(" Performing PPIDD C tests:\n");
       fflush(stdout);
    }

    PPIDD_Barrier();

    if(me==0) {
       printf(" All PPIDD C tests successful.\n");
       fflush(stdout);
    }

#ifndef MOLPRO
/* ***  Terminate and Tidy up PPIDD */
    PPIDD_Finalize();
#endif
    return 0;
}


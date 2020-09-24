/* src/mpp/sizeofctype.c $Revision: 2008.3 Patch(2008.3): ppidd_typecheck $ */
#include "machines.h"
#include "ppidd_dtype.h"

#define	sizeofctype        FORT_Extern(sizeofctype,SIZEOFCTYPE)
/* change argument type fortint to double, in order to obtain right numbers even if Integer type doesn't match between Fortran and C */
/* void sizeofctype(fortint *sizeint, fortint *sizelog, fortint *sizedouble, fortint *sizefloat) */
void sizeofctype(double *sizeint, double *sizelog, double *sizedouble, double *sizefloat)
{
    int verbose=0; 
    if (verbose) {
       printf(" The size of some C types on this machine are listed as follows:\n");
       printf("                             int:%5d bytes\n", (int)sizeof(int));
       printf("                            long:%5d bytes\n", (int)sizeof(long));
       printf("                       long long:%5d bytes\n", (int)sizeof(long long));
       printf("                           float:%5d bytes\n", (int)sizeof(float));
       printf("                          double:%5d bytes\n", (int)sizeof(double));
       printf("                         fortint:%5d bytes\n", (int)sizeof(fortint));
       printf("                     fortlogical:%5d bytes\n", (int)sizeof(fortlogical));
    }
    *sizeint=(double)sizeof(fortint);
    *sizelog=(double)sizeof(fortlogical);
    *sizedouble=(double)sizeof(double);
    *sizefloat=(double)sizeof(float);
}

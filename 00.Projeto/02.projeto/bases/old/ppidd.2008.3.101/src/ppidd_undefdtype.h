/* src/mpp/ppidd_undefdtype.h $Revision: 2008.3 Patch(2008.3): ppidd_sf_c $ */

/*! \file
  Overwrite data types (fortint and forlogical) for PPIDD C interface.
  prerequisite: include machines.h
*/

#ifndef __PPIDD_UNDEFDTYPE_H__
#define __PPIDD_UNDEFDTYPE_H__

#ifdef fortint
#undef fortint
#endif
#define fortint int

#ifdef fortlogical
#undef fortlogical
#endif
#define fortlogical int

#ifdef FALSE
#undef FALSE
#endif
#define FALSE (fortlogical) 0

#ifdef TRUE
#undef TRUE
#endif
#define TRUE (fortlogical) 1

#ifdef FORTCL_NEXT
#undef FORTCL_NEXT
#endif

#ifdef FORTCL_END
#undef FORTCL_END
#endif

#ifdef FORTINTC_DIVIDE
#undef FORTINTC_DIVIDE
#endif

#endif

/* timing.c $Revision: 2008.3 $ */
#include "machines.h"

#ifdef __alpha_linux_fort
#define	INC_CLOCK     inc_clock__
#else
#define	INC_CLOCK     FORT_Extern(inc_clock,INC_CLOCK)
#endif

#define	SECOND        FORT_Extern(second,SECOND)
#define	TIMING_MOLPRO FORT_Extern(timing_molpro,TIMING_MOLPRO)
#define	TIMDAT        FORT_Extern(timdat,TIMDAT)
#define WALLCL        FORT_Extern(wallcl,WALLCL)

#ifdef _WIN32
#include <WinSock2.h>
int gettimeofday (struct timeval *tp, void *tzp) {
return 0;
}
#endif

#ifdef __cplusplus
extern "C" {void TIMING_MOLPRO(double&, double&, double&);}
#endif

static double nwords=(double)0, nio=(double)0;

/*###################  timing etc ###############*/

/*
      this routine will return the user-time in second
*/
#ifdef _WIN32
static clock_t itt;
#else
static clock_t it; struct tms itt;
#endif

void log_io(size_t words)
{
  nwords += (double) words;
  nio++;
}

double SECOND()
{
#if defined(CATAMOUNT) || defined(_WIN32)
return (double)0;
#else
  it=times(&itt);
  return (double) itt.tms_utime / (double) TIMER_TICK;
#endif
}

void TIMING_MOLPRO(double *cpu, double *sys, double *wall)
{
#if defined(CATAMOUNT) || defined(_WIN32)
*wall=*cpu=*sys= (double)0;
#else
  it=times(&itt);
  *cpu=(double) itt.tms_utime / (double) TIMER_TICK; *sys=(double) itt.tms_stime / (double) TIMER_TICK;
  *wall=(double) it / (double) TIMER_TICK;
#endif
}

/* subroutine timdat(tim,dat,mach)
   character*(*) tim,dat,mach */
#ifdef HAS_UTSNAME
#include <sys/utsname.h>
#endif
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	255
#endif
struct timeval tpp;
char *tval, *ctime();
void TIMDAT(char *tim
#ifdef FORTCL_NEXT
	    ,fortintc ltim
#endif
	    ,char *dat
#ifdef FORTCL_NEXT
	    ,fortintc ldat
#endif
	    ,char * mach
#ifdef FORTCL_NEXT
	    ,forintc lmach
#endif
#ifdef FORTCL_END
	    ,fortintc ltim,fortintc ldat,fortintc lmach
#endif
	    )
{	
  char *os, *osver, *host, *hardw, buffer[256];
#if defined(sgi) || defined(linux)
  time_t timdum;
#endif
#ifdef _AIX
#define	nibmtab	48
  char ibmtabm[nibmtab][3]={
    "31"    ,"35"    ,"30"    ,"10"    ,"18"    ,"14"    ,"1C"    ,
    "5C"    ,"20"    ,"2E"    ,"11"    ,"37"    ,"38"    ,"41"    ,
    "63"    ,"64"    ,"66"    ,"34"    ,"43"    ,"67"    ,"75"    ,
    "76"    ,"77"    ,"78"    ,"80"    ,"71"    ,"70"    ,"47"    ,
    "49"    ,"46"    ,"58"    ,"82"    ,"57"    ,"48"    ,"43"    ,
    "42"    ,"F0"    ,"A6"    ,"A7"    ,"72"    ,"A0"    ,"A1"    ,
    "81"    ,"A3"    ,"A4"    ,"C0"    ,"C4"    ,"4C"};
  char ibmtabv[nibmtab][7]={
    "320"   ,"320H"  ,"520"   ,"530"   ,"530H"  ,"540"   ,"550"   ,
    "560"   ,"930"   ,"950"   ,"540"   ,"340"   ,"350"   ,"220"   ,
    "970"   ,"980"   ,"580"   ,"520H"  ,"M20"   ,"570"   ,"370"   ,
    "360"   ,"350"   ,"315"   ,"990"   ,"580H"  ,"590"   ,"230"   ,
    "250-80","250-66","380"   ,"R24"   ,"390"   ,"C10"   ,"M20"   ,
    "410"   ,"N40"   ,"G30"   ,"G40"   ,"590H"  ,"J30"   ,"J40"    ,
    "R24"   ,"R30"   ,"R40"   ,"E20"   ,"F30"   ,"43P"};
  int i;
#endif
#ifdef HAS_UTSNAME
  struct utsname n;
#else
#endif
#ifdef FORTINTC_DIVIDE
  ltim=ltim/FORTINTC_DIVIDE;ldat=ldat/FORTINTC_DIVIDE;lmach=lmach/FORTINTC_DIVIDE;
#endif
#ifdef SX
  gettimeofday(&tpp);
#else
  gettimeofday(&tpp,0);
#endif
#if defined(sgi) || defined(linux)
  timdum = tpp.tv_sec;
  tval=ctime(&timdum);
#else
  tval=ctime(&tpp.tv_sec);
#endif
  sprintf(tim,"%8.8s",tval+11);
  sprintf(dat,"%3.3s-%3.3s-%2.2s",tval+7,tval+4,tval+22);
#if defined(FORTCL_END) || defined(FORTCL_NEXT)
  for (os=tim+strlen(tim);os<tim+ltim;os++) *os=' ';
  for (os=dat+strlen(dat);os<dat+ldat;os++) *os=' ';
#endif
#ifdef HAS_UTSNAME
  uname(&n);
  os=n.sysname; osver=n.release; host=n.nodename; hardw=n.machine;
#ifdef _CRAYMPP
  hardw="Cray MPP";
#endif
#ifdef _AIX
  osver=(char *) malloc(strlen(n.release)+strlen(n.version)+1);
  strcpy(osver,n.version);strcat(osver,".");strcat(osver,n.release);
  for(i=0;i<nibmtab;i++) if (!strncmp(ibmtabm[i],n.machine+8,2)) hardw=ibmtabv[i];
#endif
#ifdef convex
  osver=n.version;hardw=n.release;
#endif
#else
#ifdef _WIN32
  os="Windows"; osver="0";
#else
  os="Unix"; osver="0";
  gethostname((host=(char *) malloc(MAXHOSTNAMELEN+1)),MAXHOSTNAMELEN);
#endif
#endif
  strcpy(buffer,os); strcat(buffer,"-"); strcat(buffer,osver);
  strcat(buffer,"/");strcat(buffer,host);
  if (hardw) {strcat(buffer,"(");strcat(buffer,hardw);strcat(buffer,")");}
#if defined(FORTCL_END) || defined(FORTCL_NEXT)
  strncpy(mach,buffer,lmach);
  for (os=mach+strlen(mach);os<mach+lmach;os++) *os=' ';
#else
  strcpy(mach,buffer);
#endif
}


double WALLCL()
{
#if defined(sunx) || defined(CATAMOUNT) || defined(_WIN32) /* actually this should probably work everywhere */
  return (double) time(NULL);
#else
  it=times(&itt);
  return (double) it / (double) TIMER_TICK;
#endif
}

fortint INC_CLOCK()
{
  fortint cpu;
#ifndef _WIN32
  cpu=sysconf((fortint) _SC_CLK_TCK);
#endif
/*
  cpu=CLOCKS_PER_SEC;
*/
  return(cpu);
}

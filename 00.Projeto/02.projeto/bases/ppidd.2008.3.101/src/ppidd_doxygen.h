/* src/ppidd/src/ppidd_doxygen.h $Revision: 2008.3 Patch(2008.3): ppidd_doc $ */

/*! \mainpage PPIDD Reference Manual
 *
 * Parallel Programming Interface for Distributed Data (PPIDD) Reference
 * Manual. The Fortran interface subroutine descriptions can be found
 * in ppidd_fortran.c.
  
//   Directory and file structure in PPIDD:
  <pre>

       .                                  PPIDD root directory
       |-- GNUmakefile                    Makefile to build PPIDD
       |-- README                         README for PPIDD
       |-- VERSION                        VERSION number for PPIDD
       |-- ./doc                          Document directory
       |   `-- Doxyfile                       Document configuration file
       |-- ./lib                          Final location of PPIDD library
       |-- ./src                          Source code directory for PPIDD library  
       |   |-- GNUmakefile                    Makefile for src directory
       |   |-- machines.h                     Head file for machine-related settings
       |   |-- mpi_helpmutex.c                Mutex source file using helper process
       |   |-- mpi_nxtval.c                   NXTVAL source file 
       |   |-- mpi_nxtval.h                   NXTVAL header file
       |   |-- mpi_utils.c                    MPI utility source file 
       |   |-- mpi_utils.h                    MPI utility header file
       |   |-- mpiga_base.c                   Source code for distributed data structure 
       |   |-- mpiga_base.h                   Head file for distributed data structure
       |   |-- mpimutex-hybrid.c              Mutex source file using distributed processes
       |   |-- mpimutex.h                     Mutex header file using distributed processes
       |   |-- ppidd_c.c                      C interface source code 
       |   |-- ppidd_c.h                      C interface header file
       |   |-- ppidd_doxygen.h                PPIDD document main page file
       |   |-- ppidd_dtype.h                  PPIDD data type header file
       |   |-- ppidd_eaf_c.c                  C interface source code for EAF
       |   |-- ppidd_eaf_c.h                  C interface header file for EAF
       |   |-- ppidd_eaf_fortran.c            Fortran interface source code for EAF
       |   |-- ppidd_eaf_fortran.h            Fortran interface header file for EAF
       |   |-- ppidd_fortran.c                Fortran interface source code for PPIDD
       |   |-- ppidd_fortran.h                Fortran interface header file for PPIDD
       |   |-- ppidd_sf_c.c                   C interface source code for SF
       |   |-- ppidd_sf_c.h                   C interface header file for SF
       |   |-- ppidd_sf_fortran.c             Fortran interface source code for SF
       |   |-- ppidd_sf_fortran.h             Fortran interface header file for SF
       |   `-- ppidd_undefdtype.h             Fortran data type header file for C interface
       `-- ./test                         Test code directory for PPIDD library
           |-- GNUmakefile                    Makefile for test directory
           |-- ppidd_ctest.c                  C test program
           |-- ppidd_test.F                   Fortran test program
           |-- sizeofctypes.c                 Code for determining the size of C data types
           |-- sizeoffortypes.F               Code for determining the size of Fortran data types
           |-- ppidd_test.F                   Fortran test program
           `-- timing_molpro.c                Utility tool for timing                         
 
  </pre>

//   Some examples of building the PPIDD library:
 *    The following examples are tested on a x86_64//Linux machine on which Intel Fortran and C compilers,
 *    MPI2-aware Intel Fortran and C compilers, and Intel MPI library are available. 
 *    Please be aware the options might be different on other machines.
  <pre>

     1. Build MPI-2 version of PPIDD:
     make MPICC=mpiicc MPIFC=mpiifort INT64=y FFLAGS='-i8'

     or
     make CC=icc FC=ifort INT64=y FFLAGS='-i8' INCLUDE=/software/intel/mpi/3.1/include64 \
     MPILIB='-L/software/intel/mpi/3.1/lib64 -Xlinker -rpath -Xlinker $libdir -Xlinker -rpath -Xlinker /opt/intel/mpi-rt/3.1 -lmpi -lmpiif -lmpigi -lrt -lpthread -ldl -L/usr/lib64 -libverbs -lm'     
     (the front part for MPILIB option comes from `mpiifort -show`, and the rear part '-L/usr/lib64 -libverbs -lm' is used to link with Infiniband network.)

     2. Build Global Arrays version of PPIDD.
     Global Arrays should be installed prior to building PPIDD. As mentioned in Global Arrays documentation(http://www.emsl.pnl.gov/docs/global), 
     there are three possible ways for building GA: (1) GA with MPI; (2) GA with TCGMSG-MPI; and (3) GA with TCGMSG. 
     PPIDD can be built with either of these interfaces. 

     (1) GA with MPI:
     make MPICC=mpiicc MPIFC=mpiifort INT64=y FFLAGS='-i8 -Vaxlib' INCLUDE='../../../ga-4-1-1/include /software/intel/mpi/3.1/include64' MPILIB='-L/usr/lib64 -libverbs -lm'

     or
     make CC=icc FC=ifort INT64=y FFLAGS='-i8 -Vaxlib' INCLUDE='../../../ga-4-1-1/include /software/intel/mpi/3.1/include64' \
     MPILIB='-I/software/intel/mpi/3.1/include64 -L/software/intel/mpi/3.1/lib64 -Xlinker -rpath -Xlinker $libdir -Xlinker -rpath -Xlinker \
     /opt/intel/mpi-rt/3.1 -lmpi -lmpiif -lmpigi -lrt -lpthread -ldl -L/usr/lib64 -libverbs -lm'


     (2) GA with TCGMSG-MPI:
     make MPICC=mpiicc MPIFC=mpiifort INT64=y FFLAGS='-i8 -Vaxlib' INCLUDE=../../../ga-4-1-1/include MPILIB='-L/usr/lib64 -libverbs -lm'

     or
     make CC=icc FC=ifort INT64=y FFLAGS='-i8 -Vaxlib' INCLUDE=../../../ga-4-1-1/include \
     MPILIB='-I/software/intel/mpi/3.1/include64 -L/software/intel/mpi/3.1/lib64 -Xlinker -rpath -Xlinker $libdir -Xlinker -rpath -Xlinker \
     /opt/intel/mpi-rt/3.1 -lmpi -lmpiif -lmpigi -lrt -lpthread -ldl -L/usr/lib64 -libverbs -lm'

     (3) GA with TCGMSG:
     make CC=icc FC=ifort INT64=y FFLAGS='-i8 -Vaxlib' INCLUDE=../../../ga-4-1-1/include MPILIB=../../../ga-4-1-1/lib/LINUX64/lib/


  </pre>
 *
 */


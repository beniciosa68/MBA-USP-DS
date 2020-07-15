# src/ppidd/GNUmakefile $Revision: 2008.3 $ 

VERSION=$(shell cat VERSION)
kernel=$(shell uname -s)

ifeq ($(origin CC),default)
CC=
endif
ifeq ($(origin FC),default)
FC=
endif
ifeq ($(origin INCLUDE),environment)
INCLUDE=
endif

AR=ar
ARFLAGS=-rS
RANLIB=ranlib

# Generic Linux
override MPICC_Linux=mpicc
override MPIFC_Linux=mpif90 mpif77
override CC_Linux=icc gcc
override FC_Linux=pgf90 pathf90 ifort gfortran g95

# Mac OSX
override MPICC_Darwin=mpicc
override MPIFC_Darwin=mpif90
override CC_Darwin=icc gcc
override FC_Darwin=pgf90 ifort gfortran g95

ifndef CC
MPICC=$(foreach exe,$(MPICC_$(kernel)),$(shell if type $(exe) 1>/dev/null 2>&1; then which $(exe); fi))
endif
ifndef FC
MPIFC=$(foreach exe,$(MPIFC_$(kernel)),$(shell if type $(exe) 1>/dev/null 2>&1; then which $(exe); fi))
endif
CC=$(foreach exe,$(CC_$(kernel)),$(shell if type $(exe) 1>/dev/null 2>&1; then which $(exe); fi))
FC=$(foreach exe,$(FC_$(kernel)),$(shell if type $(exe) 1>/dev/null 2>&1; then which $(exe); fi))

CC:=$(firstword $(CC))
FC:=$(firstword $(FC))
MPICC:=$(firstword $(MPICC))
MPIFC:=$(firstword $(MPIFC))
DOXYGEN=$(shell if type doxygen 1>/dev/null 2>&1 ; then which doxygen; fi)

ifeq ($(MPIFC),)
ifeq ($(notdir $(FC)),ifort)
override FFLAGS+=-Vaxlib
endif
endif
ifdef INT64
override CFLAGS+=-DINT64
endif

ifndef BUILD
ifdef INCLUDE
ifeq (mpi.h,$(filter mpi.h,$(notdir $(wildcard $(firstword $(INCLUDE))/*.h))))
BUILD=MPI2
endif
endif
endif

ifndef BUILD
ifdef INCLUDE
ifneq ($(wildcard $(firstword $(INCLUDE))/../lib/*),)
ifeq ($(wildcard $(firstword $(INCLUDE))/../lib/*),$(firstword $(wildcard $(firstword $(INCLUDE))/../lib/*)))
check_files=libtcgmsg.a libglobal.a libma.a libpario.a libarmci.a
ifeq ($(sort $(check_files)),$(sort $(filter $(check_files),$(notdir $(wildcard $(firstword $(INCLUDE))/../lib/*/*.a)))))
BUILD=GA_TCGMSG
endif
endif
endif
endif
endif

ifndef BUILD
ifdef INCLUDE
ifneq ($(wildcard $(firstword $(INCLUDE))/../lib/*),)
ifeq ($(wildcard $(firstword $(INCLUDE))/../lib/*),$(firstword $(wildcard $(firstword $(INCLUDE))/../lib/*)))
check_files=libtcgmsg-mpi.a libglobal.a libma.a libpario.a libarmci.a
ifeq ($(sort $(check_files)),$(sort $(filter $(check_files),$(notdir $(wildcard $(firstword $(INCLUDE))/../lib/*/*.a)))))
BUILD=GA_TCGMSG_MPI
endif
endif
endif
endif
endif

ifndef BUILD
ifdef INCLUDE
ifneq ($(wildcard $(firstword $(INCLUDE))/../lib/*),)
ifeq ($(wildcard $(firstword $(INCLUDE))/../lib/*),$(firstword $(wildcard $(firstword $(INCLUDE))/../lib/*)))
check_files=libglobal.a libma.a libpario.a libarmci.a
ifeq ($(sort $(check_files)),$(sort $(filter $(check_files),$(notdir $(wildcard $(firstword $(INCLUDE))/../lib/*/*.a)))))
BUILD=GA_MPI
endif
endif
endif
endif
endif

ifndef BUILD
BUILD=MPI2
endif

ifeq ($(BUILD),GA_TCGMSG)
override CFLAGS+=-DGA_TOOLS -DGA_TCGMSG
override FFLAGS+=-DGA_TOOLS -DGA_TCGMSG
override LIBS+=-L$(realpath $(wildcard $(firstword $(INCLUDE))/../lib/*)) -ltcgmsg -lglobal -lma -lpario -larmci -llinalg
endif
ifeq ($(BUILD),GA_TCGMSG_MPI)
override CFLAGS+=-DGA_TOOLS -DGA_TCGMSG_MPI
override FFLAGS+=-DGA_TOOLS -DGA_TCGMSG_MPI
override LIBS+=-L$(realpath $(wildcard $(firstword $(INCLUDE))/../lib/*)) -ltcgmsg-mpi -lglobal -lma -lpario -larmci -llinalg
endif
ifeq ($(BUILD),GA_MPI)
override CFLAGS+=-DGA_TOOLS -DGA_MPI
override FFLAGS+=-DGA_TOOLS -DGA_MPI
override LIBS+=-L$(realpath $(wildcard $(firstword $(INCLUDE))/../lib/*)) -lglobal -lma -lpario -larmci -llinalg
endif
ifeq ($(BUILD),MPI2)
override CFLAGS+=-DMPI2
override FFLAGS+=-DMPI2
ifeq ($(NXTVAL),n)
override FFLAGS+=-DNO_NXTVAL_SERVER 
override CFLAGS+=-DNO_NXTVAL_SERVER 
endif
endif


.PHONY: default
default:
	@echo
ifneq (3.81,$(firstword $(sort 3.81 $(MAKE_VERSION))))
	@echo 'Make version 3.81 or higher is required'; exit 1
endif
ifeq ($(filter $(BUILD),MPI2 GA_TCGMSG GA_TCGMSG_MPI GA_MPI),)
	@echo build '$(BUILD)' not supported; exit 1
endif
ifdef INCLUDE
ifeq ($(BUILD),MPI2)
ifneq (mpi.h,$(filter mpi.h,$(notdir $(wildcard $(firstword $(INCLUDE))/*.h))))
	@echo 'mpi.h not found in $(INCLUDE)'; exit 1
endif
else
ifeq ($(BUILD),GA_MPI)
ifneq ($(sort mafdecls.fh global.fh macommon.h eaf.fh eaf.h),$(sort $(filter mafdecls.fh global.fh macommon.h eaf.fh eaf.h,$(notdir $(wildcard $(firstword $(INCLUDE))/*)))))
	@echo 'one or more GA include files missing in $(firstword $(INCLUDE))'; exit 1
endif
else
ifneq ($(sort mafdecls.fh global.fh tcgmsg.fh macommon.h eaf.fh eaf.h),$(sort $(filter mafdecls.fh global.fh tcgmsg.fh macommon.h eaf.fh eaf.h,$(notdir $(wildcard $(firstword $(INCLUDE))/*)))))
	@echo 'one or more GA include files missing in $(firstword $(INCLUDE))'; exit 1
endif
endif
endif
else
ifndef MPICC
	@echo 'both INCLUDE and MPICC variables are unset or cannot be determined'; exit 1
endif
endif
ifneq ($(kernel),$(findstring $(kernel),Linux Darwin))
	@echo 'WARNING: Unknown kernel $(kernel)'
endif
ifndef FC
	@echo 'Unable to find a Fortran compiler'; exit 1
endif
ifndef CC
	@echo 'Unable to find a C compiler'; exit 1
endif
	@echo 'Building PPIDD Library (Version $(VERSION))'
	@echo
ifeq ($(BUILD),MPI2)
ifeq ($(NXTVAL),n)
	@echo 'Helper process will not be compiled; PPIDD_Nxtval will be disabled'
	@echo
endif
endif
	@rm -rf lib
	@mkdir lib
	$(MAKE) -C src FC='$(if $(MPIFC),$(MPIFC),$(FC))' CC='$(if $(MPICC),$(MPICC),$(CC))' CFLAGS='$(CFLAGS) $(addprefix -I,$(realpath $(INCLUDE)))' FFLAGS='$(FFLAGS) $(addprefix -I,$(realpath $(INCLUDE)))'
	@$(AR) $(ARFLAGS) lib/libppidd.a src/*.o
	@$(RANLIB) lib/libppidd.a
	@echo
	@echo 'Building PPIDD test suite'
	@echo
ifneq ($(BUILD),GA_TCGMSG)
ifndef MPILIB
ifndef MPIFC
	@echo 'both MPILIB and MPIFC variables are unset or cannot be determined'; exit 1
endif
endif
endif
	$(MAKE) -C test BUILD='$(BUILD)' FC='$(if $(MPIFC),$(MPIFC),$(FC))' CC='$(if $(MPICC),$(MPICC),$(CC))' CFLAGS='$(CFLAGS)' FFLAGS='$(FFLAGS)' LIBS='$(LIBS) $(MPILIB)'

.PHONY: doc
doc:
ifdef DOXYGEN
	@cd doc; $(DOXYGEN) Doxyfile
else
	@echo 'doxygen does not appear to be installed'
endif

clean:
	@$(foreach directory,src test,$(MAKE) -C $(directory) clean;)
	@rm -rf lib

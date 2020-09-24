# src/ppidd/test/GNUmakefile $Revision: 2008.3 $ 

.PHONY: default
default: ppidd_test.exe ppidd_ctest.exe

ppidd_ctest.exe: ppidd_ctest.o timing_molpro.o
ifeq ($(BUILD),MPI2)
	$(CC) $(CFLAGS) -o $@ $^ -L../lib -lppidd $(LIBS)
else
	$(FC) $(patsubst -Vaxlib,-nofor_main,$(FFLAGS)) -o $@ $^ -L../lib -lppidd $(LIBS)
endif

ppidd_test.exe: ppidd_test.o timing_molpro.o sizeofctypes.o sizeoffortypes.o
	$(FC) $(FFLAGS) -o $@ $^ -L../lib -lppidd $(LIBS)

%.o: %.c
	$(CC) -c -I ../src $(CFLAGS) $<

%.o: %.F
	$(FC) -c $(FFLAGS) $<

clean:
	@rm -f *.o *.exe

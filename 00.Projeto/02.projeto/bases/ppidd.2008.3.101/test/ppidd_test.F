! src/mpp/ppidd_test.F $Revision: 2008.3 Patch(2008.3): ppidd_typecheck $ 
! PPIDD standalone test suites

#ifdef MOLPRO
      subroutine ppiddtest
#else
      program main
#endif
      implicit double precision (a-h,o-z)
      integer heap, stack, fudge
      integer iprocs, nproc
#ifndef MOLPRO
      integer ma_heap, idtype
      logical status
#endif
      parameter (heap=400*400*4, fudge=100, stack=400*400*4)

c     
c...  Intitialize PPIDD library
c***  Intitialize a message passing library
c***  Initialize GA
c
#ifndef MOLPRO
      call ppidd_initialize()
#endif

c...  Get the processes number and process id
      call ppidd_size(nproc)
      call ppidd_rank(iprocs)
c
      iout=6
      if(iprocs.eq.0)then
         write(iout,*) 
         write(iout,*) 'PPIDD initialized '
         call ppidd_flush6
      endif
      call ppidd_barrier()
c
#if !defined(MOLPRO) && defined(PPIDD_USE_MA)
c***  Initialize the MA package
c     MA must be initialized before any global array is allocated
c
      ma_heap=heap
      call ppidd_uses_ma(status)
      IF (status) then
        ma_heap=heap+fudge
        if(iprocs.eq.0) write(iout,11) ma_heap
11      format(' ppidd_uses_ma= true, calling ppidd_ma_init with',
     >         ' heap size=',i15)
      else
        if(iprocs.eq.0) write(iout,22)
22      format(' ppidd_uses_ma=false, calling ppidd_ma_init with',
     >         ' nominal heap.')
      END IF
c... idtype=1: fortran double precision type
      idtype=1
      call ppidd_ma_init(idtype,stack,ma_heap,status)
      if (.not. status) call ppidd_error('ppidd_ma_init failed',-1) 
#endif
c
c
c... Synchronize processes (a barrier) before specific tests
c
      if(iprocs.eq.0)then
         write(iout,*) 
         write(iout,'(1x,2(a,i4))') 'Nprocs=',nproc,'   My proc=',iprocs
         write(iout,*) 'Performing ppidd tests.'
         write(iout,29)
29       format(1x,'SUCCESSFUL TEST SHOULD END WITH ',
     >      ''' All PPIDD tests successful.''')
         write(iout,*) 
         call ppidd_flush6
      endif

      call ppidd_barrier()
c... Checking C date types match Fortran
      if(iprocs.eq.0)then
        call ppidd_checkdtype
      endif
      call ppidd_barrier()
c... Performing PPIDD basic operation test
      call get_current_times(cpua,walla,wtimea)
      call ppidd_basictest
      call ppidd_barrier()
      call get_current_times(cpub,wallb,wtimeb)
      if(iprocs.eq.0)write(iout,*)
      if(iprocs.eq.0)write(iout,100) 'ppidd_basictest:',
     >   cpub-cpua,wallb-walla,wtimeb-wtimea
100   format(1x,'Time spent on ',a,t49,' cpu=',f16.4,' sec,',
     > ' elapsed=',f16.4,' sec,',' wtime=',f16.4,' sec')


      call ppidd_barrier()
c... Performing PPIDD shared counter test
      call get_current_times(cpua,walla,wtimea)
#ifdef NO_NXTVAL_SERVER
      if(iprocs.eq.0) write(iout,*) 
     > 'NO_NXTVAL_SERVER is defined. Skip the shared counter test.'
#else
      call ppidd_sharedcounter_test
#endif
      call ppidd_barrier()
      call get_current_times(cpub,wallb,wtimeb)
      if(iprocs.eq.0)write(iout,*)
      if(iprocs.eq.0)write(iout,100) 'ppidd_sharedcounter_test:',
     >   cpub-cpua,wallb-walla,wtimeb-wtimea
c
c***  Check support for double precision arrays
c
c      if (iprocs.eq.0) then
c         write(6,*)
c         write(6,*) ' CHECKING DOUBLES  '
c         write(6,*)
c         call ffflush(6)
c      endif

c      call check_dbl()
c
c***  Check support for integer arrays
c
c      if (iprocs.eq.0) then
c         write(6,*)
c         write(6,*) ' CHECKING INTEGERS  '
c         write(6,*)
c         call ffflush(6)
c      endif

c      call check_int()
c
c     
c      if (iprocs.eq.0) then
c         write(6,*)
c         write(6,*)'CHECKING Wrappers to Message Passing Collective ops'
c         write(6,*)
c         call ffflush(6)
c      endif

c      call check_wrappers
c
c
      if(iprocs.eq.0) write(iout,*) 
      if(iprocs.eq.0) write(iout,*) 'All PPIDD tests successful.' 
c
c***  Terminate and Tidy up PPIDD
c
#ifndef MOLPRO
      call ppidd_finalize()
#endif
c
      end


      subroutine ppidd_flush6
#ifdef MOLPRO
      call flush6()
#else
      call flush(6)
#endif
      end


! Checking whether C types match Fortran types
      subroutine ppidd_checkdtype
      implicit double precision (a-h,o-z)
      logical ok_int,ok_log,ok_dbl

      write(6,*) 'Checking whether C types match Fortran types:'
      call sizeofctype(dbyte_int1,dbyte_log1,dbyte_dbl1,dbyte_float)
      nbyte_int1=nint(dbyte_int1)
      nbyte_log1=nint(dbyte_log1)
      nbyte_dbl1=nint(dbyte_dbl1)
      nbyte_float=nint(dbyte_float)
      write(6,*) 'The size (in bytes) of some C data types:'
      write(6,10) 'float:', nbyte_float
      write(6,10) 'double:', nbyte_dbl1
      write(6,10) 'fortint:', nbyte_int1
      write(6,10) 'fortlogical:', nbyte_log1
10    format(1x,'Size of C ',a,t35,i4,' bytes')
      call sizeoffortype(nbyte_int2,nbyte_log2,nbyte_dbl2,nbyte_real)
      write(6,*) 'The size (in bytes) of some Fortran data types:'
      write(6,20) 'Real:', nbyte_real
      write(6,20) 'Doule Precision:', nbyte_dbl2
      write(6,20) 'Integer:', nbyte_int2
      write(6,20) 'Logical:', nbyte_log2
20    format(1x,'Size of Fortran ',a,t35,i4,' bytes')
      ok_int=nbyte_int1.eq.nbyte_int2
      ok_log=nbyte_log1.eq.nbyte_log2
      ok_dbl=nbyte_dbl1.eq.nbyte_dbl2
      if( ok_int .and. ok_log .and. ok_dbl) then
        write(6,30)
30      format(1x,'All data types (Integer, Logical, and ',
     >         'Double Precision) match between C and Fortran.')
      else 
        if (.not.ok_int) write(6,31) 'Integer'
        if (.not.ok_log) write(6,31) 'Logical'
        if (.not.ok_dbl) write(6,31) 'Double Precision'
31      format(1x,'ERROR: Data type ( ',a,t38,') does not match between'
     >           ,' C and Fortran types.')
        write(6,32)
32      format(1x,'Please check INT64 (related to CFLAGS/FFLAGS)',
     >            ' option, and rebuild the library.')
        call ppidd_error('Data types failed to match',0)
      endif

      end


      subroutine ppidd_basictest
      implicit double precision (a-h,o-z)
      double precision, allocatable :: buff(:)
      integer, allocatable :: ibuff(:)
      double precision totsize,bufsize,z1
      double precision cpu1,cpu2,cpu,wall1,wall2,wall,speed
      integer lentot,lenbuf,len,ioff,ilo,ihi
      integer maxprc,nloop,nprocs,iprocs,iout,i,ipr
      integer proc_ltop,proc_rbot,dest_src,sync,lenre,srcre
      integer dtype,storetype
      integer ihandle,ihandtmp
      character*80 name
      logical ok
      logical verbose

      verbose=.false.
      totsize=0.0d0
      bufsize=0.0d0
      maxprc=0
      nloop=10

      iout=6
      nrep_1side=20000
      nrep_send =200000
      nrep_bcast=100000
      nrep_gsum =200000
! ARCCA
!      nrep_1side=200000
!      nrep_send =2000000
!      nrep_bcast=1000000
!      nrep_gsum =2000000

      call ppidd_size(nprocs)
      call ppidd_rank(iprocs)
      if(maxprc.eq.0) maxprc=nprocs
      maxprc=min(maxprc,nprocs)
      if(nloop.eq.0) nloop=1
      if(dabs(totsize).lt.1.0d0) totsize=32d0
!      if(dabs(totsize).lt.1.0d0) totsize=128d0  !ARCCA
      if(dabs(bufsize).lt.1.0d0) bufsize=16d0
      lentot=nint(totsize)*1024*128
      lenbuf=nint(bufsize)*1024*128
      if(iprocs.eq.0) write(iout,1) totsize,lentot,bufsize,lenbuf
1     format(/' Starting PPIDD basic test with GA-size=',f8.2,
     >        ' MB (',i9,' words),  Buffer size=',f8.2,
     >        ' MB (',i9,' words)'/)
c
c... initialise the buffer space
      ALLOCATE( buff(lenbuf) ) 
      ALLOCATE( ibuff(lenbuf) ) 
      do i=1,lenbuf
        buff(i)=0.01d0+dble(i)*0.001d0
        ibuff(i)=i
      end do

#ifdef MPI2
#ifdef NO_NXTVAL_SERVER
      nloop_array=1
#else
      nloop_array=2
#endif
#else
      nloop_array=1
#endif
      do loop_array=1,nloop_array
      if (nloop_array.eq.1) then
        if(iprocs.eq.0) write(iout,101) ' '
101     format(/,a,'Array data are stored across distributed compute'
     >  ,' processes:')
        dtype=1
        storetype=0
      else
c... case (1) 
        if (loop_array.eq.1) then
          if(iprocs.eq.0) write(iout,101) ' (1) ' 
          dtype=1
          storetype=0
c... case (2) 
        else
          if(iprocs.eq.0) write(iout,102) 
102       format(/' (2) Array data are stored on helper process:')
          dtype=0
          storetype=1
        end if
      end if
c
c... ppidd_create
      call ppidd_barrier()
      call get_current_times(cpu1,wall1,wtime1)
      call ppidd_create('ppiddarray',lentot,dtype,storetype,ihandle,ok)
      call get_current_times(cpu2,wall2,wtime2)
      cpu=cpu2-cpu1
      wall=wall2-wall1
      wtime=wtime2-wtime1
      call ppidd_inquire_name(ihandle,name)
      if(iprocs.eq.0)write(iout,*) 
     >  'ppidd has created global array: ',name(1:len_trim(name))
      if(iprocs.eq.0)write(iout,10) 'ppidd_create:',cpu,wall,wtime
10    format(1x,a,t20,'cpu=',f9.4,' sec,',
     >       '  elapsed=',f9.4,' sec,','  wtime=',f9.4,' sec',:,
     >       ',  speed=', f12.2,' MB/sec')
      call ppidd_flush6
      if (loop_array.eq.1) then
        call ppidd_create('ppiddtmp',nprocs,dtype,storetype,ihandtmp,ok)
      else
        call ppidd_create('ppiddtmp',nprocs+1,dtype,storetype,
     >                    ihandtmp,ok)
      end if
c
c... ppidd_put
c... measure the latency
c    for arrays stored on helper process, measure the time for operating 2 elements
c    since 1 element operation is a special case
      n=1      
      nrep=nrep_1side/(nprocs*nprocs)
      if(nrep.eq.0) nrep=1
      wtimes=0.0
      do ipr=0,maxprc-1
        call ppidd_barrier()
        call ppidd_wtime(wtime1)
        if(iprocs.eq.ipr) then
          do irep=1,nrep
            do inum=1,nprocs
             if (loop_array.eq.1) then
              call ppidd_put(ihandtmp,inum,inum,buff(inum),ok)
             else
              ilo=inum
              ihi=ilo+1
              call ppidd_put(ihandtmp,ilo,ihi,ibuff(ilo),ok)
             end if
            end do
          end do
        end if
        call ppidd_barrier()
        call ppidd_wtime(wtime2)
        wtimes=wtimes+wtime2-wtime1
      end do
      call ppidd_barrier()
      wlat=wtimes/dble(nrep*nprocs*nprocs)
      if(iprocs.eq.0) write(iout,50) 'PPIDD_PUT=',wtimes,wlat*1.0d6
50    format(1x,'TIME FOR LATENCY TEST ',a,t41,f10.2,' SEC,  LATENCY=',
     >          f10.2,' MICROSEC')

c... measure the bandwidth
      cpus=0.0
      walls=0.0
      wtimes=0.0
      nopr=0
      do ipr=0,maxprc-1
        call ppidd_barrier()
        call get_current_times(cpu1,wall1,wtime1)
        if(iprocs.eq.ipr) then
          do i=1,nloop
          nrest=lentot
          do ioff=0,lentot-1,lenbuf
            len=min(lenbuf,nrest)
            ilo=ioff+1
            ihi=ioff+len
            if (loop_array.eq.1) then
              call ppidd_put(ihandle,ilo,ihi,buff,ok)
            else
              call ppidd_put(ihandle,ilo,ihi,ibuff,ok)
            end if
            nopr=nopr+1
            nrest=nrest-len
          end do
          end do
        end if
        call ppidd_barrier()
        call get_current_times(cpu2,wall2,wtime2)
        cpu=cpu2-cpu1
        wall=wall2-wall1
        wtime=wtime2-wtime1
        cpus=cpus+cpu
        walls=walls+wall
        wtimes=wtimes+wtime
        if(iprocs.eq.0 .and. verbose) then
          speed1=0.0d0
          speed2=0.0d0
          if(wall.gt.0d0) speed1=totsize*dble(nloop)/wall
          if(wtime.gt.0d0) speed2=totsize*dble(nloop)/wtime
          write(iout,20) 'ppidd_put:',ipr,cpu,wall,speed1
20        format(1x,a,t15,'  iprocs=',i3,'  cpu=',f9.4,' sec,',
     >         '  elapsed=',f9.4,' sec,  speed1=',
     >         f8.2,' MB/sec')
          write(iout,22) 'ppidd_put:',ipr,cpu,wtime,speed2
22        format(1x,a,t15,'  iprocs=',i3,'  cpu=',f9.4,' sec,',
     >         '  w  time=',f9.4,' sec,  speed2=',
     >         f8.2,' MB/sec')
          call ppidd_flush6
        end if
      end do
      spd1=0.0d0
      spd2=0.0d0
      if(walls.gt.0d0) spd1=dble(maxprc*nloop)*totsize/walls
      if(wtimes.gt.0d0) then
        spd2=dble(maxprc*nloop)*totsize/wtimes
        bdw=dble(maxprc*nloop)*totsize/(wtimes-wlat*dble(nopr*nprocs))
      end if
      if(iprocs.eq.0.and.verbose) write(iout,23) 'ppidd_put:',spd1,spd2
23    format(1x,a,t18,'  average speed1=',f8.2,' MB/sec,',
     >                '  average speed2=',f8.2,' MB/sec')
      if(iprocs.eq.0) then
        write(iout,55) 'ppidd_put:',wlat*dble(nopr*nprocs),nopr
        write(iout,10) 'ppidd_put:',cpus,walls,wtimes,spd2
        write(iout,60) 'ppidd_put:',cpus,walls,wtimes,wlat*1.0d6,bdw
      end if
55    format(1x,a,t20,' total latency time=',f9.4,' sec,  nopr=',i6)
60    format(1x,a,t20,'cpu=',f9.4,' sec,',
     >       '  elapsed=',f9.4,' sec,','  wtime=',f9.4,' sec',:,
     >     ',  latency=',f10.2,' microsec, bandwidth=', f12.2,' MB/sec')
      call ppidd_flush6


c
c... ppidd_get
c... measure the latency
      wtimes=0.0
      do ipr=0,maxprc-1
        call ppidd_barrier()
        call ppidd_wtime(wtime1)
        if(iprocs.eq.ipr) then
          do irep=1,nrep
            do inum=1,nprocs
             if (loop_array.eq.1) then
! For Bull MPI, comment out the following line to proceed
              call ppidd_get(ihandtmp,inum,inum,buff(inum),ok)
             else
              ilo=inum
              ihi=ilo+1
              call ppidd_get(ihandtmp,ilo,ihi,ibuff(ilo),ok)
             end if
            end do
          end do
        end if
        call ppidd_barrier()
        call ppidd_wtime(wtime2)
        wtimes=wtimes+wtime2-wtime1
      end do
      call ppidd_barrier()
      wlat=wtimes/dble(nrep*nprocs*nprocs)
      if(iprocs.eq.0) write(iout,50) 'PPIDD_GET=',wtimes,wlat*1.0d6

c... measure the bandwidth
      cpus=0.0
      walls=0.0
      wtimes=0.0
      nopr=0
      do ipr=0,maxprc-1
        call ppidd_barrier()
        call get_current_times(cpu1,wall1,wtime1)
        if(iprocs.eq.ipr) then
          do i=1,nloop
          nrest=lentot
          do ioff=0,lentot-1,lenbuf
            len=min(lenbuf,nrest)
            ilo=ioff+1
            ihi=ioff+len
            if (loop_array.eq.1) then
              call ppidd_get(ihandle,ilo,ihi,buff,ok)
            else
              call ppidd_get(ihandle,ilo,ihi,ibuff,ok)
            end if
            nopr=nopr+1
            nrest=nrest-len
          end do
          end do
        end if
        call ppidd_barrier()
        call get_current_times(cpu2,wall2,wtime2)
        cpu=cpu2-cpu1
        wall=wall2-wall1
        wtime=wtime2-wtime1
        cpus=cpus+cpu
        walls=walls+wall
        wtimes=wtimes+wtime
        if(iprocs.eq.0 .and. verbose) then
          speed1=0.0d0
          speed2=0.0d0
          if(wall.gt.0d0) speed1=totsize*dble(nloop)/wall
          if(wtime.gt.0d0) speed2=totsize*dble(nloop)/wtime
          write(iout,20) 'ppidd_get:',ipr,cpu,wall,speed1
          write(iout,22) 'ppidd_get:',ipr,cpu,wtime,speed2
          call ppidd_flush6
        end if
      end do
      spd1=0.0d0
      spd2=0.0d0
      if(walls.gt.0d0) spd1=dble(maxprc*nloop)*totsize/walls
      if(wtimes.gt.0d0) then
        spd2=dble(maxprc*nloop)*totsize/wtimes
        bdw=dble(maxprc*nloop)*totsize/(wtimes-wlat*dble(nopr*nprocs))
      end if
      if(iprocs.eq.0.and.verbose) write(iout,23) 'ppidd_get:',spd1,spd2
      if(iprocs.eq.0) then
        write(iout,55) 'ppidd_get:',wlat*dble(nopr*nprocs),nopr
        write(iout,10) 'ppidd_get:',cpus,walls,wtimes,spd2
        write(iout,60) 'ppidd_get:',cpus,walls,wtimes,wlat*1.0d6,bdw
      end if
      call ppidd_flush6
c
c... ppidd_acc
c... measure the latency
      iz1=1
      z1=1.0d0
      wtimes=0.0
      do ipr=0,maxprc-1
        call ppidd_barrier()
        call ppidd_wtime(wtime1)
        if(iprocs.eq.ipr) then
          do irep=1,nrep
            do inum=1,nprocs
             if (loop_array.eq.1) then
              call ppidd_acc(ihandtmp,inum,inum,buff(inum),z1,ok)
             else
              ilo=inum
              ihi=ilo+1
              call ppidd_acc(ihandtmp,ilo,ihi,ibuff(ilo),iz1,ok)
             end if
            end do
          end do
        end if
        call ppidd_barrier()
        call ppidd_wtime(wtime2)
        wtimes=wtimes+wtime2-wtime1
      end do
      call ppidd_barrier()
      wlat=wtimes/dble(nrep*nprocs*nprocs)
      if(iprocs.eq.0) write(iout,50) 'PPIDD_ACC=',wtimes,wlat*1.0d6

c... measure the bandwidth
      cpus=0.0
      walls=0.0
      wtimes=0.0
      nopr=0
      do ipr=0,maxprc-1
        call ppidd_barrier()
        call get_current_times(cpu1,wall1,wtime1)
        if(iprocs.eq.ipr) then
          do i=1,nloop
          nrest=lentot
          do ioff=0,lentot-1,lenbuf
            len=min(lenbuf,nrest)
            ilo=ioff+1
            ihi=ioff+len
            if (loop_array.eq.1) then
              call ppidd_acc(ihandle,ilo,ihi,buff,z1,ok)
            else
              call ppidd_acc(ihandle,ilo,ihi,ibuff,iz1,ok)
            end if
            nopr=nopr+1
            nrest=nrest-len
          end do
          end do
        end if
        call ppidd_barrier()
        call get_current_times(cpu2,wall2,wtime2)
        cpu=cpu2-cpu1
        wall=wall2-wall1
        wtime=wtime2-wtime1
        cpus=cpus+cpu
        walls=walls+wall
        wtimes=wtimes+wtime
        if(iprocs.eq.0 .and. verbose) then
          speed1=0.0d0
          speed2=0.0d0
          if(wall.gt.0d0) speed1=totsize*dble(nloop)/wall
          if(wtime.gt.0d0) speed2=totsize*dble(nloop)/wtime
          write(iout,20) 'ppidd_acc:',ipr,cpu,wall,speed1
          write(iout,22) 'ppidd_acc:',ipr,cpu,wtime,speed2
          call ppidd_flush6
        end if
      end do
      spd1=0.0
      spd2=0.0
      if(walls.gt.0d0) spd1=dble(maxprc*nloop)*totsize/walls
      if(wtimes.gt.0d0) then
        spd2=dble(maxprc*nloop)*totsize/wtimes
        bdw=dble(maxprc*nloop)*totsize/(wtimes-wlat*dble(nopr*nprocs))
      end if
      if(iprocs.eq.0.and.verbose) write(iout,23) 'ppidd_acc:',spd1,spd2
      if(iprocs.eq.0) then
        write(iout,55) 'ppidd_acc:',wlat*dble(nopr*nprocs),nopr
        write(iout,10) 'ppidd_acc:',cpus,walls,wtimes,spd2
        write(iout,60) 'ppidd_acc:',cpus,walls,wtimes,wlat*1.0d6,bdw
      end if
      call ppidd_flush6
c
c... ppidd_zero
      call ppidd_barrier()
      call get_current_times(cpu1,wall1,wtime1)
      do i=1,nloop
        call ppidd_zero(ihandle,ok)
      end do
      call get_current_times(cpu2,wall2,wtime2)
      cpu=cpu2-cpu1
      wall=wall2-wall1
      wtime=wtime2-wtime1
      if(iprocs.eq.0) write(iout,10) 'ppidd_zero:',cpu,wall,wtime
      call ppidd_flush6
c
c... ppidd_destroy
      call ppidd_barrier()
      call get_current_times(cpu1,wall1,wtime1)
      call ppidd_destroy(ihandle,ok)
      call get_current_times(cpu2,wall2,wtime2)
      cpu=cpu2-cpu1
      wall=wall2-wall1
      wtime=wtime2-wtime1
      if(iprocs.eq.0) write(iout,10) 'ppidd_destroy:',cpu,wall,wtime
      call ppidd_flush6

      call ppidd_destroy(ihandtmp,ok)

      end do
c...  end of loop_array

c
c... ppidd_send and ppidd_recv
      if(iprocs.eq.0)write(iout,*)
      proc_ltop=maxprc/2
      proc_rbot=(maxprc-1)/2
      dest_src=maxprc-1-iprocs
c     data type: double precision; sync: synchronous
      dtype=0   
      sync=1

c.... measure latency
      n=1      
      nrep=nrep_send*2/nprocs
      if(nrep.eq.0) nrep=1
      wtimes=0.0
      do ipr=0,proc_ltop-1
        ipr_pair=maxprc-1-ipr
        call ppidd_barrier()
        call ppidd_wtime(wtime1)
        if(iprocs.eq.ipr) then
          do irep=1,nrep
            call ppidd_send(ibuff(iprocs),n,dtype,dest_src,sync)
          end do
        end if
        if(iprocs.eq.ipr_pair) then
          do irep=1,nrep
            call ppidd_recv(ibuff(iprocs),n,dtype,dest_src,lenre,srcre,
     >                      sync)
          end do
        end if
        call ppidd_barrier()
        call ppidd_wtime(wtime2)
        wtimes=wtimes+wtime2-wtime1
      end do
      call ppidd_barrier()
      wlat=0.0
      if (proc_ltop.ne.0) wlat=wtimes/dble(nrep*proc_ltop)
      if(iprocs.eq.0)write(iout,50) 'PPIDD_SEND/RECV=',wtimes,wlat*1.d6

c.... measure bandwidth
      cpus=0.0
      walls=0.0
      wtimes=0.0
      dtype=1   
      nopr=0
!      nloop_save=nloop
!      nloop=nloop*8
      do ipr=0,proc_ltop-1
        ipr_pair=maxprc-1-ipr
        call ppidd_barrier()
        call get_current_times(cpu1,wall1,wtime1)
        if(iprocs.eq.ipr) then
          do i=1,nloop
          nrest=lentot
          do ioff=0,lentot-1,lenbuf
            len=min(lenbuf,nrest)
            call ppidd_send(buff,len,dtype,dest_src,sync)
            nopr=nopr+1
            nrest=nrest-len
          end do
          end do
        end if
        if(iprocs.eq.ipr_pair) then
          do i=1,nloop
          nrest=lentot
          do ioff=0,lentot-1,lenbuf
            len=min(lenbuf,nrest)
            call ppidd_recv(buff,len,dtype,dest_src,lenre,srcre,sync)
            nrest=nrest-len
          end do
          end do
        end if
        call ppidd_barrier()
        call get_current_times(cpu2,wall2,wtime2)
        cpu=cpu2-cpu1
        wall=wall2-wall1
        wtime=wtime2-wtime1
        cpus=cpus+cpu
        walls=walls+wall
        wtimes=wtimes+wtime
        if(iprocs.eq.0 .and. verbose) then
          speed1=0.0d0
          speed2=0.0d0
          if(wall.gt.0d0) speed1=totsize*dble(nloop)/wall
          if(wtime.gt.0d0) speed2=totsize*dble(nloop)/wtime
          write(iout,20) 'ppidd_send/recv:',ipr,cpu,wall,speed1
          write(iout,22) 'ppidd_send/recv:',ipr,cpu,wtime,speed2
          call ppidd_flush6
        end if
      end do
      spd1=0.0
      spd2=0.0
      if(walls.gt.0d0) spd1=dble(proc_ltop*nloop)*totsize/walls
      if(wtimes.gt.0d0) then
         spd2=totsize*dble(proc_ltop*nloop)/wtimes
         bdw=totsize*dble(proc_ltop*nloop)/(wtimes-
     >       wlat*dble(nopr*proc_ltop))
      end if
      if(iprocs.eq.0.and.verbose) 
     >  write(iout,23) 'ppidd_send/recv:',spd1,spd2
      if(iprocs.eq.0) write(iout,55) 'ppidd_send/recv:',
     >  wlat*dble(nopr*proc_ltop),nopr
      if(iprocs.eq.0) write(iout,10) 'ppidd_send/recv:',
     >  cpus,walls,wtimes,spd2
      if(iprocs.eq.0) write(iout,60) 'ppidd_send/recv:',
     >  cpus,walls,wtimes,wlat*1.0d6,bdw
      call ppidd_flush6
!      nloop=nloop_save
c
c... ppidd_brdcst
      n=1      
      nrep=nrep_bcast/nprocs
      if(nrep.eq.0) nrep=1
      wtimes=0.0
      call ppidd_barrier()
      call ppidd_wtime(wtime1)
      do irep=1,nrep
        do ipr=0,nprocs-1
          call ppidd_bcast(ibuff,n,0,ipr)
        end do
      end do
      call ppidd_barrier()
      call ppidd_wtime(wtime2)
      wtimes=wtime2-wtime1
      wlat=wtimes/dble(nrep*nprocs)
      if(iprocs.eq.0) write(iout,50) 'PPIDD_BRDCST=',wtimes,wlat*1.0d6

      cpus=0.0
      walls=0.0
      wtimes=0.0
      nopr=0
      totsize_save=totsize
      lentot_save=lentot
      totsize=totsize*8.0d0/dble(nprocs)
      lentot=nint(totsize)*1024*128
      do ipr=0,maxprc-1
        call ppidd_barrier()
        call get_current_times(cpu1,wall1,wtime1)
        do i=1,nloop
        nrest=lentot
        do ioff=0,lentot-1,lenbuf
          len=min(lenbuf,nrest)
          call ppidd_bcast(buff,len,1,ipr)
          nopr=nopr+1
          nrest=nrest-len
        end do
        end do
        call ppidd_barrier()
        call get_current_times(cpu2,wall2,wtime2)
        wall=wall2-wall1
        cpu=cpu2-cpu1
        wtime=wtime2-wtime1
        cpus=cpus+cpu
        walls=walls+wall
        wtimes=wtimes+wtime
        if(iprocs.eq.0 .and. verbose) then
          speed1=0.0d0
          speed2=0.0d0
          if(wall.gt.0d0) speed1=totsize*dble(nloop)/wall
          if(wtime.gt.0d0) speed2=totsize*dble(nloop)/wtime
          write(iout,20) 'ppidd_brdcst:',ipr,cpu,wall,speed1
          write(iout,22) 'ppidd_brdcst:',ipr,cpu,wtime,speed2
          call ppidd_flush6
        end if
      end do
      spd1=0.0
      spd2=0.0
      bdw=0.0d0
      if(walls.gt.0d0) spd1=dble(maxprc*nloop)*totsize/walls
      if(wtimes.gt.0d0) spd2=dble(maxprc*nloop)*totsize/wtimes
      if(wtimes-wlat*dble(nopr) .gt. 0d0) then
        bdw=dble(maxprc*nloop)*totsize/(wtimes-wlat*dble(nopr))
      end if
      if(iprocs.eq.0.and.verbose)
     >    write(iout,23) 'ppidd_brdcst:',spd1,spd2
      if(iprocs.eq.0) then 
        write(iout,55) 'ppidd_brdcst:',wlat*dble(nopr),nopr
        write(iout,10) 'ppidd_brdcst:',cpus,walls,wtimes,spd2
        write(iout,60) 'ppidd_brdcst:',cpus,walls,wtimes,wlat*1.0d6,bdw
      end if
      call ppidd_flush6
      totsize=totsize_save
      lentot=lentot_save
c
c... ppidd_gsum
      n=1      
      nrep=nrep_gsum/nprocs
      if(nrep.eq.0) nrep=1
      wtimes=0.0
      call ppidd_barrier()
      call ppidd_wtime(wtime1)
      do irep=1,nrep
        call ppidd_gsum(0,ibuff,n,'+')
      end do
      call ppidd_barrier()
      call ppidd_wtime(wtime2)
      wtimes=wtime2-wtime1
      wlat=wtimes/dble(nrep)
      if(iprocs.eq.0) write(iout,50) 'PPIDD_GSUM=',wtimes,wlat*1.0d6

      nopr=0
      call ppidd_barrier()
      call get_current_times(cpu1,wall1,wtime1)
      do i=1,nloop
      nrest=lentot
      do ioff=0,lentot-1,lenbuf
        len=min(lenbuf,nrest)
        call ppidd_gsum(1,buff,len,'+')
        nopr=nopr+1
        nrest=nrest-len
      end do
      end do
      call ppidd_barrier()
      call get_current_times(cpu2,wall2,wtime2)
      cpus=cpu2-cpu1
      walls=wall2-wall1
      wtimes=wtime2-wtime1
      spd1=0.0
      spd2=0.0
      if(wall.gt.0d0) spd1=totsize*dble(nloop)/walls
      if(wtimes.gt.0d0) spd2=totsize*dble(nloop)/wtimes
      if(wtimes.gt.0d0) bdw=totsize*dble(nloop)/(wtimes-wlat*dble(nopr))
      if(iprocs.eq.0.and.verbose) write(iout,23) 'ppidd_gsum:',spd1,spd2
      if(iprocs.eq.0) write(iout,55) 'ppidd_gsum:',wlat*dble(nopr),nopr
      if(iprocs.eq.0)write(iout,10) 'ppidd_gsum:',cpus,walls,wtimes,spd2
      if(iprocs.eq.0) write(iout,60) 'ppidd_gsum:',
     >  cpus,walls,wtimes,wlat*1.0d6,bdw
      call ppidd_flush6
c
c... release the memory in buffer space
      IF( ALLOCATED(buff) ) DEALLOCATE( buff )
      IF( ALLOCATED(ibuff) ) DEALLOCATE( ibuff )

      return
      end

      subroutine get_current_times(cpu,wall,wtime)
      implicit double precision (a-h,o-z)
      double precision cpu,wall,wtime

      cpu=dble(second())
      wall=wallcl()
      call ppidd_wtime(wtime)

      return
      end



      subroutine ppidd_sharedcounter_test
      implicit double precision (a-h,o-z)
      integer, allocatable :: task_array(:)
      double precision, allocatable :: sum_array(:)
      double precision, allocatable :: tcpu_array1(:)
      double precision, allocatable :: twall_array1(:)
      double precision, allocatable :: twt_array1(:),twt_array2(:)
      double precision cpu1,cpu2,wall1,wall2
      double precision wtime1,wtime2
      double precision sumtemp,diffm
      integer nprocs,iprocs,iout,i
      integer num_tasks,nscale_task,npart
      integer nval,junk
      logical verbose
c
c... set task number and task scale
c    please change these two numbers if carrying out large-scale, many-task test
c    eg. num_tasks=1000 nscale_task=1000 (Please be aware that it may take hours depending on machine)

      num_tasks=50
!      num_tasks=100000 !ARCCA
      nscale_task=1

c    2^31=2,147,483,648
      npart=10000000
!      npart=5000000

      verbose=.false.
!      verbose=.true.
      iout=6
      nval=0
      sumtemp=0.0d0
      diffm=0.0d0

      call ppidd_size(nprocs)
      call ppidd_rank(iprocs)

      if(iprocs.eq.0) write(iout,*) 
      if(iprocs.eq.0) write(iout,*) 'PPIDD shared counter tests:'
      call ppidd_flush6

#if defined(GA_TOOLS) && defined(GA_MPI)

      if(iprocs.eq.0) write(iout,*) 
     > 'For GA_MPI, ppidd_nxtval does nothing. Skip this test.'

#else

c... shared counter test with computing tasks

c... initialise the buffer space
      ALLOCATE( task_array(nprocs) ) 
      ALLOCATE( sum_array(nprocs) ) 
      ALLOCATE( tcpu_array1(nprocs) ) 
      ALLOCATE( twall_array1(nprocs) ) 
      ALLOCATE( twt_array1(nprocs) ) 
      ALLOCATE( twt_array2(nprocs) ) 
      do i=1,nprocs
        task_array(i)=0
        sum_array(i)=0.0d0
        tcpu_array1(i)=0.0d0
        twall_array1(i)=0.0d0
        twt_array1(i)=0.0d0
        twt_array2(i)=0.0d0
      end do

c...(1)  shared counter test with computing tasks 
      if(iprocs.eq.0) write(iout,*) 
     >  '(1) PPIDD shared counter test with computing tasks:'
      if(verbose) write(iout,30) 
     >            'Nprocs=',nprocs,'  Total tasks=', num_tasks
c... initialise the NXTVAL Server
      call ppidd_nxtval(-nprocs,junk)
      call ppidd_barrier()
      call get_current_times(cpu1,wall1,wtime1)
      call ppidd_nxtval(nprocs,nval)
      call ppidd_wtime(wtimea)
      do while (nval.lt.num_tasks)
        call compute_task(npart,nscale_task,sumtemp)
        sum_array(iprocs+1)=sum_array(iprocs+1)+sumtemp
        task_array(iprocs+1)=task_array(iprocs+1)+1
        call ppidd_wtime(wtimeb)
c... time spent on computation and other overhead
        twt_array2(iprocs+1) =twt_array2(iprocs+1)+wtimeb-wtimea
        call ppidd_nxtval(nprocs,nval)
        call ppidd_wtime(wtimea)
      enddo
      call get_current_times(cpu2,wall2,wtime2)
c... task time
      tcpu_array1(iprocs+1)=cpu2-cpu1
      twall_array1(iprocs+1)=wall2-wall1
      twt_array1(iprocs+1)=wtime2-wtime1
      call ppidd_barrier()


c... release the shared counter number in NXTVAL Server and set it to zero
      call ppidd_nxtval(-nprocs,junk)

c... broadcast the completed tasks on current process 
      do i=1,nprocs
        call ppidd_bcast(task_array(i),1,0,i-1)
        call ppidd_bcast(sum_array(i), 1,1,i-1)
        call ppidd_bcast(tcpu_array1(i), 1,1,i-1)
        call ppidd_bcast(twall_array1(i), 1,1,i-1)
        call ppidd_bcast(twt_array1(i), 1,1,i-1)
        call ppidd_bcast(twt_array2(i), 1,1,i-1)
      end do

c... check whether right numbers are obtained
      do i=2,nprocs
        diffm=sum_array(i)/dble(task_array(i))
     >        - sum_array(i-1)/dble(task_array(i-1))
        if (verbose) write(iout,*) ' difference of average=',diffm
        if (abs(diffm).gt.1.0d-5) 
     >       call ppidd_error('shared counter test failed',0)
      end do

c... print out the summary
      if(iprocs.eq.0) then
        write(iout,30) 'Nprocs=',nprocs,'  Total tasks=', num_tasks
30      format(1x,a,t15,i4,a,t40,i12)
        write(iout,19)
19      format(1x,' IPROC      TASKS      WTime/task(Sec)')
        do i=1,nprocs
          if (task_array(i).ne.0) then
            twt_task=twt_array1(i)/dble(task_array(i))
          else
            twt_task=100000.0
          end if
          write(iout,20) i-1,task_array(i),twt_task
20        format(1x,i5,i12,f20.9)
        end do
        write(iout,10) 'Time spent for iprocs=0:',
     >  tcpu_array1(1),twall_array1(1),twt_array1(1)
10      format(1x,a,t49,' cpu=',f16.6,' sec,',' elapsed=',f16.6,' sec,',
     >         ' wtime=',f16.6,' sec')
        cpu_tot1=sum(tcpu_array1)
        wall_tot1=sum(twall_array1)
        wt_tot1=sum(twt_array1)
        wt_tot2=sum(twt_array2)
        write(iout,10) 'Sum of time for all procs all tasks:',
     >  cpu_tot1,wall_tot1,wt_tot1
        write(iout,9) 'Sum of time for all procs all computations:',
     >  wt_tot2
9       format(1x,a,t105,' wtime=',f16.6,' sec')
        write(iout,9) 'Sum of time for all procs all counter callings:',
     >  wt_tot1-wt_tot2
        acpu1=cpu_tot1/dble(num_tasks)
        awall1=wall_tot1/dble(num_tasks)
        awt1=wt_tot1/dble(num_tasks)
        awt2=wt_tot2/dble(num_tasks)
        write(iout,10) 'Average time per task:',acpu1,awall1,awt1
        write(iout,9) 'Average time per computation:',awt2
        write(iout,12) 'Average time per shared counter calling:',
     >                 (awt1-awt2)*1.0d9
12      format(1x,a,t105,' wtime=',f12.1,' nanosec')
      end if
c... end of (1)  

c...(2)  distribute tasks evenly to every process without shared counter 
      call ppidd_barrier()
      do i=1,nprocs
        task_array(i)=0
        sum_array(i)=0.0d0
      end do
      if(iprocs.eq.0) write(iout,*) 
      if(iprocs.eq.0) write(iout,*) '(2) Distribute computing tasks to',
     >' processes without shared counter:'
      if(verbose) write(iout,30) 
     >            'Nprocs=',nprocs,'  Total tasks=', num_tasks
      call ppidd_barrier()
      call get_current_times(cpu1,wall1,wtime1)
      do i=1,num_tasks
        ipr=mod(i,nprocs)
        if(ipr.eq.iprocs) then
          call compute_task(npart,nscale_task,sumtemp)
          sum_array(iprocs+1)=sum_array(iprocs+1)+sumtemp
          task_array(iprocs+1)=task_array(iprocs+1)+1
        end if
      enddo
      call get_current_times(cpu2,wall2,wtime2)
      tcpu_array1(iprocs+1)=cpu2-cpu1
      twall_array1(iprocs+1)=wall2-wall1
      twt_array1(iprocs+1)=wtime2-wtime1
      call ppidd_barrier()

c... broadcast the completed tasks on current process 
      do i=1,nprocs
        call ppidd_bcast(task_array(i),1,0,i-1)
        call ppidd_bcast(sum_array(i), 1,1,i-1)
        call ppidd_bcast(tcpu_array1(i), 1,1,i-1)
        call ppidd_bcast(twall_array1(i), 1,1,i-1)
        call ppidd_bcast(twt_array1(i), 1,1,i-1)
      end do

c... check whether right numbers are obtained
      do i=2,nprocs
        diffm=sum_array(i)/dble(task_array(i))
     >        - sum_array(i-1)/dble(task_array(i-1))
        if (verbose) write(iout,*) ' difference of average=',diffm
        if (abs(diffm).gt.1.0d-5) 
     >       call ppidd_error('shared counter test failed',0)
      end do

c... print out the summary
      if(iprocs.eq.0) then
        write(iout,30) 'Nprocs=',nprocs,'  Total tasks=', num_tasks
        write(iout,19)
        do i=1,nprocs
          if (task_array(i).ne.0) then
            twt_task=twt_array1(i)/dble(task_array(i))
          else
            twt_task=100000.0
          end if
          write(iout,20) i-1,task_array(i),twt_task
        end do
        write(iout,10) 'Time spent for iprocs=0:',
     >  tcpu_array1(1),twall_array1(1),twt_array1(1)
        cpu_tot3=sum(tcpu_array1)
        wall_tot3=sum(twall_array1)
        wt_tot3=sum(twt_array1)
        write(iout,10) 'Sum of time for all procs all computing tasks:',
     >  cpu_tot3,wall_tot3,wt_tot3
        acpu3=cpu_tot3/dble(num_tasks)
        awall3=wall_tot3/dble(num_tasks)
        awt3=wt_tot3/dble(num_tasks)
        write(iout,10) 'Average time per computing task:',
     >                 acpu3,awall3,awt3
        write(iout,*) 
        write(iout,12) 'Comparing the times between (1) and (2), we also
     > get the approximate average time per counter calling:',
     >  (awt1-awt3)*1.0d9
        write(iout,13) 
13      format(1x,'This data probably can not be used to evaluate'
     >  ,' the shared counter since some extra overhead is included.')
      end if
c... end of (2) shared counter test 

c... (3) shared counter test without computing tasks
c    2^31=2,147,483,648
      num_tasks=1000000
!      num_tasks=1000000000 !ARCCA
      do i=1,nprocs
        task_array(i)=0
      end do
      if(iprocs.eq.0) write(iout,*) 
      if(iprocs.eq.0) write(iout,*) 
     >  '(3) PPIDD shared counter test without computing tasks:'
      if(verbose) write(iout,30) 
     >            'Nprocs=',nprocs,'  Total tasks=', num_tasks
      call ppidd_barrier()
      call get_current_times(cpu1,wall1,wtime1)
      call ppidd_nxtval(nprocs,nval)
      do while (nval.lt.num_tasks)
        task_array(iprocs+1)=task_array(iprocs+1)+1
        call ppidd_nxtval(nprocs,nval)
      enddo
      call get_current_times(cpu2,wall2,wtime2)
      tcpu_array1(iprocs+1)=cpu2-cpu1
      twall_array1(iprocs+1)=wall2-wall1
      twt_array1(iprocs+1)=wtime2-wtime1
      call ppidd_barrier()

c... release the shared counter number in NXTVAL Server and set it to zero
      call ppidd_nxtval(-nprocs,junk)

c... broadcast the completed tasks on current process 
      do i=1,nprocs
        call ppidd_bcast(task_array(i),1,0,i-1)
        call ppidd_bcast(tcpu_array1(i), 1,1,i-1)
        call ppidd_bcast(twall_array1(i), 1,1,i-1)
        call ppidd_bcast(twt_array1(i), 1,1,i-1)
      end do
      call ppidd_barrier()

c... print out the summary
      if(iprocs.eq.0) then
        write(iout,30) 'Nprocs=',nprocs,'  Total tasks=', num_tasks
        write(iout,19)
        do i=1,nprocs
          if (task_array(i).ne.0) then
            twt_task=twt_array1(i)/dble(task_array(i))
          else
            twt_task=100000.0
          end if
          write(iout,20) i-1,task_array(i),twt_task
        end do
        write(iout,10) 'Time spent for iprocs=0:',
     >  tcpu_array1(1),twall_array1(1),twt_array1(1)
        cpu_tot4=sum(tcpu_array1)
        wall_tot4=sum(twall_array1)
        wt_tot4=sum(twt_array1)
        write(iout,10) 'Sum of time for all procs all empty tasks:',
     >  cpu_tot4,wall_tot4,wt_tot4
c... convert time to nanosecond
        acpu4=(cpu_tot4/dble(num_tasks))*1.0d9
        awall4=(wall_tot4/dble(num_tasks))*1.0d9
        awt4=(wt_tot4/dble(num_tasks))*1.0d9
        write(iout,11) 'Average time per shared counter calling:',
     >                 acpu4,awall4,awt4
11      format(1x,a,t49,' cpu=',f12.1,' nanosec,',' elapsed=',f12.1,
     >  ' nanosec,',' wtime=',f12.1,' nanosec')
        write(iout,111) 
111     format(1x,'The data in (3) probably can not be used to evaluate'
     >  ,' the shared counter since all callings may concentrate on',
     >  ' some processes.')
        write(iout,*) 'The data in (1) are much better.' 
      end if
c... end of (3) shared counter test 

c... release the memory in buffer space
      IF( ALLOCATED(task_array) ) DEALLOCATE( task_array)
      IF( ALLOCATED(sum_array) ) DEALLOCATE( sum_array)
      IF( ALLOCATED(tcpu_array1) ) DEALLOCATE( tcpu_array1)
      IF( ALLOCATED(twall_array1) ) DEALLOCATE( twall_array1)
      IF( ALLOCATED(twt_array1) ) DEALLOCATE( twt_array1)
      IF( ALLOCATED(twt_array2) ) DEALLOCATE( twt_array2)

#endif

      return
      end



      subroutine compute_task(N,nloop,pai)
c
c This simple subroutine approximates pi by computing pi = integral
c from 0 to 1 of 4/(1+x*x)dx which is approximated by sum from
c k=1 to N of 4 / ((1 + (k-1/2)**2 ).  The input data required are N and nloop.
c nloop is used only for duplicating computation
c The returned value sum is computed pi.
c
      implicit double precision (a-h,o-z)
      integer N,nloop
      double precision totsum,sum,pai
      integer i,j
      double precision err, f, pi, w
      f(x) = 4.0d0/(1.0d0+x*x)
      pi = 4.0d0*atan(1.0d0)
   
      totsum = 0.0d0
      err = 0.0d0
      do i=1,nloop
        sum = 0.0d0
        if (N .gt. 0) then
          w = 1.0d0/dble(N)
          do j = 1,N
             sum = sum + f((dble(j)-0.5d0)*w)
          enddo
          sum = sum * w
        else
          write(6,*) 'ERROR in running compute_task: N .le. 0. STOP'
          stop
        end if
        totsum=totsum+sum
      end do
      pai=totsum/dble(nloop)
      err = pai - pi
!      write(6,*) 'running compute_task, pai=',pai,' err=',err

      return
      end

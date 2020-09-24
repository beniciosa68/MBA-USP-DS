! src/mpp/ppidd_test.F $Revision: 2008.3 $
!      program sizeoffortype
      subroutine sizeoffortype(nbyte_int,nbyte_log,nbyte_dbl,nbyte_real)
      implicit double precision (a-h,o-z)
      integer nbyte_int,nbyte_log,nbyte_dbl,nbyte_real
      integer, dimension(2)          :: array_int = 0
      logical, dimension(2)          :: array_log = .true.
      real, dimension(2)             :: array_real= 0.0
      double precision, dimension(2) :: array_dbl = 0.0d0

      nbyte_int=loc(array_int(2))-loc(array_int(1))
      nbyte_log=loc(array_log(2))-loc(array_log(1))
      nbyte_dbl=loc(array_dbl(2))-loc(array_dbl(1))
      nbyte_real=loc(array_real(2))-loc(array_real(1))

      end

! { dg-do run }
! PR77393
program testbigf0 ! Can enormous numbers be printed with F0.0 format?
  use ISO_FORTRAN_ENV
  implicit none
  integer i
  integer, parameter :: j(size(real_kinds)+4)=[REAL_KINDS, [4, 4, 4, 4]]
  character(10000) :: str

  do i=1,size(real_kinds)
    select case (i)
    case (1)
      write(str, "(f0.0)") -huge(real(1.0,kind=j(1)))
      if (len(trim(str)).lt.41) error stop "FAILED AT LINE 15"
    case (2)
      write(str, "(f0.0)") -huge(real(1.0,kind=j(2)))
      if (len(trim(str)).lt.311) error stop "FAILED AT LINE 19"
    case (3)
      write(str, "(f0.0)") -huge(real(1.0,kind=j(3)))
      if (len(trim(str)).lt.4935) error stop "FAILED AT LINE 23"
    case (4)
      write(str, "(f0.10)") -huge(real(1.0,kind=j(4)))
      if (len(trim(str)).lt.4945) error stop "FAILED AT LINE 27"
    end select
  enddo
end program testbigf0
  

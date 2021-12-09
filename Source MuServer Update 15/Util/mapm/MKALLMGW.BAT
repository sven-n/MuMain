rem
rem  mkallmgw.bat
rem
rem  Build batch file for MINGW compiler
rem  Tested with version mingw-1.0.1-20010726
rem  (This package contains GCC version 2.95.3)
rem
rem  also tested with gcc 3.2    (mingw special 20020817-1)
rem  also tested with gcc 3.2.3  (mingw special 20030504-1)
rem
rem  note that each file is compiled separately
rem  since this compiler did not support wildcards
rem  (map*.c) on my system.
rem
rem  compile the MAPM library files
rem
del map*.o
rem
gcc -v
rem
gcc -c -Wall -O2 mapm_add.c
gcc -c -Wall -O2 mapm_cpi.c
gcc -c -Wall -O2 mapm_div.c
gcc -c -Wall -O2 mapm_exp.c
gcc -c -Wall -O2 mapm_fam.c
gcc -c -Wall -O2 mapm_fft.c
gcc -c -Wall -O2 mapm_flr.c
gcc -c -Wall -O2 mapm_fpf.c
gcc -c -Wall -O2 mapm_gcd.c
gcc -c -Wall -O2 mapm_log.c
gcc -c -Wall -O2 mapm_lg2.c
gcc -c -Wall -O2 mapm_lg3.c
gcc -c -Wall -O2 mapm_lg4.c
gcc -c -Wall -O2 mapm_mul.c
gcc -c -Wall -O2 mapm_pow.c
gcc -c -Wall -O2 mapm_rcp.c
gcc -c -Wall -O2 mapm_rnd.c
gcc -c -Wall -O2 mapm_set.c
gcc -c -Wall -O2 mapm_sin.c
gcc -c -Wall -O2 mapm5sin.c
gcc -c -Wall -O2 mapmasin.c
gcc -c -Wall -O2 mapmasn0.c
gcc -c -Wall -O2 mapmcbrt.c
gcc -c -Wall -O2 mapmcnst.c
gcc -c -Wall -O2 mapmfact.c
gcc -c -Wall -O3 mapmfmul.c
gcc -c -Wall -O2 mapmgues.c
gcc -c -Wall -O2 mapmhasn.c
gcc -c -Wall -O2 mapmhsin.c
gcc -c -Wall -O2 mapmipwr.c
gcc -c -Wall -O2 mapmistr.c
gcc -c -Wall -O2 mapmpwr2.c
gcc -c -Wall -O2 mapmrsin.c
gcc -c -Wall -O2 mapmsqrt.c
gcc -c -Wall -O2 mapmstck.c
gcc -c -Wall -O3 mapmutil.c
gcc -c -Wall -O2 mapmutl1.c
gcc -c -Wall -O2 mapmutl2.c
rem
rem
rem build the library and delete object files
rem
del libmapm.a
ar rc libmapm.a map*.o
del map*.o
rem
rem
rem  build the 4 demo programs
rem
rem
gcc -Wall -O2 -o calc.exe calc.c libmapm.a -s -lm
gcc -Wall -O2 -o validate.exe validate.c libmapm.a -s -lm
gcc -Wall -O2 -o primenum.exe primenum.c libmapm.a -s -lm
rem
gcc -Wall -O2 -o cpp_demo.exe cpp_demo.cpp libmapm.a -s -lm -lstdc++
rem
rem
rem

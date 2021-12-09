rem *
rem *
rem *   Build file for GCC under DOS / Win NT/9x
rem *
rem *   This is for DJ Delories DJGPP
rem *   Version of DJGPP ??, but GCC is 2.95.2
rem *
del libmapm.a
del mapm*.o
rem *
gcc -v
rem *
gcc -c -Wall -O2 mapm*.c
rem *
del mapmutil.o
del mapmfmul.o
rem *
rem *   Use better optimizations for these two ...
rem *
gcc -c -Wall -O3 mapmutil.c
gcc -c -Wall -O3 mapmfmul.c
rem *
ar rc libmapm.a mapm*.o
rem *
del mapm*.o
rem *
gcc -Wall -O2 calc.c libmapm.a -s -o calc.exe -lm
gcc -Wall -O2 validate.c libmapm.a -s -o validate.exe -lm
gcc -Wall -O2 primenum.c libmapm.a -s -o primenum.exe -lm
rem *
rem *
rem *         to compile and link the C++ demo program
rem *         using the C++ MAPM wrapper class.
rem *         (default is commented out)
rem *
rem * gxx cpp_demo.cpp libmapm.a -s -o cpp_demo.exe -lm
rem *

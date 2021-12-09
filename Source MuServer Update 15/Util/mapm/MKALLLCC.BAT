rem *
rem *
rem *   Build file for LCC-WIN32  (Ver 3.2)
rem *   builds library and 3 demo executables
rem *
rem *
del mapm.lib
del mapm*.obj
rem *
rem *   build the library 'mapm.lib'
rem *
lc -c -O mapm*.c
rem *
lcclib /out:mapm.lib mapm*.obj
del mapm*.obj
rem *
rem *
rem *   now build the demo programs
rem *
lc -c -O validate.c
lcclnk -s -o validate.exe validate.obj mapm.lib
del validate.obj
rem *
rem *
lc -c -O calc.c
lcclnk -s -o calc.exe calc.obj mapm.lib
del calc.obj
rem *
rem *
lc -c -O primenum.c
lcclnk -s -o primenum.exe primenum.obj mapm.lib
del primenum.obj
rem *

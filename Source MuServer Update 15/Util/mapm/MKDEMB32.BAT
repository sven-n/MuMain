rem *
rem *
rem *   executable build file for Borland 5.2 & 5.5 command line
rem *
rem *   some warnings are normal 
rem *
rem *   [*** edit path names as necessary ***]
rem *
rem *
bcc32 -I"C:\borland\bcc55\include" -L"C:\borland\bcc55\lib" calc.c mapm.lib
rem *
rem *
bcc32 -I"C:\borland\bcc55\include" -L"C:\borland\bcc55\lib" validate.c mapm.lib
rem *
rem *
bcc32 -I"C:\borland\bcc55\include" -L"C:\borland\bcc55\lib" primenum.c mapm.lib
rem *
rem *
bcc32 -w-inl -I"C:\borland\bcc55\include" -L"C:\borland\bcc55\lib" cpp_demo.cpp mapm.lib
rem *
rem *

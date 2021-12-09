@echo off
rem 
rem =============================================================
rem Creating MAPM.LIB with Watcom C 11.x compiler (flat mode)
rem This should now work with Open Watcom 1.0
rem =============================================================
rem 
rem (1) Prepare for compile
      del map*.obj
      del mapm.lib
rem 
rem (2) Compiles each map*.c and creates map*.obj:
rem  
      for %%1 in (map*.c) do wcl386 -c -ox %%1
rem 
rem (3) Creates MAPM.LIB:
rem
      for %%1 in (map*.obj) do wlib mapm.lib +%%1
rem
      del map*.obj
rem
rem (4) Compiles and links validate.c, calc.c, primenum.c, cpp_demo.cpp:
rem
rem     Depending on your setup, you may not need the dos4g switch
rem     If you have link errors, try deleting /l=dos4g and link again.
rem 
      wcl386 -ox /l=dos4g validate.c mapm.lib
      wcl386 -ox /l=dos4g calc.c mapm.lib
      wcl386 -ox /l=dos4g primenum.c mapm.lib
      wcl386 -ox /l=dos4g cpp_demo.cpp mapm.lib
rem
rem (5) Final Cleaning
rem 
      del *.obj
      del *.bak
rem
rem -------
rem Andrew V. Stromnov, stromnov@mailru.com (using mkalltcc.bat as a template)
rem

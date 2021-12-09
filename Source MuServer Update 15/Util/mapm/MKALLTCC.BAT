@echo off
rem 
rem ============================================================
rem Creating MAPM.LIB with Borland's TURBO C++ 3.00 compiler
rem ============================================================
rem 
      del *.obj
      del mapm.lib
rem
rem (1) Compiles each map*.c and creates map*.obj:
rem  
      for %%1 in (map*.c) do tcc -c -N -ml -O -G %%1
rem 
rem (2) Creates mapm.LIB:
rem
      for %%1 in (map*.obj) do tlib /C /E mapm.lib +%%1
rem
      del *.obj
      del *.bak
rem 
rem (3) Compiles and links validate.c, calc.c and primenum.c:
rem 
      tcc -ml -O -N -G validate.c mapm.lib
      tcc -ml -O -N -G calc.c mapm.lib
      tcc -ml -O -N -G primenum.c mapm.lib
rem 
rem 
rem ------------------------ Remarks ---------------------------
rem 
rem   (1) Some of the Turbo C++ 3.00 options are:
rem 
rem       -2      80286 Protected Mode Inst.
rem       -Dxxx   Define macro (e.g., -DMSDOS )
rem       -G      Generate for speed
rem       -N      Check stack overflow 
rem       -O      Optimize jumps
rem       -c      Compile only
rem       -ml     Set Large Memory Model          
rem
rem    
rem   -------
rem      Lenimar N. Andrade, lenimar@mat.ufpb.br
rem 

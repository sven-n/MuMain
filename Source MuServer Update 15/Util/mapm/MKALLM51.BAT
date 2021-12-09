@echo off
rem
rem ============================================================
rem   Creating MAPM.LIB with Micro$oft C 5.1 (16 bit .EXE)
rem ============================================================
rem
      del *.obj
      del mapm.lib
rem
rem (1) Compiles each map*.c and creates map*.obj:
rem
      for %%1 in (map*.c) do cl /c /AL /NT MAPM_TEXT /Gs /Zl %%1
rem
rem
      del mapmfmul.obj
      del mapmutil.obj
rem
rem
rem   if you are using Microsoft C 6.0 (16 BIT), delete
rem   the /Oi options below if you have an internal 
rem   compiler error.
rem
      cl /c /AL /NT MAPM_TEXT /Oi /Gs /Zl mapmfmul.c
      cl /c /AL /NT MAPM_TEXT /Oi /Gs /Zl mapmutil.c
rem
rem (2) Creates MAPM.LIB:
rem
      for %%1 in (map*.obj) do lib mapm.lib +%%1;
rem
      del map*.obj
      del mapm.bak
rem
rem (3) Compiles and links validate.c, calc.c and primenum.c:
rem
rem
      cl /c /AL /Gs calc.c
      link /ST:4096 /NOI /CPARM:1 /MAP /FARCA /PACKCO calc,,,mapm.lib;
      del calc.obj
rem
rem
      cl /c /AL /Gs validate.c
      link /ST:4096 /NOI /CPARM:1 /MAP /FARCA /PACKCO validate,,,mapm.lib;
      del validate.obj
rem
rem
      cl /c /AL /Gs primenum.c
      link /ST:4096 /NOI /CPARM:1 /MAP /FARCA /PACKCO primenum,,,mapm.lib;
      del primenum.obj
rem
rem
rem -------
rem      by M. Ring using 'mkalltcc.bat' as template
rem

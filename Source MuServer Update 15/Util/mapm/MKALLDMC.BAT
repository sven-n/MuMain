rem
rem  mkalldmc.bat
rem
rem  Build batch file for Digital Mars Compiler
rem  Tested with version 8.49 (8.42n from command line output)
rem
rem
rem  compile the MAPM library files
rem
del map*.obj
rem
dmc -c -o -wx mapm_add.c
dmc -c -o -wx mapm_cpi.c
dmc -c -o -wx mapm_div.c
dmc -c -o -wx mapm_exp.c
dmc -c -o -wx mapm_fam.c
dmc -c -o -wx -w7 mapm_fft.c
dmc -c -o -wx mapm_flr.c
dmc -c -o -wx mapm_fpf.c
dmc -c -o -wx mapm_gcd.c
dmc -c -o -wx mapm_log.c
dmc -c -o -wx mapm_lg2.c
dmc -c -o -wx mapm_lg3.c
dmc -c -o -wx mapm_lg4.c
dmc -c -o -wx mapm_mul.c
dmc -c -o -wx mapm_pow.c
dmc -c -o -wx mapm_rcp.c
dmc -c -o -wx mapm_rnd.c
dmc -c -o -wx mapm_set.c
dmc -c -o -wx mapm_sin.c
dmc -c -o -wx mapm5sin.c
dmc -c -o -wx mapmasin.c
dmc -c -o -wx mapmasn0.c
dmc -c -o -wx mapmcbrt.c
dmc -c -o -wx mapmcnst.c
dmc -c -o -wx mapmfact.c
dmc -c -o -wx mapmfmul.c
dmc -c -o -wx mapmgues.c
dmc -c -o -wx mapmhasn.c
dmc -c -o -wx mapmhsin.c
dmc -c -o -wx mapmipwr.c
dmc -c -o -wx mapmistr.c
dmc -c -o -wx mapmpwr2.c
dmc -c -o -wx mapmrsin.c
dmc -c -o -wx mapmsqrt.c
dmc -c -o -wx mapmstck.c
dmc -c -o -wx mapmutil.c
dmc -c -o -wx mapmutl1.c
dmc -c -o -wx mapmutl2.c
rem
rem
rem build the library and delete object files
rem
del mapm.lib
rem
lib -c mapm mapm_add.obj mapm_cpi.obj mapm_div.obj mapm_exp.obj mapm_fam.obj
lib -n mapm mapm_fft.obj mapm_flr.obj mapm_fpf.obj mapm_gcd.obj mapm_log.obj
lib -n mapm mapm_lg2.obj mapm_lg3.obj mapm_lg4.obj mapm_mul.obj mapm_pow.obj
lib -n mapm mapm_rcp.obj mapm_rnd.obj mapm_set.obj mapm_sin.obj mapm5sin.obj
lib -n mapm mapmasin.obj mapmasn0.obj mapmcbrt.obj mapmcnst.obj mapmfact.obj
lib -n mapm mapmfmul.obj mapmgues.obj mapmhasn.obj mapmhsin.obj mapmipwr.obj
lib -n mapm mapmistr.obj mapmpwr2.obj mapmrsin.obj mapmsqrt.obj mapmstck.obj
lib -n mapm mapmutil.obj mapmutl1.obj mapmutl2.obj
rem
del map*.obj
rem
rem
rem  build the 3 demo programs
rem
rem
dmc -c -o -wx calc.c 
link calc mapm.lib
del  calc.obj
rem
dmc -c -o -wx primenum.c 
link primenum mapm.lib
del  primenum.obj
rem
dmc -c -o -wx validate.c 
link validate mapm.lib
del  validate.obj
rem
rem

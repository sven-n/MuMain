
/*
 *  M_APM  -  validate.c
 *
 *  Copyright (C) 1999 - 2007   Michael C. Ring
 *
 *  Permission to use, copy, and distribute this software and its
 *  documentation for any purpose with or without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and
 *  that both that copyright notice and this permission notice appear
 *  in supporting documentation.
 *
 *  Permission to modify the software is granted. Permission to distribute
 *  the modified code is granted. Modifications are to be distributed by
 *  using the file 'license.txt' as a template to modify the file header.
 *  'license.txt' is available in the official MAPM distribution.
 *
 *  This software is provided "as is" without express or implied warranty.
 */

/*
 *      $Id: validate.c,v 2.6 2007/12/03 02:01:56 mike Exp $
 *
 *      This file contains the validation test program. It compares the
 *	M_APM library to the standard C lbrary math functions. 	It also
 *	does a number of high precision calculations and compares the
 *	results to known quantities, like PI, log(2), etc.
 *
 *      $Log: validate.c,v $
 *      Revision 2.6  2007/12/03 02:01:56  mike
 *      Update license
 *
 *      Revision 2.5  2003/12/29 05:16:35  mike
 *      create exp_2 to exercise new code in _exp
 *
 *      Revision 2.4  2003/07/28 19:37:47  mike
 *      change 16 bit constant
 *
 *      Revision 2.3  2003/07/26 19:30:21  mike
 *      end one test early in 16 bit environments.
 *
 *      Revision 2.2  2003/07/25 18:08:53  mike
 *      fix pow test when x==0
 *
 *      Revision 2.1  2003/07/24 21:11:48  mike
 *      update comments
 *
 *      Revision 2.0  2003/07/24 21:00:49  mike
 *      initial version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "m_apm.h"

extern   void	  factorial_local(M_APM, M_APM);
extern   double   cbrt_local(double);
extern   double   asinh_local(double);
extern   double   acosh_local(double);
extern   double   atanh_local(double);
extern   M_APM    M_get_stack_var(void);
extern   void     M_restore_stack(int);
extern   void     M_log_AGM_R_func(M_APM, int, M_APM, M_APM);
extern	 int	  M_get_sizeof_int(void);

#define  DECIMAL_PLACES 22

/****************************************************************************/

void	library_version_check()
{
char	version_info[80];

m_apm_lib_short_version(version_info);
m_apm_lib_version(version_info);

fprintf(stdout,"\n%s\n\n",version_info);

/*
 *  check MAPM version info from m_apm.h to what was compiled into
 *  the library. This check could be used in your application to
 *  confirm that you are using the correct include file (m_apm.h).
 */

if (strcmp(version_info, MAPM_LIB_VERSION) != 0)
  {
   fprintf(stdout,
      "ERROR: MAPM Library compiled with different include file\n");

   fprintf(stdout, "Include file version : %s \n", MAPM_LIB_SHORT_VERSION);
   fprintf(stdout, "Library file version : %s \n",
   			m_apm_lib_short_version(version_info));
   exit(2);
  }
}

/****************************************************************************/

/*
 *      in case some compilers don't have a native cube root or
 *      inverse hyperbolic functions, we'll just make our own.
 *
 *      note that we are not doing any decent error checking on
 *      the inputs, these functions are here just to support this
 *	program.
 */

double  cbrt_local(double x)
{
if (x == 0.0)
  return(0.0);
else
  {
   if (x < 0.0)
     return(-pow(-x, 0.333333333333333333));
   else
     return(pow(x, 0.333333333333333333));
  }
}

/****************************************************************************/

double  asinh_local(double x)
{
return(log(x + sqrt(x * x + 1.0)));
}

/****************************************************************************/

double  acosh_local(double x)
{
if (x >= 1.0)
  return(log(x + sqrt(x * x - 1.0)));
else
  return(0.0);
}

/****************************************************************************/

double  atanh_local(double x)
{
if (fabs(x) < 1.0)
  return(0.5 * log((1.0 + x) / (1.0 - x)));
else
  return(0.0);
}

/****************************************************************************/
/*
 *
 *      slow but obvious version for testing
 *
 */
void	factorial_local(M_APM result, M_APM ainput)
{
M_APM	ftmp1, ftmp2, ftmp3;

if (m_apm_compare(ainput, MM_One) <= 0)
  {
   m_apm_copy(result, MM_One);
   return;
  }

ftmp1 = m_apm_init();
ftmp2 = m_apm_init();
ftmp3 = m_apm_init();

m_apm_copy(result, ainput);
m_apm_copy(ftmp1, ainput);

while (1)
  {
   m_apm_subtract(ftmp2, ftmp1, MM_One);
   m_apm_multiply(ftmp3, ftmp2, result);

   if (m_apm_compare(ftmp2, MM_Two) <= 0)
     {
      m_apm_copy(result, ftmp3);
      break;
     }

   m_apm_subtract(ftmp1, ftmp2, MM_One);
   m_apm_multiply(result, ftmp3, ftmp1);

   if (m_apm_compare(ftmp1, MM_Two) <= 0)
     break;
  }

m_apm_free(ftmp3);
m_apm_free(ftmp2);
m_apm_free(ftmp1);
}

/****************************************************************************/

/*
 *      Traditional GCD from Knuth, The Art of Computer Programming:
 *
 *      To compute GCD(u,v)
 *
 *      A1:
 *	    if (v == 0)  return (u)
 *      A2:
 *          t = u mod v
 *	    u = v
 *	    v = t
 *	    goto A1
 */

void	gcd_local(M_APM r, M_APM u, M_APM v)
{
M_APM   tmpD, tmpN, tmpU, tmpV;

tmpD = M_get_stack_var();
tmpN = M_get_stack_var();
tmpU = M_get_stack_var();
tmpV = M_get_stack_var();

m_apm_absolute_value(tmpU, u);
m_apm_absolute_value(tmpV, v);

while (1)
  {
   if (tmpV->m_apm_sign == 0)
     break;

   m_apm_integer_div_rem(tmpD, tmpN, tmpU, tmpV);
   m_apm_copy(tmpU, tmpV);
   m_apm_copy(tmpV, tmpN);
  }

m_apm_copy(r, tmpU);
M_restore_stack(4);
}

/****************************************************************************/

/*
 *	calculate log using an AGM algorithm.
 */

void	log_local(M_APM outval, int places, M_APM inval)
{
int     dplaces;
M_APM   tmp6, tmp7, tmp8, tmp9;

dplaces = places + 8;

tmp6 = M_get_stack_var();
tmp7 = M_get_stack_var();
tmp8 = M_get_stack_var();
tmp9 = M_get_stack_var();

m_apm_copy(tmp7, MM_One);
tmp7->m_apm_exponent = -places;

M_log_AGM_R_func(tmp8, dplaces, MM_One, tmp7);

m_apm_divide(tmp6, dplaces, tmp7, inval);

M_log_AGM_R_func(tmp9, dplaces, MM_One, tmp6);

m_apm_subtract(tmp6, tmp9, tmp8);
m_apm_round(outval, places, tmp6);

M_restore_stack(4);
}

/****************************************************************************/

void	mapm_show_random()
{
int     i, j;
char    buffer2[32];
M_APM   mapm1;

mapm1 = m_apm_init();

fprintf(stdout,"Output the first 10 random numbers ...\n");

for (i=1; i <= 10; i++)
  {
   m_apm_get_random(mapm1);
   m_apm_to_string(buffer2, 14, mapm1);

   fprintf(stdout,"Random Num %2d : %s \n",i,buffer2);
  }

for (j=0; j < 2; j++)
  {
   fprintf(stdout,
      "Set random seed to 2003 and output next 5 random numbers\n");

   m_apm_set_random_seed("2003");

   for (i=1; i <= 5; i++)
     {
      m_apm_get_random(mapm1);
      m_apm_to_string(buffer2, 14, mapm1);

      fprintf(stdout,"Random Num %2d : %s \n",i,buffer2);
     }
  }

m_apm_free(mapm1);
}

/****************************************************************************/

int	mapm_test_sqrt()
{
int     pass;
double  xx, xx1, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32], cbuf[32];
M_APM   mapm1, mapm2;

fprintf(stdout,"Validating the SQRT function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = 3.184E-7;
xdelta    = 2.83576;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   sprintf(cbuf, "%.6E", xx);
   xx1 = atof(cbuf);

   yy = sqrt(xx1);

   m_apm_set_string(mapm1, cbuf);
   m_apm_sqrt(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout, "x    = %17.10E   C-math-lib = %.15E \n", xx1, yy);
      fprintf(stdout, "DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 3.4E6)
     break;

   xx *= xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... SQRT function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_cbrt()
{
int     sflag, pass;
double  xx, xx1, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32], cbuf[32];
M_APM   mapm1, mapm2;

fprintf(stdout,"Validating the CBRT function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = 3.9521E-8;
xdelta    = 2.71329;
tolerance = 1.0E-13;
pass      = 0;
sflag     = 1;

while (1)
  {
   sprintf(cbuf, "%.6E", (sflag * xx));
   xx1 = atof(cbuf);

   yy = cbrt_local(xx1);

   m_apm_set_string(mapm1, cbuf);
   m_apm_cbrt(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout, "x    = %17.10E   C-math-lib = %.15E \n", xx1, yy);
      fprintf(stdout, "DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 4.1E7)
     break;

   xx *= xdelta;
   sflag = -sflag;
  }

if (pass == 0)
  fprintf(stdout,"... CBRT function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_sin()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the SIN function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -7.214;
xdelta    = 0.817;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = sin(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_sin(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   xx += xdelta;
   if (xx > 7.21)
     break;
  }

if (pass == 0)
  fprintf(stdout,"... SIN function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_cos()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the COS function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -7.315;
xdelta    = 0.813;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = cos(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_cos(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   xx += xdelta;
   if (xx > 7.21)
     break;
  }

if (pass == 0)
  fprintf(stdout,"... COS function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_sin_cos()
{
int     pass;
double  xx, xdelta, yy_sin, yy_cos, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, sinval, cosval;

fprintf(stdout, "Validating the SIN_COS function ... \n");

mapm1  = m_apm_init();
sinval = m_apm_init();
cosval = m_apm_init();

xx        = -7.715;
xdelta    = 0.737;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy_sin = sin(xx);
   yy_cos = cos(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_sin_cos(sinval, cosval, DECIMAL_PLACES, mapm1);

   m_apm_to_string(buffer, DECIMAL_PLACES, sinval);
   yy_mapm_lib = atof(buffer);

   ydiff = yy_mapm_lib - yy_sin;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n",xx,yy_sin);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n",ydiff,buffer);
     }

   m_apm_to_string(buffer, DECIMAL_PLACES, cosval);
   yy_mapm_lib = atof(buffer);

   ydiff = yy_mapm_lib - yy_cos;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n",xx,yy_cos);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n",ydiff,buffer);
     }

   xx += xdelta;
   if (xx > 7.61)
     break;
  }

if (pass == 0)
  fprintf(stdout,"... SIN_COS function passes\n");

m_apm_free(mapm1);
m_apm_free(sinval);
m_apm_free(cosval);

return(pass);
}

/****************************************************************************/

int	mapm_test_tan()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the TAN function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -7.315;
xdelta    = 0.673;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = tan(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_tan(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   xx += xdelta;
   if (xx > 7.21)
     break;
  }

if (pass == 0)
  fprintf(stdout,"... TAN function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_asin()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the ASIN function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -0.997;
xdelta    = 0.1073;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = asin(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_asin(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   xx += xdelta;
   if (xx > 0.99)
     break;
  }

if (pass == 0)
  fprintf(stdout,"... ASIN function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_acos()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the ACOS function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -0.998;
xdelta    = 0.09373;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = acos(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_acos(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   xx += xdelta;
   if (xx > 0.99)
     break;
  }

if (pass == 0)
  fprintf(stdout,"... ACOS function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_atan()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the ATAN function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -6.8215;
xdelta    = 0.3967;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = atan(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_atan(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 6.5)
     break;

   xx += xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... ATAN function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_atan2()
{
int     i, j, pass;
double  xx1, xx2, yy, yy_mapm_lib, ydiff, tolerance;
double  xarray[10], yarray[10];
char    buffer[32];
M_APM   mapmX, mapmY, mapm0;

fprintf(stdout, "Validating the ATAN2 function ... \n");

mapm0 = m_apm_init();
mapmX = m_apm_init();
mapmY = m_apm_init();

pass      = 0;
tolerance = 1.0E-13;

xarray[1] = -1.638;
xarray[2] = -2.179;
xarray[3] = 0.0;
xarray[4] = 0.842;
xarray[5] = 1.976;
xarray[6] = 9.237;
xarray[7] = -8.324;
xarray[8] = -1.287E-9;
xarray[9] = 74.861;

yarray[1] = -0.374;
yarray[2] = -4.127;
yarray[3] = 0.0;
yarray[4] = 0.732;
yarray[5] = 3.261;
yarray[6] = -6.305;
yarray[7] = 11.427;
yarray[8] = -0.8827;
yarray[9] = 7.385E10;

for (i=1; i <= 9; i++)
  {
   for (j=1; j <= 9; j++)
     {
      if (i != 3 || j != 3)        /* domain error if both == 0.0 */
        {
         xx1 = xarray[i];
         xx2 = yarray[j];

         yy = atan2(xx2, xx1);

         m_apm_set_double(mapmX, xx1);
         m_apm_set_double(mapmY, xx2);
         m_apm_atan2(mapm0, DECIMAL_PLACES, mapmY, mapmX);
         m_apm_to_string(buffer, DECIMAL_PLACES, mapm0);
         yy_mapm_lib = atof(buffer);

         ydiff = yy_mapm_lib - yy;
   
         if (fabs(ydiff) > tolerance)
           {
            pass = 1;

            fprintf(stdout,
            "x2 = %10.3E  x1 = %10.3E  C-math-lib = %.15E \n",xx2,xx1,yy);
            fprintf(stdout,
            "DIFF = %17.10E          M_APM_LIB  = %s \n",ydiff,buffer);
           }
        }
     }
  }

if (pass == 0)
  fprintf(stdout,"... ATAN2 function passes\n");

m_apm_free(mapm0);
m_apm_free(mapmX);
m_apm_free(mapmY);

return(pass);
}

/****************************************************************************/

int	mapm_test_sinh()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the SINH function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -4.2215;
xdelta    = 0.3967;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = sinh(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_sinh(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 4.2)
     break;

   xx += xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... SINH function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_cosh()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the COSH function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -4.2215;
xdelta    = 0.3816;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = cosh(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_cosh(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 4.2)
     break;

   xx += xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... COSH function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_tanh()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the TANH function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -2.1083;
xdelta    = 0.10769;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = tanh(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_tanh(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 2.1)
     break;

   xx += xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... TANH function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_asinh()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the ASINH function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -27.2215;
xdelta    = 3.1817;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = asinh_local(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_asinh(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 27.2)
     break;

   xx += xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... ASINH function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_acosh()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the ACOSH function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = 1.02;
xdelta    = 0.7816;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = acosh_local(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_acosh(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 30.0)
     break;

   xx += xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... acosh function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_atanh()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the ATANH function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -0.998;
xdelta    = 0.1167;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = atanh_local(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_atanh(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   xx += xdelta;

   if (xx > 0.99)
     break;
  }

if (pass == 0)
  fprintf(stdout,"... ATANH function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_exp()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the EXP function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = -42.215;
xdelta    = 3.387;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = exp(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_exp(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   ydiff = (yy_mapm_lib - yy) / yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 44.2)
     break;

   xx += xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... EXP function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_exp_2()
{
int     kk, dplaces, pass;
M_APM   mapm1, mapm2, mapm3, mapm4;
char	*cp;
static  char *str1[4] = { "5.2107E-4", "-7.138927E-5", "8.21735E-6", "0.0" };
static  char *str2[4] = { "128.00047", "511.99874", ".03125012", ".01562483" };

fprintf(stdout,
        "Validating calculations involving EXP & LOG ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapm3 = m_apm_init();
mapm4 = m_apm_init();
pass  = 0;

dplaces = 250;

for (kk=0; kk < 4; kk++)
  {
   cp = str1[kk];

   m_apm_set_string(mapm4, cp);
   
   fprintf(stdout,
     "Calculate log(exp(%s)) to %d decimal places ... \n", cp, dplaces);
   
   m_apm_exp(mapm3, (dplaces + 8), mapm4);
   m_apm_add(mapm1, mapm3, MM_Zero);
   m_apm_log(mapm2, dplaces, mapm1);
   m_apm_add(mapm1, MM_Zero, mapm2);
   
   if (m_apm_compare(mapm1, mapm4) == 0)
     {
      fprintf(stdout, "Verified %s == log(exp(%s)) \n", cp, cp);
     }
   else
     {
      pass = 1;
      fprintf(stdout, 
        "***** FAILED to verify %s == log(exp(%s)) \n", cp, cp);
     }
  }


for (kk=0; kk < 4; kk++)
  {
   cp = str2[kk];

   m_apm_set_string(mapm1, cp);    /* close to exact power of 2 */
   
   fprintf(stdout,
     "Calculate exp(log(%s)) to %d decimal places ... \n", cp, dplaces);
   
   m_apm_log(mapm2, (dplaces + 8), mapm1);
   m_apm_exp(mapm3, dplaces, mapm2);
   
   if (m_apm_compare(mapm1, mapm3) == 0)
     {
      fprintf(stdout, "Verified %s == exp(log(%s)) \n", cp, cp);
     }
   else
     {
      pass = 1;
      fprintf(stdout, 
        "***** FAILED to verify %s == exp(log(%s)) \n", cp, cp);
     }
  }


if (pass == 0)
  fprintf(stdout,"... EXP & LOG Calculations pass\n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapm4);

return(pass);
}

/****************************************************************************/

int	mapm_test_log()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the LOG function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = 1.7235E-20;
xdelta    = 8.3816;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = log(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_log(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 2.5E20)
     break;

   xx *= xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... LOG function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_log10()
{
int     pass;
double  xx, xdelta, yy, yy_mapm_lib, ydiff, tolerance;
char    buffer[32];
M_APM   mapm1, mapm2;

fprintf(stdout, "Validating the LOG10 function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();

xx        = 3.4235E-22;
xdelta    = 12.7816;
tolerance = 1.0E-13;
pass      = 0;

while (1)
  {
   yy = log10(xx);

   m_apm_set_double(mapm1, xx);
   m_apm_log10(mapm2, DECIMAL_PLACES, mapm1);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm2);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,"x    = %17.10E   C-math-lib = %.15E \n", xx, yy);
      fprintf(stdout,"DIFF = %17.10E   M_APM_LIB  = %s \n", ydiff, buffer);
     }

   if (xx > 2.5E22)
     break;

   xx *= xdelta;
  }

if (pass == 0)
  fprintf(stdout,"... LOG10 function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_pow()
{
int     i, pass;
double  xx1, xx2, yy, yy_mapm_lib, ydiff, tolerance;
double  xarray[22], yarray[22];
char    buffer[32];
M_APM   mapm0, mapmX, mapmY;

fprintf(stdout, "Validating the POW function ... \n");

mapm0 = m_apm_init();
mapmX = m_apm_init();
mapmY = m_apm_init();

pass      = 0;
tolerance = 1.0E-13;

xarray[1]  = 1.638;         yarray[1]  = 7.14;
xarray[2]  = 8.321e-12;     yarray[2]  = -5.2;
xarray[3]  = 1.0;           yarray[3]  = 4.19;
xarray[4]  = 7.3231e17;     yarray[4]  = 2.11;
xarray[5]  = 2.0;           yarray[5]  = 32.0;
xarray[6]  = 9.997e4;       yarray[6]  = -1.0;
xarray[7]  = 1.0528e3;      yarray[7]  = -3.83e-2;
xarray[8]  = 6.319e-6;      yarray[8]  = 3.17;
xarray[9]  = 2.108e9;       yarray[9]  = -2.0;
xarray[10] = 8.417e-3;      yarray[10] = 0.5;
xarray[11] = 8.417e-3;      yarray[11] = -6.2109;
xarray[12] = 5.347e7;       yarray[12] = 1.0;
xarray[13] = 10.0;          yarray[13] = -17.0;
xarray[14] = 10.0;          yarray[14] = 23.37;
xarray[15] = 9.0e2;         yarray[15] = 0.0;
xarray[16] = 2.0;           yarray[16] = -3.62;
xarray[17] = 2.0;           yarray[17] = 23.017;
xarray[18] = 8.27693;       yarray[18] = 5.19832;
xarray[19] = 0.0;           yarray[19] = 32.02384;
xarray[20] = 78.23;         yarray[20] = 9.2104;

for (i=1; i <= 20; i++)
  {
   xx1 = xarray[i];
   xx2 = yarray[i];
   yy  = pow(xx1, xx2);

   m_apm_set_double(mapmX, xx1);
   m_apm_set_double(mapmY, xx2);
   m_apm_pow(mapm0, DECIMAL_PLACES, mapmX, mapmY);
   m_apm_to_string(buffer, DECIMAL_PLACES, mapm0);
   yy_mapm_lib = atof(buffer);

   if (fabs(yy) > 1.0)
     ydiff = (yy_mapm_lib - yy) / yy;
   else
     ydiff = yy_mapm_lib - yy;

   if (fabs(ydiff) > tolerance)
     {
      pass = 1;
      fprintf(stdout,
      "x1 = %10.3E  x2 = %10.3E  C-math-lib = %.15E \n",xx1,xx2,yy);
       fprintf(stdout,
      "DIFF = %17.10E          M_APM_LIB  = %s \n",ydiff,buffer);
     }
  }

if (pass == 0)
  fprintf(stdout,"... POW function passes\n");

m_apm_free(mapm0);
m_apm_free(mapmX);
m_apm_free(mapmY);

return(pass);
}

/****************************************************************************/

int	mapm_test_factorial()
{
int     i, pass;
M_APM   mapm1, mapm2, mapmi;

fprintf(stdout, "Validating the FACTORIAL function ... \n");

pass = 0;

mapmi = m_apm_init();
mapm1 = m_apm_init();
mapm2 = m_apm_init();

for (i=0; i <= 15; i++)
  {
   m_apm_set_long(mapmi, (long)i);

   factorial_local(mapm2, mapmi);
   m_apm_factorial(mapm1, mapmi);

   if (m_apm_compare(mapm1, mapm2) != 0)
     {
      pass = 1;
      fprintf(stdout, "FAIL: miscompare of factorial %d \n", i);
     }
  }


m_apm_set_string(mapmi, "37");

factorial_local(mapm2, mapmi);
m_apm_factorial(mapm1, mapmi);

if (m_apm_compare(mapm1, mapm2) != 0)
  {
   pass = 1;
   fprintf(stdout, "FAIL: miscompare of factorial 37 \n");
  }


m_apm_set_string(mapmi, "738");

factorial_local(mapm2, mapmi);
m_apm_factorial(mapm1, mapmi);

if (m_apm_compare(mapm1, mapm2) != 0)
  {
   pass = 1;
   fprintf(stdout, "FAIL: miscompare of factorial 738 \n");
  }


m_apm_set_string(mapmi, "143");

factorial_local(mapm2, mapmi);
m_apm_factorial(mapm1, mapmi);

if (m_apm_compare(mapm1, mapm2) != 0)
  {
   pass = 1;
   fprintf(stdout, "FAIL: miscompare of factorial 143 \n");
  }


if (pass == 0)
  fprintf(stdout,"... FACTORIAL function passes\n");

m_apm_free(mapmi);
m_apm_free(mapm1);
m_apm_free(mapm2);

return(pass);
}

/****************************************************************************/

int	mapm_test_gcd()
{
int     i, pass;
M_APM   mapmt, mapm1, mapm2, mapmu, mapmv;

static  char  *str_u[15] = { "0", "108", "821632",
			     "15688", "2149236", "67368481619705856",
			     "66424636", "6058695",
			     "24791601236051755008",
			     "436138461283596140227642238435328",
			     "982147792896",
			     "33184916509275",
			     "8947823498749749279247894",
		             "5823614", "912759" };

static  char  *str_v[15] = { "151222", "0", "912374",
			     "3388608", "93863556", "3595508517961728",
			     "9996388", "17793890499",
			     "7938882807659495424",
			     "8728893958572592934385063297024",
			     "3004167094272",
			     "3497310791015625",
			     "783487423789",
		             "1135823614", "2759" };

fprintf(stdout, "Validating the GCD/LCM functions ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapmt = m_apm_init();
mapmu = m_apm_init();
mapmv = m_apm_init();

pass = 0;

for (i=0; i < 15; i++)
  {
   m_apm_set_string(mapmu, str_u[i]);
   m_apm_set_string(mapmv, str_v[i]);

   gcd_local(mapm1, mapmu, mapmv);
   m_apm_gcd(mapm2, mapmu, mapmv);

   if (m_apm_compare(mapm1, mapm2) != 0)
     {
      pass = 1;
      fprintf(stdout, "FAIL: miscompare of GCD %d \n", i);
     }

   /*
    *   LCM(u, v) = (u * v) / GCD(u, v)
    */

   m_apm_multiply(mapmt, mapmu, mapmv);
   m_apm_divide(mapm2, 200, mapmt, mapm1);

   m_apm_lcm(mapm1, mapmu, mapmv);

   if (m_apm_compare(mapm1, mapm2) != 0)
     {
      pass = 1;
      fprintf(stdout, "FAIL: miscompare of LCM %d \n", i);
     }
  }

if (pass == 0)
  fprintf(stdout,"... GCD/LCM functions pass\n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapmt);
m_apm_free(mapmu);
m_apm_free(mapmv);

return(pass);
}

/****************************************************************************/

int	mapm_test_PI()
{
int     i, dplaces, pass;
M_APM   mapmt, mapm1, mapm2, mapm3, mapm_pi;

fprintf(stdout, "Validating calculations involving PI ... \n");

mapm1   = m_apm_init();
mapm2   = m_apm_init();
mapm3   = m_apm_init();
mapmt   = m_apm_init();
mapm_pi = m_apm_init();

pass = 0;

dplaces = 300;

for (i=0; i < 2; i++)
  {
   fprintf(stdout, "Calculate PI to %d decimal places ... \n", dplaces);

   /*  PI = acos(-1)  */

   m_apm_negate(mapmt, MM_One);
   m_apm_arccos(mapm_pi, dplaces, mapmt);

   /*
    *
    */

   fprintf(stdout,
      "Calculate 3 * acos(0.5) to %d decimal places ... \n", dplaces);

   /* PI = 3 * acos(0.5) */

   m_apm_set_string(mapmt, "0.5");
   m_apm_arccos(mapm1, (dplaces + 8), mapmt);

   m_apm_multiply(mapm2, MM_Three, mapm1);
   m_apm_round(mapm1, dplaces, mapm2);

   if (m_apm_compare(mapm1, mapm_pi) == 0)
     {
      fprintf(stdout, "Verified PI == 3 * acos(0.5) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout, "***** FAILED to verify PI == 3 * acos(0.5) \n");
     }

   /*
    *
    */

   fprintf(stdout,
      "Calculate 4 * atan(1) to %d decimal places ... \n", dplaces);

   /* PI = 4 * atan(1) */

   m_apm_arctan(mapm1, (dplaces + 8), MM_One);

   m_apm_multiply(mapm2, mapm1, MM_Four);
   m_apm_round(mapm1, dplaces, mapm2);

   if (m_apm_compare(mapm1, mapm_pi) == 0)
     {
      fprintf(stdout, "Verified PI == 4 * atan(1) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout, "***** FAILED to verify PI == 4 * atan(1) \n");
     }

   /*
    *
    */

   fprintf(stdout,
    "Calculate 1.2 * acos(-sqrt(3) / 2) to %d decimal places ... \n", dplaces);

   /* PI = 1.2 * acos(-sqrt(3) / 2) */

   m_apm_sqrt(mapm1, (dplaces + 8), MM_Three);
   m_apm_divide(mapm2, (dplaces + 8), mapm1, MM_Two);
   m_apm_negate(mapmt, mapm2);
   m_apm_arccos(mapm1, (dplaces + 8), mapmt);

   m_apm_set_string(mapm2, "1.2");
   m_apm_multiply(mapmt, mapm1, mapm2);
   m_apm_round(mapm1, dplaces, mapmt);

   if (m_apm_compare(mapm1, mapm_pi) == 0)
     {
      fprintf(stdout, "Verified PI == 1.2 * acos(-sqrt(3) / 2) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
         "***** FAILED to verify PI == 1.2 * acos(-sqrt(3) / 2) \n");
     }


   /*
    *
    */

   fprintf(stdout,
      "Calculate 12 * atan(2 - sqrt(3)) to %d decimal places ... \n", dplaces);

   /* PI = 12 * atan(2 - sqrt(3))   */
   /* sqrt(3) = 10 ^ (log10(3) / 2) */

   m_apm_log10(mapm1, (dplaces + 8), MM_Three);
   m_apm_divide(mapm2, (dplaces + 8), mapm1, MM_Two);
   m_apm_pow(mapm1, (dplaces + 8), MM_Ten, mapm2);

   m_apm_subtract(mapm2, MM_Two, mapm1);

   m_apm_atan(mapm1, (dplaces + 8), mapm2);

   m_apm_set_long(mapm2, 12L);
   m_apm_multiply(mapmt, mapm1, mapm2);
   m_apm_round(mapm1, dplaces, mapmt);

   if (m_apm_compare(mapm1, mapm_pi) == 0)
     {
      fprintf(stdout, "Verified PI == 12 * atan(2 - sqrt(3)) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
         "***** FAILED to verify PI == 12 * atan(2 - sqrt(3)) \n");
     }

   /*
    *
    */

   fprintf(stdout,
      "Calculate cos(PI / 6) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm1, "6");
   m_apm_divide(mapmt, dplaces, mapm_pi, mapm1);
   m_apm_cos(mapm2, (dplaces - 8), mapmt);

   fprintf(stdout,
      "Calculate sqrt(0.75) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapmt, "0.75");
   m_apm_sqrt(mapm1, (dplaces - 8), mapmt);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified cos(PI / 6) == sqrt(0.75) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout, "***** FAILED to verify cos(PI / 6) == sqrt(0.75) \n");
     }

   /*
    *
    */

   fprintf(stdout,
      "Calculate tan(5 * PI / 12) to %d decimal places ... \n", dplaces);

   m_apm_multiply(mapm1, MM_Five, mapm_pi);
   m_apm_set_string(mapmt, "12");
   m_apm_divide(mapm2, dplaces, mapm1, mapmt);
   m_apm_tan(mapm1, (dplaces - 8), mapm2);

   fprintf(stdout,
      "Calculate 2 + sqrt(3) to %d decimal places ... \n", dplaces);

   m_apm_sqrt(mapmt, (dplaces - 8), MM_Three);
   m_apm_add(mapm2, MM_Two, mapmt);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified tan(5 * PI / 12) == 2 + sqrt(3) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
         "***** FAILED to verify tan(5 * PI / 12) == 2 + sqrt(3) \n");
     }

   /*
    *
    */

   fprintf(stdout,
     "Calculate 2 * (asin(x) + acos(x)) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm2, "12.730348");
   m_apm_cbrt(mapm1, 96, mapm2);
   m_apm_divide(mapm3, 92, mapm1, MM_Four);

   m_apm_asin(mapm1, (dplaces + 8), mapm3);
   m_apm_acos(mapm2, (dplaces + 8), mapm3);

   m_apm_add(mapmt, mapm1, mapm2);
   m_apm_multiply(mapm1, MM_Two, mapmt);
   m_apm_round(mapm3, dplaces, mapm1);

   if (m_apm_compare(mapm3, mapm_pi) == 0)
     {
      fprintf(stdout, "Verified PI = 2 * (asin(x) + acos(x)) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
         "***** FAILED to verify PI = 2 * (asin(x) + acos(x)) \n");
     }

   dplaces += 500;
  }

if (pass == 0)
  fprintf(stdout,"... PI Calculations pass\n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapmt);
m_apm_free(mapm_pi);

return(pass);
}

/****************************************************************************/

int	mapm_test_exp_pwr2()
{
int     i, dplaces, pass;
M_APM   mapm_E, mapm1, mapm2, mapm3, mapm4;

fprintf(stdout,
        "Validating calculations involving E & Integer POW_NR ... \n");

mapm_E = m_apm_init();
mapm1  = m_apm_init();
mapm2  = m_apm_init();
mapm3  = m_apm_init();
mapm4  = m_apm_init();
pass   = 0;

dplaces = 250;

for (i=0; i < 2; i++)
  {
   fprintf(stdout,
     "Calculate E (2.71828...) to %d decimal places ... \n", dplaces);

   /*  E = exp(1)  */

   m_apm_exp(mapm_E, (dplaces + 8), MM_One);

   /*
    *
    */

   fprintf(stdout, "Calculate exp(0.5) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm3, "0.5");
   m_apm_exp(mapm1, dplaces, mapm3);

   fprintf(stdout, "Calculate sqrt(E) to %d decimal places ... \n", dplaces);
   m_apm_sqrt(mapm2, dplaces, mapm_E);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified exp(0.5) == sqrt(E) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout, "***** FAILED to verify exp(0.5) == sqrt(E) \n");
     }

   /*
    *
    */

   fprintf(stdout, "Calculate exp(15) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm3, "15");
   m_apm_exp(mapm1, dplaces, mapm3);

   m_apm_integer_pow_nr(mapm3, mapm_E, 15);
   m_apm_round(mapm2, dplaces, mapm3);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified exp(15) == E ^ 15 \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout, "***** FAILED to verify exp(15) == E ^ 15 \n");
     }

   /*
    *
    */

   fprintf(stdout, "Calculate exp(2) to %d decimal places ... \n", dplaces);

   m_apm_exp(mapm1, dplaces, MM_Two);

   m_apm_integer_pow_nr(mapm3, mapm_E, 2);
   m_apm_round(mapm2, dplaces, mapm3);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified exp(2) == E ^ 2 \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout, "***** FAILED to verify exp(2) == E ^ 2 \n");
     }

   /*
    *
    */

   fprintf(stdout, "Calculate exp(-6.5) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm3, "-6.5");
   m_apm_exp(mapm1, dplaces, mapm3);

   m_apm_integer_pow_nr(mapm3, mapm_E, 13);
   m_apm_sqrt(mapm4, (dplaces + 8), mapm3);
   m_apm_reciprocal(mapm2, dplaces, mapm4);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified exp(-6.5) == 1 / sqrt(E ^ 13) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
         "***** FAILED to verify exp(-6.5) == 1 / sqrt(E ^ 13) \n");
     }

   /*
    *
    */

   fprintf(stdout, "Calculate exp(8) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm3, "8");
   m_apm_exp(mapm1, dplaces, mapm3);

   m_apm_integer_pow_nr(mapm3, mapm_E, 24);
   m_apm_cbrt(mapm2, dplaces, mapm3);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified exp(8) == cbrt(E ^ 24) \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
         "***** FAILED to verify exp(8) == cbrt(E ^ 24) \n");
     }

   dplaces += 350;

   if (M_get_sizeof_int() <= 2)		/* end now for 16 bit ints */
     break;
  }

if (pass == 0)
  fprintf(stdout,"... E & Integer POW_NR Calculations pass\n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapm4);
m_apm_free(mapm_E);

return(pass);
}

/****************************************************************************/

int	mapm_test_log_2()
{
int     i, dplaces, pass;
M_APM   mapm1, mapm2, mapm3, mapm4;

fprintf(stdout,
        "Validating log calculations with more digits ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapm3 = m_apm_init();
mapm4 = m_apm_init();

pass = 0;

dplaces = 220;

for (i=0; i < 2; i++)
  {
   fprintf(stdout,
      "Calculate log(3.6107) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm4, "3.6107");
   m_apm_log(mapm1, dplaces, mapm4);

   fprintf(stdout,
     "Calculate log(3.6107) with an AGM algorithm to %d decimal places ... \n",
             dplaces);

   log_local(mapm2, dplaces, mapm4);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified log(3.6107) with an AGM algorithm \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
        "***** FAILED to verify log(3.6107) with an AGM algorithm \n");
     }

   /*
    *
    */

   fprintf(stdout,
      "Calculate log(63.1874) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm4, "63.1874");
   m_apm_log(mapm1, dplaces, mapm4);

   fprintf(stdout,
   "Calculate log(63.1874) with an AGM algorithm to %d decimal places ... \n",
             dplaces);

   log_local(mapm2, dplaces, mapm4);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified log(63.1874) with an AGM algorithm \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
        "***** FAILED to verify log(63.1874) with an AGM algorithm \n");
     }

   /*
    *
    */

   fprintf(stdout,
      "Calculate log(184.9536) to %d decimal places ... \n", dplaces);

   m_apm_set_string(mapm3, "184.9536");
   m_apm_log(mapm1, dplaces, mapm3);

   fprintf(stdout,
   "Calculate log(184.9536) with an AGM algorithm to %d decimal places ... \n",
             dplaces);

   log_local(mapm2, dplaces, mapm3);

   if (m_apm_compare(mapm1, mapm2) == 0)
     {
      fprintf(stdout, "Verified log(184.9536) with an AGM algorithm \n");
     }
   else
     {
      pass = 1;
      fprintf(stdout,
        "***** FAILED to verify log(184.9536) with an AGM algorithm \n");
     }

   dplaces += 480;
  }

if (pass == 0)
  fprintf(stdout,"... log calculations pass\n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapm4);

return(pass);
}

/****************************************************************************/

int	mapm_test_floor_ceil()
{
int     ict, pass;
M_APM   mapm0, mapm1, mapm2, mapmr;

fprintf(stdout, "Validating the FLOOR/CEIL functions ... \n");

mapm0 = m_apm_init();
mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapmr = m_apm_init();
pass  = 0;

m_apm_set_string(mapm0, "26.90253936868423");
m_apm_set_long(mapmr, 26L);

ict = 50;

while (1)
  {
   m_apm_floor(mapm1, mapm0);

   if (m_apm_compare(mapm1, mapmr) != 0)
     {
      pass = 1;
      fprintf(stdout,
        "***** FAILED \'floor\' iteration %d \n", ict);
     }

   m_apm_add(mapm2, MM_One, mapmr);

   m_apm_ceil(mapm1, mapm0);

   if (m_apm_compare(mapm1, mapm2) != 0)
     {
      pass = 1;
      fprintf(stdout,
        "***** FAILED \'ceil\' iteration %d \n", ict);
     }

   m_apm_subtract(mapm2, mapm0, MM_One);
   m_apm_copy(mapm0, mapm2);

   m_apm_subtract(mapm2, mapmr, MM_One);
   m_apm_copy(mapmr, mapm2);

   if (--ict == 0)
     break;
  }

/*
 *    the following verifies that an exact int input
 *    yields the same int as a result.
 */

m_apm_set_string(mapm0, "7892379239879273");
m_apm_floor(mapm1, mapm0);

if (m_apm_compare(mapm1, mapm0) != 0)
  {
   pass = 1;
   fprintf(stdout,
     "***** FAILED \'floor\' int constant \n");
  }

m_apm_ceil(mapm1, mapm0);

if (m_apm_compare(mapm1, mapm0) != 0)
  {
   pass = 1;
   fprintf(stdout,
     "***** FAILED \'ceil\' int constant \n");
  }


m_apm_set_string(mapm0, "-834734734781238480");
m_apm_floor(mapm1, mapm0);

if (m_apm_compare(mapm1, mapm0) != 0)
  {
   pass = 1;
   fprintf(stdout,
     "***** FAILED \'floor\' int constant \n");
  }

m_apm_ceil(mapm1, mapm0);

if (m_apm_compare(mapm1, mapm0) != 0)
  {
   pass = 1;
   fprintf(stdout,
     "***** FAILED \'ceil\' int constant \n");
  }


if (pass == 0)
  fprintf(stdout,"... FLOOR/CEIL functions pass\n");

m_apm_free(mapm0);
m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapmr);

return(pass);
}

/****************************************************************************/

int 	mapm_test_log_near_1()
{
int	pass;
char    cbuf[32];
M_APM	mapm1, mapm2, mapmr;

fprintf(stdout, "Validating the LOG_NEAR_1 function ... \n");

mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapmr = m_apm_init();
pass  = 0;

strcpy(cbuf, "1.000086347");
m_apm_set_string(mapm1, cbuf);

m_apm_set_string(mapmr,
"8.6343272312377051473047957048191159556789410110004420720659541743787375E-5");

m_apm_log(mapm2, 70, mapm1);

if (m_apm_compare(mapm2, mapmr) != 0)
  {
   pass = 1;
   fprintf(stdout,
     "***** FAILED \'log_near_1\' #1\n");
  }


strcpy(cbuf, "0.99997206");
m_apm_set_string(mapm1, cbuf);

m_apm_set_string(mapmr,
"-2.7940390329070546415846319819251647850034442803891202616388034131872278E-5");

m_apm_log(mapm2, 70, mapm1);

if (m_apm_compare(mapm2, mapmr) != 0)
  {
   pass = 1;
   fprintf(stdout,
     "***** FAILED \'log_near_1\' #2\n");
  }

if (pass == 0)
  fprintf(stdout,"... LOG_NEAR_1 function passes\n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapmr);

return(pass);
}

/****************************************************************************/

int	mapm_test_asin_near_0()
{
int	dplaces, pass;
char    cbuf[32];
M_APM	mapm1, mapm2, mapm3, mapm4, mapm_pi2;

fprintf(stdout, "Validating the INV TRIG NEAR 0 functions ... \n");

mapm1    = m_apm_init();
mapm2    = m_apm_init();
mapm3    = m_apm_init();
mapm4    = m_apm_init();
mapm_pi2 = m_apm_init();
pass     = 0;
dplaces  = 320;

/*
 *  exercise the special cases in the inverse sin/cos/tan functions:
 *
 *  atan at 0, ~0
 *  asin at +1, -1, 0, ~0
 *  acos at +1, -1, 0, ~0
 */

fprintf(stdout, "Calculate PI/2 to %d decimal places ... \n", dplaces);

/*  PI/2 = acos(-1) / 2  */

m_apm_negate(mapm1, MM_One);
m_apm_acos(mapm2, (dplaces + 8), mapm1);
m_apm_divide(mapm_pi2, dplaces, mapm2, MM_Two);

/*  verify atan(0) == 0  */

m_apm_atan(mapm2, dplaces, MM_Zero);

if (m_apm_compare(mapm2, MM_Zero) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED atan(0) \n");
  }

/* check 'arctan_near_0' function */

strcpy(cbuf, "-6.1423379E-7");
m_apm_set_string(mapm1, cbuf);
m_apm_set_string(mapm3,
"-6.1423378999992275331387438418270080480455433775102970001007888571614664E-7");

m_apm_atan(mapm2, 70, mapm1);

if (m_apm_compare(mapm2, mapm3) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED atan_near_0 \n");
  }

/*  verify asin(0) == 0  */

m_apm_asin(mapm2, dplaces, MM_Zero);

if (m_apm_compare(mapm2, MM_Zero) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED asin(0) \n");
  }

/*  verify asin(1) == PI/2  */

m_apm_asin(mapm2, dplaces, MM_One);

if (m_apm_compare(mapm2, mapm_pi2) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED asin(1) = PI / 2 \n");
  }

/*  verify asin(-1) == -PI/2  */

m_apm_negate(mapm3, MM_One);
m_apm_asin(mapm2, dplaces, mapm3);
m_apm_negate(mapm1, mapm_pi2);

if (m_apm_compare(mapm2, mapm1) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED asin(-1) = -PI / 2 \n");
  }

/* check 'arcsin_near_0' function */

strcpy(cbuf, "2.9402782364E-6");
m_apm_set_string(mapm1, cbuf);
m_apm_set_string(mapm3,
"2.9402782364042365665958943683202914596951723300870666725716858547635946E-6");

m_apm_asin(mapm2, 70, mapm1);

if (m_apm_compare(mapm2, mapm3) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED asin_near_0 \n");
  }

/*  verify acos(1) == 0  */

m_apm_acos(mapm2, dplaces, MM_One);

if (m_apm_compare(mapm2, MM_Zero) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED acos(1) = 0 \n");
  }

/*  verify acos(0) == PI / 2  */

m_apm_acos(mapm2, dplaces, MM_Zero);

if (m_apm_compare(mapm2, mapm_pi2) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED acos(0) = PI / 2 \n");
  }


/* check 'arccos_near_0' function */

strcpy(cbuf, "-7.2804311E-5");
m_apm_set_string(mapm1, cbuf);
m_apm_set_string(mapm3,
 "1.5708691311059609353812900497247032950115203749494458830119113661688305");

m_apm_acos(mapm2, 70, mapm1);

if (m_apm_compare(mapm2, mapm3) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED acos_near_0 \n");
  }


/* verify (acos(x) + acos(-x)) / 2 = PI / 2 where x = 8.635254948248924E-7 */

strcpy(cbuf, "8.635254948248924E-7");
m_apm_set_string(mapm1, cbuf);
m_apm_negate(mapm2, mapm1);

m_apm_acos(mapm3, (dplaces + 8), mapm1);
m_apm_acos(mapm4, (dplaces + 8), mapm2);
m_apm_add(mapm1, mapm3, mapm4);
m_apm_divide(mapm2, dplaces, mapm1, MM_Two);

if (m_apm_compare(mapm2, mapm_pi2) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED (acos(x) + acos(-x)) / 2 == PI / 2 \n");
  }

if (pass == 0)
  fprintf(stdout,"... INV TRIG NEAR 0 functions pass\n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapm4);
m_apm_free(mapm_pi2);

return(pass);
}

/****************************************************************************/

/*
 *      verify sin^2 + cos^2 = 1
 */

int	mapm_test_sin2_cos2()
{
int	dplaces, pass;
M_APM	mapm1, mapm2, mapm3, mapm4, mapm5;

fprintf(stdout, "Validating SIN^2 + COS^2 = 1 ... \n");

mapm1   = m_apm_init();
mapm2   = m_apm_init();
mapm3   = m_apm_init();
mapm4   = m_apm_init();
mapm5   = m_apm_init();

pass    = 0;
dplaces = 150;

while (1)
  {
   m_apm_set_string(mapm3, "0.6216566124128346");

   fprintf(stdout,
     "Calculate sin(0.6216566124128346) to %d decimal places ... \n", dplaces);

   m_apm_sin(mapm1, (dplaces + 6), mapm3);

   fprintf(stdout,
     "Calculate cos(0.6216566124128346) to %d decimal places ... \n", dplaces);

   m_apm_cos(mapm2, (dplaces + 6), mapm3);

   m_apm_multiply(mapm4, mapm1, mapm1);
   m_apm_multiply(mapm5, mapm2, mapm2);

   m_apm_add(mapm3, mapm4, mapm5);
   m_apm_round(mapm4, dplaces, mapm3);

   if (m_apm_compare(mapm4, MM_One) != 0)
     {
      pass = 1;
      fprintf(stdout, "***** FAIL: SIN^2 + COS^2 != 1 \n");
     }

   m_apm_set_string(mapm3, "2.1073765187265308");

   fprintf(stdout,
     "Calculate sin(2.1073765187265308) to %d decimal places ... \n", dplaces);
   m_apm_sin(mapm1, (dplaces + 6), mapm3);

   fprintf(stdout,
     "Calculate cos(2.1073765187265308) to %d decimal places ... \n", dplaces);
   m_apm_cos(mapm2, (dplaces + 6), mapm3);

   m_apm_multiply(mapm4, mapm1, mapm1);
   m_apm_multiply(mapm5, mapm2, mapm2);

   m_apm_add(mapm3, mapm4, mapm5);
   m_apm_round(mapm4, dplaces, mapm3);

   if (m_apm_compare(mapm4, MM_One) != 0)
     {
      pass = 1;
      fprintf(stdout, "***** FAIL: SIN^2 + COS^2 != 1 \n");
     }

   if (dplaces > 480)
     break;

   dplaces += 175;
  }

if (pass == 0)
  fprintf(stdout,"... SIN^2 + COS^2 = 1 pass\n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapm4);
m_apm_free(mapm5);

return(pass);
}

/****************************************************************************/

/*
 *      this test forces the FFT multiply to malloc additional working
 *	arrays for temp data
 */

int	mapm_test_fft_digits()
{
int	digits, pass;
M_APM	mapm1, mapm2, mapm3, mapm4;

if (M_get_sizeof_int() >= 4)
  digits = 12000;
else
  digits = 900;

fprintf(stdout, "Validating FFT multiply with %d digits ... \n", digits);

mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapm3 = m_apm_init();
mapm4 = m_apm_init();
pass  = 0;

/*
 *    let a = sqrt(27.3)
 *    let b = cbrt(-19.6)
 *
 *    then verify ((a * b) ^ 2) * b = (-19.6 * 27.3)
 */

m_apm_set_string(mapm1, "27.3");
m_apm_set_string(mapm2, "-19.6");

fprintf(stdout, "Calculate sqrt(27.3) to %d decimal places ... \n", digits);

m_apm_sqrt(mapm3, (digits + 6), mapm1);

fprintf(stdout, "Calculate cbrt(-19.6) to %d decimal places ... \n", digits);

m_apm_cbrt(mapm4, (digits + 6), mapm2);

m_apm_multiply(mapm1, mapm3, mapm4);
m_apm_multiply(mapm2, mapm1, mapm1);
m_apm_multiply(mapm1, mapm2, mapm4);

m_apm_round(mapm2, digits, mapm1);

m_apm_set_string(mapm1, "-535.08");       /* 27.3 * -19.6 */

if (m_apm_compare(mapm2, mapm1) != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED: FFT multiply with %d digits\n", digits);
  }

if (pass == 0)
  fprintf(stdout,"... %d digit FFT multiply passes \n", digits);

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapm4);

return(pass);
}

/****************************************************************************/

/*
 *      this code forces the _add function to init the
 *	static variables in that module.  no pass/fail needed here.
 */

int	mapm_test_add_init()
{
M_APM	mapm0;

mapm0 = m_apm_init();

m_apm_add(mapm0, MM_Ten, MM_Three);

m_apm_free(mapm0);

return(0);
}

/****************************************************************************/

/*
 *      this test forces relatively rare corner conditions in the
 *	Knuth division algorithm.
 */

int	mapm_test_special_div()
{
int	digits, pass;
M_APM	mapm1, mapm2, mapm3, mapm4, mapm5, mapm6;

digits = 170;

mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapm3 = m_apm_init();
mapm4 = m_apm_init();
mapm5 = m_apm_init();
mapm6 = m_apm_init();
pass  = 0;

fprintf(stdout, "Calculate sqrt(3) to %d decimal places ... \n", digits);

m_apm_sqrt(mapm4, (digits + 6), MM_Three);

/*
 *        let N = sqrt(3)
 *
 *        compute  M = N / (N * 1.00000001)
 *
 *        then M * 1.0000001 should == 1
 */

m_apm_set_string(mapm1, "1.00000001");
m_apm_multiply(mapm3, mapm1, mapm4);

m_apm_divide(mapm2, (digits + 6), mapm4, mapm4);   /* n / n == 1 */

if (m_apm_compare(mapm2, MM_One) != 0)
  {
   pass = 1;
   fprintf(stdout,
      "***** FAILED: Special Division Check #1 %d digits\n", digits);
  }

m_apm_negate(mapm6, mapm3);
m_apm_set_long(mapm5, -1L);
m_apm_divide(mapm2, (digits + 6), mapm6, mapm3);   /* n / n == 1 */

if (m_apm_compare(mapm2, mapm5) != 0)
  {
   pass = 1;
   fprintf(stdout,
      "***** FAILED: Special Division Check #2 %d digits\n", digits);
  }

m_apm_divide(mapm2, (digits + 6), mapm4, mapm3);

m_apm_multiply(mapm5, mapm2, mapm1);
m_apm_round(mapm2, digits, mapm5);

if (m_apm_compare(mapm2, MM_One) != 0)
  {
   pass = 1;
   fprintf(stdout,
      "***** FAILED: Special Division Check #3 %d digits\n", digits);
  }

if (pass == 0)
  fprintf(stdout,"... %d digit Special Division Check passes \n", digits);

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapm4);
m_apm_free(mapm5);
m_apm_free(mapm6);

return(pass);
}

/****************************************************************************/

int	mapm_test_special_fpf()
{
int	digits, kk, pass;
char    *out_string, buf[64];
M_APM	mapm1, mapm2, mapm3, mapm4;

digits = 156;

mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapm3 = m_apm_init();
mapm4 = m_apm_init();
pass  = 0;

fprintf(stdout, "Validating Fixed Point Formatting ... \n");

strcpy(buf, "                +12.57564854193421");
buf[2] = '\t';             /* put some tabs in the string */
buf[7] = '\t';
m_apm_set_string(mapm1, buf);
m_apm_sqrt(mapm3, digits, mapm1);

digits = 150;

while (1)
  {
   out_string = m_apm_to_fixpt_stringexp(digits, mapm3, '.', 0, 0);

   if (out_string == NULL)
     {
      fprintf(stderr, "VALIDATE: Out of Memory \n");
      exit(100);
     }

   m_apm_set_string(mapm2, out_string);

   m_apm_subtract(mapm1, mapm3, mapm2);

   kk = m_apm_exponent(mapm1);

   if (digits <= 50)
     fprintf(stdout, "[%s] \n", out_string);

   if ((-kk) <= digits)
     {
      pass = 1;
      fprintf(stdout,
         "***** FAILED: Fixed Point Formatting: digits = %d \n", digits);
     }

   free(out_string);

   if (digits == -1)
     break;

   digits--;
  }

if ((out_string = (char *)malloc(1024)) == NULL)
  {
   fprintf(stderr, "VALIDATE: Out of Memory \n");
   exit(100);
  }

m_apm_set_string(mapm1, "7.42812659128912E+1469");
m_apm_sqrt(mapm2, 750, mapm1);
m_apm_floor(mapm1, mapm2);
m_apm_set_string(mapm3, "1.0E+23");
m_apm_multiply(mapm4, mapm3, mapm1);
m_apm_to_integer_string(out_string, mapm4);
m_apm_set_string(mapm3, out_string);

kk = m_apm_compare(mapm3, mapm4);

if (kk != 0)
  {
   pass = 1;
   fprintf(stdout, "***** FAILED: Fixed Point Formatting \n");
  }

free(out_string);

if (pass == 0)
  fprintf(stdout,"... Fixed Point Formatting passes \n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);
m_apm_free(mapm4);

return(pass);
}

/****************************************************************************/

int	mapm_test_special_set()
{
int	digits, kk, pass, alldigits;
char    *out_string, buf[64];
M_APM	mapm1, mapm2, mapm3;

digits = 350;
alldigits = -1;

mapm1 = m_apm_init();
mapm2 = m_apm_init();
mapm3 = m_apm_init();
pass  = 0;

fprintf(stdout, "Validating Set String & Fixed Point Formatting ... \n");

strcpy(buf, "  -7.52164854093421E+12");
m_apm_set_string(mapm1, buf);

while (1)
  {
   m_apm_cbrt(mapm3, digits, mapm1);

   if (digits > 1400)
     break;

   out_string = m_apm_to_fixpt_stringexp(alldigits, mapm3, '.', 0, 0);

   if (out_string == NULL)
     {
      fprintf(stderr, "VALIDATE: Out of Memory \n");
      exit(100);
     }

   m_apm_set_string(mapm2, out_string);

   kk = m_apm_compare(mapm2, mapm3);

   if (kk != 0)
     {
      pass = 1;
      fprintf(stdout,
        "***** FAILED: Set String & Fixed Point Formatting \n");
     }

   free(out_string);

   digits += 265;
  }

if (pass == 0)
  fprintf(stdout,"... Set String & Fixed Point Formatting passes \n");

m_apm_free(mapm1);
m_apm_free(mapm2);
m_apm_free(mapm3);

return(pass);
}

/****************************************************************************/

/*
 *
 *  start of main test procedure
 *
 */

int main(int argc, char *argv[])
{
int      total_pass;
M_APM    tmp;

total_pass = 0;

tmp = m_apm_init();

library_version_check();

mapm_show_random();
m_apm_trim_mem_usage();
m_apm_set_random_seed("649378126582104");
mapm_show_random();

total_pass |= mapm_test_sqrt();
total_pass |= mapm_test_cbrt();

total_pass |= mapm_test_sin();
total_pass |= mapm_test_cos();
total_pass |= mapm_test_sin_cos();
total_pass |= mapm_test_sin2_cos2();
total_pass |= mapm_test_tan();

total_pass |= mapm_test_asin();
total_pass |= mapm_test_acos();
total_pass |= mapm_test_atan();
total_pass |= mapm_test_atan2();
total_pass |= mapm_test_asin_near_0();

m_apm_trim_mem_usage();

total_pass |= mapm_test_sinh();
total_pass |= mapm_test_cosh();
total_pass |= mapm_test_tanh();

total_pass |= mapm_test_asinh();
total_pass |= mapm_test_acosh();
total_pass |= mapm_test_atanh();

m_apm_free(tmp);
m_apm_free_all_mem();
tmp = m_apm_init();

total_pass |= mapm_test_exp();
total_pass |= mapm_test_exp_2();
total_pass |= mapm_test_log();
total_pass |= mapm_test_log10();
total_pass |= mapm_test_log_near_1();
total_pass |= mapm_test_pow();

total_pass |= mapm_test_factorial();
total_pass |= mapm_test_gcd();
total_pass |= mapm_test_floor_ceil();

m_apm_trim_mem_usage();

total_pass |= mapm_test_PI();
total_pass |= mapm_test_exp_pwr2();
total_pass |= mapm_test_log_2();
total_pass |= mapm_test_fft_digits();

m_apm_free(tmp);
m_apm_free_all_mem();

total_pass |= mapm_test_add_init();

m_apm_trim_mem_usage();
tmp = m_apm_init();

total_pass |= mapm_test_special_div();
total_pass |= mapm_test_special_fpf();
total_pass |= mapm_test_special_set();

if (total_pass == 0)
  fprintf(stdout, "\nvalidate : PASS \n");
else
  fprintf(stdout, "\nvalidate : FAIL \n");

m_apm_free(tmp);
m_apm_free_all_mem();

exit(total_pass);
}

/****************************************************************************/


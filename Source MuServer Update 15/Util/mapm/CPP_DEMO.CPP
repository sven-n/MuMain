
/*
 *   $Id: cpp_demo.cpp,v 1.20 2007/12/03 01:18:31 mike Exp $
 *
 *   $Log: cpp_demo.cpp,v $
 *   Revision 1.20  2007/12/03 01:18:31  mike
 *   no changes
 *
 *   Revision 1.19  2004/05/31 22:11:27  mike
 *   add MOD operator demo
 *
 *   Revision 1.18  2003/05/14 21:21:43  mike
 *   *** empty log message ***
 *
 *   Revision 1.17  2003/05/06 22:27:32  mike
 *   increase array size
 *
 *   Revision 1.16  2003/05/06 21:56:39  mike
 *   add demo and check of library version functions
 *
 *   Revision 1.15  2002/11/03 23:34:36  mike
 *   added ipow_nr demo to show the new function
 *
 *   Revision 1.14  2001/08/28 18:41:16  mike
 *   add another fixed point format
 *
 *   Revision 1.13  2001/08/27 23:12:07  mike
 *   add demo of fixed point formatting functions
 *
 *   Revision 1.12  2001/08/07 19:49:06  mike
 *   add another PI example using atan, log, exp
 *
 *   Revision 1.11  2001/08/06 20:43:50  mike
 *   add some PI examples using asin and acos
 *
 *   Revision 1.10  2001/07/15 21:21:31  mike
 *   add is_odd, is_even examples
 *
 *   Revision 1.9  2001/07/15 21:14:45  mike
 *   add GCD and LCM example
 *
 *   Revision 1.8  2001/03/25 21:28:37  mike
 *   add new floor and ceil calls
 *
 *   Revision 1.7  2000/08/21 23:42:52  mike
 *   add is_integer demo
 *
 *   Revision 1.6  2000/06/20 21:40:15  mike
 *   added local versions of cbrt and inverse hyperbolics since
 *   not all C libraries have these functions yet.
 *
 *   Revision 1.5  2000/04/08 18:53:18  mike
 *   added more to Syntax Demo
 *
 *   Revision 1.4  2000/04/06 23:20:41  mike
 *   fix commented out code
 *
 *   Revision 1.3  2000/04/06 23:19:49  mike
 *   added SyntaxDemo function
 *
 *   Revision 1.2  2000/04/05 21:37:47  mike
 *   added a few more minor demos
 *
 *   Revision 1.1  2000/04/04 19:50:00  mike
 *   Initial revision
 *
 *   A simple file to demonstrate the use of the C++
 *   M_APM wrapper class.
 *
 *   Orion Sky Lawlor, olawlor@acm.org, 4/2/2000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "m_apm.h"

extern   double   cbrt_local(double);
extern   double   asinh_local(double);
extern   double   acosh_local(double);
extern   double   atanh_local(double);


void	library_version_check()
{
char	version_info[80];

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
   exit(10);
  }
}


// Compute the factorial of the integer n

MAPM Factorial(MAPM n)
{
	MAPM ii;
	MAPM product=1;
	for (ii=2; ii <= n; ii++)
		product *= ii;
	return product;
}


void printMAPM(const char *caption,MAPM m)
{
	char mBuf[1000];
	m.toString(mBuf,13);
	printf("%s%s\n",caption,mBuf);
}

void compare(const char *caption,double dVal,MAPM mVal)
{
	char dBuf[100],mBuf[1000];
	mVal.toFixPtString(mBuf,13);
	printf(caption,dVal,mBuf);
	sprintf(dBuf,"%.13f",dVal);
	if (0 != strncmp(dBuf,mBuf,14))
	  {
	   fprintf(stderr,"ERROR!  %s != %s \n",dBuf,mBuf);
	   exit(1);
	  }
}


void testMAPMcppRoutines(double v);
void SyntaxDemo(void);

int main(void)
{
	int n=200;
	library_version_check();
	MAPM c=Factorial(n);             /* use function from above */
	printMAPM("Factorial is ",c);
	m_apm_cpp_precision(25);         /* default is 30 */
	testMAPMcppRoutines(0.3);
	testMAPMcppRoutines(-0.6);
	testMAPMcppRoutines(1.0);
	testMAPMcppRoutines(5.4);
	SyntaxDemo();
	return 0;
}

void testMAPMcppRoutines(double x)
{
	char fBuf[256];
	MAPM m=x;
	printf("-------------------------\nFor x=%f:\n",x);
#define fmt "%.13f = %s\n"
	compare("x+0       "fmt,x+0,m+0);
	compare("x+1       "fmt,x+1,m+1);
	compare("3+x       "fmt,3+x,3+m);
	compare("2+(-x)    "fmt,2+(-x),2+(-m));
	compare("x-3.2     "fmt,x-3.2,m-3.2);
	compare("x-3.2 (as string) "fmt,x-3.2,m-"3.2");
	compare("3.2+(-x)  "fmt,3.2+(-x),3.2+(-m));
	compare("1.5*x     "fmt,1.5*x,1.5*m);
	compare("2/x       "fmt,2/x,2/m);
	int y=(int)(6+10*x+0.5);
	MAPM n=6+10*m;
	compare("y=6+10*x  "fmt,y,n);
	compare("y div 2   "fmt,y/2,n.div(2));
	compare("y mod 2   "fmt,y%2,n.rem(2));
	compare("y div -3  "fmt,y/-3,n.div(-3));
	compare("y mod -3  "fmt,y%-3,n.rem(-3));
	compare("-y div 3  "fmt,(-y)/-3,(-n).div(-3));
	compare("-y mod 3  "fmt,(-y)%-3,(-n).rem(-3));
	
	compare("fabs(x)   "fmt,fabs(x),fabs(m));
	compare("sin(x)    "fmt,sin(x),sin(m));
	compare("x.sin()   "fmt,sin(x),m.sin());
	compare("cos(x)    "fmt,cos(x),cos(m));
	compare("tan(x)    "fmt,tan(x),tan(m));
	compare("cbrt(x)   "fmt,cbrt_local(x),cbrt(m));
	compare("sinh(x)   "fmt,sinh(x),sinh(m));
	compare("cosh(x)   "fmt,cosh(x),cosh(m));
	compare("tanh(x)   "fmt,tanh(x),tanh(m));
	compare("asinh(x)  "fmt,asinh_local(x),asinh(m));

	if (fabs(m) < 1)
	  {
           compare("asin(x)   "fmt,asin(x),asin(m));
	   compare("acos(x)   "fmt,acos(x),acos(m));
	   compare("atan(x)   "fmt,atan(x),atan(m));
	   compare("atan2(x,-0.5)"fmt,atan2(x,-0.5),atan2(m,-0.5));
           compare("atanh(x)  "fmt,atanh_local(x),atanh(m));
	  }

	if (m >= 1)
           compare("acosh(x)  "fmt,acosh_local(x),acosh(m));

	if (m > 0)
	  {
	   compare("sqrt(x)   "fmt,sqrt(x),sqrt(m));
	   compare("log(x)    "fmt,log(x),log(m));
	   compare("pow(x,-2.93)"fmt,pow(x,-2.93),pow(m,-2.93));
	   compare("x.pow(-2.93)"fmt,pow(x,-2.93),m.pow(-2.93));
	  }

	compare("x.pow(2)  "fmt,pow(x,2),m.ipow(2));
	compare("x.pow(-3) "fmt,pow(x,-3),m.ipow(-3));
	compare("exp(x)    "fmt,exp(x),exp(m));

	MAPM r1, r2;
	r1 = get_random();
	r2 = get_random();
	printMAPM("A random MAPM on [0,1) is ",r1);
	printMAPM("Another random MAPM on [0,1) is ",r2);

	printMAPM("m=",m);
	printMAPM("m++=",m++);
	printMAPM("now m is ",m);
	printMAPM("++m=",++m);
	printMAPM("now m is ",m);

	MAPM a,b,c;
	m.sincos(a,b);
	printMAPM("sincos [sin part]=",a);
	printMAPM("sincos [cos part]=",b);

	a = 4.409 * m;
	c = ceil(a);
	a = floor(a);                     /* make sure it's an integer */
	b = factorial(a);                 /* use library function      */
	b.toIntegerString(fBuf);    

	printMAPM("c = ",c);
	printMAPM("a = ",a);
	printf("factorial a = %s\n",fBuf);
}



void	SyntaxDemo()
{
#define ALL_DIGITS (-1)
int	      ii, jj, kk;
static  char  obuf1[1024];
static  char  obuf2[1024];
static  char  obuf3[1024];
static  char  obuf4[1024];

MAPM a;               //a is uninitialized (value is undefined)
MAPM b = "4.5e12";    //b is 4.5 trillion
MAPM c = 3;           //c is now three
MAPM d = 6.1;         //d is now six point one
MAPM e = c;           //e is now three

m_apm_cpp_precision(40);      // set minimum precision level

fprintf(stdout, "----------------------------\n");
fprintf(stdout, "Start of \'SyntaxDemo\' \n\n");

b.toFixPtString(obuf1, ALL_DIGITS);
c.toFixPtString(obuf2, ALL_DIGITS);
d.toFixPtString(obuf3, ALL_DIGITS);
e.toFixPtString(obuf4, ALL_DIGITS);
fprintf(stdout, "b = [%s]    c = [%s]   d = [%s]   e = [%s]\n",
                    obuf1,      obuf2,      obuf3,    obuf4);

e = -c * (23.9022 - d);

e.toFixPtString(obuf4, ALL_DIGITS);
fprintf(stdout, "e = [%s]\n", obuf4);


a = sqrt(b);
MAPM f;
f = cbrt(e);

a.toString(obuf1, ALL_DIGITS);
f.toString(obuf2, 15);

fprintf(stdout, "a = [%s]\n", obuf1);
fprintf(stdout, "f = [%s]\n", obuf2);

MAPM array2d[8][4];

for (ii=0; ii < 8; ii++)
  {
   for (jj=0; jj < 4; jj++)
     {
      array2d[ii][jj] = 1000 * ii + 10 * jj;
     }
  }


for (ii=0; ii < 8; ii++)
  {
   for (jj=0; jj < 4; jj++)
     {
      array2d[ii][jj].toIntegerString(obuf1);
      printf("[%d][%d] = %5s   ",ii,jj,obuf1);
     }

   printf("\n");
  }

a = -23.93874;
b = a.neg();

if (b > a)
  printf("\'b\' > \'a\' \n");
else
  printf("ERROR: \'a\' > \'b\' \n");

a = 2;
b = a.ipow(31);
b.toIntegerString(obuf1);
printf("2 ^ 31 = [%s] \n",obuf1);

b = a.ipow(136);
b.toIntegerString(obuf1);
b.toString(obuf2,25);
printf("2 ^ 136 = [%s] \n",obuf1);
printf("2 ^ 136 = [%s] \n",obuf2);

ii = b.exponent();
jj = b.significant_digits();
kk = b.is_integer();
printf("exponent of \'b\' = %d   digits of \'b\' = %d   INTEGER flag = %d\n",
                          ii,jj,kk);
a = -3.12;
b = a.ipow(-5);
b.toString(obuf1, 25);
printf("-3.12 ^ -5 = [%s] \n",obuf1);


//
// ipow_nr is integer power function where the exponent
// must be an integer >= 0.
// this function will NOT do any rounding so it is ideal
// for integer math
//

a = "13";
b = a.ipow_nr(17);
b.toIntegerString(obuf1);
printf("13 ^ 17 = [%s] \n",obuf1);


a = "81";
b = a.ipow_nr(32);
b.toIntegerString(obuf1);
printf("81 ^ 32 = [%s] \n",obuf1);


//   try out mod operator

a = "3978381489565057925092524867";
b = "53828048319274";

f = a % b;

a.toIntegerString(obuf1);
b.toIntegerString(obuf2);
f.toIntegerString(obuf3);
printf("%s MOD %s = %s\n",obuf1,obuf2,obuf3);

a %= b;

a.toIntegerString(obuf1);
printf("a = %s \n",obuf1);


MAPM gcd1, lcm1;

a = "8";
a *= 3 * 7 * 19 * 53;

b = "16";			// GCD = 8 * 3 * 19 = 456
b *= 3 * 5 * 19;
b *= 23 * 137;

gcd1 = gcd(a, b);
lcm1 = lcm(a, b);

a.toIntegerString(obuf1);
b.toIntegerString(obuf2);
gcd1.toIntegerString(obuf3);
lcm1.toIntegerString(obuf4);

ii = gcd1.is_even();	      // return boolean 
jj = lcm1.is_odd();

printf("gcd (%s, %s) = %s  \t\t ii = %d\n",obuf1,obuf2,obuf3,ii);
printf("lcm (%s, %s) = %s  \t jj = %d\n",obuf1,obuf2,obuf4,jj);


c = "1.0e+9131";
d = log10(c);

c.toString(obuf1, 10);
d.toString(obuf2, 10);
printf("log10( %s ) = %s \n",obuf1,obuf2);


c = "6292734.982374929347e-817";
d = log10(c);

c.toString(obuf1, 20);
d.toString(obuf2, 20);
printf("log10( %s ) = %s \n",obuf1,obuf2);

f  = 5.92734;
ii = f.sign();
f.toString(obuf1, ALL_DIGITS);
printf("sign( %s ) = %d \n",obuf1,ii);

f  = 0;
ii = f.sign();
f.toString(obuf1, ALL_DIGITS);
printf("sign( %s ) = %d \n",obuf1,ii);

f  = "-4.987234892347e-12073";
ii = f.sign();
f.toString(obuf1, ALL_DIGITS);
printf("sign( %s ) = %d \n",obuf1,ii);


e = sqrt(fabs(f)); 
d = e.round(14);

e.toString(obuf1, ALL_DIGITS);
d.toString(obuf2, ALL_DIGITS);

printf("e = [%s] \n",obuf1);
printf("d = [%s] \n",obuf2);

MAPM quot, remn, numer, denom;

numer = "987598206958190375";
denom = "289041345897";

numer.toIntegerString(obuf1);
denom.toIntegerString(obuf2);

printf("numer = [%s] \n",obuf1);
printf("denom = [%s] \n",obuf2);


quot = numer.div(denom);
quot.toIntegerString(obuf1);
printf("quot  = [%s] \n",obuf1);

remn = numer.rem(denom);

remn.toIntegerString(obuf1);
printf("remn  = [%s] \n",obuf1);


numer.integer_div_rem(denom,quot,remn);
quot.toIntegerString(obuf1);
remn.toIntegerString(obuf2);
printf("quot  = [%s] \n",obuf1);
printf("remn  = [%s] \n",obuf2);


/*
 *    show 2 typical and 3 not-so-typical
 *    methods to calculate PI.
 */

MAPM  pi1, pi2, pi3, pi4, pi5;
char  *obuf;

obuf = obuf1;

printf("\n");
m_apm_cpp_precision(62);

pi1 = acos("-1");
pi2 = "2" * asin("1");
pi3 = "6" * acos(sqrt("3") / -2) / 5;
pi4 = "12" * asin((sqrt("3") - 1) * sqrt("2") * "0.25");
pi5 = "12" * atan("2" + exp(log("3") / 2)) / 5;

pi1.toFixPtString(obuf, 60);   printf("PI1 = [%s] \n",obuf); 
pi2.toFixPtString(obuf, 60);   printf("PI2 = [%s] \n",obuf); 
pi3.toFixPtString(obuf, 60);   printf("PI3 = [%s] \n",obuf); 
pi4.toFixPtString(obuf, 60);   printf("PI4 = [%s] \n",obuf); 
pi5.toFixPtString(obuf, 60);   printf("PI5 = [%s] \n",obuf); 



MAPM  u, v, w, x, y, z;  

m_apm_cpp_precision(50);

printf("\n");

x = 9.34231;
y = -21;
z = "-8.982349249829824921479824924792347921E-17";

ii = x.is_integer();
jj = y.is_integer();

if (ii)
  printf("x is an integer  ");
else
  printf("x is not an integer  ");

if (jj)
  printf("y is an integer  ");
else
  printf("y is not an integer  ");

printf("\n");

w = (82.30421 + sin(x / "21.11") / exp(y * 0.0726426)) * "4.32917E-2" / z;
v = "3.742416" * log(-w);
u = (2 + sqrt(v) + cbrt(v)) / 0.76;

printf("\n");
x.toString(obuf, 50);   printf("x = [%s] \n",obuf); 
y.toString(obuf, 50);   printf("y = [%s] \n",obuf); 
z.toString(obuf, 50);   printf("z = [%s] \n",obuf); 
w.toString(obuf, 50);   printf("w = [%s] \n",obuf); 
v.toString(obuf, 50);   printf("v = [%s] \n",obuf); 
u.toString(obuf, 50);   printf("u = [%s] \n",obuf); 


/*
 *   demo the fixed point formatting functions
 */

printf("\n");

a = u * "2.86521094E7";
a.toString(obuf, 50);            printf("a = [%s] \n",obuf); 

// convert 'a' to fixed point with 50 decimal places

a.toFixPtString(obuf, 50);       printf("a = [%s] \n",obuf); 


// convert 'a' to fixed point with 50 decimal places
// change radix to comma, there is no separator char.

a.toFixPtStringEx(obuf, 50, ',', 0, 0);   printf("a = [%s] \n",obuf); 


// convert 'a' to fixed point with 50 decimal places
// change radix to comma, use space as separator every 3 digits

a.toFixPtStringEx(obuf, 50, ',', ' ', 3);   printf("a = [%s] \n",obuf); 


// convert 'a' to fixed point with 40 decimal places
// change radix to period, use comma as separator every 4 digits

a.toFixPtStringEx(obuf, 40, '.', ',', 4);   printf("a = [%s] \n",obuf); 


// convert 'a' to fixed point with 40 decimal places
// change radix to period, use comma as separator every 5 digits
// this function returns a char pointer to the string which
// the caller is responsible for 'free'ing.

char   *dest;
char   radix = '.';
char   separator = ',';
int    separator_count = 5;
int    decimal_places = 40;

dest = a.toFixPtStringExp(decimal_places, radix, separator, separator_count);   
printf("a = [%s] \n",dest); 
free(dest);


// convert 'a' to fixed point with 0 decimal places (round to integer)
// set radix to dollar sign with no separator.
// note that '$' will not be visible since we are using 
// 0 decimal places (i.e., an integer format)

radix = '$';
decimal_places = 0;

dest = a.toFixPtStringExp(decimal_places, radix, 0, 0);   
printf("a = [%s] \n",dest); 
free(dest);

}

/****************************************************************************/

/*
 *      in case some compilers dont have a native cube root or
 *      inverse hyperbolic functions, we'll just make our own.
 *
 *      note that we are not doing any decent error checking on
 *      the inputs, these functions are here just to support 
 *      this program.
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


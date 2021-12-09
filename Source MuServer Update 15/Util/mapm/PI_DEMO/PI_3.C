/****************************************************************************/

/*
 *      PI_3.C
 *
 *      DEMO 'C' PROGRAM TO COMPUTE PI USING
 *      J.BORWEIN-P.BORWEIN'S ALGORITHM (1995)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m_apm.h"

#define  FALSE  0
#define  TRUE   1

#define Mult         m_apm_multiply
#define Add          m_apm_add
#define Subtract     m_apm_subtract
#define Divide       m_apm_divide
#define AssignString m_apm_set_string

extern   void  compute_PI_2(M_APM, int);


void m_apm_square(M_APM x, M_APM y)    /*  x = y^2  */
{  
m_apm_multiply(x, y, y);
}


/*
 *   NOTE : The following 2 functions modify the 
 *          input value ('y'). Use with caution
 *	    in another application!
 */

           /*  x = y^4  */

void m_apm_pow4(M_APM x, M_APM y)   
{
m_apm_square(x, y);
m_apm_copy(y, x);
m_apm_square(x, y);
}

           /*  x = sqrt(sqrt(y))  */

void m_apm_root4(M_APM x, int dec_places, M_APM y)
{
m_apm_sqrt(x, dec_places, y);
m_apm_copy(y, x);
m_apm_sqrt(x, dec_places, y);
}



int main(argc, argv)
int argc;  char *argv[];
{
int      ii, dplaces;
char     *buffer;
M_APM    pi_mapm;            /* declare the M_APM variable ... */

if (argc < 2)
  {
   fprintf(stdout,"\nUsage: pi_3  digits\t\t\t\t[Version 1.1]\n");
   fprintf(stdout,
      "       Compute PI to \'digits\' decimal places of accuracy\n");

   exit(4);
  }

pi_mapm = m_apm_init();         /* now initialize the M_APM variable ... */

dplaces = atoi(argv[1]);

if (dplaces < 2)
  {
   fprintf(stdout,"\nUsage: pi  digits \n");
   exit(6);
  }

fprintf(stderr, "\nstart Borwein-Borwein PI calculation \n");

compute_PI_2(pi_mapm, dplaces);

fprintf(stderr, "Borwein-Borwein PI calculation complete \n\n");

/*
 *   find out how many digits so we can
 *   convert the entire value into a string.
 *   (plus some pad for decimal point, exponent, etc)
 */

ii = m_apm_significant_digits(pi_mapm) + 12;

if ((buffer = (char *)malloc(ii)) == NULL)
  {
   fprintf(stdout,"PI demo: Out of memory \n");
   exit(8);
  }


/* now convert the MAPM number into a string */

m_apm_to_string(buffer, dplaces, pi_mapm);
printf("Borwein PI = [%s] \n",buffer);

m_apm_free(pi_mapm);

free(buffer);

m_apm_free_all_mem();

exit(0);
}

/****************************************************************************/

/*
 *      Calculate PI using the J. Borwein - P. Borwein's Algorithm (1995)
 *
 *
 *      Init :  A0 = 6 - 4 * sqrt(2)
 *              B0 = sqrt(2) - 1
 *
 *      Iterate:
 *
 *                                  4
 *                  1 - quart( 1 - B  ) 
 *                                  k  
 *      B      =   --------------------
 *       k+1                        4
 *                  1 + quart( 1 - B  )       
 *                                  k
 *
 *
 *                                4    2k+3                   2
 *      A      =   A  ( 1 + B    )  - 2     B   ( 1 + B    + B    )    
 *       k+1        k        k+1             k+1       k+1    k+1
 *
 *
 *
 *
 *                  where quart(x) = sqrt(sqrt(x)) = 4th root of x
 *      
 *                     1
 *      A      --->  -----
 *       k+1          PI
 *
 */

void	compute_PI_2(outv, places)
M_APM   outv;
int     places;
{
M_APM   tmp1, tmp2, tmp3, tmp4, a0, a1, b0, b1, MM_Six;
int     kk, ct, nn, dplaces, dflag;

tmp1   = m_apm_init();
tmp2   = m_apm_init();
tmp3   = m_apm_init();
tmp4   = m_apm_init();
a0     = m_apm_init();
a1     = m_apm_init();
b0     = m_apm_init();
b1     = m_apm_init();
MM_Six = m_apm_init();

dplaces = places + 16;              /* compute a few extra digits */
				    /* in the intermediate math   */
dflag   = FALSE;
ct      = 0;
kk      = 0;

AssignString(MM_Six, "6");

m_apm_sqrt(tmp1, dplaces, MM_Two);
Mult(tmp2, tmp1, MM_Four);
Subtract(a0, MM_Six, tmp2);
Subtract(b0, tmp1, MM_One);

while (TRUE)
  {
   m_apm_pow4(tmp1, b0);             /* tmp1 = b0^4              */
   Subtract(tmp2, MM_One, tmp1);     /* tmp2 = 1 - b0^4          */ 
   m_apm_root4(tmp3, dplaces, tmp2); /* tmp3 = sqrt[4](1 - b0^4) */
   Subtract(tmp1, MM_One, tmp3);     /* tmp1 = 1 - tmp3          */
   Add(tmp2, MM_One, tmp3);          /* tmp2 = 1 + tmp3          */
   Divide(b1, dplaces, tmp1, tmp2);  /* b1 = tmp1 / tmp2         */

   Add(tmp1, MM_One, b1);            /* tmp1 = 1 + b1            */
   m_apm_pow4(tmp2, tmp1);           /* tmp2 = (1 + b1)^4        */
   Mult(tmp3, a0, tmp2);             /* tmp3 = a0 * (1 + b1)^4   */

   m_apm_square(tmp1, b1);           /* tmp1 = b1^2              */
   Add(tmp2, tmp1, b1);              /* tmp2 = b1 + b1^2         */
   Add(tmp1, tmp2, MM_One);          /* tmp1 = 1 + b1 + b1^2     */
   Mult(tmp2, tmp1, b1);             /* tmp2 = b1(1 + b1 + b1^2) */

   m_apm_integer_pow(tmp1, dplaces, 
             MM_Two, (2 * kk + 3));  /* tmp1 = 2^(2k+3) */

   Mult(tmp4, tmp1, tmp2);           /* tmp4 = 2^(2k+3) b1 (1 + b1 + b1^2) */

   Subtract(tmp1, tmp3, tmp4);       /* a1 = a0(1 + b1)^4 - tmp4 */
   m_apm_round(a1, dplaces, tmp1);
  
   if (dflag)
     break;

   /*
    *   compute the difference from this 
    *   iteration to the last one.
    */

   m_apm_subtract(tmp2, a0, a1);

   if (++ct >= 4)
     {
      /*
       *  if diff == 0, we're done.
       */

      if (m_apm_sign(tmp2) == 0)
        break;
     }

   /*
    *   if the exponent of the error term (small error like 2.47...E-65)
    *   is small enough, break out after the *next* a1 is calculated.
    *
    *   get the exponent of the error term, which will be negative.
    */

   nn = -m_apm_exponent(tmp2);

   /*
    *  normally, this wouldn't be here. it's nice in the demo though.
    */

   fprintf(stderr, "PI now known to %d digits of accuracy ... \n",(4 * nn));

   if ((15 * nn) >= dplaces)
     dflag = TRUE;

   /* set up for the next iteration */

   m_apm_copy(b0, b1);
   m_apm_copy(a0, a1);
   kk++;
  }

/* round to the accuracy asked for after taking the reciprocal */

m_apm_reciprocal(tmp1, dplaces, a1);
m_apm_round(outv, places, tmp1);

m_apm_free(MM_Six);
m_apm_free(b1);
m_apm_free(b0);
m_apm_free(a1);
m_apm_free(a0);
m_apm_free(tmp4);
m_apm_free(tmp3);
m_apm_free(tmp2);
m_apm_free(tmp1);
}

/****************************************************************************/


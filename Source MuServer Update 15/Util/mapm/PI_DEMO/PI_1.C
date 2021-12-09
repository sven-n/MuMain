
/*
 *      PI_1.C
 *
 *      DEMO 'C' PROGRAM TO COMPUTE PI USING
 *      BORWEIN'S QUARTICALLY CONVERGENT ALGORITHM
 *
 *
 *      Output from 'pi-1 200' :  (I line wrapped for this page).
 *
 *      start Borwein-Quartic PI calculation 
 *      PI now known to 2 digits of accuracy ... 
 *      PI now known to 8 digits of accuracy ... 
 *      PI now known to 18 digits of accuracy ... 
 *      PI now known to 38 digits of accuracy ... 
 *      PI now known to 82 digits of accuracy ... 
 *      PI now known to 168 digits of accuracy ... 
 *      Borwein-Quartic PI calculation complete 
 *       
 *      Borwein PI = [3.1415926535897932384626433832795028841971693993751
 *                    058209749445923078164062862089986280348253421170679
 *                    821480865132823066470938446095505822317253594081284
 *                    8111745028410270193852110555964462294895493038196E+0] 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m_apm.h"

#define  FALSE  0
#define  TRUE   1

extern   void  compute_PI(M_APM, int);


int main(argc, argv)
int argc;  char *argv[];
{
int      ii, dplaces;
char     *buffer;
M_APM    pi_mapm;               /* declare the M_APM variable ... */


if (argc < 2)
  {
   fprintf(stdout,"\nUsage: pi_1  digits\t\t\t\t[Version 1.1]\n");
   fprintf(stdout,
      "       Compute PI to \'digits\' decimal places of accuracy\n");

   exit(4);
  }

pi_mapm = m_apm_init();         /* now initialize the M_APM variable ... */

dplaces = atoi(argv[1]);

if (dplaces < 2)
  {
   fprintf(stdout,"\nUsage: pi-1  digits \n");
   exit(6);
  }

fprintf(stderr, "\nstart Borwein-Quartic PI calculation \n");

compute_PI(pi_mapm, dplaces);

fprintf(stderr, "Borwein-Quartic PI calculation complete \n\n");

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
 *      Calculate PI using the Borwein's Quartically Convergent Algorithm
 *
 *      Init :  U0 = sqrt(2)
 *              B0 = 0
 *              P0 = 2 + sqrt(2)
 *
 *      Iterate:
 *
 *
 *      U      =   0.5 * [ sqrt(U ) + 1 / sqrt(U ) ]
 *       k+1                     k              k
 *
 *
 *                  sqrt(U ) * [ 1 + B  ]
 *                        k           k
 *      B      =   -----------------------
 *       k+1             U   +   B
 *                        k       k
 *
 *
 *                  P  *  B    *  [ 1 + U    ]
 *                   k     k+1           k+1
 *      P      =   ----------------------------
 *       k+1               1   +   B
 *                                  k+1
 *
 *
 *      P      --->  PI
 *       k+1
 *
 */
void	compute_PI(outv, places)
M_APM   outv;
int     places;
{
M_APM   tmp1, tmp2, tmp3, c0_5, u0, u1, b0, b1, p0, p1;
int     ct, nn, dplaces, dflag;

tmp1 = m_apm_init();
tmp2 = m_apm_init();
tmp3 = m_apm_init();
c0_5 = m_apm_init();
u0   = m_apm_init();
u1   = m_apm_init();
b0   = m_apm_init();
b1   = m_apm_init();
p0   = m_apm_init();
p1   = m_apm_init();

dplaces = places + 16;              /* compute a few extra digits */
				    /* in the intermediate math   */
dflag   = FALSE;
ct      = 0;

m_apm_set_string(c0_5, "0.5");      /* need this constant */

m_apm_sqrt(u0, dplaces, MM_Two);
m_apm_add(p0, u0, MM_Two);

/* b0 initialized to 0 from m_apm_init() */

while (TRUE)
  {
   m_apm_sqrt(tmp1, dplaces, u0);
   m_apm_reciprocal(tmp2, dplaces, tmp1);
   m_apm_add(tmp3, tmp1, tmp2);
   m_apm_multiply(u1, c0_5, tmp3);

   m_apm_add(tmp2, MM_One, b0);
   m_apm_multiply(tmp3, tmp1, tmp2);
   m_apm_add(tmp2, b0, u0);
   m_apm_divide(b1, dplaces, tmp3, tmp2);

   m_apm_add(tmp3, MM_One, u1);
   m_apm_multiply(tmp2, b1, tmp3);
   m_apm_multiply(tmp1, p0, tmp2);
   m_apm_add(tmp3, MM_One, b1);
   m_apm_divide(p1, dplaces, tmp1, tmp3);

   if (dflag)
     break;

   /*
    *   compute the difference from this 
    *   iteration to the last one.
    */

   m_apm_subtract(tmp2, p0, p1);

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
    *   is small enough, break out after the *next* p1 is calculated.
    *
    *   get the exponent of the error term, which will be negative.
    */

   nn = -m_apm_exponent(tmp2);

   /*
    *  normally, this wouldn't be here. it's nice in the demo though.
    */

   fprintf(stderr, "PI now known to %d digits of accuracy ... \n",(2 * nn));

   if ((4 * nn) >= dplaces)
     dflag = TRUE;

   /* set up for the next iteration */

   m_apm_copy(b0, b1);
   m_apm_copy(u0, u1);
   m_apm_copy(p0, p1);
  }

/* round to the accuracy asked for */

m_apm_round(outv, places, p1);

m_apm_free(p1);
m_apm_free(p0);
m_apm_free(b1);
m_apm_free(b0);
m_apm_free(u1);
m_apm_free(u0);
m_apm_free(c0_5);
m_apm_free(tmp3);
m_apm_free(tmp2);
m_apm_free(tmp1);
}

/****************************************************************************/


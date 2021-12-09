
/****************************************************************************/

/*
 *      PI_4.CPP
 *
 *      DEMO 'C++' PROGRAM TO COMPUTE PI USING
 *      J.BORWEIN-P.BORWEIN'S ALGORITHM (1995)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m_apm.h"

#define  FALSE  0
#define  TRUE   1


extern   MAPM   compute_PI_2(int);


int main(int argc, char *argv[])
{
int      ii, dplaces;
char     *buffer;
MAPM     pi_mapm;            /* declare the MAPM variable ... */

if (argc < 2)
  {
   fprintf(stdout,"\nUsage: pi_4  digits\t\t\t\t[Version 1.1]\n");
   fprintf(stdout,
      "       Compute PI to \'digits\' decimal places of accuracy\n");

   exit(4);
  }

dplaces = atoi(argv[1]);

if (dplaces < 2)
  {
   fprintf(stdout,"\nUsage: pi-4  digits \n");
   exit(6);
  }

fprintf(stderr, "\nstart Borwein-Borwein PI calculation \n");

pi_mapm = compute_PI_2(dplaces);

fprintf(stderr, "Borwein-Borwein PI calculation complete \n\n");

/*
 *   find out how many digits so we can
 *   convert the entire value into a string.
 *   (plus some pad for decimal point, exponent, etc)
 */

ii = pi_mapm.significant_digits() + 12;

if ((buffer = (char *)malloc(ii)) == NULL)
  {
   fprintf(stdout,"PI demo: Out of memory \n");
   exit(8);
  }


/* now convert the MAPM number into a string */

pi_mapm.toString(buffer, dplaces);
printf("Borwein PI = [%s] \n",buffer);

free(buffer);

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

MAPM	compute_PI_2(int places)
{
MAPM    t1, t2, c2, a0, a1, b0, b1;
int     kk, nn, dplaces, dflag;


dplaces = places + 16;               // compute a few extra digits 
				     // in the intermediate math 

m_apm_cpp_precision(dplaces);        // tell C++ our precision requirements

dflag = FALSE;
kk    = 0;

t1 = sqrt("2.0");                    // make sure we use the MAPM sqrt
a0 = 6 - 4 * t1;
b0 = t1 - 1;
c2 = 2;

while (TRUE)
  {
   t1 = b0.ipow(4);                  // t1 = b0 ^ 4
   t1 = sqrt(sqrt(1 - t1));          // t1 = root_4(1 - t1)
   t1 = t1.round(dplaces);

   b1 = (1 - t1) / (1 + t1);
   b1 = b1.round(dplaces);

   t2 = 1 + b1;
   t2 = t2.ipow(4);                  // t2 = (1 + b1) ^ 4
   t2 = t2.round(dplaces);

   t1 = c2.ipow(2 * kk + 3);         // t1 = 2 ^ (2*kk+3)

   a1 = a0 * t2 - t1 * b1 * (1 + b1 + b1 * b1);
   a1 = a1.round(dplaces);

   if (dflag)
     break;
   
   //  compute the difference from this 
   //  iteration to the last one.

   t1 = a1 - a0;

   if (kk >= 3)
     {
      //  if diff == 0, we're done.

      if (t1 == 0)
        break;
     }

   /*
    *   if the exponent of the error term (small error like 2.47...E-65)
    *   is small enough, break out after the *next* a1 is calculated.
    *
    *   get the exponent of the error term, which will be negative.
    */

   nn = -(t1.exponent());

   //  normally, this wouldn't be here. it's nice in the demo though.

   fprintf(stderr, "PI now known to %d digits of accuracy ... \n",(4 * nn));

   if ((15 * nn) >= dplaces)
     dflag = TRUE;

   //  set up for the next iteration

   b0 = b1;
   a0 = a1;
   kk++;
  }

//  round to the accuracy asked for after taking the reciprocal

t1 = 1 / a1;
return(t1.round(places));
}

/****************************************************************************/


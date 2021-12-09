/****************************************************************************/

/*
 *      PI_2.CPP
 *
 *      DEMO 'C++' PROGRAM TO COMPUTE PI USING
 *      BORWEIN'S QUARTICALLY CONVERGENT ALGORITHM
 *
 *
 *      Output from 'pi-2 200' :  (I line wrapped for this page).
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

extern   MAPM  compute_PI(int);


int main(int argc, char **argv)
{
int      ii, dplaces;
char     *buffer;
MAPM     pi_mapm;               /* declare the MAPM variable ... */


if (argc < 2)
  {
   fprintf(stdout,"\nUsage: pi_2  digits\t\t\t\t[Version 1.1]\n");
   fprintf(stdout,
      "       Compute PI to \'digits\' decimal places of accuracy\n");

   exit(4);
  }

dplaces = atoi(argv[1]);

if (dplaces < 2)
  {
   fprintf(stdout,"\nUsage: pi-2  digits \n");
   exit(6);
  }


fprintf(stderr, "\nstart Borwein-Quartic PI calculation \n");

pi_mapm = compute_PI(dplaces);

fprintf(stderr, "Borwein-Quartic PI calculation complete \n\n");

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
MAPM	compute_PI(int places)
{
MAPM    t1, c0_5, u0, u1, b0, b1, p0, p1;
int     ct, nn, dplaces, dflag;


dplaces = places + 16;              // compute a few extra digits
				    // in the intermediate math

m_apm_cpp_precision(dplaces);       // tell C++ about our precision 

dflag = FALSE;
ct    = 0;

c0_5  = "0.5"; 

u0    = sqrt("2");                  // make sure we use the MAPM sqrt
b0    = 0;
p0    = 2 + u0;

while (TRUE)
  {
/*
 *      U      =   0.5 * [ sqrt(U ) + 1 / sqrt(U ) ]
 *       k+1                     k              k
 */
   
   t1 = sqrt(u0);
   u1 = c0_5 * (t1 + 1 / t1);


/*
 *                  sqrt(U ) * [ 1 + B  ]
 *                        k           k
 *      B      =   -----------------------
 *       k+1             U   +   B
 *                        k       k
 *
 */

   b1 = (t1 * (1 + b0)) / (u0 + b0);


/*
 *                  P  *  B    *  [ 1 + U    ]
 *                   k     k+1           k+1
 *      P      =   ----------------------------
 *       k+1               1   +   B
 *                                  k+1
 */

   p1 = (p0 * b1 * (1 + u1)) / (1 + b1);


   if (dflag)
     break;

   /*
    *   compute the difference from this 
    *   iteration to the last one.
    */

   t1 = p1 - p0;

   if (++ct >= 4)
     {
      /*
       *  if diff == 0, we're done.
       */

      if (t1 == 0)
        break;
     }

   /*
    *   if the exponent of the error term (small error like 2.47...E-65)
    *   is small enough, break out after the *next* p1 is calculated.
    *
    *   get the exponent of the error term, which will be negative.
    */

   nn = -(t1.exponent());

   /*
    *  normally, this wouldn't be here. it's nice in the demo though.
    */

   fprintf(stderr, "PI now known to %d digits of accuracy ... \n",(2 * nn));

   if ((4 * nn) >= dplaces)
     dflag = TRUE;


   /* set up for the next iteration        */
   /*                                      */
   /* also, keep the number of significant */
   /* digits from growing without bound    */

   b0 = b1.round(dplaces);
   u0 = u1.round(dplaces);
   p0 = p1.round(dplaces);
  }

/* round to the accuracy asked for */

return (p1.round(places));
}

/****************************************************************************/



/* 
 *  M_APM  -  primenum.c
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
 *      $Id: primenum.c,v 1.10 2007/12/03 02:05:01 mike Exp $
 *
 *      PRIME Number Generator using the MAPM Library
 *
 *	$Log: primenum.c,v $
 *	Revision 1.10  2007/12/03 02:05:01  mike
 *	update version
 *
 *	Revision 1.9  2007/12/03 02:00:42  mike
 *	Update license
 *
 *	Revision 1.8  2003/05/31 22:42:25  mike
 *	update version for display
 *
 *	Revision 1.7  2003/05/15 21:38:12  mike
 *	add MAPM version to output
 *
 *	Revision 1.6  2002/11/03 23:00:21  mike
 *	Updated function parameters to use the modern style
 *
 *	Revision 1.5  2002/02/12 20:52:16  mike
 *	add some needed comments and change the initial setups
 *
 *	Revision 1.4  2001/02/18 16:02:33  mike
 *	filter out multiples of 2,3,5, and 7
 *
 *	Revision 1.3  2001/02/15 23:11:13  mike
 *	slightly improved algorithm
 *
 *	Revision 1.2  1999/07/12 02:36:23  mike
 *	added more usage and added more comments
 *
 *	Revision 1.1  1999/07/12 02:31:23  mike
 *	Initial revision
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "m_apm.h"

extern  int     is_number_prime(M_APM);
extern  void    init_working_mapm(void);
extern  void    free_working_mapm(void);

#define FALSE 0
#define TRUE 1

char	buffer[2048];

static  M_APM   M_limit, M_digit, M_quot, M_rem, M_tmp0, M_tmp1;


/****************************************************************************/

int main(int argc, char *argv[])
{
char	 version_info[80];
int      ct;
				/* declare the M_APM variables ... */
M_APM    aa_mapm;
M_APM    bb_mapm;
M_APM    cc_mapm;
M_APM    dd_mapm;

if (argc < 2)
  {
   m_apm_lib_short_version(version_info);

   fprintf(stdout,
      "Usage: primenum number\t\t\t[Version 1.4, MAPM Version %s]\n",
      	      version_info);
   fprintf(stdout,
      "       find the first 10 prime numbers starting with \'number\'\n");

   exit(4);
  }
				/* now initialize the M_APM variables ... */
aa_mapm = m_apm_init();
bb_mapm = m_apm_init();
cc_mapm = m_apm_init();
dd_mapm = m_apm_init();

init_working_mapm();

m_apm_set_string(dd_mapm, argv[1]);

/*
 *  if input < 3, set start point = 3
 */

if (m_apm_compare(dd_mapm, MM_Three) == -1)
  {
   m_apm_copy(dd_mapm, MM_Three);
  }

/*
 *  make sure we start with an odd integer
 */

m_apm_integer_divide(aa_mapm, dd_mapm, MM_Two);
m_apm_multiply(bb_mapm, MM_Two, aa_mapm);
m_apm_add(aa_mapm, MM_One, bb_mapm);

ct = 0;

while (TRUE)
  {
   if (is_number_prime(aa_mapm))
     {
      m_apm_to_integer_string(buffer, aa_mapm);
      fprintf(stdout,"%s\n",buffer);

      if (++ct == 10)
        break;
     }

   m_apm_add(cc_mapm, MM_Two, aa_mapm);
   m_apm_copy(aa_mapm, cc_mapm);
  }

free_working_mapm();

m_apm_free(aa_mapm);
m_apm_free(bb_mapm);
m_apm_free(cc_mapm);
m_apm_free(dd_mapm);

m_apm_free_all_mem();

exit(0);
}

/****************************************************************************/

/*
 *      functions returns TRUE if the M_APM input number is prime
 *                        FALSE if it is not
 */
int     is_number_prime(M_APM input)
{
int     ii, ret, index;
char    sbuf[32];

/*
 *      for reference:
 *
 *      table size of 2 to filter multiples of 2 and 3 
 *      table size of 8 to filter multiples of 2, 3 and 5
 *      table size of 480 to filter multiples of 2,3,5,7, and 11
 *
 *      this increment table will filter out all numbers
 *      that are multiples of 2,3,5 and 7.
 */

static  char  incr_table[48] = {
        2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2,
        6, 4, 6, 8, 4, 2, 4, 2, 4, 8, 6, 4, 6, 2, 4, 6,
        2, 6, 6, 4, 2, 4, 6, 2, 6, 4, 2, 4, 2, 10, 2, 10 };
   
/* 
 *  since the real algorithm starts at 11 (to syncronize
 *  with the increment table), we will cheat for numbers < 10.
 */

if (m_apm_compare(input, MM_Ten) <= 0)
  {
   m_apm_to_integer_string(sbuf, input);
   ii = atoi(sbuf);

   if (ii == 2 || ii == 3 || ii == 5 || ii == 7)
     return(TRUE);
   else
     return(FALSE);
  }

ret   = FALSE;
index = 0;

/*
 *    see if the input number is a
 *    multiple of 3, 5, or 7.
 */

m_apm_integer_div_rem(M_quot, M_rem, input, MM_Three);
if (m_apm_sign(M_rem) == 0)               /* remainder == 0 */
  return(ret);

m_apm_integer_div_rem(M_quot, M_rem, input, MM_Five);
if (m_apm_sign(M_rem) == 0)
  return(ret);

m_apm_set_long(M_digit, 7L);
m_apm_integer_div_rem(M_quot, M_rem, input, M_digit);
if (m_apm_sign(M_rem) == 0)
  return(ret);

ii = m_apm_exponent(input) + 16;

m_apm_sqrt(M_tmp1, ii, input);
m_apm_add(M_limit, MM_Two, M_tmp1);
   
m_apm_set_long(M_digit, 11L);              /* now start at '11' to check */
   
while (TRUE)
  {
   if (m_apm_compare(M_digit, M_limit) >= 0)
     {
      ret = TRUE;
      break;
     }
   
   m_apm_integer_div_rem(M_quot, M_rem, input, M_digit);
   
   if (m_apm_sign(M_rem) == 0)         /* remainder == 0 */
     break;
   
   m_apm_set_long(M_tmp1, (long)incr_table[index]);
   m_apm_add(M_tmp0, M_digit, M_tmp1);
   m_apm_copy(M_digit, M_tmp0);

   if (++index == 48)
     index = 0;
  }

return(ret);
}

/****************************************************************************/

void    init_working_mapm()
{
M_limit = m_apm_init();
M_digit = m_apm_init();
M_quot  = m_apm_init();
M_rem   = m_apm_init();
M_tmp0  = m_apm_init();
M_tmp1  = m_apm_init();
}

/****************************************************************************/

void    free_working_mapm()
{
m_apm_free(M_limit);
m_apm_free(M_digit);
m_apm_free(M_quot);
m_apm_free(M_rem);
m_apm_free(M_tmp0);
m_apm_free(M_tmp1);
}

/****************************************************************************/

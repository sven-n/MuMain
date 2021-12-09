
/* 
 *  M_APM  -  mapm_mt.c
 *
 *  Copyright (C) 2002 - 2007   
 *  Martin Pfingstl (Martin.Pfingstl@epost.de)
 *  Michael C. Ring
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
 *      $Id: mapm_mt.c,v 1.4 2007/12/03 02:24:27 mike Exp $
 *
 *      This file contains all wrapper functions for the library 
 *	using semaphores
 *
 *      $Log: mapm_mt.c,v $
 *      Revision 1.4  2007/12/03 02:24:27  mike
 *      update license
 *
 *      Revision 1.3  2003/05/07 22:52:22  mike
 *      add 2 new version functions
 *
 *      Revision 1.2  2002/11/10 22:42:44  mike
 *      add new integer_pow_nr function
 *
 *      Revision 1.1  2002/06/13 20:39:09  mike
 *      Initial revision
 */


#include "m_apm.h"

extern   void    m_apm_enter(void);
extern   void    m_apm_leave(void);
extern   void	 m_apm_init_semaphore(void);
extern   void	 m_apm_free_semaphore(void);

/* **************************************************************
   Platform specific stuff
   ************************************************************** */


#ifdef _MSC_VER   			/* for micro$oft compilers */

#include <windows.h> 
static CRITICAL_SECTION m_apm_semaphore;

void	m_apm_enter(void)
{
	EnterCriticalSection(&m_apm_semaphore);
}

void	m_apm_leave(void)
{
	LeaveCriticalSection(&m_apm_semaphore);
}

void	m_apm_init_semaphore(void)
{
	InitializeCriticalSection(&m_apm_semaphore);
}

void	m_apm_free_semaphore(void)
{
	DeleteCriticalSection(&m_apm_semaphore);
}

#endif


/* *************************************************************
   END Platform specific stuff
   ************************************************************* */


/****************************************************************************/

char	*m_apm_lib_version_mt(char *s)
{
	m_apm_enter();
	m_apm_lib_version(s);
	m_apm_leave();
	return(s);
}

/****************************************************************************/

char	*m_apm_lib_short_version_mt(char *s)
{
	m_apm_enter();
	m_apm_lib_short_version(s);
	m_apm_leave();
	return(s);
}

/****************************************************************************/

M_APM 	m_apm_init_mt(void)
{
M_APM 	t;

	m_apm_enter();
	t=m_apm_init();
	m_apm_leave();
	return(t);
}

/****************************************************************************/

void	m_apm_free_mt(M_APM t)
{
	m_apm_enter();
	m_apm_free(t);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_free_all_mem_mt(void)
{
	m_apm_enter();
	m_apm_free_all_mem();
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_trim_mem_usage_mt(void)
{
	m_apm_enter();
	m_apm_trim_mem_usage();
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_set_string_mt(M_APM ctmp, char *s_in)
{
	m_apm_enter();
	m_apm_set_string(ctmp,s_in);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_set_double_mt(M_APM atmp,double dd)
{
	m_apm_enter();
	m_apm_set_double(atmp,dd);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_set_long_mt(M_APM atmp, long mm)
{
	m_apm_enter();
	m_apm_set_long(atmp,mm);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_to_string_mt(char *s, int places, M_APM mtmp)
{
	m_apm_enter();
	m_apm_to_string(s,places,mtmp);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_add_mt(M_APM r, M_APM a, M_APM b)
{
	m_apm_enter();
	m_apm_add(r,a,b);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_subtract_mt(M_APM r, M_APM a, M_APM b)
{
	m_apm_enter();
	m_apm_subtract(r,a,b);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_absolute_value_mt(M_APM d, M_APM s)
{
	m_apm_enter();
	m_apm_absolute_value(d,s);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_negate_mt(M_APM d, M_APM s)
{
	m_apm_enter();
	m_apm_negate(d,s);
	m_apm_leave();
}

/****************************************************************************/

int	m_apm_compare_mt(M_APM ltmp, M_APM rtmp)
{
int 	ret;

	m_apm_enter();
	ret=m_apm_compare(ltmp,rtmp);
	m_apm_leave();
	return(ret);
}


/****************************************************************************/

void	m_apm_multiply_mt(M_APM r, M_APM a, M_APM b)
{
	m_apm_enter();
	m_apm_multiply(r,a,b);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_divide_mt(M_APM rr, int places, M_APM aa, M_APM bb)
{
	m_apm_enter();
	m_apm_divide(rr,places,aa,bb);
	m_apm_leave();
}

/****************************************************************************/

void m_apm_cpp_precision_mt(int digits)
{
	m_apm_enter();
	m_apm_cpp_precision(digits);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_to_fixpt_string_mt(char *ss, int dplaces, M_APM mtmp)
{
	m_apm_enter();
	m_apm_to_fixpt_string(ss,dplaces,mtmp);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_round_mt(M_APM btmp, int places, M_APM atmp)
{
	m_apm_enter();
	m_apm_round(btmp,places,atmp);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_sqrt_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_sqrt(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_sin_mt(M_APM r, int places, M_APM a)
{
	m_apm_enter();
	m_apm_sin(r,places,a);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_arccos_mt(M_APM r, int places, M_APM x)
{
	m_apm_enter();
	m_apm_arccos(r,places,x);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_arctan2_mt(M_APM rr, int places, M_APM yy, M_APM xx)
{
	m_apm_enter();
	m_apm_arctan2(rr,places,yy,xx);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_copy_mt(M_APM dest, M_APM src)
{
	m_apm_enter();
	m_apm_copy(dest,src);
	m_apm_leave();
}


/****************************************************************************/

void	m_apm_reciprocal_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_reciprocal(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_to_fixpt_stringex_mt(char *s, int dplaces, M_APM atmp,
			char ch_radix, char ch_sep, int count_sep)
{
	m_apm_enter();
	m_apm_to_fixpt_stringex(s,dplaces,atmp,ch_radix,ch_sep,count_sep);
	m_apm_leave();
}

/****************************************************************************/

char	*m_apm_to_fixpt_stringexp_mt(int dplaces, M_APM atmp, char ch_radx, 
				char ch_sep, int ct_sep)
{
char 	*s;

	m_apm_enter();
	s=m_apm_to_fixpt_stringexp(dplaces,atmp,ch_radx,ch_sep,ct_sep);
	m_apm_leave();
	return(s);
}

/****************************************************************************/

void m_apm_to_integer_string_mt(char *s, M_APM mtmp)
{
	m_apm_enter();
	m_apm_to_integer_string(s,mtmp);
	m_apm_leave();
}

/****************************************************************************/

int	m_apm_sign_mt(M_APM m)
{
int 	i;

	m_apm_enter();
	i=m_apm_sign(m);
	m_apm_leave();
	return(i);
}

/****************************************************************************/

int	m_apm_exponent_mt(M_APM m)
{
int 	i;

	m_apm_enter();
	i=m_apm_exponent(m);
	m_apm_leave();
	return(i);
}

/****************************************************************************/

int	m_apm_significant_digits_mt(M_APM m)
{
int 	i;

	m_apm_enter();
	i=m_apm_significant_digits(m);
	m_apm_leave();
	return(i);
}

/****************************************************************************/

int	m_apm_is_integer_mt(M_APM m)
{
int 	i;

	m_apm_enter();
	i=m_apm_is_integer(m);
	m_apm_leave();
	return(i);
}

/****************************************************************************/

int	m_apm_is_even_mt(M_APM m)
{
int 	i;

	m_apm_enter();
	i=m_apm_is_even(m);
	m_apm_leave();
	return(i);
}

/****************************************************************************/

int	m_apm_is_odd_mt(M_APM m)
{
int 	i;

	m_apm_enter();
	i=m_apm_is_odd(m);
	m_apm_leave();
	return(i);
}

/****************************************************************************/

void	m_apm_gcd_mt(M_APM r, M_APM u, M_APM v)
{
	m_apm_enter();
	m_apm_gcd(r,u,v);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_lcm_mt(M_APM r, M_APM u, M_APM v)
{
	m_apm_enter();
	m_apm_lcm(r,u,v);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_integer_divide_mt(M_APM rr, M_APM aa, M_APM bb)
{
	m_apm_enter();
	m_apm_integer_divide(rr,aa,bb);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_integer_div_rem_mt(M_APM qq, M_APM rr, M_APM aa, M_APM bb)
{
	m_apm_enter();
	m_apm_integer_div_rem(qq,rr,aa,bb);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_factorial_mt(M_APM moutput, M_APM  minput)
{
	m_apm_enter();
	m_apm_factorial(moutput,minput);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_floor_mt(M_APM bb, M_APM aa)
{
	m_apm_enter();
	m_apm_floor(bb,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_ceil_mt(M_APM bb, M_APM aa)
{
	m_apm_enter();
	m_apm_ceil(bb,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_get_random_mt(M_APM m)
{
	m_apm_enter();
	m_apm_get_random(m);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_set_random_seed_mt(char *s)
{
	m_apm_enter();
	m_apm_set_random_seed(s);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_cbrt_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_cbrt(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_log_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_log(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_log10_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_log10(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_exp_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_exp(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_pow_mt(M_APM rr, int places, M_APM xx, M_APM yy)
{
	m_apm_enter();
	m_apm_pow(rr,places,xx,yy);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_integer_pow_mt(M_APM rr, int places, M_APM aa, int mexp)
{
	m_apm_enter();
	m_apm_integer_pow(rr,places,aa,mexp);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_integer_pow_nr_mt(M_APM rr, M_APM aa, int mexp)
{
	m_apm_enter();
	m_apm_integer_pow_nr(rr,aa,mexp);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_sin_cos_mt(M_APM sinv, M_APM cosv, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_sin_cos(sinv,cosv,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_cos_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_cos(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_tan_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_tan(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_arcsin_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_arcsin(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void	m_apm_arctan_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_arctan(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void    m_apm_sinh_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_sinh(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void    m_apm_cosh_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_cosh(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void    m_apm_tanh_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_tanh(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void    m_apm_arcsinh_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_arcsinh(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void    m_apm_arccosh_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_arccosh(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/

void    m_apm_arctanh_mt(M_APM rr, int places, M_APM aa)
{
	m_apm_enter();
	m_apm_arctanh(rr,places,aa);
	m_apm_leave();
}

/****************************************************************************/


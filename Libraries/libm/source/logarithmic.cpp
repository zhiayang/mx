// log.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../include/math.h"
namespace math
{
	double log(double x)
	{
		double ln2;
		asm("fldln2" : "=t"(ln2));
		return ln2 * log2(x);
	}

	float log(float x)
	{
		float ln2;
		asm("fldln2" : "=t"(ln2));
		return ln2 * log2(x);
	}

	long double log(long double x)
	{
		long double ln2;
		asm("fldln2" : "=t"(ln2));
		return ln2 * log2(x);
	}

	double log10(double x)
	{
		double log10_2;
		asm("fldlg2" : "=t"(log10_2));
		return log10_2 * log2(x);
	}

	float log10(float x)
	{
		double log10_2;
		asm("fldlg2" : "=t"(log10_2));
		return (float)(log10_2 * log2(x));
	}

	long double log10(long double x)
	{
		double log10_2;
		asm("fldlg2" : "=t"(log10_2));
		return log10_2 * log2(x);
	}

	double log1p(double x)
	{
		return log(1 + x);
	}

	float log1p(float x)
	{
		return log(1 + x);
	}

	long double log1p(long double x)
	{
		return log(1 + x);
	}

	double log2(double x)
	{
		asm("fld1 ; fxch ; fyl2x" : "+t"(x));
		return x;
	}

	float log2(float x)
	{
		asm("fld1 ; fxch ; fyl2x" : "+t"(x));
		return x;
	}

	long double log2(long double x)
	{
		asm("fld1 ; fxch ; fyl2x" : "+t"(x));
		return x;
	}
}


extern "C" double		log(double x)		{ return math::log(x); }
extern "C" float		logf(float x)		{ return math::log(x); }
extern "C" long double	logl(long double x)	{ return math::log(x); }

extern "C" double		log10(double x)	{ return math::log10(x); }
extern "C" float		log10f(float x)		{ return math::log10(x); }
extern "C" long double	log10l(long double x)	{ return math::log10(x); }

extern "C" double		log1p(double x)	{ return math::log1p(x); }
extern "C" float		log1pf(float x)		{ return math::log1p(x); }
extern "C" long double	log1pl(long double x)	{ return math::log1p(x); }

extern "C" double		log2(double x)		{ return math::log2(x); }
extern "C" float		log2f(float x)		{ return math::log2(x); }
extern "C" long double	log2l(long double x)	{ return math::log2(x); }













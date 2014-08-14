// exp.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../libc/include/stdint.h"
#include "../include/math.h"

namespace math
{
	double exp(double x)
	{
		asm("fldl2e ; fmulp ; f2xm1" : "+t"(x));
		return x + 1;
	}

	float exp(float x)
	{
		asm("fldl2e ; fmulp ; f2xm1" : "+t"(x));
		return x + 1;
	}

	long double exp(long double x)
	{
		asm("fldl2e ; fmulp ; f2xm1" : "+t"(x));
		return x + 1;
	}

	double exp2(double x)
	{
		asm("f2xm1" : "+t"(x));
		return x + 1;
	}

	float exp2(float x)
	{
		asm("f2xm1" : "+t"(x));
		return x + 1;
	}

	long double exp2(long double x)
	{
		asm("f2xm1" : "+t"(x));
		return x + 1;
	}

	double expm1(double x)
	{
		return math::exp(x) - 1;
	}

	float expm1(float x)
	{
		return math::exp(x) - 1;
	}

	long double expm1(long double x)
	{
		return math::exp(x) - 1;
	}

	double ldexp(double x, int ex)
	{
		return x * (1 << ex);
	}

	float ldexp(float x, int ex)
	{
		return x * (1 << ex);
	}

	long double ldexp(long double x, int ex)
	{
		return x * (1 << ex);
	}
}


extern "C" double		exp(double x)			{ return math::exp(x);		}
extern "C" float		expf(float x)			{ return math::exp(x);		}
extern "C" long double	expl(long double x)		{ return math::exp(x);		}
extern "C" double		exp2(double x)		{ return math::exp2(x);	}
extern "C" float		exp2f(float x)			{ return math::exp2(x);	}
extern "C" long double	exp2l(long double x)		{ return math::exp2(x);	}
extern "C" double		expm1(double x)		{ return math::expm1(x);	}
extern "C" float		expm1f(float x)		{ return math::expm1(x);	}
extern "C" long double	expm1l(long double x)	{ return math::expm1(x);	}
extern "C" double		ldexp(double x, int ex)		{ return math::ldexp(x, ex);	}
extern "C" float		ldexpf(float x, int ex)		{ return math::ldexp(x, ex);	}
extern "C" long double	ldexpl(long double x, int ex)	{ return math::ldexp(x, ex);	}









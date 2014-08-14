// power.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../libc/include/stdint.h"
#include "../include/math.h"
namespace math
{
	double cbrt(double x)
	{
		return pow(x, 1.0 / 3.0);
	}

	float cbrt(float x)
	{
		return math::pow(x, 1.0f / 3.0f);
	}

	long double cbrt(long double x)
	{
		return math::pow(x, 1.0L / 3.0L);
	}

	double fabs(double x)
	{
		asm("fabs" : "+t"(x));
		return x;
	}

	float fabs(float x)
	{
		asm("fabs" : "+t"(x));
		return x;
	}

	long double fabs(long double x)
	{
		asm("fabs" : "+t"(x));
		return x;
	}

	double hypot(double x, double y)
	{
		return math::sqrt((x * x) + (y * y));
	}

	float hypot(float x, float y)
	{
		return math::sqrt((x * x) + (y * y));
	}

	long double hypot(long double x, long double y)
	{
		return math::sqrt((x * x) + (y * y));
	}

	double pow(double x, double y)
	{
		return (double) math::pow((long double) x, (long double) y);
	}

	float pow(float x, float y)
	{
		return (float) math::pow((long double) x, (long double) y);
	}

	long double pow(long double x, long double y)
	{
		if(x > 0)
			return math::exp(y * math::log(x));

		else if((int64_t) y == y)
		{
			long double r = 1;
			if(y > 0)
			{
				for(int64_t i = 0; i < y; i++)
				{
					r *= x;
				}
			}
			else
			{
				y = -y;
				for(int64_t i = 0; i < y; i++)
				{
					r *= x;
				}
				r = 1.0L / r;
			}

			return r;
		}
		else
		{
			// can't do this shit
			return 0;
		}
	}


	double sqrt(double x)
	{
		asm("fsqrt" : "+t"(x));
		return x;
	}

	float sqrt(float x)
	{
		asm("fsqrt" : "+t"(x));
		return x;
	}

	long double sqrt(long double x)
	{
		asm("fsqrt" : "+t"(x));
		return x;
	}
}




extern "C" double		cbrt(double x)				{ return math::cbrt(x); }
extern "C" float		cbrtf(float x)				{ return math::cbrt(x); }
extern "C" long double	cbrtl(long double x)			{ return math::cbrt(x); }
extern "C" double		fabs(double x)				{ return math::fabs(x); }
extern "C" float		fabsf(float x)				{ return math::fabs(x); }
extern "C" long double	fabsl(long double x)			{ return math::fabs(x); }
extern "C" double		hypot(double x, double y)		{ return math::hypot(x, y); }
extern "C" float		hypotf(float x, float y)			{ return math::hypot(x, y); }
extern "C" long double	hypotl(long double x, long double y)	{ return math::hypot(x, y); }
extern "C" double		pow(double x, double y)		{ return math::pow(x, y); }
extern "C" float		powf(float x, float y)			{ return math::pow(x, y); }
extern "C" long double	powl(long double x, long double y)	{ return math::pow(x, y); }
extern "C" double		sqrt(double x)				{ return math::sqrt(x); }
extern "C" float		sqrtf(float x)				{ return math::sqrt(x); }
extern "C" long double	sqrtl(long double x)			{ return math::sqrt(x); }


















// other.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../libc/include/stdint.h"
#include "../include/math.h"
namespace math
{
	double ceil(double x)
	{
		// first we have to modify the CR field in the x87 control register
		uint_least16_t controlWord = 0;
		asm volatile("fstcw %0" : : "m"(controlWord) : "memory");
		controlWord |= 0x400; controlWord &= ~0x800;
		asm volatile("fldcw %0" : : "m"(controlWord));
		return math::nearbyint(x);
	}

	float ceil(float x)
	{
		// first we have to modify the CR field in the x87 control register
		uint_least16_t controlWord = 0;
		asm volatile("fstcw %0" : : "m"(controlWord) : "memory");
		controlWord |= 0x400; controlWord &= ~0x800;
		asm volatile("fldcw %0" : : "m"(controlWord));
		return math::nearbyint(x);
	}

	long double ceil(long double x)
	{
		// first we have to modify the CR field in the x87 control register
		uint_least16_t controlWord = 0;
		asm volatile("fstcw %0" : : "m"(controlWord) : "memory");
		controlWord |= 0x400; controlWord &= ~0x800;
		asm volatile("fldcw %0" : : "m"(controlWord));
		return math::nearbyint(x);
	}

	double floor(double x)
	{
		// first we have to modify the CR field in the x87 control register
		uint_least16_t controlWord = 0;
		asm volatile("fstcw %0" : : "m"(controlWord) : "memory");
		controlWord |= 0x800; controlWord &= ~0x400;
		asm volatile("fldcw %0" : : "m"(controlWord));
		return math::nearbyint(x);
	}

	float floor(float x)
	{
		// first we have to modify the CR field in the x87 control register
		uint_least16_t controlWord = 0;
		asm volatile("fstcw %0" : : "m"(controlWord) : "memory");
		controlWord |= 0x800; controlWord &= ~0x400;
		asm volatile("fldcw %0" : : "m"(controlWord));
		return math::nearbyint(x);
	}

	long double floor(long double x)
	{
		// first we have to modify the CR field in the x87 control register
		uint_least16_t controlWord = 0;
		asm volatile("fstcw %0" : : "m"(controlWord) : "memory");
		controlWord |= 0x800; controlWord &= ~0x400;
		asm volatile("fldcw %0" : : "m"(controlWord));
		return math::nearbyint(x);
	}



	double nearbyint(double x)
	{
		asm("frndint" : "+t"(x));
		return x;
	}

	float nearbyint(float x)
	{
		asm("frndint" : "+t"(x));
		return x;
	}

	long double nearbyint(long double x)
	{
		asm("frndint" : "+t"(x));
		return x;
	}

	double remainder(double x, double y)
	{
		double result;

		asm("fldl %2 ; fldl %1 ; fprem ; fxch ; fincstp" : "=t"(result) : "m"(x), "m"(y));
		return result;
	}

	float remainder(float x, float y)
	{
		float result;
		asm("fldl %2 ; fldl %1 ; fprem ; fxch ; fincstp" : "=t"(result) : "m"(x), "m"(y));
		return result;
	}

	long double remainder(long double x, long double y)
	{
		long double result;
		asm("fldl %2 ; fldl %1 ; fprem ; fxch ; fincstp" : "=t"(result) : "m"(x), "m"(y));
		return result;
	}

	double remquo(double x, double y, int* quo)
	{
		*quo = (int) x / (int) y;
		return math::remainder(x, y);
	}

	float remquo(float x, float y, int* quo)
	{
		*quo = (int) x / (int) y;
		return math::remainder(x, y);
	}

	long double remquo(long double x, long double y, int* quo)
	{
		*quo = (int) x / (int) y;
		return math::remainder(x, y);
	}

	double copysign(double x, double y)
	{
		auto magnitude = math::fabs(x);
		return signbit(y) ? -magnitude : magnitude;
	}

	float copysign(float x, float y)
	{
		auto magnitude = math::fabs(x);
		return (signbit(y) ? -magnitude : magnitude);
	}

	long double copysign(long double x, long double y)
	{
		auto magnitude = math::fabs(x);
		return signbit(y) ? -magnitude : magnitude;
	}

	double fdim(double x, double y)
	{
		return (x > y) ? (x - y) : 0;
	}

	float fdim(float x, float y)
	{
		return (x > y) ? (x - y) : 0;
	}

	long double fdim(long double x, long double y)
	{
		return (x > y) ? (x - y) : 0;
	}

	double fmax(double x, double y)
	{
		return (x > y) ? x : y;
	}

	float fmax(float x, float y)
	{
		return (x > y) ? x : y;
	}

	long double fmax(long double x, long double y)
	{
		return (x > y) ? x : y;
	}

	double fmin(double x, double y)
	{
		return (x < y) ? x : y;
	}

	float fmin(float x, float y)
	{
		return (x < y) ? x : y;
	}

	long double fmin(long double x, long double y)
	{
		return (x < y) ? x : y;
	}

	double fma(double x, double y, double z)
	{
		return x * y + z;
	}

	float fma(float x, float y, float z)
	{
		return x * y + z;
	}

	long double fma(long double x, long double y, long double z)
	{
		return x * y + z;
	}

	double round(double x)
	{
		return x > 0 ? math::floor(x + 0.5) : math::ceil(x - 0.5);
	}

	float round(float x)
	{
		return x > 0 ? math::floor(x + 0.5f) : math::ceil(x - 0.5f);
	}

	long double round(long double x)
	{
		return x > 0 ? math::floor(x + 0.5L) : math::ceil(x - 0.5L);
	}

	long lround(double x)
	{
		return (long) math::round(x);
	}

	long lround(float x)
	{
		return (long) math::round(x);
	}

	long lround(long double x)
	{
		return (long) math::round(x);
	}

	long long llround(double x)
	{
		return (long long) math::round(x);
	}

	long long llround(float x)
	{
		return (long long) math::round(x);
	}

	long long llround(long double x)
	{
		return (long long) math::round(x);
	}

	double trunc(double n)
	{
		return n - (long long) n;
	}

	float trunc(float n)
	{
		return n - (long long) n;
	}

	long double trunc(long double n)
	{
		return n - (long long) n;
	}

	long long min(long long a, long long b)
	{
		return a < b ? a : b;
	}

	long long max(long long a, long long b)
	{
		return a > b ? a : b;
	}

	long long abs(long long a)
	{
		return a < 0 ? -a : a;
	}
}


extern "C" double		ceil(double x)						{ return math::ceil(x); }
extern "C" float		ceilf(float x)						{ return math::ceil(x); }
extern "C" long double	ceill(long double x)					{ return math::ceil(x); }
extern "C" double		floor(double x)						{ return math::floor(x); }
extern "C" float		floorf(float x)						{ return math::floor(x); }
extern "C" long double	floorl(long double x)					{ return math::floor(x); }
extern "C" double		nearbyint(double x)					{ return math::nearbyint(x); }
extern "C" float		nearbyintf(float x)					{ return math::nearbyint(x); }
extern "C" long double	nearbyintl(long double x)				{ return math::nearbyint(x); }
extern "C" double		remainder(double x, double y)				{ return math::remainder(x, y); }
extern "C" float		remainderf(float x, float y)				{ return math::remainder(x, y); }
extern "C" long double	remainderl(long double x, long double y)		{ return math::remainder(x, y); }
extern "C" double		remquo(double x, double y, int* quo)			{ return math::remquo(x, y, quo); }
extern "C" float		remquof(float x, float y, int* quo)			{ return math::remquo(x, y, quo); }
extern "C" long double	remquol(long double x, long double y, int* quo)	{ return math::remquo(x, y, quo); }
extern "C" double		copysign(double x, double y)				{ return math::copysign(x, y); }
extern "C" float		copysignf(float x, float y)				{ return math::copysign(x, y); }
extern "C" long double	copysignl(long double x, long double y)		{ return math::copysign(x, y); }
extern "C" double		fdim(double x, double y)				{ return math::fdim(x, y); }
extern "C" float		fdimf(float x, float y)					{ return math::fdim(x, y); }
extern "C" long double	fdiml(long double x, long double y)			{ return math::fdim(x, y); }
extern "C" double		fmax(double x, double y)				{ return math::fmax(x, y); }
extern "C" float		fmaxf(float x, float y)					{ return math::fmax(x, y); }
extern "C" long double	fmaxl(long double x, long double y)			{ return math::fmax(x, y); }
extern "C" double		fmin(double x, double y)				{ return math::fmin(x, y); }
extern "C" float		fminf(float x, float y)					{ return math::fmin(x, y); }
extern "C" long double	fminl(long double x, long double y)			{ return math::fmin(x, y); }
extern "C" double		fma(double x, double y, double z)			{ return math::fma(x, y, z); }
extern "C" float		fmaf(float x, float y, float z)				{ return math::fma(x, y, z); }
extern "C" long double	fmal(long double x, long double y, long double z)	{ return math::fma(x, y, z); }
extern "C" double		round(double x)					{ return math::round(x); }
extern "C" float		roundf(float x)						{ return math::round(x); }
extern "C" long double	roundl(long double x)					{ return math::round(x); }
extern "C" long		lround(double x)					{ return (long) math::round(x); }
extern "C" long		lroundf(float x)					{ return (long) math::round(x); }
extern "C" long		lroundl(long double x)					{ return (long) math::round(x); }
extern "C" long long		llround(double x)					{ return (long long) math::round(x); }
extern "C" long long		llroundf(float x)					{ return (long long) math::round(x); }
extern "C" long long		llroundl(long double x)				{ return (long long) math::round(x); }
extern "C" double		trunc(double x)					{ return math::trunc(x); }
extern "C" float		truncf(float x)						{ return math::trunc(x); }
extern "C" long double	truncl(long double x)					{ return math::trunc(x); }











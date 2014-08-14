// trigonometric.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../include/math.h"
namespace math
{
	long double pi()
	{
		long double pi;
		asm("fldpi" : "=t"(pi));
		return pi;
	}

	double acos(double x)
	{
		double pi;
		asm("fldpi" : "=t"(pi));
		return (pi / 2) - math::asin(x);
	}

	float acos(float x)
	{
		float pi;
		asm("fldpi" : "=t"(pi));
		return (pi / 2) - math::asin(x);
	}

	long double acos(long double x)
	{
		long double pi;
		asm("fldpi" : "=t"(pi));
		return (pi / 2) - math::asin(x);
	}

	double asin(double x)
	{
		return 2.0 * math::atan(x / (1.0 + math::sqrt(1.0 - x * x)));
	}

	float asin(float x)
	{
		return 2.0f * math::atan(x / (1.0f + math::sqrt(1.0f - x * x)));
	}

	long double asin(long double x)
	{
		return 2.0L * math::atan(x / (1.0L + math::sqrt(1.0L - x * x)));
	}


	double atan(double x)
	{
		return math::atan2(x, 1.0);
	}

	float atan(float x)
	{
		return math::atan2(x, 1.0f);
	}

	long double atan(long double x)
	{
		return math::atan2(x, 1.0L);
	}

	double atan2(double y, double x)
	{
		double result;
		asm("fldl %1 ; fldl %2 ; fpatan" : "=t"(result) : "m"(y), "m"(x));
		return result;
	}

	float atan2(float y, float x)
	{
		float result;
		asm("flds %1 ; flds %2 ; fpatan" : "=t"(result) : "m"(y), "m"(x));
		return result;
	}

	long double atan2(long double y, long double x)
	{
		long double result;
		asm("fldt %1 ; fldt %2 ; fpatan" : "=t"(result) : "m"(y), "m"(x));
		return result;
	}

	double cos(double x)
	{
		asm("fcos" : "+t"(x));
		return x;
	}

	float cos(float x)
	{
		asm("fcos" : "+t"(x));
		return x;
	}

	long double cos(long double x)
	{
		asm("fcos" : "+t"(x));
		return x;
	}

	double sin(double x)
	{
		asm("fsin" : "+t"(x));
		return x;
	}

	float sin(float x)
	{
		asm("fsin" : "+t"(x));
		return x;
	}

	long double sin(long double x)
	{
		asm("fsin" : "+t"(x));
		return x;
	}


	double tan(double x)
	{
		return math::sin(x) / math::cos(x);
	}

	float tan(float x)
	{
		return math::sin(x) / math::cos(x);
	}

	long double tan(long double x)
	{
		return math::sin(x) / math::cos(x);
	}


	double acosh(double x)
	{
		return math::log(x + math::sqrt(x * x - 1));
	}

	float acosh(float x)
	{
		return math::log(x + math::sqrt(x * x - 1));
	}

	long double acosh(long double x)
	{
		return math::log(x + math::sqrt(x * x - 1));
	}

	double asinh(double x)
	{
		return math::log(x + math::sqrt(x * x + 1));
	}

	float asinh(float x)
	{
		return math::log(x + math::sqrt(x * x + 1));
	}

	long double asinh(long double x)
	{
		return math::log(x + math::sqrt(x * x + 1));
	}


	double atanh(double x)
	{
		return 0.5 * math::log((1.0 + x) / (1.0 - x));
	}

	float atanh(float x)
	{
		return 0.5f * math::log((1.0f + x) / (1.0f - x));
	}

	long double atanh(long double x)
	{
		return 0.5L * math::log((1.0L + x) / (1.0L - x));
	}


	double cosh(double x)
	{
		return (math::exp(x) + math::exp(-x)) * 0.5;
	}

	float cosh(float x)
	{
		return (math::exp(x) + math::exp(-x)) * 0.f;
	}

	long double cosh(long double x)
	{
		return (math::exp(x) + math::exp(-x)) * 0.5L;
	}

	double sinh(double x)
	{
		return (math::exp(x) - math::exp(-x)) * 0.5;
	}

	float sinh(float x)
	{
		return (math::exp(x) - math::exp(-x)) * 0.5f;
	}

	long double sinh(long double x)
	{
		return (math::exp(x) - math::exp(-x)) * 0.5L;
	}

	double tanh(double x)
	{
		return math::sinh(x) / math::cosh(x);
	}

	float tanh(float x)
	{
		return math::sinh(x) / math::cosh(x);
	}

	long double tanh(long double x)
	{
		return math::sinh(x) / math::cosh(x);
	}
}


extern "C" double		acos(double x)				{ return math::acos(x); }
extern "C" float		acosf(float x)				{ return math::acos(x); }
extern "C" long double	acosl(long double x)			{ return math::acos(x); }
extern "C" double		asin(double x)				{ return math::asin(x); }
extern "C" float		asinf(float x)				{ return math::asin(x); }
extern "C" long double	asinl(long double x)			{ return math::asin(x); }
extern "C" double		atan(double x)				{ return math::atan(x); }
extern "C" float		atanf(float x)				{ return math::atan(x); }
extern "C" long double	atanl(long double x)			{ return math::atan(x); }
extern "C" double		atan2(double y, double x)		{ return math::atan2(y, x); }
extern "C" float		atan2f(float y, float x)			{ return math::atan2(y, x); }
extern "C" long double	atan2l(long double y, long double x)	{ return math::atan2(y, x); }
extern "C" double		cos(double x)				{ return math::cos(x); }
extern "C" float		cosf(float x)				{ return math::cos(x); }
extern "C" long double	cosl(long double x)			{ return math::cos(x); }
extern "C" double		sin(double x)				{ return math::sin(x); }
extern "C" float		sinf(float x)				{ return math::sin(x); }
extern "C" long double	sinl(long double x)			{ return math::sin(x); }
extern "C" double		tan(double x)				{ return math::tan(x); }
extern "C" float		tanf(float x)				{ return math::tan(x); }
extern "C" long double	tanl(long double x)			{ return math::tan(x); }
extern "C" double		acosh(double x)			{ return math::acos(x); }
extern "C" float		acoshf(float x)				{ return math::acos(x); }
extern "C" long double	acoshl(long double x)			{ return math::acos(x); }
extern "C" double		asinh(double x)			{ return math::asin(x); }
extern "C" float		asinhf(float x)				{ return math::asin(x); }
extern "C" long double	asinhl(long double x)			{ return math::asin(x); }
extern "C" double		atanh(double x)			{ return math::atan(x); }
extern "C" float		atanhf(float x)				{ return math::atan(x); }
extern "C" long double	atanhl(long double x)			{ return math::atan(x); }
extern "C" double		cosh(double x)				{ return math::cosh(x); }
extern "C" float		coshf(float x)				{ return math::cosh(x); }
extern "C" long double	coshl(long double x)			{ return math::cosh(x); }
extern "C" double		sinh(double x)				{ return math::sinh(x); }
extern "C" float		sinhf(float x)				{ return math::sinh(x); }
extern "C" long double	sinhl(long double x)			{ return math::sinh(x); }
extern "C" double		tanh(double x)				{ return math::tanh(x); }
extern "C" float		tanhf(float x)				{ return math::tanh(x); }
extern "C" long double	tanhl(long double x)			{ return math::tanh(x); }
extern "C" double		pi()					{ return (double) math::pi(); }
extern "C" float		pif()					{ return (float) math::pi(); }
extern "C" long double	pil()					{ return math::pi(); }










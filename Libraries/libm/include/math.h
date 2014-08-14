/*

	Copyright 2009 Pierre KRIEGER

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		 http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

*/


#pragma once

#define		isinf(x)		(((x) != 0) && (2 * (x) == (x)))
#define		isnan(x)	((x) != (x))
#define		isfinite(x)	(!isinf(x))
#define		isnormal(x)	(!isnan(x) && isfinite(x) && ((x) != 0))
#define		signbit(x)	(isnan(x) ? 0 : ((x) < 0))
#define		fpclassify(x)	(((x) == 0) ? 2 : (isinf(x) ? 3 : (isnan(x) ? 4 : 1)))


#ifdef __cplusplus
extern "C"
{
#endif



// note: still to be tested

double			acos(double x);
float			acosf(float x);
long double		acosl(long double x);

double			asin(double x);
float			asinf(float x);
long double		asinl(long double x);

double			atan(double x);
float			atanf(float x);
long double		atanl(long double x);

double			atan2(double y, double x);
float			atan2f(float y, float x);
long double		atan2l(long double y, long double x);

double			cos(double x);
float			cosf(float x);
long double		cosl(long double x);

double			sin(double x);
float			sinf(float x);
long double		sinl(long double x);

double			tan(double x);
float			tanf(float x);
long double		tanl(long double x);

double			acosh(double x);
float			acoshf(float x);
long double		acoshl(long double x);

double			asinh(double x);
float			asinhf(float x);
long double		asinhl(long double x);

double			atanh(double x);
float			atanhf(float x);
long double		atanhl(long double x);

double			cosh(double x);
float			coshf(float x);
long double		coshl(long double x);

double			sinh(double x);
float			sinhf(float x);
long double		sinhl(long double x);

double			tanh(double x);
float			tanhf(float x);
long double		tanhl(long double x);

double			pi();
float			pif();
long double		pil();





double			exp(double x);
float			expf(float x);
long double		expl(long double x);

double			exp2(double x);
float			exp2f(float x);
long double		exp2l(long double x);

double			expm1(double x);
float			expm1f(float x);
long double		expm1l(long double x);

double			frexp(double x, int* exp);
float			frexpf(float x, int* exp);
long double		frexpl(long double x, int* exp);

int			ilogb(double x);
int			ilogbf(float x);
int			ilogbl(long double x);

double			ldexp(double x, int exp);
float			ldexpf(float x, int exp);
long double		ldexpl(long double x, int exp);

double			log(double x);
float			logf(float x);
long double		logl(long double x);

double			log10(double x);
float			log10f(float x);
long double		log10l(long double x);

double			log1p(double x);
float			log1pf(float x);
long double		log1pl(long double x);

double			log2(double x);
float			log2f(float x);
long double		log2l(long double x);

double			logb(double x);
float			logbf(float x);
long double		logbl(long double x);

double			modf(double value, double* iptr);
float			modff(float value, float* iptr);
long double		modfl(long double value, long double* iptr);

double			scalbn(double x, int n);
float			scalbnf(float x, int n);
long double		scalbnl(long double x, int n);

double			scalbln(double x, long int n);
float			scalblnf(float x, long int n);
long double		scalblnl(long double x, long int n);

double			cbrt(double x);
float			cbrtf(float x);
long double		cbrtl(long double x);

double			fabs(double x);
float			fabsf(float x);
long double		fabsl(long double x);

double			hypot(double x, double y);
float			hypotf(float x, float y);
long double		hypotl(long double x, long double y);

double			pow(double x, double y);
float			powf(float x, float y);
long double		powl(long double x, long double y);

double			sqrt(double x);
float			sqrtf(float x);
long double		sqrtl(long double x);

double			erf(double x);
float			erff(float x);
long double		erfl(long double x);

double			erfc(double x);
float			erfcf(float x);
long double		erfcl(long double x);

double			lgamma(double x);
float			lgammaf(float x);
long double		lgammal(long double x);

double			tgamma(double x);
float			tgammaf(float x);
long double		tgammal(long double x);

double			ceil(double x);
float			ceilf(float x);
long double		ceill(long double x);

double			floor(double x);
float			floorf(float x);
long double		floorl(long double x);

double			nearbyint(double x);
float			nearbyintf(float x);
long double		nearbyintl(long double x);

double			rint(double x);
float			rintf(float x);
long double		rintl(long double x);

long int		lrint(double x);
long int		lrintf(float x);
long int		lrintl(long double x);

long long int		llrint(double x);
long long int		llrintf(float x);
long long int		llrintl(long double x);

double			round(double x);
float			roundf(float x);
long double		roundl(long double x);

long int		lround(double x);
long int		lroundf(float x);
long int		lroundl(long double x);

long long int		llround(double x);
long long int		llroundf(float x);
long long int		llroundl(long double x);

double			trunc(double x);
float			truncf(float x);
long double		truncl(long double x);

double			fmod(double x, double y);
float			fmodf(float x, float y);
long double		fmodl(long double x, long double y);

double			remainer(double x, double y);
float			remainerf(float x, float y);
long double		remainerl(long double x, long double y);

double			remquo(double x, double y, int* quo);
float			remquof(float x, float y, int* quo);
long double		remquol(long double x, long double y, int* quo);

double			copysign(double x, double y);
float			copysignf(float x, float y);
long double		copysignl(long double x, long double y);

double			nan(const char* tagp);
float			nanf(const char* tagp);
long double		nanl(const char* tagp);

double			nextafter(double x, double y);
float			nextafterf(float x, float y);
long double		nextafterl(long double x, long double y);

double			nexttoward(double x, long double y);
float			nexttowardf(float x, long double y);
long double		nexttowardl(long double x, long double y);

double			fdim(double x, double y);
float			fdimf(float x, float y);
long double		fdiml(long double x, long double y);

double			fmax(double x, double y);
float			fmaxf(float x, float y);
long double		fmaxl(long double x, long double y);

double			fmin(double x, double y);
float			fminf(float x, float y);
long double		fminl(long double x, long double y);

double			fma(double x, double y, double z);
float			fmaf(float x, float y, float z);
long double		fmal(long double x, long double y, long double z);


#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
namespace math
{
	// exp.cpp
	double		exp(double x);
	float		exp(float x);
	long double	exp(long double x);

	double		exp2(double x);
	float		exp2(float x);
	long double	exp2(long double x);

	double		expm1(double x);
	float		expm1(float x);
	long double	expm1(long double x);

	double		ldexp(double x, int ex);
	float		ldexp(float x, int ex);
	long double	ldexp(long double x, int ex);


	// log.cpp
	double		log(double x);
	float		log(float x);
	long double	log(long double x);

	double		log10(double x);
	float		log10(float x);
	long double	log10(long double x);

	double		log1p(double x);
	float		log1p(float x);
	long double	log1p(long double x);

	double		log2(double x);
	float		log2(float x);
	long double	log2(long double x);


	// misc.cpp
	double		ceil(double x);
	float		ceil(float x);
	long double	ceil(long double x);
	double		floor(double x);
	float		floor(float x);
	long double	floor(long double x);
	double		nearbyint(double x);
	float		nearbyint(float x);
	long double	nearbyint(long double x);
	double		remainder(double x, double y);
	float		remainder(float x, float y);
	long double	remainder(long double x, long double y);
	double		remquo(double x, double y, int* quo);
	float		remquo(float x, float y, int* quo);
	long double	remquo(long double x, long double y, int* quo);
	double		copysign(double x, double y);
	float		copysign(float x, float y);
	long double	copysign(long double x, long double y);
	double		fdim(double x, double y);
	float		fdim(float x, float y);
	long double	fdim(long double x, long double y);
	double		fmax(double x, double y);
	float		fmax(float x, float y);
	long double	fmax(long double x, long double y);
	double		fmin(double x, double y);
	float		fmin(float x, float y);
	long double	fmin(long double x, long double y);
	double		fma(double x, double y, double z);
	float		fma(float x, float y, float z);
	long double	fma(long double x, long double y, long double z);
	double		round(double x);
	float		round(float x);
	long double	round(long double x);
	long		lround(double x);
	long		lround(float x);
	long		lround(long double x);
	long long	llround(double x);
	long long	llround(float x);
	long long	llround(long double x);
	double		trunc(double n);
	float		trunc(float n);
	long double	trunc(long double n);



	// power.cpp
	double		cbrt(double x);
	float		cbrt(float x);
	long double	cbrt(long double x);
	double		fabs(double x);
	float		fabs(float x);
	long double	fabs(long double x);
	double		hypot(double x, double y);
	float		hypot(float x, float y);
	long double	hypot(long double x, long double y);
	double		pow(double x, double y);
	float		pow(float x, float y);
	long double	pow(long double x, long double y);
	double		sqrt(double x);
	float		sqrt(float x);
	long double	sqrt(long double x);



	// trigonometric.cpp
	long double	pi();
	double		acos(double x);
	float		acos(float x);
	long double	acos(long double x);
	double		asin(double x);
	float		asin(float x);
	long double	asin(long double x);
	double		atan(double x);
	float		atan(float x);
	long double	atan(long double x);
	double		atan2(double y, double x);
	float		atan2(float y, float x);
	long double	atan2(long double y, long double x);
	double		cos(double x);
	float		cos(float x);
	long double	cos(long double x);
	double		sin(double x);
	float		sin(float x);
	long double	sin(long double x);
	double		tan(double x);
	float		tan(float x);
	long double	tan(long double x);
	double		acosh(double x);
	float		acosh(float x);
	long double	acosh(long double x);
	double		asinh(double x);
	float		asinh(float x);
	long double	asinh(long double x);
	double		atanh(double x);
	float		atanh(float x);
	long double	atanh(long double x);
	double		cosh(double x);
	float		cosh(float x);
	long double	cosh(long double x);
	double		sinh(double x);
	float		sinh(float x);
	long double	sinh(long double x);
	double		tanh(double x);
	float		tanh(float x);
	long double	tanh(long double x);


	long long	min(long long a, long long b);
	long long	max(long long a, long long b);
	long long	abs(long long a);
}
#endif

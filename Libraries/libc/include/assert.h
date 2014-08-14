// assert.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


/* Rid ourselves of any previous declaration of assert. */
#ifdef assert
#undef assert
#endif

/* If not debugging, we'll declare a no-operation assert macro. */
// #if defined(NDEBUG)
// #define assert(ignore) ((void) (0))
// #endif


#ifdef __cplusplus
extern "C" {
#endif

void __assert(const char* filename, unsigned long line, const char* function_name, const char* expression);

/* Otherwise, declare the normal assert macro. */
// #if !defined(NDEBUG)
#define assert(invariant) ((invariant) ? (void) (0) : __assert(__FILE__, __LINE__, __PRETTY_FUNCTION__, #invariant))
// #endif


#ifdef __cplusplus
}
#endif

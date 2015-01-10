// sighandler.s
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

.global _sighandler
.type _sighandler, @function

.section .text

_sighandler:
	// when we get called, there are two things on the stack: %rdi, then the actual return address.
	pop %rdi

	// go to that address.
	ret

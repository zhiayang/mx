// CodingStyle.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>

/* The following file uses some conditional modifiers:

	1. ALWAYS / MUST
		The following has to be done under all circumstances.

	2. NEVER / MUST NOT
		The following cannot be done under any circumstances.

	3. SHOULD
		The following ALWAYS has to be done, unless EXCEPTIONS are present.

	4. SHOULD NOT
		The following can NEVER be done, unless EXCEPTIONS are present.

	5. MAY
		The following can be done at discretion.

	6. MAY NOT
		The following can be omitted at discretion.

	7. EXCEPTIONS
		Circumstances requiring special care and/or attention that may migitate ALWAYS, NEVER, SHOULD and SHOULD NOT conditions.

	8. UNLESS
		Qualifier for exceptions.

*/

#define UNUSED(x)	((void) x)

void DoSomething();
void SomethingElse();

namespace OnlyNamespaceDeclarations {
namespace AreAllowedToHaveBraces {
namespace OnTheSameLineToAvoid {
namespace ExcessiveIndentation {
namespace ButTheFinalNamespace {
namespace IsIndentedProperly
{
	void FunctionName(uint64_t Argument1, uint64_t Argument2)
	{
		// ALWAYS ALWAYS ALWAYS USE TABS.
		// Spaces WILL BE SHOT.
		// Braces are ALWAYS on a newline, except under two circumstances:
		// 1. namespace declarations (see above)
		// 2. Single line functions, like so:
		// void Function() { DoSomething(); }

		// Function names are SHOULD be in ProperCamelCase, UNLESS the name is a single word, in which case
		// names like 'something()' are allowed.

		// Member variable names CAN be either lowercase or CamelCase.
		// They can NEVER be halfAssedCamelCase, or stupid_underscore_naming.
		// variable names CAN be completelylowercase, although this SHOULD be avoided in new code.

		// Unused parameters MUST be silenced, either using
		UNUSED(Argument1);

		// or manually using if the UNUSED macro is not defined.
		(void) Argument1;

		// Casts MUST be appended with a single space after the closing bracket:
		(void) ((uint8_t) ((uint32_t) Argument2));

		// Code SHOULD use the types defined in stdint.h: uint*_t, int*_t etc, and not size_t etc.

		// Pointers MUST ALWAYS be declared with the asterisk AFTER the typename, like so:
		// Dereferencing and/or taking the address places the asterisk or ampersand directly next to the variable name.
		uint64_t* Pointer = &Argument2;
		*Pointer = 41;

		// Variable names in a small scope (function, loop etc) can use non-verbose names:
		// However, variable names in global scope must be verbose and in CamelCase.
		// Global scope here refers to variables that will be referenced (using 'extern') from another source file.
		for(uint64_t i = 0; i < 51; i++)
		{
			*Pointer = i;
		}

		// Do not place extra spaces:
		if(*Pointer > 14)
		{
			DoSomething();
		}

		// Else and Else If clauses MUST be on a new line.
		// However, omitting braces and indenting is allowed.
		else
			SomethingElse();

		// However, if two or more consecutive if statements without braces are needed,
		// All the if blocks MUST have braces except the inner one, like so:
		if(true)
		{
			if(false)
			{
				if(true)
					DoSomething();
			}
		}

		// EXCEPTIONs to the braces rule include a do-while loop:
		do
		{
			// Note that there must be an empty line
			// between the last statement in the loop
			// and the closing brace.

		} while(false);

		// Another EXCEPTION is struct attributes:
		struct Thing
		{
			// Again, the last member must be separated from the closing brace
			// by an empty line.

			// Another EXCEPTION to the extra spaces rule is after __attribute__ markers.

		} __attribute__ ((packed));


		{
			// Blocks of code performing a task SHOULD be scoped with braces.
			// This can be used when the code requires variables or context from surrounding code.
		}

		// Code SHOULD use brackets to separate parts of expressions, even when precedence doesn't require it.
		*Pointer += (4 * 301);


		// This applies mostly to kernel code.
		// Subsystems SHOULD declare an Initialise() function taking zero or more arguments
		// to initialise the system, for consistency.

		// NEVER use 'using namespace xxx' in a header file.
		// Code SHOULD NOT produce ANY warnings, even on -Weverything mode.
		// If that warning is legit, but code cannot silence it, place the warning in the makefile.

		// As part of a new rule, code SHOULD NOT #include <Global.hpp>.
		// Instead, code SHOULD include specific files, organised by namespace under source/Kernel/HeaderFiles.

		// colons after anything should have spaces on both sides.
		class Class
		{
			// Public/Private/Protected qualifiers MUST be indented one tab from the braceline.
			public:
				// Empty code blocks must be preceeded by a space after the closing argument bracket.
				// They can choose to have a space between the opening and closing brace.
				Class(int val) : somemember(val) { }
				Class() : somemember(31) { }
				int somemember;
		};

		class Class2
		{
			public:
				int member;
		};

		// Initialisations SHOULD include the empty brackets
		// when the object has a non-default constructor.

		Class* f1 = new Class();
		Class* f2 = new Class(4);

		// However, when the class uses the default constructor, the brackets MAY be omitted.
		Class2* c1 = new Class2;

		delete f1;
		delete f2;
		delete c1;




		uint64_t value = 1024;
		// increment/decrements MUST be postscript:
		value++;
		value++;
		value--;

		// They can NEVER be prefixed; if such an effect is desired, increment before using.
	}
}
}
}
}
}
}

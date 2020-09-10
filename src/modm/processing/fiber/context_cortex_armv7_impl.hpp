/*
 * Copyright (c) 2020, Erik Henriksson
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/platform/device.hpp>
#include <modm/debug/logger.hpp>

// Stack layout:
// r4
// r5
// r6
// r7
// r8
// r9
// r10
// r11
// LR
// PC


modm_context
modm_makecontext(modm_stack_t* stack, std::size_t stack_size,
				 void (*fn)(void), void (*end)(void))
{
	modm_stack_t* sp = stack + stack_size / sizeof(modm_stack_t);
	*--sp = (modm_stack_t) fn; // PC
	*--sp = (modm_stack_t) end; // LR
	sp -= 8; // r4-r11
	return {sp, stack_size};
}

void
modm_startcontext(const modm_context &to)
{
	modm_stack_t** main_sp = &main_context.sp;
	modm_stack_t* to_sp = to.sp;
	asm volatile
	(
		"adr r3, 1f"			"\n\t" // Load address of instruction after "pop {pc}" into r3
		"add r3, r3, #1"		"\n\t" // Stay in thumb mode
		"push {r3}"				"\n\t" // Save instruction address to stack (to be used for PC later)

		"push {r4-r11, lr}"		"\n\t"

		"mov r3, sp"			"\n\t"
		"str r3, [%[main_sp]]"	"\n\t" // Store the main SP in "main_sp"
		"msr psp, %[to_sp]"		"\n\t" // Set PSP to top of stack
		"mrs r3, control"		"\n\t"
		"mov r2, #0x2"			"\n\t" // Set SPSEL
		"orr r3, r2"			"\n\t"
		"msr control, r3"		"\n\t"

		"pop {r4-r11}"			"\n\t"
		"pop {r3}"				"\n\t"
		"mov lr, r3"			"\n\t"

		"pop {pc}"				"\n\t" // Perform the jump
		".align 2"				"\n\t"
		"1:"					"\n\t"

		/*outputs*/: [main_sp] "+r" (main_sp), [to_sp] "+r" (to_sp)
		/*inputs*/:
		/*clobbers*/: "r2", "r3", "memory"
	);
}

void
modm_jumpcontext(modm_context* from, const modm_context &to)
{
	register modm_stack_t** from_sp asm("r0") = &from->sp;
	register modm_stack_t* to_sp asm("r1") = to.sp;
	asm volatile
	(
		"adr r3, 1f"			"\n\t" // Load address of instruction after "pop {pc}" into r3
		"add r3, r3, #1"		"\n\t" // Stay in thumb mode
		"push {r3}"				"\n\t" // Save instruction address to stack (to be used for PC later)

		"push {r4-r11, lr}"		"\n\t"

		"mov r3, sp"			"\n\t"
		"str r3, [%[from_sp]]"	"\n\t" // Store the SP in "from"
		"mov sp, %[to_sp]"		"\n\t" // Restore SP from "to"

		"pop {r4-r11}"			"\n\t"
		"pop {r3}"				"\n\t"
		"mov lr, r3"			"\n\t"

		"pop {pc}"				"\n\t" // Perform the jump
		".align 2"				"\n\t"
		"1:"					"\n\t"

		/*outputs*/: [from_sp] "+r" (from_sp), [to_sp] "+r" (to_sp)
		/*inputs*/:
		/*clobbers*/: "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "memory"
	);
}

void
modm_endcontext()
{
	asm volatile
	(
		"mrs r1, control"		"\n\t"
		"mov r2, #0x2"			"\n\t" // Unset SPSEL
		"bic r1, r1, r2"		"\n\t"
		"msr control, r1"		"\n\t"

		"pop {r4-r11}"			"\n\t"
		"pop {r3}"				"\n\t"
		"mov lr, r3"			"\n\t"

		"pop {pc}"				"\n\t" // Perform the jump

		/*outputs*/:
		/*inputs*/:
		/*clobbers*/: "r1", "r2", "memory"
	);
}

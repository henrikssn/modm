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

extern "C"
modm_always_inline
void
modm_startcontext(const modm_context to) {
  modm_stack_t* to_sp = to.sp;
  asm volatile(
    "msr psp, %[to_sp]\n\t" // Set PSP to top of stack
    "mrs r1, control\n\t"
    "mov r2, #0x2\n\t" // Set SPSEL
    "orr r1, r1, r2\n\t"
    "msr control, r1\n\t"
    "pop {pc}\n\t" // Perform the jump
    /*outputs*/: [to_sp] "+r" (to_sp)
    /*inputs*/:
    /*clobbers*/: "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory");
}

extern "C"
void
modm_jumpcontext(modm_context* from, const modm_context to) {
  modm_stack_t** from_sp = &from->sp;
  modm_stack_t* to_sp = to.sp;
  asm volatile(
    "adr r3, 1f\n\t" // Load address of instruction after "pop {pc}" into r3
    "add r3, r3, #1\n\t" // Stay in thumb mode
    "push {r3}\n\t" // Push instruction address to stack (to be used for PC later)
    "mov r3, sp\n\t"
    "str r3, [%[from_sp]]\n\t" // Store the SP in "from"
    "mov sp, %[to_sp]\n\t" // Restore SP from "to"
    "pop {pc}\n\t" // Perform the jump
    ".align 2\n\t"
    "1:\n\t"
    /*outputs*/: [from_sp] "+r" (from_sp), [to_sp] "+r" (to_sp)
    /*inputs*/:
    /*clobbers*/: "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory");
}

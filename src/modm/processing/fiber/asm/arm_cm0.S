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

.file "arm_cm0.S"
.text
.globl modm_makecontext, modm_jumpcontext
.align 2
.type modm_makecontext,%function
.type modm_jumpcontext,%function
.syntax unified

/* Implementation of modm_makecontext / modm_jumpcontext for Cortex-M0(+) based boards.
 *
 * Important note: We are not preserving r8-r11 because they are quite useless on Cortex-M0
 * and storing them takes a lot of cycles (as they aren't supported by the push instruction).
 *
 * Stack layout after makecontext:
 * 0x00: r4 (= returned SP)
 * 0x04: r5
 * 0x08: r6
 * 0x0C: r7
 * 0x10: Address of function (PC)
 */

/* Registers:
 *   r0: Address of modm_context (return value)
 *   r1: Address of stack memory
 *   r2: Size of stack memory
 *   r3: Address of handler function
 */
modm_makecontext:
  @ 1. Move stack pointer to end of stack array
  adds r1, r1, r2
  @ 0x14 = Size of registers (0x10) + sizeof(void*) (0x04)
  subs r1, r1, 0x14

  @ 3. Initialize stack values
  @ Leave registers undefined
  str r3, [r1, 0x10] @ Set function pointer to address of handler function

  @ 4. Fill return value (modm_context)
  str r1, [r0] @ sp
  str r2, [r0, 0x04] @ stack_size
  bx lr

/* Registers:
 *   r0: Address to "from" context
 *   r1: SP in "to" context
 *   r2: Stack size of "to" context
 *   r3: Used internally as tmp register for SP
 * Latency: 20 cycles
 */
modm_jumpcontext:
  @ 1. Push registers to stack
  push {r4-r7, lr}

  @ 2. Store SP in "from"
  mov r3, sp
  str r3, [r0]

  @ 3. Replace SP with "to"
  mov sp, r1

  @ 4. Pop registers from stack
  pop {r4-r7}

  @ 5. Perform the jump
  pop {pc} @ Off we go!

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

#pragma once

#include <modm/architecture/utils.hpp>

#include <cstddef>

typedef uint32_t modm_stack_t;

struct modm_context {
  modm_stack_t* sp;
  std::size_t stack_size = 0;
};

/* Prepares the stack to look like the "from" arg after modm_jumpcontext as run. */
extern "C"
modm_always_inline
modm_context
modm_makecontext(modm_stack_t* stack, std::size_t stack_size, void (*fn)(void)) {
  modm_stack_t* sp = stack + stack_size / sizeof(modm_stack_t) - 1;
  *sp = (modm_stack_t) fn;
  return {sp, stack_size};
}

/* Switches control from the main thread to the user thread. */
extern "C"
modm_always_inline
void
modm_startcontext(const modm_context to);

/* Jumps from the "from" context to the "to" context. */
extern "C"
modm_always_inline
void
modm_jumpcontext(modm_context* from, const modm_context to);

#include "context_cortex_impl.hpp"
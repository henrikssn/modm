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

struct modm_context {
  void* sp;
  std::size_t stack_size = 0;
};

/* Prepares the stack to look like the "from" arg after modm_jumpcontext as run. */
extern "C"
modm_context
modm_makecontext(void* stack, std::size_t stack_size, void (*fn)(void));

/* Pushes all "from" registers to stack, restores "to" registers and performs a jump. */
extern "C"
void
modm_jumpcontext(modm_context* from, const modm_context to);
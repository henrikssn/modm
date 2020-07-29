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

#include <modm/debug/logger.hpp>
#include <modm/processing.hpp>
#include <modm/board.hpp>

#define	MODM_LOG_LEVEL modm::log::INFO

using namespace Board;
using namespace std::chrono_literals;


modm_fastdata uint32_t stack1[32], stack2[32]; // 256 bytes stack
modm_fastdata modm_context f1_ctx, f2_ctx, m_ctx;

void f1() {
  MODM_LOG_DEBUG << "f1: entered" << modm::endl;
  while (1) {
    A0::set();
    modm_jumpcontext(&f1_ctx, f2_ctx);
  }
}

void f2() {
  MODM_LOG_DEBUG << "f2: entered" << modm::endl;
  while (1) {
    A0::reset();
    modm_jumpcontext(&f2_ctx, f1_ctx);
  }
}

// Frequency of A0 is 642kHz, resulting in ~37 CPU cycles per context switch (incl. overhead).
int main() {
  Board::initialize();
  A0::setOutput();
  f1_ctx = modm_makecontext(stack1, sizeof(stack1), &f1);
  f2_ctx = modm_makecontext(stack2, sizeof(stack2), &f2);
  MODM_LOG_DEBUG
    << "main: Jumping to f1 with sp-addr: "
    << modm::hex << f1_ctx.sp
    << " and f-addr: " << modm::hex << *(uint32_t*)(f1_ctx.sp + 0x10)
    << modm::endl;
  modm_jumpcontext(&m_ctx, f1_ctx);
  // Will never get here.
  return 0;
}
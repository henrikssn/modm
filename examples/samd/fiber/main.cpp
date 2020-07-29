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
#include <modm/board.hpp>
#include <modm/processing.hpp>

using namespace Board;
using namespace std::chrono_literals;

#ifdef MODM_BOARD_HAS_LOGGER
#define STACK_SIZE 256
#else
#define STACK_SIZE 32
#endif

modm_fiber
void f1() {
#ifdef MODM_BOARD_HAS_LOGGER
  MODM_LOG_INFO << "f1: entered" << modm::endl;
#endif
  while (1) {
    A0::set();
    modm::yield();
  }
}

modm_fiber
void f2() {
#ifdef MODM_BOARD_HAS_LOGGER
  MODM_LOG_INFO << "f2: entered" << modm::endl;
#endif
  while (1) {
    A0::reset();
    modm::yield();
  }
}

modm::fiber::Stack<STACK_SIZE> stack1, stack2;
modm::Fiber fiber1(stack1, &f1), fiber2(stack2, &f2);

// Frequency of A0 is 417.3kHz, resulting in ~57 CPU cycles per context switch (incl. GPIO overhead).
int main( int argc, char * argv[])
{
  Board::initialize();
  A0::setOutput();
#ifdef MODM_BOARD_HAS_LOGGER
  MODM_LOG_INFO << "Starting scheduler..." << modm::endl;
#endif
  modm::fiber::scheduler().start();
  // Will never get here.
  return 0;
}

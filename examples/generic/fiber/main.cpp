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

constexpr uint32_t cycles = 1000'000;
volatile uint32_t counter = 0;

void f1() {
  while (++counter < cycles) {
    modm::yield();
  }
}

void f2() {
  while (++counter < cycles) {
    asm volatile ("");
    modm::yield();
  }
}

modm::fiber::Stack<2048> stack1, stack2;
modm::Fiber fiber1(stack1, &f1), fiber2(stack2, &f2);


// Blue pill (M3 64MHz): Executed 1000000 in 1520841us (657530.94 yields per second)
// Feather M0 (M0+ 48MHz): Executed 1000000 in 2190108us (456598.50 yields per second)
int main( int argc, char * argv[])
{
  Board::initialize();
  MODM_LOG_INFO << "Starting fiber modm::yield benchmark..." << modm::endl;
  MODM_LOG_INFO.flush();
  const modm::PreciseTimestamp start = modm::PreciseClock::now();
  modm::fiber::scheduler.start();  const auto diff = (modm::PreciseClock::now() - start);
  MODM_LOG_INFO << "Benchmark done!" << modm::endl;
  MODM_LOG_INFO << "Executed " << cycles << " in " << diff << " (";
  MODM_LOG_INFO.printf("%.2f", cycles * 1'000'000.0 / std::chrono::microseconds(diff).count());
  MODM_LOG_INFO << " yields per second)" << modm::endl;
  MODM_LOG_INFO.flush();
  while (1);
  return 0;
}

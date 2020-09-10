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

#include "fiber.hpp"

#define MODM_BOARD_HAS_LOGGER

#ifdef MODM_BOARD_HAS_LOGGER
#include <modm/debug/logger.hpp>
#endif

namespace modm {

void Fiber::dumpStack()  {
#ifdef MODM_BOARD_HAS_LOGGER
  MODM_LOG_DEBUG
    << "Fiber: " << modm::hex << this
    << " Stack pointer: " << ctx_.sp
    << " Stack size: " << modm::ascii << ctx_.stack_size << modm::endl;
  size_t* stack = (size_t*) stack_;
  for (size_t i = 0; i < ctx_.stack_size / sizeof(modm_stack_t); ++i)
    MODM_LOG_INFO << stack+i << " (" << i << "): " << modm::hex << *(stack + i) << modm::endl;
#endif
}

void Waitable::wait() {
  using ::modm::fiber::scheduler;
  pushWaiter(scheduler.removeCurrent());
  yield();
}

void Waitable::signal() {
  using ::modm::fiber::scheduler;
  Fiber* waiter = popWaiter();
  if (waiter != nullptr) {
    scheduler.runNext(waiter);
    yield();
  }
}

Fiber* Waitable::popWaiter() {
  if (!last_waiter_) {
    return nullptr;
  }
  Fiber* first = last_waiter_->next();
  if (first == last_waiter_) {
    last_waiter_ = nullptr;
  } else {
    last_waiter_->next(first->next());
  }
  first->next(nullptr);
  return first;
}

void Waitable::pushWaiter(Fiber* waiter) {
  if (last_waiter_) {
    waiter->next(last_waiter_->next());
    last_waiter_->next(waiter);
  } else {
    waiter->next(waiter);
  }
  last_waiter_ = waiter;
}

} // namespace modm

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

#include <modm/architecture/interface/assert.hpp>

#include <utility>

namespace modm
{

/*
 * Important: Since this function is using callee's stack, you should not use any stack. That means
 * all function calls need to be inlined and all variables register allocated.
 */
void yield()
{
  using fiber::scheduler;
  Fiber* current = scheduler.currentFiber();
  Fiber* next;
  if (scheduler.last_fiber_->next() != current) {
    next = scheduler.lastFiber()->next();
  } else {
    next = current->next();
    scheduler.last_fiber_ = current;
  }
  if (current == next) return;
  scheduler.current_fiber_ = next;
  current->jump(*next);
}

template<int size>
Fiber::Fiber(fiber::Stack<size>& stack, void(*f)())
	: stack_(stack.memory_)
{
	ctx_ = modm_makecontext(stack.memory_, sizeof(stack.memory_), f, done);
	fiber::scheduler.registerFiber(this);
}

void Fiber::done() {
  using fiber::scheduler;
  scheduler.removeCurrent();
  if (scheduler.empty()) {
// #ifdef MODM_BOARD_HAS_LOGGER
//   MODM_LOG_DEBUG << "Fiber::done: Returning control to main thread." << modm::endl;
// #endif
    modm_endcontext();
  }
  yield();
}

template<class Data_t>
void Channel<Data_t>::send(const Data_t& data) {
  if (full()) {
    wait();
  }
  // Now we are in empty or ready state.
  modm_assert(!full(), "Channel::push.full", "Channel should not be full");
  if (empty()) {
    data_ = data;
  } else {
    buffer_[size_ - 1] = data;
  }
  ++size_;
  signal();
}

template<class Data_t>
Data_t Channel<Data_t>::recv() {
  if (empty()) {
    wait();
  }
  // Now we are in full or ready state.
  modm_assert(!empty(), "Channel::poll.empty", "Channel should not be empty");
  Data_t result;
  if (--size_) {
    result = std::move(buffer_[size_]);
  } else {
    result = std::move(data_);
  }
  signal();
  return result;
}

namespace fiber {

void Scheduler::registerFiber(Fiber* fiber) {
  if (last_fiber_ == nullptr) {
    fiber->next(fiber);
    last_fiber_ = fiber;
    current_fiber_ = fiber;
    return;
  }
  runLast(fiber);
}

void Scheduler::start() {
  if (last_fiber_ == nullptr) return;
// #ifdef MODM_BOARD_HAS_LOGGER
//   MODM_LOG_DEBUG << "Starting scheduler with fibers [ current = " << currentFiber()
//   << ", last = " << lastFiber()
//   << " ] " << modm::endl;
//   for (Fiber* fiber = currentFiber();; fiber = fiber->next()) {
//     MODM_LOG_DEBUG
//       << "(" << modm::hex << fiber
//       << ") { sp: " << fiber->ctx_.sp
//       << ", next: " << fiber->next() << " }"
//       << modm::endl;
//     if (fiber->next() == currentFiber()) break;
//   }
// #endif
  modm_startcontext(currentFiber()->ctx_);
}

}

} // namespace modm

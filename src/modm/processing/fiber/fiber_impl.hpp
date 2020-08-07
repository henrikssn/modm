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

namespace modm {

/*
 * Important: Since this function is using callee's stack, you should not use any stack. That means
 * all function calls need to be inlined and all variables register allocated.
 */
void yield() {
  using fiber::scheduler;
  Fiber& current = *scheduler().current_fiber_;
  scheduler().current_fiber_ = current.next_;
  current.jump(*current.next_);
}

template<int size>
Fiber::Fiber(fiber::Stack<size>& stack, void(*f)()) : stack_(stack.memory_) {
  ctx_ = modm_makecontext(stack.memory_, sizeof(stack.memory_), f);
  fiber::scheduler().registerFiber(this);
}

void Fiber::dumpStack()  {
#ifdef MODM_BOARD_HAS_LOGGER
  MODM_LOG_DEBUG
    << "Fiber: " << modm::hex << this
    << " Stack pointer: " << ctx_.sp
    << " Stack size: " << modm::ascii << ctx_.stack_size << modm::endl;
  uint32_t* stack = stack_;
  for (size_t i = 0; i < ctx_.stack_size/4; ++i)
    MODM_LOG_INFO << stack << " (" << i << "): " << modm::hex << *stack++ << modm::endl;
#endif
}

namespace fiber {

void Scheduler::registerFiber(Fiber* fiber) {
  if (!fibers_size_) {
    fiber->next_ = fiber;
  } else {
    fibers_.at(fibers_size_ - 1)->next_ = fiber;
    fiber->next_ = fibers_.at(0);
  }
  fibers_.at(fibers_size_++) = fiber;
}

void Scheduler::start() {
  current_fiber_ = fibers_[0];
  Fiber& fiber = *current_fiber_;
#ifdef MODM_BOARD_HAS_LOGGER
  MODM_LOG_DEBUG << "Starting scheduler with fibers " << modm::endl;
  for (int i = 0; i < 2; ++i) {
    MODM_LOG_DEBUG
      << i << ": (" << modm::hex << fibers_[i]
      << ") { sp: " << fibers_[i]->ctx_.sp
      << ", addr: " << *fibers_[i]->ctx_.sp
      << ", next: " << fibers_[i]->next_ << " }"
      << modm::endl;
  }
  MODM_LOG_DEBUG
    << "scheduler: Jumping to fiber with sp-addr: " << modm::hex << fiber.ctx_.sp
    << " and f-addr: " << modm::hex << *fiber.ctx_.sp
    << modm::endl;
#endif
  modm_startcontext(fiber.ctx_);
}

}

} // namespace modm
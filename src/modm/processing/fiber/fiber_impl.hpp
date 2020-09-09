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

namespace modm
{

/*
 * Important: Since this function is using callee's stack, you should not use any stack. That means
 * all function calls need to be inlined and all variables register allocated.
 */
void yield()
{
	Fiber* current = fiber::scheduler.current_fiber_;
	if (current->next_ != current) {
		fiber::scheduler.current_fiber_ = current->next_;
		current->jump(*current->next_);
	}
}

template<int size>
Fiber::Fiber(fiber::Stack<size>& stack, void(*f)())
	: stack_(stack.memory_)
{
	ctx_ = modm_makecontext(stack.memory_, sizeof(stack.memory_), f, done);
	fiber::scheduler.registerFiber(this);
}

void Fiber::dumpStack()
{
#ifdef MODM_BOARD_HAS_LOGGER
	MODM_LOG_DEBUG
		<< "Fiber: " << modm::hex << this
		<< " Stack pointer: " << ctx_.sp
		<< " Stack size: " << modm::ascii << ctx_.stack_size << modm::endl;
	size_t* stack = (size_t*) stack_;
	for (size_t i = 0; i < ctx_.stack_size/4; ++i)
		MODM_LOG_INFO << stack+i << " (" << i << "): " << modm::hex << *(stack + i) << modm::endl;
#endif
}

void Fiber::done()
{
	Fiber* current = fiber::scheduler.current_fiber_;
	if (current->next_ == current) {
		fiber::scheduler.current_fiber_ = nullptr;
		modm_endcontext();
	}
	Fiber* prev_fiber = current;
	while(prev_fiber->next_ != fiber::scheduler.current_fiber_)
		prev_fiber = prev_fiber->next_;
	prev_fiber->next_ = prev_fiber->next_->next_;
	yield();
}

namespace fiber
{

void Scheduler::registerFiber(Fiber* fiber)
{
	if (current_fiber_ == nullptr)
	{
		fiber->next_ = fiber;
		current_fiber_ = fiber;
		return;
	}
	fiber->next_ = current_fiber_->next_;
	current_fiber_->next_ = fiber;
}

void Scheduler::start()
{
	Fiber& fiber = *current_fiber_;

#ifdef MODM_BOARD_HAS_LOGGER
	MODM_LOG_DEBUG << "Starting scheduler with fibers " << modm::endl;
	for (Fiber* fiber = current_fiber_;; fiber = fiber->next_) {
		MODM_LOG_DEBUG
			<< "(" << modm::hex << fiber
			<< ") { sp: " << fiber->ctx_.sp
			<< ", addr: " << *fiber->ctx_.sp
			<< ", next: " << fiber->next_ << " }"
			<< modm::endl;
		if (fiber->next_ == current_fiber_) break;
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

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

#include "context.hpp"
#ifdef MODM_BOARD_HAS_LOGGER
#include <modm/debug/logger.hpp>
#endif

namespace modm
{

class Fiber;
inline void
yield();

namespace fiber
{

class Scheduler;

template<int size>
class Stack
{
	static_assert(size % 4 == 0, "Stack size must be multiple of 4.");
	static_assert(size >= 40, "Stack size must at least 40 bytes.");
	friend class ::modm::Fiber;
	friend class Scheduler;

public:
	Stack() = default;
	Stack(const Stack&) = delete;

private:
	modm_aligned(8)
	modm_stack_t memory_[size / sizeof(modm_stack_t)];
};

} // namespace fiber

class Fiber
{
	friend class fiber::Scheduler;
	friend void yield();
 public:
	template<int size>
	Fiber(fiber::Stack<size>& stack, void(*f)());

	inline void
	dumpStack();

protected:
	inline void
	jump(Fiber& other)
	{ modm_jumpcontext(&ctx_, other.ctx_); }

private:
	Fiber() = default;
	Fiber(const Fiber&) = delete;

	// Removes the current fiber from the scheduler and yields execution. This is called on fiber
	// return.
	static inline void
	done();

private:
	modm_context ctx_;
	modm_stack_t stack_;
	Fiber* next_;
};

namespace fiber
{

class Scheduler
{
	friend class modm::Fiber;
	friend void modm::yield();
public:
	Scheduler() = default;

	inline void
	start();

protected:
	inline void
	registerFiber(Fiber*);

	inline Fiber*
	currentFiber() { return current_fiber_; }

 private:
	Scheduler(const Scheduler&) = delete;
	Fiber* current_fiber_ = nullptr;
};

static inline Scheduler scheduler;

} // namespace fiber

} // namespace modm

#include "fiber_impl.hpp"

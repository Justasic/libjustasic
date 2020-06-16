// Copyright (c) 2014-2020, Justin Crawford <Justin@stacksmash.net>
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.

// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Timer.h"
#include <algorithm>

// All our timers :D
std::vector<Timer *> TimerHandler::timers;

// Initialize our timer class.
Timer::Timer(time_t timeout, bool repeat)
	: TimeOut(repeat ? time(NULL) + timeout : timeout), RepeatInterval(repeat ? timeout : 0UL), Repeat(repeat)
{
	TimerHandler::RegisterTimer(this);
}

Timer::~Timer() { TimerHandler::DelistTimer(this); }

void TimerHandler::TickTimers()
{
	// Get our current time and store it.
	time_t thetime = time(NULL);
	for (auto it : timers)
	{
		// We found a timer to tick.
		if (it->TimeOut <= thetime)
		{
			it->Tick(thetime);
			// Get the current time from the time function because
			// some Tick function calls may take extraordinarily long
			if (it->Repeat)
				it->TimeOut = time(NULL) + it->RepeatInterval;
			else
				delete it;
		}
	}
}

void TimerHandler::RegisterTimer(Timer *t) { timers.push_back(t); }

void TimerHandler::DelistTimer(Timer *t)
{
	auto iter = std::find(timers.begin(), timers.end(), t);
	if (iter != timers.end())
		timers.erase(iter);
}

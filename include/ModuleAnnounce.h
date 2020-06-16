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

#pragma once
#include "Exceptions.h"
#include "Utilities.h"
#include <mutex>

enum EventResult
{
	EVENT_CONTINUE,
	EVENT_STOP
};

extern std::mutex eventmux;

/**************************************************************/
#ifdef HAVE_SETJMP_H
// We have setjmp, try and recover from Segmentation Faults
#	define FOREACH_MOD(y, ...)                                                                                                      \
		if (true)                                                                                                                    \
		{                                                                                                                            \
			eventmux.lock();                                                                                                         \
			std::vector<Module *>::iterator safei;                                                                                   \
			for (std::vector<Module *>::iterator _i = ModuleHandler::EventHandlers[I_##y].begin();                                   \
				 _i != ModuleHandler::EventHandlers[I_##y].end();)                                                                   \
			{                                                                                                                        \
				safei = _i;                                                                                                          \
				++safei;                                                                                                             \
				try                                                                                                                  \
				{                                                                                                                    \
					SET_SEGV_LOCATION(*_i, nullptr);                                                                                 \
					if (setjmp(sigbuf) == 0)                                                                                         \
					{                                                                                                                \
						LastRunModule = *_i;                                                                                         \
						(*_i)->y(__VA_ARGS__);                                                                                       \
					} else                                                                                                           \
						throw ModuleException("%s failed to run ## y event. Segmentation fault occured.", LastRunModule->GetName()); \
				}                                                                                                                    \
				catch (const ModuleException &modexcept)                                                                             \
				{                                                                                                                    \
					"Exception caught: {}"_le(modexcept.what());                                                                     \
				}                                                                                                                    \
				_i = safei;                                                                                                          \
			}                                                                                                                        \
			eventmux.unlock();                                                                                                       \
		} else                                                                                                                       \
			static_cast<void>(0)

#	define FOREACH_RESULT(y, v, ...)                                                                                              \
		if (true)                                                                                                                  \
		{                                                                                                                          \
			eventmux.lock();                                                                                                       \
			std::vector<Module *>::iterator safei;                                                                                 \
			v = EVENT_CONTINUE;                                                                                                    \
			for (std::vector<Module *>::iterator _i = ModuleHandler::EventHandlers[I_##y].begin();                                 \
				 _i != ModuleHandler::EventHandlers[I_##y].end();)                                                                 \
			{                                                                                                                      \
				safei = _i;                                                                                                        \
				++safei;                                                                                                           \
				try                                                                                                                \
				{                                                                                                                  \
					if (setjmp(sigbuf) == 0)                                                                                       \
					{                                                                                                              \
						SET_SEGV_LOCATION(*_i, nullptr);                                                                           \
						EventResult res = (*_i)->y(__VA_ARGS__);                                                                   \
						if (res != EVENT_CONTINUE)                                                                                 \
						{                                                                                                          \
							v = res;                                                                                               \
							break;                                                                                                 \
						}                                                                                                          \
					} else                                                                                                         \
						throw ModuleException("%s failed to run an event. Segmentation fault occured.", LastRunModule->GetName()); \
				}                                                                                                                  \
				catch (const ModuleException &modexcept)                                                                           \
				{                                                                                                                  \
					"Exception caught: {}"_le(modexcept.what());                                                                   \
				}                                                                                                                  \
				_i = safei;                                                                                                        \
			}                                                                                                                      \
			eventmux.unlock();                                                                                                     \
		} else                                                                                                                     \
			static_cast<void>(0)

#else // HAVE_SETJMP_H
// We don't have setjmp
#	define FOREACH_MOD(y, ...)                                                                    \
		if (true)                                                                                  \
		{                                                                                          \
			eventmux.lock();                                                                       \
			std::vector<Module *>::iterator safei;                                                 \
			for (std::vector<Module *>::iterator _i = ModuleHandler::EventHandlers[I_##y].begin(); \
				 _i != ModuleHandler::EventHandlers[I_##y].end();)                                 \
			{                                                                                      \
				safei = _i;                                                                        \
				++safei;                                                                           \
				try                                                                                \
				{                                                                                  \
					SET_SEGV_LOCATION(*_i, nullptr);                                               \
					(*_i)->y(__VA_ARGS__);                                                         \
				}                                                                                  \
				catch (const ModuleException &modexcept)                                           \
				{                                                                                  \
					"Exception caught: {}"_le(modexcept.what());                                   \
				}                                                                                  \
				_i = safei;                                                                        \
			}                                                                                      \
			eventmux.unlock();                                                                     \
		} else                                                                                     \
			static_cast<void>(0)

#	define FOREACH_RESULT(y, v, ...)                                                              \
		if (true)                                                                                  \
		{                                                                                          \
			eventmux.lock();                                                                       \
			std::vector<Module *>::iterator safei;                                                 \
			v = EVENT_CONTINUE;                                                                    \
			for (std::vector<Module *>::iterator _i = ModuleHandler::EventHandlers[I_##y].begin(); \
				 _i != ModuleHandler::EventHandlers[I_##y].end();)                                 \
			{                                                                                      \
				safei = _i;                                                                        \
				++safei;                                                                           \
				try                                                                                \
				{                                                                                  \
					SET_SEGV_LOCATION(*_i, nullptr);                                               \
					EventResult res = (*_i)->y(__VA_ARGS__);                                       \
					if (res != EVENT_CONTINUE)                                                     \
					{                                                                              \
						v = res;                                                                   \
						break;                                                                     \
					}                                                                              \
				}                                                                                  \
				catch (const ModuleException &modexcept)                                           \
				{                                                                                  \
					"Exception caught: {}"_le(modexcept.what());                                   \
				}                                                                                  \
				_i = safei;                                                                        \
			}                                                                                      \
			eventmux.unlock();                                                                     \
		} else                                                                                     \
			static_cast<void>(0)

#endif // HAVE_SETJMP_H

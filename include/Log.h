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
#include "Flux.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#define SPDLOG_ENABLE_SYSLOG 1
#include <spdlog/spdlog.h>

namespace spd = spdlog;

// Forward declare because we dont need recursive includes.
class Command;
class User;

/**
 * @brief Basic logging class to make things MUCH nicer and easier to write.
 * @class Log
 */
class Log
{
	spd::level::level_enum				lvl;
	std::string							message;
	bool								alreadylogged;
	static std::shared_ptr<spd::logger> logsink;

  public:
	/// @brief Defined for user-defined literal.
	/// @param l The log level used for spdlog. Values can be spd::level::info,warn,err,critical.
	/// @param str the log string.
	/// @param len the length of log string.
	Log(spd::level::level_enum l, const char *str, size_t len) : lvl(l), message(str, len), alreadylogged(false) {}

	~Log()
	{
		if (!this->alreadylogged)
			Log::logsink->log(this->lvl, this->message.c_str());
	}

	// C++ likes us to have the call operator defined.
	template<typename... Args>
	Log &operator()(Args &&... args)
	{
		Log::logsink->log(this->lvl, this->message.c_str(), args...);
		this->alreadylogged = true;
		return *this;
	}

	// Used for spdlog.
	static inline void Initialize()
	{
		try
		{
			// Create an async console and syslog logger.
			spd::set_async_mode(8192);
			// Make a vector of sinks we want to use (currently console ansi color and syslog.)
			std::vector<spd::sink_ptr> sinks;
			sinks.push_back(std::make_shared<spd::sinks::ansicolor_stdout_sink_mt>());
			sinks.push_back(std::make_shared<spd::sinks::syslog_sink>("silicon"));

			// Initialize the sink pointer.
			Log::logsink = std::make_shared<spd::logger>("silicon", begin(sinks), end(sinks));
			// Register the sink with spdlog.
			spd::register_logger(Log::logsink);
		}
		catch (const spd::spdlog_ex &ex)
		{
			// Oops we blew up :(
			std::cerr << "Log initialization failed: " << ex.what() << std::endl;
		}
	}

	static inline void Terminate()
	{
		try
		{
			spd::drop_all();
		}
		catch (const spd::spdlog_ex &ex)
		{
			// Oops we blew up :(
			std::cerr << "Log termination failed: " << ex.what() << std::endl;
		}
	}
};

// Allow for "Here is the value: %s!"_l("the value!"); style logs.
inline Log operator"" _l(const char *str, size_t len) { return Log(spd::level::info, str, len); }
inline Log operator"" _lw(const char *str, size_t len) { return Log(spd::level::warn, str, len); }
inline Log operator"" _le(const char *str, size_t len) { return Log(spd::level::err, str, len); }
inline Log operator"" _lc(const char *str, size_t len) { return Log(spd::level::critical, str, len); }

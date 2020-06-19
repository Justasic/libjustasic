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

#include "Config/ConfigurationDriver.h"
#include "File.h"
#include "Log.h"
#include "parser.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <map>
#include <new>
#include <unistd.h>
#include <vector>

ConfigurationDriver::ConfigurationDriver(const std::string &filepath) : unsuccessful(false), filename(filepath), file(filepath)
{
	if (!FileSystem::FileExists(file))
		throw std::filesystem::filesystem_error("File does not exist", this->file, std::error_code());

	if (!std::filesystem::is_regular_file(this->file))
		throw std::filesystem::filesystem_error("Not regular file", this->file, std::error_code());
}

ConfigurationDriver::~ConfigurationDriver() {}
void ConfigurationDriver::error(const std::string &m)
{
	"[Configuration] {}"_lc(m);
	unsuccessful = true;
}
void ConfigurationDriver::error(const yy::location &l, const std::string &m)
{
	"[Configuration] {}: line -> {}: column -> {}-{}: {}"_lc(
		(l.begin.filename ? l.begin.filename->c_str() : "(undefined)"), l.begin.line + 1, l.begin.column, l.end.column, m);
	unsuccessful = true;
}

void ConfigurationDriver::Parse()
{
	std::ifstream f(this->file);

	if (!f.good())
	{
		this->error("Cannot open config file (parse step)\n");
		unsuccessful = true;
		return;
	}

	lexer.switch_streams(&f, nullptr);

	yy::Parser parser(this);
	parser.parse();
}

// Find a section.
std::optional<ConfigurationDriver::Section> ConfigurationDriver::FindSection(std::string_view key)
{
	auto value = this->values.find(key.data());
	if (value == this->values.end())
		return std::nullopt;

	ConfigurationDriver::Section sect(value->first, value->second);
	return std::make_optional(sect);
}

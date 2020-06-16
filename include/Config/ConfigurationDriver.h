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

#include "location.hh"
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string_view>

#include "Config/yyLexFlex.h"
#include "parser.hpp"

namespace std
{
	namespace filesystem = std::experimental::filesystem;
}

class ConfigurationException : public std::exception
{
	const char *value;

  public:
	ConfigurationException(const char *what) : value(what) {}
	const char *what() const noexcept override { return this->value; }
};

class ConfigurationDriver
{
	// Wether or not we're successful in parsing the file
	bool unsuccessful;

  public:
	ConfigurationDriver(const std::string &file);
	virtual ~ConfigurationDriver();

	class Section
	{
		// Our internal variables, read-only
		std::string name{};

		// Only the configuration class can construct a section.
		Section(const std::string &name, std::map<std::string, expression> data) : name(name), values(data) {}

		// Friend so the config class can actually use above constructor.
		friend ConfigurationDriver;

	  public:
		std::map<std::string, expression> values{};
		inline std::string_view			  GetName() const { return this->name; }

		// This function will either return a value according to the value specified
		// or it will return nullopt which is testable when getting values.
		template<typename T>
		std::optional<T> GetValue(std::string_view key)
		{
			std::stringstream ss;
			auto			  value = values.find(key.data());
			if (value == values.end())
				return std::nullopt;

			switch (value->second.type)
			{
				case ex_type::INTEGER:
					ss << value->second.number;
					break;
				case ex_type::STRING:
					ss << value->second.string;
					break;
				case ex_type::FLOATING:
					ss << value->second.floating;
					break;
				case ex_type::BOOLEAN:
					ss << value->second.boolean;
					break;
				case ex_type::UNKNOWN:
				default:
					return std::nullopt;
			}

			T type{};
			ss >> type;
			return std::make_optional(type);
		}
	};

	// The current section we're parsing.
	// TODO: There's probably a better way to handle this.
	std::string csect{};

	// The current location we're at.
	yy::location locat;

	// Current scanner.
	ConfFlexLexer lexer;

	// Current filename
	std::string filename{};

	// Current filesystem path for the file.
	std::filesystem::path file{};

	// Parse shit
	void Parse();

	// Ensure we were successful.
	inline bool good() const { return !this->unsuccessful; }

	// Find a section.
	std::optional<ConfigurationDriver::Section> FindSection(std::string_view key);

	// Read a value from the config file.
	template<typename T, typename Y>
	T value_or(std::string_view section, std::string_view key, Y &&default_value)
	{
		// First, try and find the section or return the default_value
		auto sect = this->FindSection(section);
		if (!sect)
			return static_cast<T>(std::forward<Y>(default_value));

		// Then try and find the key or return the default_value
		auto retkey = sect->GetValue<T>(key);
		if (!retkey)
			return static_cast<T>(std::forward<Y>(default_value));

		// Return a value :D
		return *retkey;
	}

	// The raw map of parsed values.
	std::map<std::string, std::map<std::string, expression>> values{};

	// Error handling.
	void error(const yy::location &l, const std::string &m);
	void error(const std::string &m);
};
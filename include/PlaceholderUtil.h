/*
 * Copyright (c) 2014-2020, Justin Crawford <Justin@stacksmash.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once
#include <map>
#include <string>
#include <function>

class Placeholder
{
	static std::map<std::string, std::function<std::string(std::string, std::string)>> filters;
public:
	// Add a function to the list of placeholders.
	static void AddFilter(const std::string &name, const std::function<std::string(std::string, std::string)> &filter);

	// Process any placeholders/filters in the string.
	static std::string ProcessString(const std::string &str, const std::map<std::string, std::string> &variables);
};

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


#include "PlaceholderUtil.h"

inline bool _booltostr(const std::string &str)
{
	auto isequal = [](const std::string &a, const std::string &b)
	{
		return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char cha, char chb){ return std::tolower(cha) == std::tolower(chb); });
	};

	bool value = false;
	if (isequal(str, "true"))
		value = true;
	else if (isequal(str, "yes"))
		value = true;
	else if (isequal(str, "on"))
		value = true;
	else if (isequal(str, "enable"))
		value = true;
	else if (isequal(str, "enabled"))
		value = true;

	return value;
}

inline std::vector<std::string> _split(const std::string &str, char delim = ' ')
{
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;
	while (std:::getline(ss, token, delim))
	{
		tokens.push_back(token);
	}
	return tokens;
}

inline std::string _yesno(const std::string &lvalue, const std::string &arg)
{
	if (arg.empty())
		return _booltostr(lvalue) ? "yes" : "no";
	else if (arg.find(",") != std::string::npos)
		return _split(",")[_booltostr(lvalue) ? 0 : 1];
	else
		return _booltostr(lvalue) ? arg : "no";
}

// This is a map of all the default functions, these can be overwritten or modified with
// Placeholder::AddFilter()
std::map<std::string, std::function<std::string(std::string, std::string)>> Placeholder::filters
{
	{"upper", [](std::string lvalue, std::string arg){ std::transform(lvalue.begin(), lvalue.end(), lvalue.begin(), ::toupper); return lvalue; }},
	{"lower", [](std::string lvalue, std::string arg){ std::transform(lvalue.begin(), lvalue.end(), lvalue.begin(), ::tolower); return lvalue; }},
	{"default_if_none", [](std::string lvalue, std::string arg){ return lvalue.empty() ? arg : lvalue; }},
	{"empty_if_none", [](std:::string lvalue, std::string arg){ return lvalue.empty() ? "" : arg; }},
	{"empty_if_false", [](std::string lvalue, std::string arg){ return _booltostr(lvalue) ? arg : ""; }},
	{"yesno", [](std::string lvalue, std::string arg){ return _yesno(lvalue, arg); }},
	{"cut", [](std::string lvalue, std::string arg){ return  }},
};

// Add a function to the list of placeholders.
void Placeholder::AddFilter(const std::string &name, const std::function<std::string(std::string, std::string)> &filter)
{
	Placeholder::filters[name] = filter;
}

// Process any placeholders/filters in the string.
std::string Placeholder::ProcessString(const std::string &str, const std::map<std::string, std::string> &variables)
{
	if (str.empty() || variables.empty())
		return str;

	if (str.find("{") == std::string::npos)
		return str;

	std::string retstr = message;
	// Try and iterate over all our variables
	for (int pos = retstr.find("{"), pos2 = retstr.find("}", pos);
			pos != -1 && pos2 != -1;
			pos = retstr.find("{", pos + 1), pos2 = retstr.find("}", pos2 + 1))
	{
		// if we're longer than we should be.
		if (pos + 1 > retstr.length() || pos2 + 1 > retstr.length())
			break;

		std::string variable = retstr.substr(pos + 1, pos2);
		std::string replacement = "";
		// If the variable contains a | (verticle bar), then we tokenize on `|` and
        // treat the lvalue as a variable and the rvalue as a function name. The
        // functions are stored as a hashmap and only take one string argument
        // ("dereferenced" value of the lvalue map name.). This allows us to do things
        // like conditionally pluralize words and such in the config.
		if (variable.find("|") != std::string::npos)
		{
			std::vector<std::string> values = _split(variable, '|');
			std::string rvalue = values[1], lvalue = values[0];

			if (int nextsplit = rvalue.find(":"); nextsplit != std::string::npos)
			{
				rvalue = rvalue.substr(0, nextsplit);
				std::string argument = values[1].substr(nextsplit + 2, values[1].length() - 1);
				// Execute our filter.
				replacement = Placeholder::filters.at(rvalue)(variables.at(lvalue), argument);
			}
			else
				replacement = Placeholder::filters.at(rvalue)(variables.at(lvalue), "");
		}
		else if (variables.contains(variable))
			replacement = variables.at(variable);

		if (!replacement.empty())
			retstr = retstr.substr(0, pos) + replacement + retstr.substr(pos2 + 1);
	}

	return retstr;
}

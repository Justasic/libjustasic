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

#ifndef YY_DECL
#	define YY_DECL yy::Parser::symbol_type ConfFlexLexer::yylex(ConfigurationDriver *ctx)
#endif

// We need this for yyFlexLexer. If we don't #undef yyFlexLexer, the
// preprocessor chokes on the line `#define yyFlexLexer yyFlexLexer`
// in `FlexLexer.h`:
#undef yyFlexLexer
#include <FlexLexer.h>

// We need this for the yy::Parser::symbol_type:
#include "parser.hpp"

class ConfFlexLexer : public yyFlexLexer
{
  public:
	// Make clang shut up about the override
	using yyFlexLexer::yylex;
	// Use the superclass's constructor:
	using yyFlexLexer::yyFlexLexer;

	// Provide the interface to `yylex`; `flex` will emit the
	// definition into `calc++-scanner.cc`:
	virtual yy::Parser::symbol_type yylex(ConfigurationDriver *ctx);
};
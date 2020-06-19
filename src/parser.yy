/* MIT License
 *
 * Copyright (c) 2018 Justin Crawford <Justin@stacksmash.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */ 
// Use newer C++ code
%skeleton "lalr1.cc"
%require "3.0"
%defines
// Tell bison we want to use this as our class name
%define api.parser.class { Parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%file-prefix "parser"

%code requires
{
	#include <string>
	class ConfigurationDriver;

	enum ex_type
	{
		UNKNOWN = 0,
		INTEGER,
		STRING,
		BOOLEAN,
		FLOATING
	};

	// Our variable type structure
	struct expression
	{
		ex_type		type;
		std::string string;
		long		number;
		double		floating;
		bool		boolean;
	};
}

// Add our custom Config class to the parser
// so we can use it. Forward declare it because
// it cannot be included in the definitions file.
%param { ConfigurationDriver *ctx }

%locations
%initial-action
{
	// Initialize the initial location
	@$.begin.filename = @$.end.filename = &ctx->filename;
};

// Enable tracing and verbose errors (which may be wrong!)
%define parse.trace
%define parse.error verbose

%code
{
	#include "Config/ConfigurationDriver.h"
	/* this "connects" the bison parser in the driver to the flex scanner class
	* object. it defines the yylex() function call to pull the next token from the
	* current lexer object of the driver context. */
	//#undef yylex
	#define yylex ctx->lexer.yylex
	#define M(x) std::move(x)
}

%token CINT STR IDENTIFIER BOOL FLOAT UNKNOWN
%token END 0 "end of file"
%type <expression> item_type
%type <long> CINT
%type <double> FLOAT
%type <std::string> STR IDENTIFIER
%type <bool> BOOL

%start conf

%%

 /* Start of the document we're parsing. */
conf: | conf conf_item;

 /* Identify the section we're currently parsing */
conf_item: IDENTIFIER
{
	ctx->csect = M($1);
	ctx->values[ctx->csect];
} '{' section_items '}';

 /* Concatenate the sections in bison so we parse an unlimited number */
section_items: | section_items section_item;

 /* Parse each section statement */
section_item: IDENTIFIER '=' item_type ';'
{
	ctx->values[ctx->csect][$1] = M($3);
};

 /* Define all our possible types */
item_type: CINT  { $$.number = M($1);   $$.type = ex_type::INTEGER;  }
		 | STR   { $$.string = M($1);   $$.type = ex_type::STRING;   }
		 | BOOL  { $$.boolean = M($1);  $$.type = ex_type::BOOLEAN;  }
		 | FLOAT { $$.floating = M($1); $$.type = ex_type::FLOATING; }
		 ;
%%

void yy::Parser::error(const location_type &l, const std::string &m)
{
	ctx->error(l, m);
}

%{
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
	#include "Config/ConfigurationDriver.h"
	using namespace std::literals::string_literals;
	/*  int lineno = 0;*/
	int commentstart = 0;

	// Work around an incompatibility in flex (at least versions
	// 2.5.31 through 2.5.33): it generates code that does
	// not conform to C89.  See Debian bug 333231
	// <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.
	# undef yywrap
	# define yywrap() 1

	#define YY_USER_ACTION ctx->locat.columns(yyleng);
	
	// CHANGE: We must exclude unistd.h or the compiler will choke on the
	// `isatty()` declaration emitted by `flex` having a different
	// exception specifier from the one in `unistd.h`:
	#define YY_NO_UNISTD_H
%}

%option c++
%option noyywrap nounput noinput yylineno batch debug

%x COMMENT

%%

%{
	// start where previous token ended.
	ctx->locat.step();
%}

 /* Precidence of the actions below matters. Certain rules must match first
 before other rules may continue. By allowing, for example, float to preceed
 integers, this allows the parser to recognize 3.14 as a float instead of as
 the integer 3 or some syntax error. */

 /* Ignore white space */
[ \t\r]+               { ctx->locat.step(); }
\n                     { ctx->locat.lines(yyleng); ctx->locat.step(); }
 /* Match floating point values */
[0-9]+"."[0-9]*        { return yy::Parser::make_FLOAT(atof(yytext), ctx->locat); }
 /* Match integer values */
[0-9]+                 { return yy::Parser::make_CINT(atoi(yytext), ctx->locat); }
 /* Match quoted strings */
\"[^\"\n]*[\"\n]       { return yy::Parser::make_STR(std::string(yytext+1, yytext+(yyleng-1)), ctx->locat); }
 /* Match boolean values */
true|yes               { return yy::Parser::make_BOOL(true, ctx->locat); }
false|no               { return yy::Parser::make_BOOL(false, ctx->locat); }
 /* match identifiers */
[A-Za-z][A-Za-z0-9_-]* { return yy::Parser::make_IDENTIFIER(std::string(yytext, yyleng), ctx->locat); }
 /* Match special chars we use */
[{};=]                 { return yy::Parser::symbol_type(yy::Parser::token_type(*yytext), ctx->locat); }
 /* Match c/c++ style comments */
"/*"                   { BEGIN(COMMENT); commentstart = ctx->locat.begin.line; }
<COMMENT>"*/"          { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.  ;
<COMMENT><<EOF>>       { ctx->error(ctx->locat, "Unterminated comment"); return yy::Parser::make_END(ctx->locat); }
"//".*\n               { /* Ignore comments */ }

 /* All other text */
.                      { ctx->error(ctx->locat, "Unknown token "s + std::string(yytext)); }

 /* End of file */
<<EOF>>                { return yy::Parser::make_END(ctx->locat); }

%%

int yyFlexLexer::yylex()
{
    std::cerr << "'int yyFlexLexer::yylex()' should never be called." << std::endl;
    exit(1);
}
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

// Just don't even bother formatting this file... It's format is too specific and non-standard to the rest
// of the codebase that formatting it with clang-format would be incredibly unreadable.
// clang-format off
#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm> //For std::transform for Flux::string.tolower() and toupper()
#include <sstream>
#include <iomanip>   // For std::setprecision
#include <bitset>
#include <cmath>     // Required for nlohmann's JSON.hpp
#include <ios>
#include <string_view>
#include <experimental/filesystem> // For conversion from std::experimental::filesystem::path to our str.
// workaround for now until the filesystem lib has moved out of experimental.
namespace std
{
	namespace filesystem = std::experimental::filesystem;
}

// Include the fmt library via spdlog.
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/ostr.h"

#include "sysconf.h" /* we include the config header from ./configure */
#include "nlohmann/json.hpp" /* Include json.hpp */
using json = nlohmann::json;


#ifdef __GNUC__
# define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
# define DEPRECATED(func) __declspec(deprecated) func
#else
# define DEPRECATED(func) func
#endif
typedef std::basic_string<char, std::char_traits<char>, std::allocator<char> > base_string;
namespace Flux
{
    class string;
}

// Make sure we can use fmtlib's udl's.
using namespace fmt::literals;

// a pseudonym for bit-wise flags.
typedef unsigned long flags_t;


/** Percent-encoding map for HTML output **/
static const char* url_escape_table[256] = {
  "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07", "%08", "%09",
  "%0a", "%0b", "%0c", "%0d", "%0e", "%0f", "%10", "%11", "%12", "%13",
  "%14", "%15", "%16", "%17", "%18", "%19", "%1a", "%1b", "%1c", "%1d",
  "%1e", "%1f", "%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
  0, 0, "%2a", "%2b", "%2c", "%2d", 0, "%2f", 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, "%3a", "%3b", "%3c", "%3d", "%3e", "%3f", "%40", 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "%5b", "%5c", "%5d", "%5e", 0, "%60", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "%7b", "%7c", "%7d", 0,
  "%7f", "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87", "%88",
  "%89", "%8a", "%8b", "%8c", "%8d", "%8e", "%8f","%90", "%91", "%92",
  "%93", "%94", "%95", "%96", "%97", "%98", "%99","%9a", "%9b", "%9c",
  "%9d", "%9e", "%9f", "%a0", "%a1", "%a2", "%a3", "%a4", "%a5", "%a6",
  "%a7", "%a8", "%a9", "%aa", "%ab", "%ac", "%ad", "%ae", "%af", "%b0",
  "%b1", "%b2", "%b3", "%b4", "%b5", "%b6", "%b7", "%b8", "%b9", "%ba",
  "%bb", "%bc", "%bd", "%be", "%bf", "%c0", "%c1", "%c2", "%c3", "%c4",
  "%c5", "%c6", "%c7", "%c8", "%c9", "%ca", "%cb", "%cc", "%cd", "%ce",
  "%cf", "%d0", "%d1", "%d2", "%d3", "%d4", "%d5", "%d6", "%d7", "%d8",
  "%d9", "%da", "%db", "%dc", "%dd", "%de", "%df", "%e0", "%e1", "%e2",
  "%e3", "%e4", "%e5", "%e6", "%e7", "%e8", "%e9", "%ea", "%eb", "%ec",
  "%ed", "%ee", "%ef", "%f0", "%f1", "%f2", "%f3", "%f4", "%f5", "%f6",
  "%f7", "%f8", "%f9", "%fa", "%fb", "%fc", "%fd", "%fe", "%ff"
};

/** The ci namespace contains a number of helper classes.
 */
namespace ci
{
	/** The ci_char_traits class is used for ASCII-style comparison of strings.
	 * This class is used to implement ci::string, a case-insensitive, ASCII-
	 * comparing string class.
	 */
	struct ci_char_traits : std::char_traits<char>
	{
		/** Check if two chars match.
		 * @param c1st First character
		 * @param c2nd Second character
		 * @return true if the characters are equal
		 */
		static bool eq(char c1st, char c2nd);

		/** Check if two chars do NOT match.
		 * @param c1st First character
		 * @param c2nd Second character
		 * @return true if the characters are unequal
		 */
		static bool ne(char c1st, char c2nd);

		/** Check if one char is less than another.
		 * @param c1st First character
		 * @param c2nd Second character
		 * @return true if c1st is less than c2nd
		 */
		static bool lt(char c1st, char c2nd);

		/** Compare two strings of size n.
		 * @param str1 First string
		 * @param str2 Second string
		 * @param n Length to compare to
		 * @return similar to strcmp, zero for equal, less than zero for str1
		 * being less and greater than zero for str1 being greater than str2.
		 */
		static int compare(const char *str1, const char *str2, size_t n);

		/** Find a char within a string up to position n.
		 * @param s1 String to find in
		 * @param n Position to search up to
		 * @param c Character to search for
		 * @return Pointer to the first occurance of c in s1
		 */
		static const char *find(const char *s1, int n, char c);
	};

	/** This typedef declares ci::string based upon ci_char_traits.
	 */
	typedef std::basic_string<char, ci_char_traits, std::allocator<char> > string;

	struct less
	{
	  /** Compare two Flux::strings as ci::strings and find which one is less
	   * @param s1 The first string
	   * @param s2 The second string
	   * @return true if s1 < s2, else false
	   */
	  bool operator()(const Flux::string &s1, const Flux::string &s2) const;
	};
}

/** Operator >> for ci::string
 */
inline std::istream &operator>>(std::istream &is, ci::string &str)
{
	base_string tmp;
	is >> tmp;
	str = tmp.c_str();
	return is;
}

/** Define operators for + and == with ci::string to base_string for easy assignment
 * and comparison
 *
 * Operator +
 */
inline base_string operator+(base_string &leftval, ci::string &rightval)
{
  return leftval + base_string(rightval.c_str());
}

/** Define operators for + and == with ci::string to base_string for easy assignment
 * and comparison
 *
 * Operator +
 */
inline ci::string operator+(ci::string &leftval, base_string &rightval)
{
	return leftval + ci::string(rightval.c_str());
}

/** Define operators for + and == with ci::string to base_string for easy assignment
 * and comparison
 *
 * Operator ==
 */
inline bool operator==(const base_string &leftval, const ci::string &rightval)
{
	return leftval.c_str() == rightval;
}

/** Define operators for + and == with ci::string to base_string for easy assignment
 * and comparison
 *
 * Operator ==
 */
inline bool operator==(const ci::string &leftval, const base_string &rightval)
{
	return leftval == rightval.c_str();
}

/** Define operators != for ci::string to base_string for easy comparison
 */
inline bool operator!=(const ci::string &leftval, const base_string &rightval)
{
	return !(leftval == rightval.c_str());
}

/** Define operators != for ci::string to irc::string for easy comparison
 */
inline bool operator!=(const base_string &leftval, const ci::string &rightval)
{
	return !(leftval.c_str() == rightval);
}

namespace Flux
{
	class string
	{
	private:
		base_string _string;
	public:
		typedef base_string::iterator iterator;
		typedef base_string::const_iterator const_iterator;
		typedef base_string::reverse_iterator reverse_iterator;
		typedef base_string::const_reverse_iterator const_reverse_iterator;
		typedef base_string::size_type size_type;
		static const size_type npos = static_cast<size_type>(-1);

		string() : _string("") { }

        // number conversions.
        explicit string(int i) { this->_string = std::to_string(i); }
        explicit string(long i) { this->_string = std::to_string(i); }
        explicit string(long long i) { this->_string = std::to_string(i); }
        explicit string(unsigned i) { this->_string = std::to_string(i); }
        explicit string(unsigned long i) { this->_string = std::to_string(i); }
        explicit string(unsigned long long i) { this->_string = std::to_string(i); }
        explicit string(float i) { this->_string = std::to_string(i); }
        explicit string(double i) { this->_string = std::to_string(i); }
        explicit string(long double i) { this->_string = std::to_string(i); }

        // Other conversions
		string(char chr) : _string(1, chr) { }
		string(size_type n, char chr) : _string(n, chr) { }
        string(const json &j) : _string(j.dump()) { }
		string(std::string_view &s) : _string(s) { }
		string(const std::filesystem::path &p) : _string(p.native()) { }
		string(const char *_str) : _string(_str) { }
        string(const char *_str, size_type len) : _string(_str, len) { }
		string(const base_string &_str) : _string(_str) { }
		string(const ci::string &_str) : _string(_str.c_str()) { }
		string(const string &_str, size_type pos = 0, size_type n = npos) : _string(_str._string, pos, n) { }
		string(const std::vector<string> &_vec, const std::string &delim) { this->implode(_vec, delim); }
		string(const std::vector<string> &_vec) { this->implode(_vec, " "); }
		template <class InputIterator> string(InputIterator first, InputIterator last) : _string(first, last) { }

        // Wide-character functions.
        string(wchar_t chr) : string(&chr, 1) { }
        string(const wchar_t *_str) : string(_str, std::wcslen(_str)) {  } // Call the constructor below
        string(const wchar_t *_str, size_type sz) : _string() { char *str = new char[sz]; std::wcstombs(str, _str, sz); this->_string = str; delete[] str; }

		// Used for formatting strings using the user-defined literal below.
		template<typename... Args>
		string operator ()(const Args&... args) { return fmt::format(this->_string, args...); }

		inline string &operator=(char chr) { this->_string = chr; return *this; }
		inline string &operator=(const char *_str) { this->_string = _str; return *this; }
		inline string &operator=(const base_string &_str) { this->_string = _str; return *this; }
		inline string &operator=(const ci::string &_str) { this->_string = _str.c_str(); return *this; }
		inline string &operator=(const string &_str) { if (this != &_str) this->_string = _str._string; return *this; }
        inline string &operator=(const json &j) { this->_string = j.dump(); return *this; }

		inline bool operator==(const char *_str) const { return this->_string == _str; }
		inline bool operator==(const base_string &_str) const { return this->_string == _str; }
		inline bool operator==(const ci::string &_str) const { return ci::string(this->_string.c_str()) == _str; }
		inline bool operator==(const string &_str) const { return this->_string == _str._string; }

		inline bool equals_cs(const char *_str) const { return this->_string == _str; }
		inline bool equals_cs(const base_string &_str) const { return this->_string == _str; }
		inline bool equals_cs(const ci::string &_str) const { return this->_string == _str.c_str(); }
		inline bool equals_cs(const string &_str) const { return this->_string == _str._string; }

		inline bool equals_ci(const char *_str) const { return ci::string(this->_string.c_str()) == _str; }
		inline bool equals_ci(const base_string &_str) const { return ci::string(this->_string.c_str()) == _str.c_str(); }
		inline bool equals_ci(const ci::string &_str) const { return _str == this->_string.c_str(); }
		inline bool equals_ci(const string &_str) const { return ci::string(this->_string.c_str()) == _str._string.c_str(); }

		inline bool operator!=(const char *_str) const { return !operator==(_str); }
		inline bool operator!=(const base_string &_str) const { return !operator==(_str); }
		inline bool operator!=(const ci::string &_str) const { return !operator==(_str); }
		inline bool operator!=(const string &_str) const { return !operator==(_str); }

		inline string &operator+=(char chr) { this->_string += chr; return *this; }
		inline string &operator+=(const char *_str) { this->_string += _str; return *this; }
		inline string &operator+=(const base_string &_str) { this->_string += _str; return *this; }
		inline string &operator+=(const ci::string &_str) { this->_string += _str.c_str(); return *this; }
		inline string &operator+=(const string &_str) { if (this != &_str) this->_string += _str._string; return *this; }

		inline const string operator+(char chr) const { return string(*this) += chr; }
		inline const string operator+(const char *_str) const { return string(*this) += _str; }
		inline const string operator+(const base_string &_str) const { return string(*this) += _str; }
		inline const string operator+(const ci::string &_str) const { return string(*this) += _str; }
		inline const string operator+(const string &_str) const { return string(*this) += _str; }

		friend const string operator+(char chr, const string &str);
		friend const string operator+(const char *_str, const string &str);
		friend const string operator+(const base_string &_str, const string &str);
		friend const string operator+(const ci::string &_str, const string &str);
		friend const string operator+(const string &str, const base_string &_str);

		inline bool operator<(const string &_str) const { return this->_string < _str._string; }

		inline const char *c_str() const { return this->_string.c_str(); }
		// NOTE: this is only used for idiot libraries which accept char* instead of const char*
		inline char *cc_str() const { return const_cast<char*>(this->_string.c_str()); }
		inline const char *data() const { return this->_string.data(); }
		inline ci::string ci_str() const { return ci::string(this->_string.c_str()); }
		inline const base_string &std_str() const { return this->_string; }
		inline base_string &std_str() { return this->_string; }
		inline string url_str() const
		{
			string ret;
			const char *t = this->_string.c_str();
			while(t && *t)
			{
				int c = *t;
				const char *e = url_escape_table[c];
				if(e)
					ret += e;
				else
					ret += c;
				t++;
			}
			return ret;
		}

		inline std::vector<string> explode(const string &delim) const
		{
			size_t start = 0, end = 0;
			std::vector<string> ret;

			while (end != string::npos)
			{
				end = this->_string.find(delim._string, start);

				// If at end, use length=maxLength.  Else use length=end-start.
				ret.push_back(this->_string.substr(start, (end == string::npos) ? string::npos : end - start));

				// If at end, use start=maxSize.  Else use start=end+delimiter.
				start = ((end > (string::npos - delim.size())) ? string::npos : end + delim.size());
			}

			return ret;
		}

		inline string implode(const std::vector<string> &_vec, const string &delim)
		{
			for (auto it = _vec.begin(), it_end = _vec.end(); it != it_end; ++it)
			{
				if (it + 1 == it_end)
					this->_string += (*it)._string;
				else
					this->_string += (*it)._string + delim._string;
			}

			return *this;
		}

		inline bool empty() const { return this->_string.empty(); }
		inline size_type length() const { return this->_string.length(); }
		inline size_type size() const { return this->_string.size(); }
		inline size_type capacity() const { return this->_string.capacity(); }
		inline size_type max_size() const { return this->_string.max_size(); }
		inline void swap(string &_str) { this->_string.swap(_str._string); }
		inline void push_back(char c) { return this->_string.push_back(c); }
		inline void push_back(const string &_str) { if (this != &_str) this->_string += _str._string; }
		inline void resize(size_type n) { return this->_string.resize(n); }

		inline string erase(size_t pos = 0, size_t n = base_string::npos) { return this->_string.erase(pos, n); }
		inline iterator erase(const iterator &i) { return this->_string.erase(i); }
		inline iterator erase(const iterator &first, const iterator &last) { return this->_string.erase(first, last); }
		//inline void erase(size_type pos = 0, size_type n = base_string::npos) { this->_string.erase(pos, n); }

		inline string trim()
		{
			while(!this->_string.empty() && isspace(this->_string[0]))
				this->_string.erase(this->_string.begin());
			while(!this->_string.empty() && isspace(this->_string[this->_string.length() - 1]))
				this->_string.erase(this->_string.length() - 1);

			return *this;
		}

		inline string tolower() { std::transform(_string.begin(), _string.end(), _string.begin(), ::tolower); return *this; }
		inline string toupper() { std::transform(_string.begin(), _string.end(), _string.begin(), ::toupper); return *this; }
		inline string tolower() const { base_string tmp = this->_string; std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower); return tmp; }
		inline string toupper() const { base_string tmp = this->_string; std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper); return tmp; }
		inline void clear() { this->_string.clear(); }
		inline bool search(const string &_str) { if(_string.find(_str._string) != base_string::npos) return true; return false; }
		inline bool search(const string &_str) const { if(_string.find(_str._string) != base_string::npos) return true; return false; }
		inline bool search_ci(const string &_str) { if(ci::string(this->_string.c_str()).find(ci::string(_str.c_str())) != ci::string::npos) return true; return false; }
		inline bool search_ci(const string &_str) const { if(ci::string(this->_string.c_str()).find(ci::string(_str.c_str())) != ci::string::npos) return true; return false; }

		inline size_type find(const string &_str, size_type pos = 0) const { return this->_string.find(_str._string, pos); }
		inline size_type find(char chr, size_type pos = 0) const { return this->_string.find(chr, pos); }
		inline size_type find_ci(const string &_str, size_type pos = 0) const { return ci::string(this->_string.c_str()).find(ci::string(_str._string.c_str()), pos); }
		inline size_type find_ci(char chr, size_type pos = 0) const { return ci::string(this->_string.c_str()).find(chr, pos); }

		inline size_type rfind(const string &_str, size_type pos = npos) const { return this->_string.rfind(_str._string, pos); }
		inline size_type rfind(char chr, size_type pos = npos) const { return this->_string.rfind(chr, pos); }
		inline size_type rfind_ci(const string &_str, size_type pos = npos) const { return ci::string(this->_string.c_str()).rfind(ci::string(_str._string.c_str()), pos); }
		inline size_type rfind_ci(char chr, size_type pos = npos) const { return ci::string(this->_string.c_str()).rfind(chr, pos); }

		inline size_type find_first_of(const string &_str, size_type pos = 0) const { return this->_string.find_first_of(_str._string, pos); }
		inline size_type find_first_of_ci(const string &_str, size_type pos = 0) const { return ci::string(this->_string.c_str()).find_first_of(ci::string(_str._string.c_str()), pos); }

		inline size_type find_first_not_of(const string &_str, size_type pos = 0) const { return this->_string.find_first_not_of(_str._string, pos); }
		inline size_type find_first_not_of_ci(const string &_str, size_type pos = 0) const { return ci::string(this->_string.c_str()).find_first_not_of(ci::string(_str._string.c_str()), pos); }

		inline size_type find_last_of(const string &_str, size_type pos = npos) const { return this->_string.find_last_of(_str._string, pos); }
		inline size_type find_last_of_ci(const string &_str, size_type pos = npos) const { return ci::string(this->_string.c_str()).find_last_of(ci::string(_str._string.c_str()), pos); }

		inline size_type find_last_not_of(const string &_str, size_type pos = npos) const { return this->_string.find_last_not_of(_str._string, pos); }
		inline size_type find_last_not_of_ci(const string &_str, size_type pos = npos) const { return ci::string(this->_string.c_str()).find_last_not_of(ci::string(_str._string.c_str()), pos); }

		inline bool is_number_only() const { return this->find_first_not_of("0123456789.-") == npos; }
		inline bool is_pos_number_only() const { return this->find_first_not_of("0123456789.") == npos; }

		inline string replace(size_type pos, size_type n, const string &_str) { return string(this->_string.replace(pos, n, _str._string)); }
		inline string replace(size_type pos, size_type n, const string &_str, size_type pos1, size_type n1) { return string(this->_string.replace(pos, n, _str._string, pos1, n1)); }
		inline string replace(size_type pos, size_type n, size_type n1, char chr) { return string(this->_string.replace(pos, n, n1, chr)); }
		inline string replace(iterator first, iterator last, const string &_str) { return string(this->_string.replace(first, last, _str._string)); }

		inline string append(const string &_str) { return this->_string.append(_str._string); }
		inline string append(const string &_str, size_t pos, size_t n) { return this->_string.append(_str._string, pos, n); }
		inline string append(const char* s, size_t n) { return this->_string.append(s, n); }
		inline string append(const char* s) { return this->_string.append(s); }
		inline string append(size_t n, char c) { return this->_string.append(n, c); }

		inline int compare(const string &_str) const { return this->_string.compare(_str._string); }
		inline int compare(const char *s) const { return this->_string.compare(s); }
		inline int compare(size_t pos1, size_t n1, const string &_str) const { return this->_string.compare(pos1, n1, _str._string); }
		inline int compare(size_t pos1, size_t n1, const char *s) const { return this->_string.compare(pos1, n1, s); }
		inline int compare(size_t pos1, size_t n1, const string &_str, size_t pos2, size_t n2) const { return this->_string.compare(pos1, n1, _str._string, pos2, n2); }
		inline int compare(size_t pos1, size_t n1, const char *s, size_t n2) const { return this->_string.compare(pos1, n1, s, n2); }

		inline string insert(size_t pos1, const string &_str) { return this->_string.insert(pos1, _str._string); }
		inline string insert(size_t pos1, const string &_str, size_t pos2, size_t n) { return this->_string.insert(pos1, _str._string, pos2, n); }
		inline string insert(size_t pos1, const char* s, size_t n) { return this->_string.insert(pos1, s, n); }
		inline string insert(size_t pos1, const char* s) { return this->_string.insert(pos1, s); }
		inline string insert(size_t pos1, size_t n, char c) { return this->_string.insert(pos1, n, c); }
		inline iterator insert(iterator p, char c) { return this->_string.insert(p, c); }
		inline void insert(iterator p, size_t n, char c) { this->_string.insert(p, n, c); }
		template<class InputIterator> inline void insert(iterator p, InputIterator first, InputIterator last) { return this->_string.insert(p, first, last); }

		inline string assign(const string &str) { return this->_string.assign(str._string); }
		inline string assign(const string &str, size_t pos, size_t n) { return this->_string.assign(str._string, pos, n); }
		inline string assign(const char* s, size_t n) { return this->_string.assign(s, n); }
		inline string assign(const char* s) { return this->_string.assign(s); }
		inline string assign(size_t n, char c) { return this->_string.assign(n, c); }
		template <class InputIterator> inline string assign(InputIterator first, InputIterator last) { return this->_string.assign(first, last); }

		inline string replace(iterator first, iterator last, size_type n, char chr) { return string(this->_string.replace(first, last, n, chr)); }
		template <class InputIterator> inline string replace(iterator first, iterator last, InputIterator f, InputIterator l) { return string(this->_string.replace(first, last, f, l)); }
		inline string replace_all_cs(const string &_orig, const string &_repl) const
		{
			Flux::string new_string = *this;
			size_type pos = new_string.find(_orig), orig_length = _orig.length(), repl_length = _repl.length();
			while (pos != npos)
			{
				new_string = new_string.substr(0, pos) + _repl + new_string.substr(pos + orig_length);
				pos = new_string.find(_orig, pos + repl_length);
			}
			return new_string;
		}
		inline string replace_all_ci(const string &_orig, const string &_repl) const
		{
			Flux::string new_string = *this;
			size_type pos = new_string.find_ci(_orig), orig_length = _orig.length(), repl_length = _repl.length();
			while (pos != npos)
			{
				new_string = new_string.substr(0, pos) + _repl + new_string.substr(pos + orig_length);
				pos = new_string.find_ci(_orig, pos + repl_length);
			}
			return new_string;
		}
		inline string substr(size_type pos = 0, size_type n = npos) const { return this->_string.substr(pos, n).c_str(); }

		inline iterator begin() { return this->_string.begin(); }
		inline const_iterator begin() const { return this->_string.begin(); }
		inline iterator end() { return this->_string.end(); }
		inline const_iterator end() const { return this->_string.end(); }
		inline reverse_iterator rbegin() { return this->_string.rbegin(); }
		inline const_reverse_iterator rbegin() const { return this->_string.rbegin(); }
		inline reverse_iterator rend() { return this->_string.rend(); }
		inline const_reverse_iterator rend() const { return this->_string.rend(); }

		inline const char &at(size_t pos) const { return this->_string.at(pos); }
		inline char &at(size_t pos) { return this->_string.at(pos); }
		inline std::allocator<char> get_allocator() const { return this->_string.get_allocator(); }
		inline char &operator[](size_type n) { return this->_string[n]; }
		inline const char &operator[](size_type n) const { return this->_string[n]; }

		inline string isolate(char b, char e) const
		{
			string to_find;
			size_t pos = _string.find(b);
			pos += 1;

			for (unsigned i = pos; i < _string.length(); i++)
			{
				if (_string[i] == e)
					break;
				else
					to_find = to_find+_string[i];
			}
			return to_find;
		}

        // Short Format intended to be used with string literals
        // Eg: "the %s fox jumps %d times over the fence!"_F.format("quick brown", 10);
        template<typename... Args>
        string format(const Args&... args) { return fmt::format(this->_string, args...); }

		/* Strip Return chars */
		inline string strip()
		{
			string new_buf = *this;
			new_buf = new_buf.replace_all_cs("\n", "");
			new_buf = new_buf.replace_all_cs("\r", "");
			return new_buf;
		}

		inline string strip() const
		{
			string new_buf = *this;
			new_buf = new_buf.replace_all_cs("\n", "");
			new_buf = new_buf.replace_all_cs("\r", "");
			return new_buf;
		}
		/* Strip specific chars */
		inline string strip(const char &_delim)
		{
			string new_buf = *this;
			new_buf = new_buf.replace_all_cs(_delim, "");
			return new_buf;
		}

		inline string strip(const char &_delim) const
		{
			string new_buf = *this;
			new_buf = new_buf.replace_all_cs(_delim, "");
			return new_buf;
		}
		/* Cast into an integer */
		inline operator int() { return std::stoi(this->_string); }
		/* Cast into a long integer */
		inline operator long() { return std::stol(this->_string); }
        /* Cast to long long integer */
        inline operator long long() { return std::stoll(this->_string); }
        /* Cast to unsigned long integer */
        inline operator unsigned long() { return std::stoul(this->_string); }
        /* Cast to unsigned long long integer */
        inline operator unsigned long long() { return std::stoull(this->_string); }
        /* Cast into a float */
        inline operator float() { return std::stof(this->_string); }
        /* Cast into a double */
        inline operator double() { return std::stod(this->_string); }
        /* Cast to long double */
        inline operator long double() { return std::stold(this->_string); }

		friend std::ostream &operator<<(std::ostream &os, const string &_str);
		friend std::istream &operator>>(std::istream &os, string &_str);
	}; //end of string class

	template<typename T> class map : public std::map<string, T> { };
	template<typename T> class insensitive_map : public std::map<string, T, ci::less> { };
	typedef std::vector<string> vector;
	inline std::ostream &operator<<(std::ostream &os, const string &_str) { return os << _str._string; }
	inline std::istream &operator>>(std::istream &os, string &_str) { return os >> _str._string; }
	inline const string operator+(char chr, const string &str) { string tmp(chr); tmp += str; return tmp; }
	inline const string operator+(const char *_str, const string &str) { string tmp(_str); tmp += str; return tmp; }
	inline const string operator+(const base_string &_str, const string &str) { string tmp(_str); tmp += str; return tmp; }
	inline const string operator+(const ci::string &_str, const string &str) { string tmp(_str); tmp += str; return tmp; }

	// For json.hpp compatibility for Flux::string
	inline void to_json(json &j, const string &str) { j = json(str.std_str()); }
	inline void from_json(const json &j, string &str) { str = j.dump(); }

	template<typename... Args>
	Flux::string format(const Flux::string &str, const Args&... args)
	{
		return fmt::format(str.std_str(), args...);
	}
}//end of namespace

// User-defined literal for Flux::string.
inline Flux::string operator "" _F(const char *str, size_t sz) { return Flux::string(str, sz); }
// Formatter for {fmt} library
inline void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const Flux::string &s)
{
	f.writer().write(s.c_str());
}

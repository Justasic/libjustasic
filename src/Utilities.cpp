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


#include "Utilities.h"
#include "Flux.h"
#include "Module.h"
#include <algorithm>
#include <ctime>
#include <mutex>
#include <random>
#include <string>

#ifdef HAVE_SETJMP_H
#	include <setjmp.h>
jmp_buf sigbuf; // Global var used for escaping crashes.
#endif

#ifdef HAS_CXXABI_H
#	include <cxxabi.h>
#endif

// Used for segmentation fault tracking
char	segv_location[255];
Module *LastRunModule = nullptr;

ManagedBuffer::ManagedBuffer()
{
	this->data = new internaldata_t;
	memset(this->data, 0, sizeof(internaldata_t));
	this->data->refs++;
	this->data->allocatedsz = 1024;
	this->data->size		= 0;
	this->data->data		= malloc(this->data->allocatedsz);
	if (!this->data->data)
		throw std::bad_alloc();

	// Initialize the buffer.
	memset(this->data->data, 0, this->data->allocatedsz);
}

ManagedBuffer::ManagedBuffer(const ManagedBuffer &other)
{
	this->data = other.data;
	this->data->refs++;
}

ManagedBuffer &ManagedBuffer::operator=(const ManagedBuffer &other)
{
	this->data = other.data;
	this->data->refs++;
	return *this;
}

ManagedBuffer::~ManagedBuffer()
{
	this->data->refs--;
	if (!this->data->refs)
	{
		// condition should never happen.
		assert(this->data && this->data->data);
		free(this->data->data);
		delete this->data;
		this->data = nullptr;
	}
}

void ManagedBuffer::Write(const void *ddata, size_t size)
{
	if (this->data->size + size > this->data->allocatedsz)
	{
		size_t newsz = std::max(this->data->size + size, this->data->size + 1024UL);
		void * ptr   = realloc(this->data->data, newsz);
		if (!ptr)
			throw std::bad_alloc();

		this->data->data		= ptr;
		this->data->allocatedsz = newsz;
		// Initialize the new area
		memset(reinterpret_cast<uint8_t *>(this->data->data) + this->data->size, 0, newsz - this->data->size);
	}

	// Copy our data.
	memcpy(reinterpret_cast<uint8_t *>(this->data->data) + this->data->size, ddata, size);
	this->data->size += size;
}

void ManagedBuffer::AllocateAhead(size_t sz)
{
	size_t newsz = this->data->allocatedsz + sz;
	void * ptr   = realloc(this->data->data, newsz);
	if (!ptr)
		throw std::bad_alloc();

	this->data->data		= ptr;
	this->data->allocatedsz = newsz;
	memset(reinterpret_cast<uint8_t *>(this->data->data) + this->data->size, 0, newsz - this->data->size);
}

// This class is going to attempt to be similar to Python's DateTime class.
DateTime::DateTime(time_t time) : tm(nullptr) { this->SetTime(time == 0 ? ::time(nullptr) : time); }

DateTime::~DateTime() { delete this->tm; }

// Getters/Setters
time_t DateTime::GetTime() const { return this->TheTime.load(); }
void   DateTime::SetTime(time_t time)
{
	this->TheTime.store(time);
	this->SetTM();
}

void DateTime::SetTM()
{
	std::unique_lock<std::shared_mutex> lck{this->tmux};
	if (!this->tm)
		this->tm = new struct tm();
	memset(this->tm, 0, sizeof(struct tm));
	time_t	t = this->GetTime();
	struct tm tmptm;
	memcpy(this->tm, localtime_r(&t, &tmptm), sizeof(struct tm));
}

const struct tm *DateTime::GetTM()
{
	std::shared_lock<std::shared_mutex> lck{this->tmux};
	return this->tm;
}

// Get various time pieces
time_t DateTime::GetYear() { return this->GetTM()->tm_year; }
time_t DateTime::GetMonth() { return this->GetTM()->tm_mon; }
time_t DateTime::GetDay() { return this->GetTM()->tm_yday; }
time_t DateTime::GetHour() { return this->GetTM()->tm_hour; }
time_t DateTime::GetMinute() { return this->GetTM()->tm_min; }
time_t DateTime::GetSecond() { return this->GetTM()->tm_sec; }

DateTime::DateTime(const DateTime &dt) { this->SetTime(dt.GetTime()); }

DateTime &DateTime::operator=(const DateTime &dt)
{
	this->SetTime(dt.GetTime());
	return *this;
}

// Previously DateTime DateTime::now() { return DateTime::DateTime(); }
// but G++ seems to think this is invalid behavior even though clang sees it
// as being valid. Whatever G++ you're a piece of shit anyway.
DateTime DateTime::now() { return DateTime(); }

void DateTime::strptime(const Flux::string &time, const Flux::string &fmt)
{
	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));
	::strptime(time.c_str(), fmt.c_str(), &tm);
	this->SetTime(mktime(&tm));
}

Flux::string DateTime::strftime(const Flux::string &fmt)
{
#if 1
	std::stringstream ss;
	ss << std::put_time(this->GetTM(), fmt.c_str());
	Flux::string retstr = ss.str();
#else
	// Because strftime wants a buffer and such, we must call it twice.
	const struct tm *tm = this->GetTM();
	// This is fucking stupid. Apparently fucking no one had a good idea
	// to maybe tell us what size string we need. This limits the capability
	// of strftime to basically exclusively 512-character time strings
	// making neat shit like "Hey this is a really cool time string with lots of time shit in it %A %B %C %D %F %l %g"
	// or whatever basically impossible because some idiot ass at microshaft decided to carpet bomb ASSERT inside wcsftime
	// instead of pretending to write the string and returning the required size for the string to actually be written
	// so then we can have an asprintf-style time formatted string.
	// Microsoft Windows is the fucking reason we can't have anything nice in this world.
	wchar_t buf[512];
	wcsftime(buf, sizeof(buf), fmt.c_str(), tm);
	std::wstring retstr = std::wstring(buf);
#endif
	return retstr;
}

Flux::string DateTime::duration()
{
	/* We first calculate everything */
	time_t days	= (this->TheTime / 86400);
	time_t hours   = (this->TheTime / 3600) % 24;
	time_t minutes = (this->TheTime / 60) % 60;
	time_t seconds = (this->TheTime) % 60;

	if (!days && !hours && !minutes)
		return "%ld second%c"_F.format(seconds, seconds != 1 ? 's' : 0);
	else
	{
		bool		 need_comma = false;
		Flux::string buffer;

		if (days)
		{
			buffer	 = "%ld day%c"_F.format(days, days != 1 ? 's' : 0);
			need_comma = true;
		}

		if (hours)
		{
			buffer += "%s%ld hour%c"_F.format(need_comma ? ", " : "", hours, hours != 1 ? 's' : 0);
			need_comma = true;
		}

		if (minutes)
			buffer += "%s%ld munute%c"_F.format(need_comma ? ", " : "", minutes, minutes != 1 ? 's' : 0);

		return buffer;
	}
}

bool TimingSafeStrcmp(const Flux::string &str1, const Flux::string &str2)
{
	size_t il   = str1.length();
	size_t tmax = str2.length() - 1;
	int	ret  = 0;

	for (size_t n = 0; n < il; ++n)
	{
		ret |= (str1[n] ^ (n <= tmax ? str2[n] : str2[tmax]));
	}

	return !ret;
}

Flux::string RandomDataGenerator(size_t len)
{
	char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	// Get random data from the OS
	std::random_device rd;
	// Use the mt19937 algorithm and seed with data from the random device.
	std::mt19937 mt(rd());
	// Get a random set of characters
	std::uniform_int_distribution<> dis(0, sizeof(characters));
	// Start our string
	Flux::string ret;
	for (size_t i = 0; i < len; ++i)
		ret += characters[dis(mt)];

	return ret;
}

/** Case insensitive map, ASCII rules.
 * This is faster than using toupper()/tolower()
 * That is;
 * [ != {, but A == a.
 */
unsigned const char ascii_case_insensitive_map[256] = {
	0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  /* 0-19 */
	20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  /* 20-39 */
	40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  /* 40-59 */
	60,  61,  62,  63,  64,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, /* 60-79 */
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91,  92,  93,  94,  95,  96,  97,  98,  99,  /* 80-99 */
	100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, /* 100-119 */
	120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, /* 120-139 */
	140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, /* 140-159 */
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, /* 160-179 */
	180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, /* 180-199 */
	200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, /* 200-219 */
	220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, /* 220-239 */
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255						/* 240-255 */
};

bool ci::ci_char_traits::eq(char c1st, char c2nd)
{
	return ascii_case_insensitive_map[static_cast<unsigned char>(c1st)] == ascii_case_insensitive_map[static_cast<unsigned char>(c2nd)];
}
bool ci::ci_char_traits::ne(char c1st, char c2nd)
{
	return ascii_case_insensitive_map[static_cast<unsigned char>(c1st)] != ascii_case_insensitive_map[static_cast<unsigned char>(c2nd)];
}
bool ci::ci_char_traits::lt(char c1st, char c2nd)
{
	return ascii_case_insensitive_map[static_cast<unsigned char>(c1st)] < ascii_case_insensitive_map[static_cast<unsigned char>(c2nd)];
}
int ci::ci_char_traits::compare(const char *str1, const char *str2, size_t n)
{
	for (unsigned i = 0; i < n; ++i)
	{
		if (ascii_case_insensitive_map[static_cast<unsigned char>(*str1)] > ascii_case_insensitive_map[static_cast<unsigned char>(*str2)])
			return 1;
		if (ascii_case_insensitive_map[static_cast<unsigned char>(*str1)] < ascii_case_insensitive_map[static_cast<unsigned char>(*str2)])
			return -1;
		if (!*str1 || !*str2)
			return 0;
		++str1;
		++str2;
	}
	return 0;
}
const char *ci::ci_char_traits::find(const char *s1, int n, char c)
{
	while (n-- > 0
		   && ascii_case_insensitive_map[static_cast<unsigned char>(*s1)] != ascii_case_insensitive_map[static_cast<unsigned char>(c)])
		++s1;
	return n >= 0 ? s1 : nullptr;
}
/** Compare two Flux::strings as ci::strings and find which one is less
 * @param s1 The first string
 * @param s2 The second string
 * @return true if s1 < s2, else false
 */
bool ci::less::operator()(const Flux::string &s1, const Flux::string &s2) const { return s1.ci_str().compare(s2.ci_str()) < 0; }

Flux::string DemangleSymbol(const Flux::string &sym)
{
#if HAS_CXXABI_H
	int   status	= 0;
	char *demangled = abi::__cxa_demangle(sym.c_str(), 0, 0, &status);
	// If we failed to demangle, simply return the symbol name.
	if (status != 0)
		return sym;

	Flux::string ret = demangled;
	free(demangled);
	return ret;
#else
	// If we don't have the demangler, just simply return the symbol name.
	return sym;
#endif
}

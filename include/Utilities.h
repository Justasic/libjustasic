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
#include <atomic>
#include <list>
#include <shared_mutex>

#ifdef HAVE_SETJMP_H
#	include <setjmp.h>
extern jmp_buf sigbuf; // Global var used for escaping crashes.
#endif

class Module;

extern char				   segv_location[255];
extern Module *			   LastRunModule;
extern std::list<Module *> Modules;
extern bool				   TimingSafeStrcmp(const Flux::string &str1, const Flux::string &str2);
extern Flux::string		   RandomDataGenerator(size_t len);
extern Flux::string		   DemangleSymbol(const Flux::string &sym);

#define SET_SEGV_LOCATION(m, p)                                                                              \
	if (true)                                                                                                \
	{                                                                                                        \
		snprintf(segv_location, sizeof(segv_location), "%s %d %s", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
		LastRunModule = m;                                                                                   \
	}

#define HIDE_SYMBOL __attribute__((visibility("hidden")))

class DateTime
{
	std::atomic<time_t>		  TheTime; //!
	struct tm *				  tm;
	mutable std::shared_mutex tmux; // lawl
	void					  SetTM();

  public:
	DateTime(time_t time = 0);
	~DateTime();

	// Getters/Setters
	time_t GetTime() const;
	void   SetTime(time_t time);

	// Get various time pieces
	time_t			 GetYear();
	time_t			 GetMonth();
	time_t			 GetDay();
	time_t			 GetHour();
	time_t			 GetMinute();
	time_t			 GetSecond();
	const struct tm *GetTM();

	DateTime	 now();
	void		 strptime(const Flux::string &time, const Flux::string &fmt);
	Flux::string strftime(const Flux::string &fmt);
	Flux::string duration();

	// class copy things
	DateTime(const DateTime &dt);
	DateTime &operator=(const DateTime &dt);
};

// This is intended to be used so that you can write to it
// as if it were a file of infinite size but it's a memory
// block. This memory block is self-expanding and self-freeing.
class ManagedBuffer
{
	typedef struct
	{
		void * data;
		size_t size;
		size_t allocatedsz;
		int	refs;
	} internaldata_t;
	internaldata_t *data;

  public:
	ManagedBuffer();
	~ManagedBuffer();

	void		  Write(const void *data, size_t size);
	void		  AllocateAhead(size_t sz);
	inline size_t length() const { return this->data->size; }
	inline size_t size() const { return this->data->size; }
	inline void * GetPointer() const { return this->data->data; }
	inline void * GetEndPointer() const
	{
		return reinterpret_cast<void *>((reinterpret_cast<uint8_t *>(this->data->data) + this->data->size));
	}

	inline void operator+=(size_t sz) { this->data->size += sz; }
	inline void operator=(size_t sz) { this->data->size = sz; }
	// Conveninece function
	inline void *operator*() const { return this->data->data; }

	// Used for reference counting.
	// See http://www.linuxprogrammingblog.com/cpp-objects-reference-counting
	ManagedBuffer(const ManagedBuffer &other);
	ManagedBuffer &operator=(const ManagedBuffer &other);
};

template<typename T, size_t Size = 32>
class Flags
{
  protected:
	std::bitset<Size> Flag_Values;

  public:
	/** Add a flag to this item
	 * @param Value The flag
	 */
	void SetFlag(T Value) { Flag_Values[Value] = true; }

	/**
	 * Add a set of flags to this item
	 * @method SetFlags
	 * @param  value    the first flag to add
	 * @param  values   the remaining flags to add
	 */
	template<typename... Args>
	void SetFlags(Args... values)
	{
		// C++ 17 fold expression
		(this->SetFlag(values), ...);
	}

	/** Remove a flag from this item
	 * @param Value The flag
	 */
	void RemoveFlag(T Value) { Flag_Values[Value] = false; }

	/**
	 * Remove a set of flags from this item
	 * @method RemoveFlags
	 * @param  value    the first flag to Remove
	 * @param  values   the remaining flags to Remove
	 */
	template<typename... Args>
	void RemoveFlags(Args... values)
	{
		// C++ 17 fold expression
		(this->SetFlag(values), ...);
	}

	/** Check if this item has a flag
	 * @param Value The flag
	 * @return true or false
	 */
	bool HasFlag(T Value) const { return Flag_Values.test(Value); }

	/** Check how many flags are set
	 * @return The number of flags set
	 */
	size_t FlagCount() const { return Flag_Values.count(); }

	/** Unset all of the flags
	 */
	void ClearFlags() { Flag_Values.reset(); }
};

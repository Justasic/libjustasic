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
#include <cstdint>
#include <vector>
#include <string>
#include <unistd.h>
#include "cstr.h"
#include "Flux.h"

inline const char *GetHighestSize(unsigned long long int &size)
{
    static const char *sizes[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
    int si = 0;
    for (; 1024 < size; si++)
        size /= 1024;

    if (si > sizeof(sizes))
        return "(hello future!)";
    else
        return sizes[si];
}

// mode parm for Seek
typedef enum
{
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

typedef enum
{
	FS_READ,
	FS_WRITE,
	FS_APPEND
} fsMode_t;

class File
{
	// The file descriptor for this file
	int fd;
	// The length of the file (a cached variable)
	size_t len;
	// Modes originally set
	fsMode_t modes;
	// The full path of the file
	Flux::string path;
	// Mark this class as being a friend of the FileSystem class
	// This allows us to create these file objects via the FileSystem class.
	friend class FileSystem;

  public:
	virtual ~File() {}
	// Read from the file
	virtual size_t Read(void *buffer, size_t len);
	// Write to the file
	virtual size_t Write(const void *buffer, size_t len);
	// Write a line to a file
	virtual size_t Putln(const Flux::string &str);
	// Get the length of the file
	virtual size_t Length();
	// Get the position we're currently at
	virtual size_t GetPosition();
	// Set the position
	virtual size_t SetPosition(size_t offset, fsOrigin_t whence);
	// Flush the buffer
	virtual void Flush();
	// Flush userspace and kernel buffers -- forces a write to device
	virtual void KFlush();
	// Get a libc FILE pointer
	virtual FILE *GetFILE();
	// Get the file path
	virtual inline Flux::string GetPath() { return this->path; }

	// String-style write functions
	template<typename... Args>
	int printf(const Flux::string &str, const Args &... args)
	{
		return this->Putln(Flux::format(str.c_str(), args...));
	}

	// Endian-portable, type safe, binary Read and write functions.
	// Useful for floats, ints, and other POD types.
	// These will always write as little-endian (eg, x86 compatible)
	template<typename T>
	size_t Read(T &t)
	{
		// Read into a temporary variable
		T	  tmp;
		size_t len = this->Read(&tmp, sizeof(T));
#ifdef BIGENDIAN
		// Reverse and copy
		memrev(&t, &tmp, sizeof(T));
#else // We're already little-endian so just copy.
		t = tmp;
#endif
		return len;
	}

	template<typename T>
	size_t Write(T t)
	{
#ifdef BIGENDIAN
		// Copy temporarily
		T tmp = t;
		// Reverse and copy
		memrev(&t, &tmp, sizeof(T));
#endif
		return this->Write(&t, sizeof(T));
	}
};

class FileSystem
{
  public:
	static File *OpenFile(const Flux::string &path, fsMode_t mode);
	static File *OpenTemporaryFile(const Flux::string &templatepath);
	static void  CloseFile(File *f);

	static bool CopyFile(File *dest, File *src);

	// Static functions used for simple reasons
	static bool			IsDirectory(const Flux::string &str);
	static bool			IsFile(const Flux::string &str);
	static bool			FileExists(const Flux::string &str);
	static bool			Delete(const Flux::string &file);
	static void			MakeDirectory(const Flux::string &str);
	static Flux::string GetCurrentDirectory();
	static Flux::vector DirectoryList(const Flux::string &dir);
	static Flux::string Dirname(const Flux::string &str);
	static Flux::string Basename(const Flux::string &str);
	static Flux::string RealPath(const Flux::string &str);

	static inline Flux::string GetApplicationDirectory()
	{
		char *str = new char[PATH_MAX];
		bzero(str, PATH_MAX);

		size_t		 r   = readlink("/proc/self/exe", str, PATH_MAX - 1);
		Flux::string ret = Dirname(Flux::string(str, r));
		delete[] str;
		return ret;
	}
};

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
#include "ModuleAnnounce.h"
//#include JSON_HPP_INCLUDE_PATH

#ifdef HAVE_FCNTL_H
#	include <dlfcn.h>
#else
#	error dlfcn.h is required to compile modules!
#endif

enum ModType
{
	MOD_UNDEFINED,
	MOD_ENCRYPTION,
	MOD_PROTOCOL,
	MOD_SOCKETENGINE,
	MOD_DATABASE,
	MOD_NORMAL
};

enum ModErr
{
	MOD_ERR_OK,
	MOD_ERR_MEMORY,
	MOD_ERR_PARAMS,
	MOD_ERR_EXISTS,
	MOD_ERR_NOEXIST,
	MOD_ERR_NOLOAD,
	MOD_ERR_UNKNOWN,
	MOD_ERR_FILE_IO,
	MOD_ERR_EXCEPTION,
	MOD_ERR_DEPENDS
};

class Module
{
	// Ensure we're a friend so we can modify the class when loading it.
	friend class ModuleHandler;
	// About the module.
	Flux::string author, version, description;
	// more stuff
	Flux::string name, filename, filepath;
	// Module dependencies
	Flux::vector dependencies;
	// When it was loaded
	time_t loadtime;
	// Whether the module should stay within the application until application termination
	bool permanent;
	// The kind of module it is (what it provides, eg database, encryption, sockets, etc.)
	ModType type;
	// the dlfcn handle for the actual shared object file.
	void *handle;

  protected:
	void SetAuthor(const Flux::string &);
	void SetVersion(const Flux::string &);
	void SetDescription(const Flux::string &);
	void SetPermanent(bool);

  public:
	inline Flux::string GetName() const { return this->name; }
	inline Flux::string GetFilePath() const { return this->filepath; }
	inline Flux::string GetFileName() const { return this->filename; }
	inline Flux::string GetAuthor() const { return this->author; }
	inline Flux::string GetVersion() const { return this->version; }
	inline Flux::string GetDescription() const { return this->description; }
	inline bool			GetPermanent() const { return this->permanent; }
	inline time_t		GetLoadTime() const { return this->loadtime; }
	inline ModType		GetModuleType() const { return this->type; }
	inline Flux::vector GetDependencies() const { return this->dependencies; }

	Module(const Flux::string &, ModType = MOD_UNDEFINED);
	virtual ~Module();
};

class ModuleHandler
{
  public:
	static Flux::string			 ModuleStrerror(ModErr e);
	static ModErr				 LoadModule(const Flux::string &);
	static void					 LoadModules();
	static void					 SanitizeRuntime();
	static void					 UnloadAll();
	static bool					 Unload(Module *);
	static Module *				 FindModule(const Flux::string &name);

  private:
	static bool DeleteModule(Module *);
};

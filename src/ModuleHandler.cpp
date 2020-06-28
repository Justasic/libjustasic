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

#include "File.h"
#include "Flux.h"
#include "Log.h"
#include "Module.h"
#include "ModuleAnnounce.h"
#include "Utilities.h"
#include <cassert>
#include <dlfcn.h>
#include <functional>
#include <list>
#include <mutex>
#include <regex>
extern std::list<Module *> Modules;

// Even in C++11 this stupid fucking cast function has to exist.
// I would've rather have used std::function but nooooooo.
template<class TYPE>
TYPE function_cast(void *symbol)
{
	union
	{
		void *symbol;
		TYPE  function;
	} cast;
	cast.symbol = symbol;
	return cast.function;
}

Flux::string ModuleHandler::ModuleStrerror(ModErr e)
{
	switch (e)
	{
		case MOD_ERR_OK: return "Module Loaded Ok";
		case MOD_ERR_MEMORY: return "Module Memory Error";
		case MOD_ERR_PARAMS: return "invalid module parameters";
		case MOD_ERR_EXISTS: return "module alread loaded";
		case MOD_ERR_NOEXIST: return "Module Does Not Exist";
		case MOD_ERR_NOLOAD: return "Unable to load module";
		case MOD_ERR_UNKNOWN: return "Unknown error";
		case MOD_ERR_FILE_IO: return "File I/O Error";
		case MOD_ERR_EXCEPTION: return "Module threw an exception";
		case MOD_ERR_DEPENDS: return "Module dependency could not be satisfied";
		default: return "unknown";
	};
}
ModErr ModuleHandler::LoadModule(const Flux::string &modname)
{
	// These have to be up here because of the goto statements.
	std::function<Module *(const Flux::string &)> f;
	Flux::vector *								  ptrdepends, depends;
	Module *									  m = nullptr;

	SET_SEGV_LOCATION(nullptr, nullptr);
	if (modname.empty())
		return MOD_ERR_PARAMS;

	Flux::string modbasename = FileSystem::Basename(modname);
	ModErr		 reterr		 = MOD_ERR_OK;

	// Make sure we're not already loaded...
	if (FindModule(modbasename))
		return MOD_ERR_EXISTS;

	"* Loading Module:\t{}"_l(modbasename);

	Flux::string mdir  = "runtime/" + (modbasename.search(".so") ? modbasename : modbasename + ".so");
	Flux::string input = modname;

	File *dest = FileSystem::OpenTemporaryFile(mdir);
	File *src  = FileSystem::OpenFile(input, FS_READ);

	// TODO: This assert shouldn't be here tbh.
	assert(FileSystem::CopyFile(dest, src));

	dlerror();

	auto *		handle = dlopen(dest->GetPath().c_str(), RTLD_LAZY);
	const char *err	= dlerror();

	if (!handle && err && *err)
	{
		// Try and resolve symbols first
		Flux::string error = err;
		if (error.search("_Z"))
		{
			// C++ symbols always start with _Z (maybe _ZN?)
			size_t pos = error.find("_Z");
			// Demangle the symbol (If possible)
			Flux::string sym = DemangleSymbol(error.substr(pos));
			Flux::string msg = error.substr(0, pos);
			"[{}]{}{}"_lw(modbasename, msg, sym);
		}
		else
			"[{}] {}"_lw(modbasename, err);

		reterr = MOD_ERR_NOLOAD;
		goto fail;
	}

	dlerror();
	// Get module dependencies first,
	ptrdepends = reinterpret_cast<Flux::vector *>(dlsym(handle, "dependencies"));

	if (!ptrdepends)
		; // Log(LOG_INFO) << "[" << modbasename << "]: Module has no dependencies. Continuing.";
	else
	{
		depends = Flux::vector(ptrdepends->begin(), ptrdepends->end());
		for (auto it : depends)
		{
			if (FindModule(it))
				"[{}]: module has satisfied dependency on {}"_l(modbasename, it);
			else
			{
				"[{}]: module depends on {} and is not loaded, attempting to load first"_l(modbasename, it);
				ModErr e = LoadModule("modules/" + it);
				if (e != MOD_ERR_OK)
				{
					"[{}]: Failed to load because dependent module {} failed to load: {}"_lw(modbasename, it, ModuleStrerror(e));
					reterr = MOD_ERR_DEPENDS;
					goto fail;
				}
			}
		}
	}

	f   = function_cast<Module *(*)(const Flux::string &)>(dlsym(handle, "initialization"));
	err = dlerror();

	if (f == nullptr && err && *err)
	{
		"[{}]: Invalid module: {}"_lw(modbasename, err);
		reterr = MOD_ERR_NOLOAD;
		goto fail;
	}

	if (f == nullptr)
	{
		"[{}]: Invalid module but no error loading the initialization function."_lw(modbasename);
		reterr = MOD_ERR_NOLOAD;
		goto fail;
	}

	// NOTICE: We can't call setjmp here because the module actually hasn't been made. Any
	// module code that causes a Segmentation Fault inside of it's constructor will
	// subsequently cause a program crash and termination.
	try
	{
		SET_SEGV_LOCATION(m, nullptr);
		m = f(modbasename);
	}
	catch (const ModuleException &e)
	{
		"Error loading {}: {}"_lw(modname, e.what());
		reterr = MOD_ERR_EXCEPTION;
		goto fail;
	}

	m->filepath		= dest->GetPath();
	m->filename		= (modname.search(".so") ? modname : modname + ".so");
	m->handle		= reinterpret_cast<void *>(handle); // we'll convert to auto later, for now reinterpret_cast.
	m->dependencies = depends;

	FileSystem::CloseFile(src);
	FileSystem::CloseFile(dest);

	return MOD_ERR_OK;

fail:
	if (src)
		FileSystem::CloseFile(src);
	if (dest)
		FileSystem::CloseFile(dest);
	if (handle)
		dlclose(handle);
	return reterr;
}

bool ModuleHandler::DeleteModule(Module *m)
{
	SET_SEGV_LOCATION(m, nullptr);
	assert(m); // idiot check.
	if (!m->handle)
		return false;

	auto *		 handle		= m->handle;
	Flux::string modulename = m->filepath;
	"[Module Engine] Checking dependencies for {} before unloading..."_l(m->name);

	// Check dependencies of this module and unload them.
	for (auto it = Modules.begin(), it_end = Modules.end(); it != it_end;)
	{
		Module *mod = *it++;
		for (auto it2 : mod->GetDependencies())
		{
			if (it2.equals_cs(m->name))
			{
				" {0} depends on {1}, unloading {1} first..."_l(mod->name, m->name);
				// We don'r call ModuleHandler::Unload() because it checks if the module is permanent.
				ModuleHandler::DeleteModule(mod);
			}
		}
	}

	dlerror();

	std::function<void(Module *)> df = function_cast<void (*)(Module *)>(dlsym(handle, "terminate"));

	const char *err = dlerror();
	if (df == nullptr && err && *err)
	{
		"[Module Engine] No destroy function found for {}, chancing delete()..."_lw(modulename);
		delete m; /* we just have to chance they haven't overwrote the delete operator then... */
	}

	SET_SEGV_LOCATION(nullptr, nullptr); // Not safe to set the ptr if we crash here, we could crash again.
	if (df == nullptr)
	{
		"[{}.so] Module has no destroy function? (wtf?)"_lw(modulename);
		return false;
	}
	else
	{
		"[Module Engine] Calling termination function for {}"_l(m->name);
		df(m);
	}

	if (handle)
		if (dlclose(handle))
			"[{}.so] {}"_lw(modulename, dlerror());

	return true;
}

bool ModuleHandler::Unload(Module *m)
{
	assert(m); // idiot check
	if (m->GetPermanent())
		return false;

	return DeleteModule(m);
}

void ModuleHandler::UnloadAll()
{
	"Unloading all modules...."_l;
	Modules.sort([](Module *a, Module *b) { return a->GetName() < b->GetName(); });
	for (auto it = Modules.begin(), it_end = Modules.end(); it != it_end;)
	{
		Module *m = *it;
		DeleteModule(m);
		// Because DeleteModule will recursively call Modules.erase() for dependencies
		// we must do an awkward iteration to make sure we don't have a memory error
		// because it can not only erase our current iterator's location but it will
		// likely erase the predicted iterators location (eg: it++). Instead, we just
		// get the front most iterator always since we're removing every module anyway.
		it = Modules.begin();
	}
	Modules.clear();
	"Unloading complete."_l;
}

void ModuleHandler::SanitizeRuntime()
{
	"[Module Engine] Cleaning up runtime directory."_l;
	Flux::string dirbuf = "runtime/";

	if (!FileSystem::IsDirectory(dirbuf))
		FileSystem::MakeDirectory(dirbuf);
	else
	{
		Flux::vector files = FileSystem::DirectoryList(dirbuf);
		for (auto it : files)
		{
			"[FileSystem] Deleting {}"_l(it);
			FileSystem::Delete(it);
		}
	}
}

/**
 * \fn Module *FindModule(const Flux::string &name)
 * \brief Find a Module in the Module list
 * \param name A string containing the Module name you're looking for
 */
Module *ModuleHandler::FindModule(const Flux::string &name)
{
	for (auto it : Modules)
	{
		Module *m = it;
		if (m->name.equals_ci(name))
			return m;
	}

	return nullptr;
}

void ModuleHandler::LoadModules()
{
	ModuleHandler::SanitizeRuntime();
	"[Module Engine] Loading modules..."_l;
	auto files = FileSystem::DirectoryList("modules/");
	std::sort(files.begin(), files.end());

	for (unsigned i = 0; i < files.size(); ++i)
	{
		// Make sure we're a shared object and not something dumb.
		std::regex IsSharedObject("(.*)\\.so$");
		if (!std::regex_match(files[i].std_str(), IsSharedObject))
			continue;

		ModErr e = ModuleHandler::LoadModule(files[i]);
		if (e != MOD_ERR_OK)
			"[{}] Could not load module: {}"_lw(FileSystem::Basename(files[i]), ModuleStrerror(e));
	}
	"[Module Engine] Loading modules complete."_l;
}
/******************End configuration variables********************/

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

#include "Module.h"
#include "Exceptions.h"
#include "Log.h"
#include <list>
// This code sucks, you know it and I know it.
// Move on and call me an idiot later.
std::list<Module *> Modules;

Module::Module(const Flux::string &n, ModType m)
	: author(""), version(""), description(""), name(n), filename(""), filepath(""), loadtime(time(NULL)), permanent(false), type(m),
	  handle(nullptr)
{
	if (ModuleHandler::FindModule(this->name))
		throw ModuleException("Module already exists!");

	Modules.push_back(this);
}

Module::~Module()
{
	"[{}]: Unloading from core"_l(this->name);

	auto it = std::find(Modules.begin(), Modules.end(), this);
	if (it != Modules.end())
		Modules.erase(it);
	else
		"Could not find {} in Module map!"_lw(this->name);
}

void Module::SetAuthor(const Flux::string &person) { this->author = person; }
void Module::SetVersion(const Flux::string &ver) { this->version = ver; }
void Module::SetPermanent(bool state) { this->permanent = state; }
void Module::SetDescription(const Flux::string &desc) { this->description = desc; }

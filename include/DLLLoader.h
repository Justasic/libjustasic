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
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <dlfcn.h>
#include <cassert>

class DynamicLibrary
{
    protected:
        void *handle;
        std::string name;
    public:
        DynamicLibrary(const std::string &str);
        ~DynamicLibrary();

        template<typename T> T ResolveSymbol(const std::string &str)
        {
            // Union-cast to get around C++ warnings.
            union {
                T func;
                void *ptr;
            } fn;

            fn.ptr = dlsym(this->handle, str.c_str());
            if (!fn.ptr)
            {
                fprintf(stderr, "Failed to resolve symbol %s: %s\n", str.c_str(), dlerror());
                return T();
            }

            return fn.func;
        }

        inline std::string GetName() const
        {
            return this->name;
        }
};

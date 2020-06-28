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

#include "SocketMultiplexer.h"
#include "Flux.h"

/**
 * This file is entirely a stub to make sure that the application links correctly.
 * These are actually implemented in the socket engine modules.
 */

Socket *SocketMultiplexer::FindSocket(int sock_fd)
{
	for (auto it : SocketMultiplexer::Sockets)
	{
		if (it->GetFD() == sock_fd)
			return it;
	}
	return nullptr;
}

// Initalizers
void SocketMultiplexer::Initialize() {}
void SocketMultiplexer::Terminate() {}

// Sockets interact with these functions.
bool SocketMultiplexer::AddSocket(Socket *s) { return false; }
bool SocketMultiplexer::RemoveSocket(Socket *s) { return false; }
bool SocketMultiplexer::UpdateSocket(Socket *s) { return false; }

// This is called in the event loop to slow the program down and process sockets.
void SocketMultiplexer::Multiplex(time_t sleep) {}

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

#include <string.h>
#include "vec.h"
#include "sysconf.h"
#include "MessageChannel.h"

#if __cplusplus
extern "C" {
#endif 

static vec_t(MessageListener_t) _callbacks;

bool AnnounceMessage(PluginMessage_t *msg)
{
	bool stop = false;

	// Announce the message to every message listener
	MessageListener_t call;
	int i;
	vec_foreach(&_callbacks, call, i)
	{
		if (!call(msg))
			stop = true;
	}

	// Deallocate our message once finished.
	free(msg);

	return stop;
}

void AddMessageListener(MessageListener_t caller)
{
	// Here specifically we do a sizeof(MessageListener_t*) since we want
	// to store the pointer size for pointer data.
	PluginMessage_t *msg = CreateMessage("CORE", "NewReceiver", sizeof(intptr_t));
	intptr_t callerptr = (intptr_t)caller;
	memcpy(msg->data, &callerptr, sizeof(intptr_t));
	
	if (AnnounceMessage(msg))
		return;

	vec_push(&_callbacks, caller);
}

void RemoveMessageListener(MessageListener_t caller)
{
	PluginMessage_t *msg = CreateMessage("CORE", "DeleteReceiver", sizeof(intptr_t));
	intptr_t callerptr = (intptr_t)caller;
	memcpy(msg->data, &callerptr, sizeof(intptr_t));

	if (AnnounceMessage(msg))
		return;
	
	vec_remove(&_callbacks, caller);
}

PluginMessage_t *CreateMessage(const char *ChannelName, const char *MessageName, size_t Datalen)
{
	PluginMessage_t *msg = (PluginMessage_t*)malloc(sizeof(PluginMessage_t) + Datalen);
	msg->ChannelName = ChannelName;
	msg->MessageName = MessageName;
	msg->AllocSize = Datalen;
	// Assume our size will be of Datalen.
	msg->Length = Datalen;
	return msg;
}

#if __cplusplus
}
#endif

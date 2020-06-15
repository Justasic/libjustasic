
#include "vec.h"
#include "MessageChannel.h""

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
	memcpy(msg->data, (intptr_t)caller, sizeof(intptr_t));
	
	if (AnnouceMessage(msg))
		return;

	vec_push(&_callbacks, caller);
}

void RemoveMessageListener(MessageListener_t caller)
{
	PluginMessage_t *msg = CreateMessage("CORE", "DeleteReceiver", sizeof(intptr_t));
	memcpy(msg->data, (intptr_t)caller, sizeof(intptr_t));

	if (AnnouceMessage(msg))
		return;
	
	vec_remove(&_callbacks, caller);
}

PluginMessage_t *CreateMessage(const char *ChannelName, const char *MessageName, size_t Datalen)
{
	PluginMessage_t *msg = malloc(sizeof(PluginMessage_t) + Datalen);
	msg->ChannelName = ChannelName;
	msg->MessageName = MessageName;
	msg->AllocSize = Datalen;
	// Assume our size will be of Datalen.
	msg->Length = Datalen;
	return msg;
}

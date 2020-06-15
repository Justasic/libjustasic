typedef struct PluginMessage_s
{
	const char *ChannelName; // Name of the message channel
	const char *MessageName; // Name of the message itself
	size_t Length; // Length of the data
	size_t AllocSize; // The allocated size of the data
	uint8_t data[1]; // The data of the plugin message
} PluginMessage_t;

typedef bool (*MessageListener_t)(PluginMessage_t*);

// Announce a message
extern bool AnnounceMessage(PluginMessage_t *msg);

// Handle message receivers
extern void AddMessageReceiver(MessageListener_t);
extern void RemoveMessageReceiver(MessageListener_t);

// Allocator
extern PluginMessage_t *CreateMessage(size_t Datalen);

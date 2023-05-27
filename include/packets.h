#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <enet/enet.h>

extern HANDLE clientBase;

struct packetMsg
{
	uint8_t packetId;
	uint8_t playerId;
	uint8_t chat_type;
	char msg[255];
};

struct extPacket
{
	uint8_t extId;
	uint8_t version;
};

struct packetExtInfo
{
	uint8_t packetId;
	uint8_t length;
	struct extPacket packet;
};

struct packetVersion
{
	uint8_t packetId;
	uint8_t identifier;
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_revision;
	char os[255];
};

struct packetHandshakeBack
{
	uint8_t packetId;
	int challenge;
};

void packetHandler(void);
void sendMsg(char *msg);
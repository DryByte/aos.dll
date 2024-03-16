#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <enet/enet.h>

extern int client_base;

struct __attribute__((__packed__)) packet_block_action
{
	uint8_t packet_id;
	uint8_t player_id;
	uint8_t action_type;
	int x;
	int y;
	int z;
};

struct packet_chat
{
	uint8_t packet_id;
	uint8_t player_id;
	uint8_t chat_type;
	char msg[255];
};

struct packet_extension
{
	uint8_t ext_id;
	uint8_t version;
};

struct packet_ext_info
{
	uint8_t packet_id;
	uint8_t length;
	struct packet_extension packet;
};

struct packet_client_info
{
	uint8_t packet_id;
	uint8_t identifier;
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t version_revision;
	char os[255];
};

struct packet_handshake_back
{
	uint8_t packet_id;
	int challenge;
};

void map_packet_hook(void);
void packet_hook(void);
void send_msg(char *msg);
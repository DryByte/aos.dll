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

struct __attribute__((__packed__)) packet_state_data 
{
	uint8_t packet_id;
	uint8_t player_id;
	uint8_t fog_blue;
	uint8_t fog_green;
	uint8_t fog_red;
	uint8_t team1_blue;
	uint8_t team1_green;
	uint8_t team1_red;
	uint8_t team2_blue;
	uint8_t team2_green;
	uint8_t team2_red;
	char team1_name[10];
	char team2_name[10];
	uint8_t game_mode_id;
};

void map_packet_hook(void);
void packet_hook(void);
void send_msg(char *msg);
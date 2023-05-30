#include <packets.h>

ENetPacket* PacketBuffer;
ENetPeer* peer;

void sendMsg(char *msg) {
	struct packetMsg* msgPacket = malloc(sizeof(struct packetMsg));
	msgPacket->packetId = 17;
	msgPacket->playerId = *(uint8_t*)(clientBase+0x13B1CF0);
	msgPacket->chat_type = 0;

	strncpy(msgPacket->msg, msg, strlen(msg));
	msgPacket->msg[strlen(msg)] = '\0';
	ENetPacket* pack = enet_packet_create(msgPacket, sizeof(msgPacket)+strlen(msg), ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(peer, 0, pack);
}

void sendExtInfo() {
	struct packetExtInfo* packetExt = malloc(sizeof(struct packetExtInfo));
	packetExt->packetId = 60;
	packetExt->length = 1;

	struct extPacket* extC = malloc(sizeof(struct extPacket));
	extC->extId = 193;
	extC->version = 1;

	packetExt->packet = *extC;

	ENetPacket* pack = enet_packet_create(packetExt, sizeof(packetExt), ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(peer, 0, pack);

	free(packetExt);
	free(extC);
}

void sendClientInfo() {
	struct packetVersion* packetV = malloc(sizeof(struct packetVersion));
	packetV->packetId = 34;
	packetV->identifier = 68;
	packetV->version_major = 1;
	packetV->version_minor = 1;
	packetV->version_revision = 1;

	char *a = "just testing";
	strncpy(packetV->os, a, strlen(a));

	ENetPacket* pack = enet_packet_create(packetV, sizeof(packetV)+strlen(a)+1, ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(peer, 0, pack);

	free(packetV);
}

void sendHandshakeBack(int fds) {
	struct packetHandshakeBack* packetHandBack = malloc(sizeof(struct packetHandshakeBack));
	packetHandBack->packetId = 32;
	packetHandBack->challenge = fds;

	ENetPacket* pack = enet_packet_create(packetHandBack, sizeof(packetHandBack)+1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, pack);
	free(packetHandBack);
}

__declspec(naked) void packetHandler() {
	asm volatile("movl %%esi, %0": "=r" (PacketBuffer));
	asm volatile("movl 40(%%esp), %0": "=r" (peer));

	if (PacketBuffer->data[0] != 2)
		printf("%i\n", PacketBuffer->data[0]);

	switch(PacketBuffer->data[0]) {
		case 17:
			uint8_t* buf = (uint8_t*)malloc(PacketBuffer->dataLength*sizeof(uint8_t));
			memcpy(buf, PacketBuffer->data, PacketBuffer->dataLength*sizeof(uint8_t));
			struct packetMsg* p = (struct packetMsg*)buf;

			printf("%i\n", p->chat_type);

			switch(p->chat_type) {
				case 3:
				case 4:
				case 5:
				case 6:
					addInfoMessage(p->msg);
					break;
			}
			break;
		case 31:
			struct packetHandshakeBack* fds = (struct packetHandshakeBack*)PacketBuffer->data;
			sendHandshakeBack(fds->challenge);
			break;
		case 33:
			sendClientInfo();
			break;
		case 60:
			sendExtInfo();
			break;
	}

	asm volatile(
		"movl %0, %%esi\n\t"
		"mov %1, %%ecx\n\t"
		"movl 8(%%esi), %%edx\n\t"
		"movb (%%edx), %%al\n\t"
		"sub %%ebx, %%ebx\n\t" //fix register mess issue with pointers
		"jmp *%%ecx"::"r"(PacketBuffer), "r"(clientBase+0x343ed));
}
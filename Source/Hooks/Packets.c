#include <Packets.h>
#include <Voxlap.h>
#include <Rendering.h>
#include <Menu.h>

ENetPacket* PacketBuffer;
ENetPeer* peer;

extern struct ItemMultitext* LoggerMultitext;

void sendPacket(void* packet, size_t size) {
	asm volatile(
		"mov %0, %%esi\n\t"
		"push $0x1\n\t"
		"push %2\n\t"
		"push %1\n\t"
		"mov %%esi, %%eax\n\t"
		"add $0x3e5e0, %%eax\n\t"
		"call *%%eax\n\t"
		"add $0xc, %%esp\n\t"

		"push %%eax\n\t"
		"add $0x38690, %%esi\n\t"
		"call *%%esi\n\t"
		"add $0x4, %%esp"
	:: "r" (clientBase), "r" (packet), "g" (size));
}

void sendMsg(char *msg) {
	struct packetMsg* msgPacket = malloc(sizeof(struct packetMsg));
	msgPacket->packetId = 17;
	msgPacket->playerId = *(uint8_t*)(clientBase+0x13B1CF0);
	msgPacket->chat_type = 0;

	strncpy(msgPacket->msg, msg, strlen(msg));
	msgPacket->msg[strlen(msg)] = '\0';

	sendPacket(msgPacket, sizeof(msgPacket)+strlen(msg));
}

void sendExtInfo() {
	struct packetExtInfo* packetExt = malloc(sizeof(struct packetExtInfo));
	packetExt->packetId = 60;
	packetExt->length = 1;

	struct extPacket* extC = malloc(sizeof(struct extPacket));
	extC->extId = 193;
	extC->version = 1;

	packetExt->packet = *extC;

	sendPacket(packetExt, sizeof(packetExt));

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

	sendPacket(packetV, sizeof(packetV)+strlen(a)+1);

	free(packetV);
}

void sendHandshakeBack(int fds) {
	struct packetHandshakeBack* packetHandBack = malloc(sizeof(struct packetHandshakeBack));
	packetHandBack->packetId = 32;
	packetHandBack->challenge = fds;

	sendPacket(packetHandBack, sizeof(packetHandBack)+1);
	free(packetHandBack);
}

int packet_handler() {
	if (PacketBuffer->data[0] != 2)
		printf("%i\n", PacketBuffer->data[0]);

	// if enabled it will jump to the end of the aos packet handler
	// so we can "override" behaviours
	int skip = 0;
	switch(PacketBuffer->data[0]) {
		case 13:
			{
				struct packetBlockAction* blockAction = (struct packetBlockAction*)PacketBuffer->data;

				if (blockAction->actionType == 0) {
					long block = getcube(blockAction->xPos, blockAction->yPos, blockAction->zPos);

					if (block != 0) {
						long color = *(long*)(clientBase+0x7ce8c+(blockAction->playerId*936));

						*(long*)block = color|0x7f000000;
						skip = 1;
					}
				}
			}
			break;
		case 17:
			{
				uint8_t* buf = (uint8_t*)malloc(PacketBuffer->dataLength*sizeof(uint8_t));
				memcpy(buf, PacketBuffer->data, PacketBuffer->dataLength*sizeof(uint8_t));
				struct packetMsg* p = (struct packetMsg*)buf;

				printf("%i\n", p->chat_type);

				addNewText(LoggerMultitext, p->msg);
				if (p->chat_type > 2)
					addCustomMessage(p->chat_type, p->msg);
			}
			break;
		case 31:
			{
				struct packetHandshakeBack* fds = (struct packetHandshakeBack*)PacketBuffer->data;
				sendHandshakeBack(fds->challenge);
			}
			break;
		case 33:
			sendClientInfo();
			break;
		case 60:
			sendExtInfo();
			break;
	}

	int leaveoffset = clientBase+0x343ed;
	if (skip == 1) {
		printf("fds?\n");
		leaveoffset = clientBase+0x355f2;
	}

	return leaveoffset;
}

__declspec(naked) void packet_hook() {
	asm volatile("movl %%esi, %0": "=r" (PacketBuffer));
	asm volatile("movl 40(%%esp), %0": "=r" (peer));

	int leaveoffset = packet_handler();

	asm volatile(
		"movl %0, %%esi\n\t"
		"mov %1, %%ecx\n\t"
		"movl 8(%%esi), %%edx\n\t"
		"movb (%%edx), %%al\n\t"
		"sub %%ebx, %%ebx\n\t" //fix register mess issue with pointers
		"jmp *%%ecx"::"r"(PacketBuffer), "r"(leaveoffset));
}
#include <Packets.h>
#include <Voxlap.h>
#include <Rendering.h>
#include <Menu.h>

ENetPacket* PacketBuffer;
ENetPeer* peer;

extern struct ItemMultitext* LoggerMultitext;

void send_packet(void* packet, size_t size) {
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
	:: "r" (client_base), "r" (packet), "g" (size));
}

void send_msg(char *msg) {
	struct packet_chat* msgp = malloc(sizeof(struct packet_chat));
	msgp->packet_id = 17;
	msgp->player_id = *(uint8_t*)(client_base+0x13B1CF0);
	msgp->chat_type = 0;

	strncpy(msgp->msg, msg, strlen(msg));
	msgp->msg[strlen(msg)] = '\0';

	send_packet(msgp, sizeof(msgp)+strlen(msg));
}

void send_ext_info() {
	struct packet_ext_info* extinfop = malloc(sizeof(struct packet_ext_info));
	extinfop->packet_id = 60;
	extinfop->length = 1;

	struct packet_extension* extC = malloc(sizeof(struct packet_extension));
	extC->ext_id = 193;
	extC->version = 1;

	extinfop->packet = *extC;

	send_packet(extinfop, sizeof(extinfop));

	free(extinfop);
	free(extC);
}

void send_client_info() {
	struct packet_client_info* packetV = malloc(sizeof(struct packet_client_info));
	packetV->packet_id = 34;
	packetV->identifier = 68;
	packetV->version_major = 1;
	packetV->version_minor = 1;
	packetV->version_revision = 1;

	char *a = "just testing";
	strncpy(packetV->os, a, strlen(a));

	send_packet(packetV, sizeof(packetV)+strlen(a)+1);

	free(packetV);
}

void send_handshake_back(int challenge) {
	struct packet_handshake_back* packetHandBack = malloc(sizeof(struct packet_handshake_back));
	packetHandBack->packet_id = 32;
	packetHandBack->challenge = challenge;

	send_packet(packetHandBack, sizeof(packetHandBack)+1);
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
				struct packet_block_action* block_action = (struct packet_block_action*)PacketBuffer->data;

				if (block_action->action_type == 0) {
					long block = getcube(block_action->x, block_action->y, block_action->z);

					if (block != 0) {
						long color = *(long*)(client_base+0x7ce8c+(block_action->player_id*936));

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
				struct packet_chat* p = (struct packet_chat*)buf;

				printf("%i\n", p->chat_type);

				add_new_text(LoggerMultitext, p->msg);
				if (p->chat_type > 2)
					add_custom_message(p->chat_type, p->msg);
			}
			break;
		case 31:
			{
				struct packet_handshake_back* fds = (struct packet_handshake_back*)PacketBuffer->data;
				send_handshake_back(fds->challenge);
			}
			break;
		case 33:
			send_client_info();
			break;
		case 60:
			send_ext_info();
			break;
	}

	int leaveoffset = client_base+0x343ed;
	if (skip == 1)
		leaveoffset = client_base+0x355f2;

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
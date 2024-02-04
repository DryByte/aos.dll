#include <Aos.h>

void load_kv6_files() {
	asm volatile(
		"call *%0"
	:: "r"(client_base+0x33640));
}

// this function from what i saw so far
// is used to assign kv6 files to the right spot
// used always after loadkv6files in statedata
void load_world_objects() {
	asm volatile(
		"call *%0"
	:: "r"(client_base+0x292e0));
}

void load_player_team_skin(int player_id) {
	int teamid = *(int*)(client_base+0x7ce58+0x3a8*player_id);

	asm volatile(
		"mov %0, %%ecx\n\t"
		"lea (%1), %%eax\n\t"
		"call *%2"
	:: "g" (teamid), "r" (client_base+0x7cb70+0x3a8*player_id), "g"(client_base+0x3dc50));
}

void load_player_weapon_skin(int player_id) {
	int toolid = *(int*)(client_base+0x7ce88+0x3a8*player_id);

	asm volatile(
		"push %0\n\t"
		"lea (%1), %%eax\n\t"
		"call *%2"
	:: "r" (toolid), "r" (client_base+0x7cb70+0x3a8*player_id), "g"(client_base+0x3dd10));
}

// this function not really exists
// its a loop inside statedata
void load_player_skins() {
	for (int i = 0; i < 32; i++) {
		int toolid = *(int*)(client_base+0x7ce88+0x3a8*i);

		load_player_team_skin(i);

		if (toolid < 2) {
			int* toolkv6 = (int*)(client_base+0x7cbc0+0x3a8*i);
			*toolkv6 = *(int*)(client_base+0x13cf840);
		} else {
			load_player_weapon_skin(i);
		}
	}
}

void load_image_files() {
	asm volatile("call *%0"::"r"(client_base+0x32290));
}
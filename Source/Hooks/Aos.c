#include <Aos.h>

void loadkv6Files() {
	asm volatile(
		"call *%0"
	:: "r"(clientBase+0x33640));
}

// this function from what i saw so far
// is used to assign kv6 files to the right spot
// used always after loadkv6files in statedata
void loadworldObjects() {
	asm volatile(
		"call *%0"
	:: "r"(clientBase+0x292e0));
}

void loadPlayerTeamSkin(int playerId) {
	int teamid = *(int*)(clientBase+0x7ce58+0x3a8*playerId);

	asm volatile(
		"mov %0, %%ecx\n\t"
		"lea (%1), %%eax\n\t"
		"call *%2"
	:: "g" (teamid), "r" (clientBase+0x7cb70+0x3a8*playerId), "g"(clientBase+0x3dc50));
}

void loadPlayerWeaponSkin(int playerId) {
	int toolid = *(int*)(clientBase+0x7ce88+0x3a8*playerId);

	asm volatile(
		"push %0\n\t"
		"lea (%1), %%eax\n\t"
		"call *%2"
	:: "r" (toolid), "r" (clientBase+0x7cb70+0x3a8*playerId), "g"(clientBase+0x3dd10));
}

// this function not really exists
// its a loop inside statedata
void loadplayerSkins() {
	for (int i = 0; i < 32; i++) {
		int toolid = *(int*)(clientBase+0x7ce88+0x3a8*i);

		loadPlayerTeamSkin(i);

		if (toolid < 2) {
			int* toolkv6 = (int*)(clientBase+0x7cbc0+0x3a8*i);
			*toolkv6 = *(int*)(clientBase+0x13cf840);
		} else {
			loadPlayerWeaponSkin(i);
		}
	}
}

void loadimageFiles() {
	asm volatile("call *%0"::"r"(clientBase+0x32290));
}
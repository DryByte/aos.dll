#include <Aos.h>
#include <stdio.h>

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

__declspec(noinline) void load_player_team_skin(int player_id) {
	int teamid = *(int*)(client_base+0x7ce58+0x3a8*player_id);

	asm volatile(
		"mov %1, %%esi\n\t"
		"mov %0, %%ecx\n\t"
		"lea (%%esi), %%eax\n\t"
		"call *%2"
	:: "g" (teamid), "r" (client_base+0x7cb70+0x3a8*player_id), "g"(client_base+0x3dc50));
}

__declspec(noinline) void load_player_weapon_skin(int player_id) {
	int toolid = *(int*)(client_base+0x7ce88+0x3a8*player_id);

	asm volatile(
		"mov %1, %%esi\n\t"
		"push %0\n\t"
		"lea (%%esi), %%eax\n\t"
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

void update_minimap() {
	asm volatile("call *%0"::"r"(client_base+0x294b0));
}

void first_person_spec(int player_id) {
	int local_struct = (*(int*)(client_base+0x13b1cf0))*0x3a8;
	int dest_struct = player_id*0x3a8;

	float* velocity = (float*)(client_base+local_struct+0x7ce28);

	float* local_position = (float*)(client_base+local_struct+0x7ce1c);
	float* dest_position = (float*)(client_base+dest_struct+0x7ce1c);

	float* local_ori = (float*)(client_base+local_struct+0x7ce34);
	float* dest_ori = (float*)(client_base+dest_struct+0x7ce34);

	float* local_ori_dunno = (float*)(client_base+local_struct+0x7ce40);
	float* dest_ori_dunno = (float*)(client_base+dest_struct+0x7ce40);

	float* local_ori_idk = (float*)(client_base+local_struct+0x7ce4c);
	float* dest_ori_idk = (float*)(client_base+dest_struct+0x7ce4c);

	// yes fuck this, imma use loop
	// i make ugly code become more ugly
	for (int i = 0; i < 3; i++) {
		*velocity = 0.0; // x
		velocity++;

		*local_position = *dest_position;
		local_position++;
		dest_position++;

		*local_ori = *dest_ori;
		local_ori++;
		dest_ori++;

		*local_ori_dunno = *dest_ori_dunno;
		local_ori_dunno++;
		dest_ori_dunno++;

		*local_ori_idk = *dest_ori_idk;
		local_ori_idk++;
		dest_ori_idk++;
	}

	asm volatile("sub $0x4, %esp");
}

// later move this stuff to mod folder
int spec_fps = 0;
int old_spec_id = -1;
void handle_spectate_input() {
	char key = *(char*)(client_base+0x864a0+47);
	if(!key)
		return;

	spec_fps = !spec_fps;

	//if (!spec_fps && old_spec_id >= 0)
	//	load_player_team_skin(old_spec_id);
}

void handle_spec() {
	if (spec_fps) {
		int spec_id = *(int*)(client_base+0x13b1ce4);
		// if we remove players torso causes crash when player shot
		/*
		if (spec_id != old_spec_id) {
			if (old_spec_id >= 0)
				load_player_team_skin(old_spec_id);


			*((int*)(client_base+0x7cb70+0x3a8*spec_id+0xf8)) = 0; // remove player's torso
			old_spec_id = spec_id;
		}*/

		first_person_spec(spec_id);
	}
}

__declspec(naked) void spectator_movement_hook() {
	handle_spec();

	asm volatile(
		"mov %0, %%esi\n\t"
		"mov 0x13b75b0(%%esi), %%al\n\t"
		"mov 0x13cf6d0(%%esi), %%cl\n\t"
		"add $0x2bb15, %%esi\n\t"
		"jmp *%%esi"
	:: "r" (client_base));
}
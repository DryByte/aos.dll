#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <Hook.h>
#include <Rendering.h>
#include <Packets.h>
#include <AosConfig.h>
#include <Menu.h>
#include <Inputs.h>
#include <Voxlap.h>
#include <Aos.h>
#include <Modloader.h>
#include <Macro.h>
#include <Config.h>
#include <Presence.h>

HANDLE clientHandle;
int client_base; // this is hacky, should we just change the original clienthandle to int?
struct ItemMultitext* LoggerMultitext;
int presence_enabled;

DWORD WINAPI LoopFunction(LPVOID lpParam)
{
	clientHandle = GetModuleHandle(NULL);

	if(clientHandle == NULL) {
		printf("Cant load the module\n");
		return 0;
	}

	client_base = (int)clientHandle;

	printf("\n----------------\n");
	printf("DLL Injected :D\n");
	printf("----------------\n");

	//83 7e 0c 02
	// remove the block for packets of length less than 2 so we receive use versionget
	DWORD _old_protect;
	uint8_t* lengthLimit = (uint8_t*)(client_base+0x343e1);
	VirtualProtect((void*)lengthLimit, 1, PAGE_EXECUTE_READWRITE, &_old_protect);
	*lengthLimit = 1;

	create_hook(client_base, 0x33b12, map_packet_hook, 5);
	create_hook(client_base, 0x33df1, load_skin_hook, 10); // probably we can change this to a create call later
	create_hook(client_base, 0x343e4, packet_hook, 7);
	create_hook(client_base, 0x334a0, rendering_hook_bi, 5);
	create_hook(client_base, 0x334aa, rendering_hook_ai, 6);
	create_hook(client_base, 0x3126d, hook_inputs, 6);
	create_hook(client_base, 0x2bb10, spectator_movement_hook, 11);

	struct Menu* LoggerMenu = create_menu(300, 200, 0, "Logger");
	LoggerMultitext = create_multitext(LoggerMenu, 0xffffffff);

	struct Menu* demomenu = create_menu(500, 300, 0, "demo");
	struct ItemText* txt = create_text(demomenu, 8, 0xffff0000, "ok isso é um teste");
	txt->x_pos = 15;
	txt->y_pos = 15;
	int testint = 3;
	struct ItemSlide* slide = create_slide(demomenu, 0, 15, &testint);
	slide->show_status = 1;
	slide->x_pos = 15;
	slide->y_pos = 30;

	init_menu();
	init_config();
	init_macro();
	init_mod_loader();
	load_aos_config();
	init_rich_presence();

	set_max_fps(config_get_int_entry(NULL, "max_fps", 60));

	struct WindowSize fds = get_config_window_size();
	printf("%i\n", fds.width);
	printf("%i\n", fds.height);
	printf("%i\n", get_config_volume());
	printf("%.2f\n", get_config_mouse_sensitivity());

	struct player_struct* p_ar = (void*)(client_base+0x7cb70);
	int isMenuHidden = 0;
	while (1) {
		if (GetAsyncKeyState(VK_MENU)) {
			printf("Player #0: %i, is_alive: %i, hp: %i %s\n", p_ar[0].player_id, p_ar[0].is_alive, p_ar[0].hp, p_ar[0].player_name);
			printf("Player #1: %i, is_alive: %i, hp: %i %s\n", p_ar[1].player_id, p_ar[1].is_alive, p_ar[1].hp, p_ar[1].player_name);
			printf("Player #2: %i, is_alive: %i, hp: %i %s\n", p_ar[2].player_id, p_ar[2].is_alive, p_ar[2].hp, p_ar[2].player_name);
			if (isMenuHidden) {
				isMenuHidden = 0;
				show_all_menus();
			} else {
				isMenuHidden = 1;
				hide_all_menus();
			}
		} else if (GetAsyncKeyState(VK_NEXT)) {
			LoggerMultitext->current_pos+=1;
		} else if (GetAsyncKeyState(VK_PRIOR)) {
			if (LoggerMultitext->current_pos > 0)
				LoggerMultitext->current_pos-=1;
		}

		if (presence_enabled) { update_presence(); }
		Sleep(250);
	}

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwAttached, LPVOID lpvReserved)
{
	if (dwAttached == DLL_PROCESS_ATTACH) {
		CreateThread(NULL,0,&LoopFunction,NULL,0,NULL);
	}

	return 1;
}
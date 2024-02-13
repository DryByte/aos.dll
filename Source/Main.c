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

HANDLE clientHandle;
int client_base; // this is hacky, should we just change the original clienthandle to int?
struct ItemMultitext* LoggerMultitext;

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

	set_max_fps(120);

	//83 7e 0c 02
	// remove the block for packets of length less than 2 so we receive use versionget
	DWORD _old_protect;
	uint8_t* lengthLimit = (uint8_t*)(client_base+0x343e1);
	VirtualProtect((void*)lengthLimit, 1, PAGE_EXECUTE_READWRITE, &_old_protect);
	*lengthLimit = 1;

	create_hook(client_base, 0x343e4, packet_hook, 7);
	create_hook(client_base, 0x32f00, rendering_hook_bi, 6);
	create_hook(client_base, 0x334aa, rendering_hook_ai, 6);
	create_hook(client_base, 0x3126d, hook_inputs, 6);

	struct Menu* LoggerMenu = create_menu(300, 200, 0, "Logger");
	LoggerMultitext = create_multitext(LoggerMenu, 0xffffff);

	initmacro();
	initmodloader();
	load_aos_config();
	struct WindowSize fds = get_config_window_size();
	printf("%i\n", fds.width);
	printf("%i\n", fds.height);
	printf("%i\n", get_config_volume());
	printf("%.2f\n", get_config_mouse_sensitivity());

	int isMenuHidden = 0;
	while (1) {
		if (GetAsyncKeyState(VK_MENU)) {
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
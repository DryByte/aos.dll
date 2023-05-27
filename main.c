#include "windows.h"
#include "stdio.h"
#include <stdint.h>
#include <enet/enet.h>
#include <hook.h>
#include <rendering.h>
#include <packets.h>

HANDLE clientBase;

DWORD WINAPI LoopFunction(LPVOID lpParam)
{
	HANDLE aos = GetCurrentProcess();
	clientBase = GetModuleHandle(NULL);

	if(clientBase == NULL) {
		printf("Cant load the module\n");
		return 0;
	}

	if (enet_initialize())
	{
		printf("Can't init enet\n");
	}

	printf("\n----------------\n");
	printf("DLL Injected :D\n");
	printf("----------------\n");

	setMaxFPS(120);

	//83 7e 0c 02
	// remove the block for packets of length less than 2 so we receive use versionget
	DWORD _old_protect;
	uint8_t* lengthLimit = (uint8_t*)(clientBase+0x343e1);
	VirtualProtect((void*)lengthLimit, 1, PAGE_EXECUTE_READWRITE, &_old_protect);
	*lengthLimit = 1;

	createHook(clientBase, 0x343e4, packetHandler);
	createHook(clientBase, 0x334aa, renderingHook);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwAttached, LPVOID lpvReserved)
{
	if (dwAttached == DLL_PROCESS_ATTACH) {
		CreateThread(NULL,0,&LoopFunction,NULL,0,NULL);
	}

	return 1;
}
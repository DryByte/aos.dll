#include <Hook.h>

void create_hook(int client_base, int address, void* func, int length) {
	if (length < 5) { return; }

	DWORD oldProtect;

	unsigned char* hookLoc = (unsigned char*)(client_base + address);
	VirtualProtect((LPVOID*)hookLoc, length, PAGE_EXECUTE_READWRITE, &oldProtect);

	memset(hookLoc, 0x90, length);

	DWORD relativeAddr = (DWORD)func - ((DWORD)hookLoc + 5);

	*(BYTE*)hookLoc = 0xE9;
	*(DWORD*)(hookLoc + 1) = relativeAddr;

	DWORD temp;
	VirtualProtect(hookLoc, length, oldProtect, &temp);
}
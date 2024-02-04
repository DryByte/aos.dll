#include <Hook.h>

void createHook(int clientBase, int address, void* func) {
	DWORD old_protect;

	unsigned char* hookLoc = (unsigned char*)(clientBase+address);
	VirtualProtect((void*)hookLoc, 6, PAGE_EXECUTE_READWRITE, &old_protect);

	*hookLoc = 0xE9;
	*(DWORD*)(hookLoc + 1) = (DWORD)func - ((DWORD)hookLoc + 5);
	*(hookLoc + 5) = 0x90;
}
#include <window.h>
#include <stdio.h>

// depending of the use you need to call showwindow
HWND getHWND() {
	return *(HWND*)(clientBase+0x85cfc);
}

RECT getWindowRect() {
	HWND wind = getHWND();

	RECT winrect;
	// since the window isnt updated because its the same state
	// this probably is not a useful call, but it fixes the results
	// for getclientrect
	ShowWindow(wind, 1);
	GetClientRect(wind, &winrect);

	return winrect;
}

// maybe we should use native windows func?
void aosToggleCursor() {
	asm volatile (
		"mov %0, %%ebx\n\t"
		"mov %%ebx, %%ecx\n\t"
		"add $0x51474, %%ecx\n\t" // cursor state, hidden or visible
		"mov (%%ecx), %%ecx\n\t"
		"xor $0x1, %%ecx\n\t"
		"add $0x19a50, %%ebx\n\t"
		"mov %%ecx, %%eax\n\t"
		"call *%%ebx"
	:: "r" (clientBase));
}
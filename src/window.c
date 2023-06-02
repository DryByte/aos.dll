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
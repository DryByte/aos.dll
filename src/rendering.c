#include <rendering.h>
#include <stdio.h>

void setMaxFPS(int fps) {
	DWORD old_protect;
	float* fpsRegion = (float*)(clientBase+0x48e00);

	VirtualProtect((void*)fpsRegion, 4, PAGE_EXECUTE_READWRITE, &old_protect);
	*fpsRegion = (float)1/fps;
}

void drawText(int x, int y, int color, char* msg) {
	asm volatile(
		"push %1\n\t"
		"push %2\n\t"
		"mov %3, %%edi\n\t"
		"mov %4, %%ecx\n\t"

		"call *%0\n\t"
		"add $8, %%esp"
		:: "r" (clientBase+0x20290), "r"(msg), "r"(y), "r"(color),  "r" (x)
	);
}


void drawCustomFontText(int x, int y, int color, int fontid, char *msg) {
	fontid = fontid*4;

	asm volatile(
		"mov %5, %%esi\n\t" // msg
		"mov %0, %%edi\n\t" // clientBase

		"mov $0x85cf0, %%ecx\n\t"
		"add %4, %%ecx\n\t"
		"add %%edi, %%ecx\n\t" // font
		"mov (%%ecx), %%ecx\n\t"

		"mov $0x3c3d0, %%eax\n\t"
		"add %%edi, %%eax\n\t"
		"call *%%eax\n\t" // setTextFontBasedOnText

		"mov $0x86aa0, %%eax\n\t" // font pointer
		"add %%edi, %%eax\n\t"
		"movl %%ecx, 4(%%eax)\n\t" // set font pointer
		"mov %3, %%eax\n\t" // color
		"mov $0x3c460, %%ebx\n\t"
		"add %%edi, %%ebx\n\t"
		"call *%%ebx\n\t" // setTextColor

		"push %%esi\n\t" // text
		"push %2\n\t" // y
		"push %1\n\t" // x
		"push 0x84aa8(%%edi)\n\t"//"push $40\n\t" // idk

		"mov 0x86aa0(%%edi), %%edx\n\t" // font pointer
		"push %%edx\n\t" // fontpointer
		"mov $0x3c2b0, %%eax\n\t"
		"add %%edi, %%eax\n\t"
		"call *%%eax"
	:: "r" (clientBase), "g" (x), "g" (y), "g" (color), "g" (fontid), "r" (msg));
}

void drawLine(int x1, int y1, int x2, int y2, int color) {
	asm volatile(
		"mov %0, %%edi\n\t"
		"push %5\n\t"
		"push %4\n\t" 
		"push %3\n\t"
		"push %1\n\t"
		"mov %2, %%eax\n\t"
		"add $0x1ef80, %%edi\n\t"
		"call *%%edi\n\t"
		// weird stack isnt getting clear in the function?
		"pop %%esi\n\t"
		"pop %%esi\n\t"
		"pop %%esi\n\t"
		"pop %%esi\n\t"
	:: "r" (clientBase), "g" (x1), "g" (y1), "g" (x2), "g" (y2), "g" (color));
}

void renderStats() {
	char fps[20];
	int maxFps = (int)(1 / *(float*)(clientBase+0x48e00))+1;
	int currentFps = (int)(1 / *(float*)(clientBase+0x13cf83c))+1;

	drawLine(1, 1, 10*8, 1, 0xff0000);
	drawLine(1, 10, 10*8, 10, 0xff0000);
	drawLine(1, 1, 1, 10, 0xff0000);
	drawLine(10*8, 1, 10*8, 10, 0xff0000);

	sprintf(fps, "FPS: %i/%i", currentFps, maxFps);
	drawText(2,2, 0xffffff, fps);
}

char info_msg[255];

void addInfoMessage(char *msg) {
	strcpy(info_msg, msg);
}

void test() {
	RECT b = getWindowRect();
	printf("%i\n", b.bottom-b.top);
	printf("%i\n", b.right-b.left);
}
__declspec(naked) void renderingHook() {
	asm volatile("pusha");
	renderStats();

	printf("-------------\n");
	test();

	//MessageBoxA(*(HWND*)(clientBase+0x85cfc), "fds?", "Error", MB_OK);

	drawCustomFontText(255, 255, 0xffffff, 2, info_msg);

	asm volatile("popa");
	asm volatile (
		"add $0x85cd0, %%eax\n\t"
		"movl (%%eax), %%ecx\n\t"
		"movl $0x0, %%ebp\n\t" // this not sounds right, but its working, so its right
		"jmp *%0"
		:: "r" (clientBase+0x334b0), "r" (clientBase) //probably we can change it later, im tired rn
	);
}
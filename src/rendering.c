#include <rendering.h>
#include <stdio.h>
#include <aos_config.h>
#include <menu.h>

struct customMessage customMessagesBuffer[4];

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

int getCustomFontSize(int fontid, char *msg) {
	fontid = fontid*4;
	int len;

	asm volatile(
		"mov %1, %%edi\n\t"
		"mov %2, %%esi\n\t"

		"mov $0x85cf0, %%ecx\n\t"
		"add %3, %%ecx\n\t"
		"add %%edi, %%ecx\n\t" // font
		"mov (%%ecx), %%ecx\n\t"

		"mov $0x3c3d0, %%eax\n\t"
		"add %%edi, %%eax\n\t"
		"call *%%eax\n\t" // getTextLength
		"mov %%eax, %0\n\t"
	:"=r" (len) : "r" (clientBase), "r" (msg),"g" (fontid));

	return len;
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

		"mov $0x86aa0, %%eax\n\t" // font pointer
		"add %%edi, %%eax\n\t"
		"mov (%%eax), %%eax\n\t"
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

// colors in argb
void drawProgressBar(float progress, int progressColor, int backgroundColor) {
	asm volatile(
		"mov %0, %%eax\n\t"
		"add $0x2ba20, %%eax\n\t"
		"push %2\n\t"
		"push %3\n\t"
		"push %1\n\t"//progress
		"call *%%eax\n\t"
		// clean our mess
		"pop %%eax\n\t"
		"pop %%eax\n\t"
		"pop %%eax\n\t"
		:: "r" (clientBase), "g"(progress), "g" (progressColor), "g" (backgroundColor));
}

//(tf,tp,tx,ty,tcx,tcy): Tile source, (tcx&tcy) is texel (<<16) at (sx,sy)
//(sx,sy,xz,yz) screen coordinates and x&y zoom, all (<<16)
//(black,white): black & white shade scale (ARGB format)
//   Note: if alphas of black&white are same, then alpha channel ignored
void drawtile(long tf, long tp, long tx, long ty, long tcx, long tcy, long sx, long sy, long xz, long yz, long black) {
	asm volatile(
		"push %11\n\t" //black
		"push %10\n\t" // zoom
		"push %9\n\t" // zoom

		"movl (0x486aac), %%edi\n\t" // sy
		"sub %8, %%edi\n\t"
		"shl $0x10, %%edi\n\t"
		"push %%edi\n\t"
		"movl (0x486204), %%esi\n\t" // sx
		"sub %7, %%esi\n\t"
		"shl $0x10, %%esi\n\t"

		"push %6\n\t" // tcy
		"push %5\n\t" // tcx

		"mov %4, %%ecx\n\t" // ty
		"push %3\n\t" // tx
		
		"push %2\n\t" // tp

		"push %1\n\t" // buffer
		"mov %0, %%edi\n\t"
		"mov %%esi, %%eax\n\t"

		"add $0x22390, %%edi\n\t"
		"call *%%edi\n\t"
		"add $0x24, %%esp\n\t"
	:: "r" (clientBase), "g" (tf), "g" (tp), "g" (tx), "g" (ty), "g" (tcx), "g" (tcy), "g" (sx), "g" (sy), "g" (xz), "g" (yz), "g" (black));
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

void addCustomMessage(int type, char *msg) {
	struct customMessage c_msg;
	c_msg.timestamp = time(NULL);
	strncpy(c_msg.msg, msg, 255);

	customMessagesBuffer[type-3] = c_msg;
}

void renderCustomMessages() {
	time_t currentTimestamp = time(NULL);
	for (int i = 0; i < 4; i++) {
		struct customMessage c_msg = customMessagesBuffer[i];

		if (currentTimestamp - c_msg.timestamp < 10) {
			struct WindowSize wind = getConfigWindowSize();
			int x;
			int y;
			int color;
			int fontid;

			printf("%i\n", i+3);
			switch(i+3) {
				case MESSAGE_STATUS:
					color = 0xffffff;
					fontid = 2;
					x = (wind.width - getCustomFontSize(fontid, c_msg.msg)) / 2;
					y = wind.height*10/100;
					break;
				case MESSAGE_NOTICE:
					color = 0xffffff;
					fontid = 1;
					x = (wind.width - getCustomFontSize(fontid, c_msg.msg)) / 2;
					y = wind.height*60/100;

					break;
				case MESSAGE_WARNING:
					color = 0xffee00;
					fontid = 1;
					x = (wind.width - getCustomFontSize(fontid, c_msg.msg)) / 2;
					y = wind.height*63/100;

					break;
				case MESSAGE_ERROR:
					color = 0xff0000;
					fontid = 1;
					x = (wind.width - getCustomFontSize(fontid, c_msg.msg)) / 2;
					y = wind.height*66/100;

					break;
			}

			drawCustomFontText(x, y, color, fontid, c_msg.msg);
		}
	}
}

long colors[] = {
		0x00ffffff,0x00ffffff,0x00ffffff, 0xffff00ff, 0xffff00ff, 0x00ffffff,0x00ffffff,0x00ffffff,
		0x00ffffff,0x00ffffff,0x00ffffff, 0xffff00ff, 0xffff00ff, 0x00ffffff,0x00ffffff,0x00ffffff,
		0x00ffffff,0x00ffffff,0x00ffffff, 0xffff00ff, 0xffff00ff, 0x00ffffff,0x00ffffff,0x00ffffff,
		0x00ffffff,0x00ffffff,0x00ffffff, 0xffff00ff, 0xffff00ff, 0x00ffffff,0x00ffffff,0x00ffffff,
		0x00ffffff,0x00ffffff,0x00ffffff, 0xffff00ff, 0xffff00ff, 0x00ffffff,0x00ffffff,0x00ffffff,
		0xffff00ff,0xffff00ff,0xffff00ff, 0xffff00ff, 0xffff00ff, 0xffff00ff,0xffff00ff,0xffff00ff,
		0xffff00ff,0xffff00ff,0xffff00ff, 0x00ffffff, 0x00ffffff, 0xffff00ff,0xffff00ff,0xffff00ff,
		0xffff00ff,0xffff00ff,0xffff00ff, 0x00ffffff, 0x00ffffff, 0xffff00ff,0xffff00ff,0xffff00ff,
	};
__declspec(naked) void renderingHook() {
	asm volatile("pusha");
	drawtile(colors, 4*8, 8, 8, 0x0, 0x0, 0x80, 0x80, 0x80000, 0x80000, -1);

	renderStats();
	renderCustomMessages();
	drawMenus();
	//drawProgressBar(0.8, 0xff454545, 0xffffff33);

	asm volatile("popa");
	asm volatile (
		"add $0x85cd0, %%eax\n\t"
		"movl (%%eax), %%ecx\n\t"
		"movl $0x0, %%ebp\n\t" // this not sounds right, but its working, so its right
		"movl $0x18FFC58, 8(%%esp)\n\t" // is this fine to do? At the end of renderUI function it will try to pop this, but drawtile fucked up everything by moving stuff, what reseted the stack
		"jmp *%0"
		:: "r" (clientBase+0x334b0), "r" (clientBase) //probably we can change it later, im tired rn
	);
}
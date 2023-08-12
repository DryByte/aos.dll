#include <rendering.h>
#include <stdio.h>
#include <aos_config.h>
#include <menu.h>
#include <voxlap.h>

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

// colors in argb
void drawProgressBar(float progress, int progressColor, int backgroundColor) {
	asm volatile(
		"mov %0, %%eax\n\t"
		"add $0x2ba20, %%eax\n\t"
		"push %2\n\t"
		"push %3\n\t"
		"push %1\n\t"//progress
		"call *%%eax\n\t"
		"add $0xc, %%esp"
		:: "r" (clientBase), "g"(progress), "g" (progressColor), "g" (backgroundColor));
}

void renderStats() {
	char fps[20];
	int maxFps = (int)(1 / *(float*)(clientBase+0x48e00))+1;
	int currentFps = (int)(1 / *(float*)(clientBase+0x13cf83c))+1;

	drawline2d(1, 1, 10*8, 1, 0xff0000);
	drawline2d(1, 10, 10*8, 10, 0xff0000);
	drawline2d(1, 1, 1, 10, 0xff0000);
	drawline2d(10*8, 1, 10*8, 10, 0xff0000);

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

void renderChatshadow() {
	char *chatBuffer = (char*)(clientBase+0x840c0);
	float currentTS = *(float*)(clientBase+0x13cf8d4);

	int yPos = 505;
	int ySize = 1;
	int xSize = 1;

	int hasChatLog = *chatBuffer;
	int isChatOpen = *(int*)(clientBase+0x84660);
	if (isChatOpen) {
		yPos -= 20;
		ySize += 20;

		char msgLen = strlen((char*)(clientBase+0x12b16e0));
		char globalLen = strlen("global: ");

		xSize = msgLen > globalLen ? msgLen : globalLen;
	}


	if (hasChatLog) {
		while (chatBuffer < clientBase+0x840c0+0x74*6) {
			if (!*chatBuffer)
				break;

			if (chatBuffer != clientBase+0x840c0) {
				float *msgTimestamp = (float *)chatBuffer;

				if (currentTS >= *msgTimestamp) {
					break;
				}
				chatBuffer += 0x4;
			}

			xSize = xSize >= strlen(chatBuffer) ? xSize : strlen(chatBuffer)+1;
			ySize+=11;

			chatBuffer += 0x74;
		}
	}

	if (isChatOpen || hasChatLog) {
		long color[] = {0xe0000000};
		drawtile(color, 1, 1, 1, 0x0, 0x0, 10, yPos, 6*xSize, ySize, -1);
	}
}

// hook before interface render
__declspec(naked) void renderingHookBI() {
	asm volatile("pusha");

	renderChatshadow();

	renderStats();
	//drawProgressBar(0.8, 0xff454545, 0xffffff33);

	asm volatile("popa");
	asm volatile (
		"mov %0, %%esi\n\t"
		"mov %%esi, %%eax\n\t"
		"add $0x13b1e04, %%eax\n\t"
		"movl (%%eax), %%eax\n\t"
		"inc %%eax\n\t"

		"add $0x32f05, %%esi\n\t"
		"jmp *%%esi"
		:: "r" (clientBase) //probably we can change it later, im tired rn
	);

	
}

// hook after interface render
// to render customFontText, you should do this after the interface render
__declspec(naked) void renderingHookAI() {
	asm volatile("pusha");

	renderCustomMessages();
	drawMenus();

	asm volatile("popa");
	asm volatile(
		"mov %0, %%ecx\n\t"
		"movl 0x85cd0(%%ecx), %%ecx\n\t"
		"add $0x334b0, %0\n\t"
		"jmp *%0"
	:: "r" (clientBase));
}
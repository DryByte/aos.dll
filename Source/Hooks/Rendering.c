#include <Rendering.h>
#include <stdio.h>
#include <AosConfig.h>
#include <Menu.h>
#include <Voxlap.h>

struct custom_message customMessagesBuffer[4];

void set_max_fps(int fps) {
	DWORD old_protect;
	float* fps_region = (float*)(client_base+0x48e00);

	VirtualProtect((void*)fps_region, 4, PAGE_EXECUTE_READWRITE, &old_protect);
	*fps_region = (float)1/fps;
}

void draw_text(int x, int y, int color, char* msg) {
	asm volatile(
		"push %1\n\t"
		"push %2\n\t"
		"mov %3, %%edi\n\t"
		"mov %4, %%ecx\n\t"

		"call *%0\n\t"
		"add $8, %%esp"
		:: "r" (client_base+0x20290), "r"(msg), "r"(y), "r"(color),  "r" (x)
	);
}

int get_custom_font_size(int font_id, char *msg) {
	font_id = font_id*4;
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
	:"=r" (len) : "r" (client_base), "r" (msg),"g" (font_id));

	return len;
}

void draw_custom_font_text(int x, int y, int color, int font_id, char *msg) {
	font_id = font_id*4;

	asm volatile(
		"mov %5, %%esi\n\t" // msg
		"mov %0, %%edi\n\t" // client_base

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
	:: "r" (client_base), "g" (x), "g" (y), "g" (color), "g" (font_id), "r" (msg));
}

// colors in argb
void draw_progress_bar(float progress, int progress_color, int background_color) {
	asm volatile(
		"mov %0, %%eax\n\t"
		"add $0x2ba20, %%eax\n\t"
		"push %2\n\t"
		"push %3\n\t"
		"push %1\n\t"//progress
		"call *%%eax\n\t"
		"add $0xc, %%esp"
		:: "r" (client_base), "g"(progress), "g" (progress_color), "g" (background_color));
}

void render_stats() {
	char fps[20];
	int max_fps = (int)(1 / *(float*)(client_base+0x48e00))+1;
	int current_fps = (int)(1 / *(float*)(client_base+0x13cf83c))+1;

	draw_square(1, 1, 10*8, 10, 0xff0000);
	sprintf(fps, "FPS: %i/%i", current_fps, max_fps);
	draw_text(2,2, 0xffffff, fps);
}

void add_custom_message(int type, char *msg) {
	struct custom_message c_msg;
	c_msg.timestamp = time(NULL);
	strncpy(c_msg.msg, msg, 255);

	customMessagesBuffer[type-3] = c_msg;
}

void render_custom_messages() {
	time_t current_timestamp = time(NULL);
	for (int i = 0; i < 4; i++) {
		struct custom_message c_msg = customMessagesBuffer[i];

		if (current_timestamp - c_msg.timestamp < 10) {
			struct WindowSize wind = get_config_window_size();
			int x;
			int y;
			int color;
			int fontid;

			printf("%i\n", i+3);
			switch(i+3) {
				case MESSAGE_STATUS:
					color = 0xffffff;
					fontid = 2;
					x = (wind.width - get_custom_font_size(fontid, c_msg.msg)) / 2;
					y = wind.height*10/100;
					break;
				case MESSAGE_NOTICE:
					color = 0xffffff;
					fontid = 1;
					x = (wind.width - get_custom_font_size(fontid, c_msg.msg)) / 2;
					y = wind.height*60/100;

					break;
				case MESSAGE_WARNING:
					color = 0xffee00;
					fontid = 1;
					x = (wind.width - get_custom_font_size(fontid, c_msg.msg)) / 2;
					y = wind.height*63/100;

					break;
				case MESSAGE_ERROR:
					color = 0xff0000;
					fontid = 1;
					x = (wind.width - get_custom_font_size(fontid, c_msg.msg)) / 2;
					y = wind.height*66/100;

					break;
			}

			draw_custom_font_text(x, y, color, fontid, c_msg.msg);
		}
	}
}

void render_chat_shadow() {
	char chat_buffer = *(char*)(client_base+0x840c0);
	float current_ts = *(float*)(client_base+0x13cf8d4);

	struct WindowSize wins = get_config_window_size();
	unsigned int y = wins.height-95;
	unsigned int ys = 1;
	unsigned int xs = 1;

	int is_chat_open = *(int*)(client_base+0x84660);
	if (is_chat_open) {
		y -= 20;
		ys += 20;

		char msg_len = strlen((char*)(client_base+0x12b16e0));
		char global_len = strlen("global: ");

		xs = msg_len > global_len ? msg_len : global_len;
	}


	if (chat_buffer) {
		int mem_address = client_base+0x840c0;
		while (mem_address < client_base+0x840c0+0x74*6) {
			char* content = (char*)mem_address;
			if (!content)
				break;

			if (mem_address != client_base+0x840c0) {
				float msg_timestamp = *(float *)mem_address;

				if (current_ts >= msg_timestamp) {
					break;
				}
				mem_address += 0x4;
			}

			xs = xs >= strlen(content) ? xs : strlen(content)+1;
			ys+=11;

			mem_address += 0x74;
		}
	}

	if (is_chat_open || chat_buffer) {
		long color[] = {0xe0000000};
		drawtile((long)color, 1, 1, 1, 0x0, 0x0, 10, y, 6*xs, ys, -1);
	}
}

void draw_square(int x1, int y1, int x2, int y2, int color) {
	drawline2d(x1, y1, x2, y1, color);
	drawline2d(x2, y1, x2, y2, color);
	drawline2d(x1, y2, x2, y2, color);
	drawline2d(x1, y1, x1, y2, color);
}

// hook before interface render
__declspec(naked) void rendering_hook_bi() {
	asm volatile("pusha");

	render_chat_shadow();

	render_stats();
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
		:: "r" (client_base) //probably we can change it later, im tired rn
	);

	
}

// hook after interface render
// to render customFontText, you should do this after the interface render
__declspec(naked) void rendering_hook_ai() {
	asm volatile("pusha");

	render_custom_messages();
	draw_menus();

	asm volatile("popa");
	asm volatile(
		"mov %0, %%ecx\n\t"
		"movl 0x85cd0(%%ecx), %%ecx\n\t"
		"add $0x334b0, %0\n\t"
		"jmp *%0"
	:: "r" (client_base));
}
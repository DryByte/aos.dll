#include <AosConfig.h>

int CONFIG_X_SIZE;
int CONFIG_Y_SIZE;
int CONFIG_VOLUME;
float CONFIG_MOUSE_SENSITIVITY; // the current sensitivity is stored on base+0x13b1cac
uint8_t CONFIG_WINDOWED;
char CONFIG_NAME[16];
uint8_t CONFIG_LANGUAGE;
uint8_t CONFIG_SHOW_NEWS;

struct WindowSize get_config_window_size() {
	struct WindowSize wins;
	wins.width = CONFIG_X_SIZE;
	wins.height = CONFIG_Y_SIZE;

	return wins;
}

int get_config_volume() {
	return CONFIG_VOLUME;
}

float get_config_mouse_sensitivity() {
	return CONFIG_MOUSE_SENSITIVITY;
}

void load_aos_config() {
	CONFIG_X_SIZE = *(int*)(client_base+0x86204);
	CONFIG_Y_SIZE = *(int*)(client_base+0x86aac);
	CONFIG_VOLUME = *(int*)(client_base+0x12b1b5c);
	CONFIG_MOUSE_SENSITIVITY = *(float*)(client_base+0x13b75c4);
	CONFIG_WINDOWED = *(uint8_t*)(client_base+0x85c64);
	strncpy(CONFIG_NAME, (char*)(client_base+0x13b1f10), 16);
	//CONFIG_NAME = *(char[16])(client_base+0x13b1f10);
	CONFIG_LANGUAGE = *(uint8_t*)(client_base+0x13cfc40);
	CONFIG_SHOW_NEWS = *(uint8_t*)(client_base+0x13cf8d8);
}
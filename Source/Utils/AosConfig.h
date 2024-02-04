#include <windows.h>
#include <stdint.h>

extern int client_base;

struct WindowSize {
	int width;
	int height;
};

struct WindowSize get_config_window_size();
int get_config_volume();
float get_config_mouse_sensitivity();
void load_aos_config();

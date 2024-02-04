#include <windows.h>
#include <time.h>

extern int client_base;

struct custom_message {
	time_t timestamp;
	char msg[255];
};

enum CustomMessageTypes
{
	MESSAGE_STATUS = 3,
	MESSAGE_NOTICE = 4,
	MESSAGE_WARNING = 5,
	MESSAGE_ERROR = 6
};

int get_custom_font_size(int fontid, char* msg);
void draw_text(int x, int y, int color, char* msg);
void draw_custom_font_text(int x, int y, int color, int fontid, char *msg);
void draw_progress_bar(float progress, int progress_color, int background_color);
void add_custom_message(int type, char* msg);
void draw_square(int x1, int y1, int x2, int y2, int color);
void rendering_hook_bi(void);
void rendering_hook_ai(void);
void set_max_fps(int fps);
#include <stdint.h>
#include <time.h>

#define MAX_MENU_ENTRIES 100
#define MAX_MENU_ITEMS 255

enum itemTypes {
	TEXT_ITEM,
	CLICKABLE_BUTTON_ITEM,
	MULTITEXT_ITEM,
	TEXTINPUT_ITEM,
	SLIDE_ITEM
};

struct Item {
	uint8_t type;
};

struct ItemText {
	uint8_t type;
	int id;
	int color;
	int font_size;
	char text[32];
	int x_pos, y_pos;
	struct Menu* menu;
};

struct MultitextNode {
	char text[128];
	void* next;
	void* previous;
};

struct ItemMultitext {
	uint8_t type;
	int id;
	int color;
	int x_size, y_size;
	int x_pos, y_pos;

	int current_pos;
	struct Menu* menu;
	struct MultitextNode* selected;
	struct MultitextNode* first_node;
	struct MultitextNode* last_node;
};

struct ItemClickableButton
{
	uint8_t type;
	int id;
	int is_clicking;
	long color;
	long hold_color;
	char text[32];
	int font_size;
	int x_size, y_size;
	int x_pos, y_pos;
	int is_toolbar;
	int interval;
	time_t last_interaction;
	struct Menu* menu;
	void (*event)();
};

struct ItemTextInput
{
	uint8_t type;
	long background_color;
	int id;
	char placeholder[128];
	char input[128];
	int max_length;
	int x_size, y_size;
	int x_pos, y_pos;
	struct Menu* menu;
};

struct ItemSlide {
	uint8_t type;
	int id;
	int x_size, y_size;
	int x_pos, y_pos;
	int slider_size;
	int max_value;
	int min_value;
	int show_status;
	long slider_color;
	long background_color;
	int* interact_int;
	struct Menu* menu;
};

struct Menu {
	int id;
	int x_pos, y_pos;
	int x_size, y_size;
	int fixed_size;
	int hidden;
	int minimized;
	int pin;
	int outline_color;
	int background_color;
	int always_hidden;
	int update;
	int is_interacting;
	int buffer_x;
	int buffer_y;
	int display_x;
	int display_y;
	int max_y;
	int offset_x;
	int offset_y;
	int* draw_buffer;
	int* display_buffer;
	char title[32];
	void* items[MAX_MENU_ITEMS];
};

void show_all_menus();
void hide_all_menus();
int get_next_available_menu_id();
struct ItemText* create_text(struct Menu* menu, int font_size, int color, char* text);
void add_new_text(struct ItemMultitext* multitext, char* text);
struct ItemMultitext* create_multitext(struct Menu* menu, int color);
struct ItemClickableButton* create_clickable_button(struct Menu* menu, char* text, void (*func)());
struct ItemTextInput* create_text_input(struct Menu* menu, int x_size, int y_size, long backgroundColor, char* placeholder);
struct ItemSlide* create_slide(struct Menu* menu, int minValue, int maxValue, int* interact);
void handle_keyboard();
struct Menu* create_menu(int x, int y, int outline, char* title);
void draw_to_buffer(struct Menu* menu, int* copy_buff, int offset_x, int offset_y, int size_x, int size_y);
void draw_line(struct Menu* menu, int color, int x1, int y1, int x2, int y2);
void draw_menus();
void init_menu();
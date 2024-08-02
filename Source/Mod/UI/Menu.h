#include <stdint.h>
#include <time.h>

#define MAX_MENU_ENTRIES 100
#define MAX_MENU_ITEMS 255

enum itemTypes {
	TEXT_ITEM,
	CLICKABLE_BUTTON_ITEM,
	MULTITEXT_ITEM,
	TEXTINPUT_ITEM,
	SLIDE_ITEM,
	SWITCH_BUTTON_ITEM
};

struct Item {
	uint8_t type;
};

struct ItemText {
	uint8_t type;
	int id;
	int color;
	int font_id;
	char text[32];
	int x_pos, y_pos;
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
	int x_size, y_size;
	int x_pos, y_pos;
	int is_toolbar;
	int interval;
	time_t last_interaction;
	void (*event)();
};

struct ItemSwitchButton
{
	uint8_t type;
	int id;
	int is_holding;
	int enabled;
	long enabled_color;
	long hold_color;
	long disabled_color;
	long color;
	char label[32];
	int x_size, y_size;
	int x_pos, y_pos;
	int interval;
	time_t last_interaction;
	void (*enable_event)();
	void (*disable_event)();
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
	int buffer_x;
	int buffer_y;
	int* buffer;
	char title[32];
	void* items[MAX_MENU_ITEMS];
};

void show_all_menus();
void hide_all_menus();
int get_next_available_menu_id();
struct ItemText* create_text(struct Menu* menu, int fontid, int color, char* text);
void add_new_text(struct ItemMultitext* multitext, char* text);
struct ItemMultitext* create_multitext(struct Menu* menu, int color);
struct ItemClickableButton* create_clickable_button(struct Menu* menu, char* text, void (*func)());
struct ItemTextInput* create_text_input(struct Menu* menu, int x_size, int y_size, long backgroundColor, char* placeholder);
struct ItemSlide* create_slide(struct Menu* menu, int minValue, int maxValue, int* interact);
struct ItemSwitchButton* create_switch_button(struct Menu* menu, char* label, void (*enable_func)(), void (*disable_func)(), int enabled);
void handle_keyboard();
struct Menu* create_menu(int x, int y, int outline, char* title);
void draw_menus();
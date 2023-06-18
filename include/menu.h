#include <stdint.h>

#define MAX_MENU_ENTRIES 100
#define MAX_MENU_ITEMS 255

enum itemTypes {
	TEXT_ITEM,
	BUTTON_ITEM
};

struct Item {
	uint8_t type;
};

struct ItemText {
	uint8_t type;
	int id;
	int color;
	int fontId;
	char text[32];
};

struct Menu {
	int x, y;
	int outlineColor;
	int id;
	char title[32];
	void* items[MAX_MENU_ITEMS];
};

int getNextAvailableMenuId();
void createText(struct Menu* menu, int fontid, int color, char* text);
struct Menu* createMenu(int x, int y, int outline, char* title);
void drawMenus();
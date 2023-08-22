#include <stdint.h>

#define MAX_MENU_ENTRIES 100
#define MAX_MENU_ITEMS 255

enum itemTypes {
	TEXT_ITEM,
	BUTTON_ITEM,
	MULTITEXT_ITEM
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

struct MultitextNode {
	char text[128];
	void* next;
	void* previous;
};

struct ItemMultitext {
	uint8_t type;
	int id;
	int color;

	int currentPos;
	struct MultitextNode* firstNode;
	struct MultitextNode* lastNode;
};

struct Menu {
	int id;
	int x, y;
	int xSize, ySize;
	int fixedSize;
	int hidden;
	int outlineColor;
	int backgroundColor;

	char title[32];
	void* items[MAX_MENU_ITEMS];
};

void showAllMenus();
void hideAllMenus();
int getNextAvailableMenuId();
void createText(struct Menu* menu, int fontid, int color, char* text);
void addNewText(struct ItemMultitext* multitext, char* text);
struct ItemMultitext* createMultitext(struct Menu* menu, int color);
struct Menu* createMenu(int x, int y, int outline, char* title);
void drawMenus();
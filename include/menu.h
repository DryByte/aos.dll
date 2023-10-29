#include <stdint.h>

#define MAX_MENU_ENTRIES 100
#define MAX_MENU_ITEMS 255

enum itemTypes {
	TEXT_ITEM,
	CLICKABLE_BUTTON_ITEM,
	MULTITEXT_ITEM,
	TEXTINPUT_ITEM
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

struct ItemClickableButton
{
	uint8_t type;
	int id;
	int isClicking;
	long color;
	long holdColor;
	char text[32];
	int xSize, ySize;
	void (*event)();
};

struct ItemTextInput
{
	uint8_t type;
	uint8_t isActive;
	long backgroundColor;
	int id;
	char placeholder[128];
	char input[128];
	int xSize, ySize;
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
void createClickableButton(struct Menu* menu, char* text, void (*func)());
void createTextInput(struct Menu* menu, int xSize, int ySize, long backgroundColor, char* placeholder);
struct Menu* createMenu(int x, int y, int outline, char* title);
void drawMenus();
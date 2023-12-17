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
	int fontId;
	char text[32];
	int xPos, yPos;
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
	int xSize, ySize;
	int xPos, yPos;

	int currentPos;
	struct MultitextNode* selected;
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
	int xPos, yPos;
	int isToolbar;
	int interval;
	time_t lastInteraction;
	void (*event)();
};

struct ItemTextInput
{
	uint8_t type;
	long backgroundColor;
	int id;
	char placeholder[128];
	char input[128];
	int xSize, ySize;
	int xPos, yPos;
};

struct ItemSlide {
	uint8_t type;
	int id;
	int xSize, ySize;
	int xPos, yPos;
	int sliderSize;
	int maxValue;
	int minValue;
	int showStatus;
	long sliderColor;
	long backgroundColor;
	int* interactInt;
};

struct Menu {
	int id;
	int x, y;
	int xSize, ySize;
	int fixedSize;
	int hidden;
	int minimized;
	int pin;
	int outlineColor;
	int backgroundColor;

	char title[32];
	void* items[MAX_MENU_ITEMS];
};

void showAllMenus();
void hideAllMenus();
int getNextAvailableMenuId();
struct ItemText* createText(struct Menu* menu, int fontid, int color, char* text);
void addNewText(struct ItemMultitext* multitext, char* text);
struct ItemMultitext* createMultitext(struct Menu* menu, int color);
struct ItemClickableButton* createClickableButton(struct Menu* menu, char* text, void (*func)());
struct ItemTextInput* createTextInput(struct Menu* menu, int xSize, int ySize, long backgroundColor, char* placeholder);
struct ItemSlide* createSlide(struct Menu* menu, int minValue, int maxValue, int* interact);
void handleKeyboard();
struct Menu* createMenu(int x, int y, int outline, char* title);
void drawMenus();
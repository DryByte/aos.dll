#include <menu.h>
#include <stdio.h>
#include <rendering.h>
#include <windows.h>
#include <window.h>
#include <voxlap.h>
#include <aos_config.h>

#define MAX(x,y) (x>y) ? x : y
#define MIN(x,y) (x<y) ? x : y

struct Menu* menus[MAX_MENU_ENTRIES];

int mouseXPos = 0;
int mouseYPos = 0;
int showCursor = 0;

int getNextAvailableMenuId() {
	int i = 0;
	for (; i <= MAX_MENU_ENTRIES; i++) {
		if (menus[i] == NULL)
			break;
	}

	return i;
}

int getNextAvailableItemId(struct Menu* menu) {
	int i = 0;
	for (; i <= MAX_MENU_ITEMS; i++) {
		if (menu->items[i] == NULL)
			break;
	}

	return i;
}

void createText(struct Menu* menu, int fontid, int color, char* text) {
	int id = getNextAvailableItemId(menu);

	struct ItemText* txtItem = malloc(sizeof(struct ItemText)+32);
	txtItem->type = 0;
	txtItem->id = id;
	txtItem->color = color;
	txtItem->fontId = fontid;
	strncpy(txtItem->text, text, 32);

	menu->items[id] = txtItem;
	menus[menu->id] = menu;
}

void addNewText(struct ItemMultitext* multitext, char* text) {
	int strLen = strlen(text);
	struct MultitextNode* node = malloc(sizeof(struct MultitextNode) + (strLen < 128 ? strLen : 128));

	node->next = 0;
	strncpy(node->text, text, 128);

	if (multitext->lastNode == 0) {
		node->previous = 0;

		multitext->firstNode = node;
		multitext->lastNode = node;
	} else {
		node->previous = multitext->lastNode;

		multitext->lastNode->next = node;
		multitext->lastNode = node;
	}
}

struct ItemMultitext* createMultitext(struct Menu* menu, int color) {
	int id = getNextAvailableItemId(menu);

	struct ItemMultitext* multitextItem = malloc(sizeof(struct ItemMultitext));
	multitextItem->type = 2;
	multitextItem->id = id;
	multitextItem->color = color;
	multitextItem->currentPos = 0;
	multitextItem->firstNode = 0;
	multitextItem->lastNode = 0;

	menu->items[id] = multitextItem;
	menus[menu->id] = menu;

	return multitextItem;
}

struct Menu* createMenu(int x, int y, int outline, char* title) {
	int menuId = getNextAvailableMenuId();
	struct Menu* menu = malloc(sizeof(struct Menu)+32);	

	menu->id = menuId;
	menu->outlineColor = outline;
	menu->x = x;
	menu->y = y;
	menu->xSize = 150;
	menu->ySize = 100;
	menu->fixedSize = 1;
	menu->hidden = 1;
	strncpy(menu->title, title, 32);

	menus[menuId] = menu;
	return menu;
}

void showAllMenus() {
	showCursor = 1;
	int menusLen = getNextAvailableMenuId();

	for (int menuId = 0; menuId < menusLen; menuId++) {
		struct Menu* menu = (struct Menu*)menus[menuId];
		menu->hidden = 0;
	}
}

void hideAllMenus() {
	showCursor = 0;
	int menusLen = getNextAvailableMenuId();

	for (int menuId = 0; menuId < menusLen; menuId++) {
		struct Menu* menu = (struct Menu*)menus[menuId];
		menu->hidden = 1;
	}
}

void renderMenuText(struct Menu* menu, struct ItemText* item, int y) {
	drawCustomFontText(menu->x, menu->y+y, item->color, item->fontId, item->text);
}

//returns 1 if an interaction happened
int handleCursor() {
	int mx;
	int my;
	int status;

	struct WindowSize winsize = getConfigWindowSize();
	getmousechange(&mx, &my, &status);

	mouseXPos += mx;
	mouseYPos += my;

	if (mouseXPos < 0) {
		mouseXPos = 0;
	} else {
		mouseXPos = MIN(mouseXPos, winsize.width);
	}

	if (mouseYPos < 0) {
		mouseYPos = 0;
	} else {
		mouseYPos = MIN(mouseYPos, winsize.height);
	}

	return status;
}

void drawCursor() {
	// todo load mouse cursor image
	long color[] = {0xffffffff};
	drawtile(color, 1, 1, 1, 0x0, 0x0, mouseXPos, mouseYPos, 15, 15, -1);
}

void drawMenus() {
	int menusLen = getNextAvailableMenuId();

	int interaction = 0;
	if (showCursor)
		interaction = handleCursor();

	for (int menuId = 0; menuId < menusLen; menuId++) {
		struct Menu* menu = (struct Menu*)menus[menuId];
		if (menu->hidden)
			continue;

		int itemsLen = getNextAvailableItemId(menu);

		int largestX = MAX(menu->xSize, strlen(menu->title)*8);
		int largestY = 8;

		// title background
		long color[] = {0xe0000000};
		drawtile(color, 1, 1, 1, 0x0, 0x0, menu->x, menu->y, menu->xSize, largestY, -1);

		// content background
		color[0] = 0xc0000000;
		drawtile(color, 1, 1, 1, 0x0, 0x0, menu->x, menu->y, menu->xSize, menu->ySize, -1);

		drawText(menu->x, menu->y, 0xffffff, menu->title);

		for (int itemId = 0; itemId < itemsLen; itemId++) {
			void* item = menu->items[itemId];
			int itemType = ((struct Item*)item)->type;

			switch(itemType) {
				case TEXT_ITEM:
					struct ItemText* txtItem = (struct ItemText*)item;
					int txtSizeX = getCustomFontSize(txtItem->fontId, txtItem->text);
					if (menu->fixedSize && txtSizeX > menu->xSize) {
						int singleCharSize = txtSizeX/strlen(txtItem->text);
						for (int i = txtSizeX; i > menu->xSize; i--) {
							txtItem->text[i/singleCharSize] = '\0';
						}
					}

					renderMenuText(menu, item, largestY);

					largestX = MAX(largestX, txtSizeX);
					largestY += MAX(largestY, ((txtItem->fontId+1)*8)+8);
					break;
				case BUTTON_ITEM:
					printf("nop\n");
					break;
				case MULTITEXT_ITEM:
					struct ItemMultitext* multitext = (struct ItemMultitext*)item;
					struct MultitextNode* lastNode = multitext->lastNode;

					for (int i = 0; i < 5; i++) {
						int txtSizeX = strlen(lastNode->text)*6; // drawText uses 6x8

						if (menu->fixedSize && txtSizeX > menu->xSize) {
							char copyTxtNode[128];
							int characterCopy = 0;

							for (int characterNode = 0; characterNode < txtSizeX/6; characterNode++) {
								int copyTxtNodeLen = strlen(copyTxtNode)*6;
								if (copyTxtNodeLen+2 > menu->xSize) {
									drawText(menu->x, menu->y+largestY, multitext->color, copyTxtNode);
									largestY += 8;
									memset(copyTxtNode, 0, sizeof copyTxtNode);
									characterCopy = 0;
								}

								copyTxtNode[characterCopy] = lastNode->text[characterNode];
								characterCopy+=1;
							}

							drawText(menu->x, menu->y+largestY, multitext->color, copyTxtNode);
						} else {
							drawText(menu->x, menu->y+largestY, multitext->color, lastNode->text);
						}

						largestX = MAX(largestX, txtSizeX);
						largestY += 10;

						if (lastNode->previous == 0)
							break;

						lastNode = lastNode->previous;
					}
			}
		}

		largestY = MAX(menu->ySize, largestY);
		if (menu->fixedSize) {
			largestX = menu->xSize;
			largestY = menu->ySize;
		} else {
			menu->xSize = largestX;
			menu->ySize = largestY;
		}

		// separator title | content
		drawline2d(menu->x, menu->y+8, menu->x+largestX, menu->y+8, menu->outlineColor);

		drawline2d(menu->x, menu->y, menu->x+largestX, menu->y, menu->outlineColor);
		drawline2d(menu->x, menu->y+largestY, menu->x+largestX, menu->y+largestY, menu->outlineColor);

		drawline2d(menu->x, menu->y, menu->x, menu->y+largestY, menu->outlineColor);
		drawline2d(menu->x+largestX, menu->y, menu->x+largestX, menu->y+largestY, menu->outlineColor);
	}

	if (showCursor)
		drawCursor();
}
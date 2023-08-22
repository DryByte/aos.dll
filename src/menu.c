#include <menu.h>
#include <stdio.h>
#include <rendering.h>
#include <windows.h>
#include <window.h>
#include <voxlap.h>

#define MAX(x,y) (x>y) ? x : y

struct Menu* menus[MAX_MENU_ENTRIES];

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
	menu->xSize = 1;
	menu->ySize = 1;
	menu->fixedSize = 0;
	menu->hidden = 1;
	strncpy(menu->title, title, 32);

	menus[menuId] = menu;
	return menu;
}

void showAllMenus() {
	aosToggleCursor();
	int menusLen = getNextAvailableMenuId();

	for (int menuId = 0; menuId < menusLen; menuId++) {
		struct Menu* menu = (struct Menu*)menus[menuId];
		menu->hidden = 0;
	}
}

void hideAllMenus() {
	aosToggleCursor();
	int menusLen = getNextAvailableMenuId();

	for (int menuId = 0; menuId < menusLen; menuId++) {
		struct Menu* menu = (struct Menu*)menus[menuId];
		menu->hidden = 1;
	}
}

void renderMenuText(struct Menu* menu, struct ItemText* item, int y) {
	drawCustomFontText(menu->x, menu->y+y, item->color, item->fontId, item->text);
}

void drawMenus() {
	int menusLen = getNextAvailableMenuId();
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
					for (int i=0; i<5; i++) {
						drawText(menu->x, menu->y+largestY, multitext->color, lastNode->text);
						largestX = MAX(largestX, strlen(lastNode->text)*8);
						largestY += 8;

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
}
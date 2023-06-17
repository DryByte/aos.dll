#include <menu.h>
#include <stdio.h>
#include <rendering.h>

#define MAX(x,y) (x>y) ? x : y

struct Menu* menus[MAX_MENU_ENTRIES];

int getNextAvailableMenuId() {
	int i = 0;
	for (; i <= MAX_MENU_ENTRIES; i++) {
		if (*(menus+i*4) == NULL)
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

	struct ItemText* txtItem = malloc(sizeof(txtItem)+32);
	txtItem->type = 0;
	txtItem->id = id;
	txtItem->color = color;
	txtItem->fontId = fontid;
	strcpy(txtItem->text, text);

	menu->items[id] = txtItem;
	menus[menu->id] = menu;
}

struct Menu* createMenu(int x, int y, int outline) {
	int menuId = getNextAvailableMenuId();
	struct Menu* menu = malloc(sizeof(struct Menu));	

	menu->id = menuId;
	menu->outlineColor = outline;
	menu->x = x;
	menu->y = y;

	menus[menuId] = menu;
	return menu;
}

void renderMenuText(struct Menu* menu, struct ItemText* item, int y) {
	drawCustomFontText(menu->x, menu->y+y, item->color, item->fontId, item->text);
}

void drawMenus() {
	int menusLen = getNextAvailableMenuId();
	for (int menuId = 0; menuId < menusLen; menuId++) {
		struct Menu* menu = (struct Menu*)menus[menuId];
		int itemsLen = getNextAvailableItemId(menu);

		int largestX = 0;
		int largestY = 0;

		for (int itemId = 0; itemId < itemsLen; itemId++) {
			void* item = menu->items[itemId];
			int itemType = ((struct Item*)item)->type;

			switch(itemType) {
				case TEXT_ITEM:
					struct ItemText* txtItem = (struct ItemText*)item;
					int txtSizeX = getCustomFontSize(txtItem->fontId, txtItem->text);
					renderMenuText(menu, item, largestY);

					largestX = MAX(largestX, txtSizeX);
					largestY += MAX(largestY, ((txtItem->fontId+1)*8)+8);
					break;
				case BUTTON_ITEM:
					printf("nop\n");
					break;
			}
		}

		drawLine(menu->x, menu->y, menu->x+largestX, menu->y, menu->outlineColor);
		drawLine(menu->x, menu->y+largestY, menu->x+largestX, menu->y+largestY, menu->outlineColor);

		drawLine(menu->x, menu->y, menu->x, menu->y+largestY, menu->outlineColor);
		drawLine(menu->x+largestX, menu->y, menu->x+largestX, menu->y+largestY, menu->outlineColor);
	}
}
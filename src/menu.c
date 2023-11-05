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

// only one can be active per time
struct ItemTextInput* activeInputItem = 0;

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

void minimizeButtonHandler(struct Menu* menu, struct ItemClickableButton* btn) {
	if (!btn->isClicking)
		return;

	menu->minimized = !menu->minimized;
	if (menu->minimized) {
		strncpy(btn->text, "+", 32);
	} else {
		strncpy(btn->text, "-", 32);
	}
}

void pinButtonHandler(struct Menu* menu, struct ItemClickableButton* btn) {
	if (!btn->isClicking)
		return;

	menu->pin = !menu->pin;
	if (menu->pin) {
		strncpy(btn->text, "X", 32);
	} else {
		strncpy(btn->text, " ", 32);
	}
}

struct ItemText* createText(struct Menu* menu, int fontid, int color, char* text) {
	int id = getNextAvailableItemId(menu);

	struct ItemText* txtItem = malloc(sizeof(struct ItemText)+32);
	txtItem->type = 0;
	txtItem->id = id;
	txtItem->color = color;
	txtItem->fontId = fontid;
	txtItem->xPos = 0;
	txtItem->yPos = 0;
	strncpy(txtItem->text, text, 32);

	menu->items[id] = txtItem;
	menus[menu->id] = menu;

	return txtItem;
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

struct ItemClickableButton* createClickableButton(struct Menu* menu, char* text, void (*func)()) {
	int id = getNextAvailableItemId(menu);

	struct ItemClickableButton* btn = malloc(sizeof(struct ItemClickableButton) + 32);
	btn->type = 1;
	btn->id = id;
	btn->color = 0xffff0000;
	btn->holdColor = 0xff00ff00;
	btn->xSize = 60;
	btn->ySize = 20;
	btn->xPos = 0;
	btn->yPos = 0;
	btn->isToolbar = 0;
	btn->interval = 0;
	btn->lastInteraction = 0;
	btn->event = func;
	strncpy(btn->text, text, 32);

	menu->items[id] = btn;
	menus[menu->id] = menu;

	return btn;
}

struct ItemTextInput* createTextInput(struct Menu* menu, int xSize, int ySize, long backgroundColor, char* placeholder) {
	int id = getNextAvailableItemId(menu);

	struct ItemTextInput* input = malloc(sizeof(struct ItemTextInput) + 256);
	input->type = 3;
	input->id = id;
	input->xSize = xSize;
	input->ySize = ySize;
	input->xPos = 0;
	input->yPos = 0;
	input->backgroundColor = backgroundColor;
	strncpy(input->placeholder, placeholder, 128);

	menu->items[id] = input;
	menus[menu->id] = menu;

	return input;
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
	menu->pin = 0;
	menu->minimized = 0;
	strncpy(menu->title, title, 32);

	menus[menuId] = menu;

	struct ItemClickableButton* minbtn = createClickableButton(menu, "-", &minimizeButtonHandler);
	minbtn->xSize = 8;
	minbtn->ySize = 8;
	minbtn->xPos = -10;
	minbtn->yPos = 1;
	minbtn->isToolbar = 1;
	minbtn->interval = 1;

	struct ItemClickableButton* pinbtn = createClickableButton(menu, " ", &pinButtonHandler);
	pinbtn->xSize = 8;
	pinbtn->ySize = 8;
	pinbtn->xPos = -20;
	pinbtn->yPos = 1;
	pinbtn->isToolbar = 1;
	pinbtn->interval = 1;

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

void handleKeyboard() {
	if (!activeInputItem)
		return;

	long key = keyread()&255;
	if(!key)
		return;

	int currentLen = strlen(activeInputItem->input);
	if (currentLen > 127)
		return;

	if(key == 8) {
		activeInputItem->input[currentLen-1] = 0;
		return;
	} else if (key == 13) {
		activeInputItem = 0;
		return;
	}

	activeInputItem->input[currentLen] = key;

}

int checkCursorOver(int areaX1, int areaY1, int areaX2, int areaY2) {
	return (mouseXPos >= areaX1 && mouseXPos <= areaX2 && mouseYPos >= areaY1 && mouseYPos <= areaY2);
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
		if (menu->hidden && !menu->pin)
			continue;

		int itemsLen = getNextAvailableItemId(menu);

		int largestX = MAX(menu->xSize, strlen(menu->title)*8);
		int largestY = 8;

		// title background
		long color[] = {0xe0000000};
		drawtile(color, 1, 1, 1, 0x0, 0x0, menu->x, menu->y, menu->xSize, largestY, -1);
		drawText(menu->x, menu->y, 0xffffff, menu->title);

		if (!menu->minimized) {
			// content background
			color[0] = 0xc0000000;
			drawtile(color, 1, 1, 1, 0x0, 0x0, menu->x, menu->y+8, menu->xSize, menu->ySize-8, -1);
		}

		for (int itemId = 0; itemId < itemsLen; itemId++) {
			void* item = menu->items[itemId];
			int itemType = ((struct Item*)item)->type;

			if (menu->minimized && itemType != CLICKABLE_BUTTON_ITEM)
				continue;

			switch(itemType) {
				case TEXT_ITEM:
					struct ItemText* txtItem = (struct ItemText*)item;
					int txtSizeX = getCustomFontSize(txtItem->fontId, txtItem->text);
					int txtSizeY = (txtItem->fontId+1)*8;
					if (menu->fixedSize && txtSizeX > menu->xSize) {
						int singleCharSize = txtSizeX/strlen(txtItem->text);
						for (int i = txtSizeX; i > menu->xSize; i--) {
							txtItem->text[i/singleCharSize] = '\0';
						}
					}

					int xpos = menu->x+txtItem->xPos;
					int ypos = menu->y;

					if (txtItem->xPos < 0) {
						xpos += menu->xSize;
					}

					if (txtItem->yPos >= 0) {
						ypos += (!txtItem->yPos) ? largestY : txtItem->yPos;
					} else {
						ypos += menu->ySize+txtItem->yPos;
					}

					drawCustomFontText(xpos, ypos, txtItem->color, txtItem->fontId, txtItem->text);
					if (txtItem->xPos >= 0)
						largestX = MAX(largestX, txtSizeX+txtItem->xPos);
					if(txtItem->yPos >= 0)
						largestY = MAX(largestY, txtSizeY+8+ypos-menu->y);
					break;
				case CLICKABLE_BUTTON_ITEM:
					struct ItemClickableButton* clickBtn = (struct ItemClickableButton*)item;

					if (menu->minimized && !clickBtn->isToolbar)
						break;

					int clickBtnXpos = menu->x+clickBtn->xPos;
					int clickBtnYpos = menu->y;

					if (clickBtn->xPos < 0) {
						clickBtnXpos += menu->xSize;
					}

					if (clickBtn->yPos >= 0) {
						clickBtnYpos += (!clickBtn->yPos) ? largestY : clickBtn->yPos;
					} else {
						clickBtnYpos += menu->ySize+clickBtn->yPos;
					}

					if (interaction && checkCursorOver(clickBtnXpos, clickBtnYpos,
													   clickBtnXpos+clickBtn->xSize,
													   clickBtnYpos+clickBtn->ySize))
					{
						int interval = 0;

						if (clickBtn->interval > 0) {
							if (time(NULL)-clickBtn->lastInteraction < clickBtn->interval) {
								interval = 1;
							} else {
								clickBtn->lastInteraction = time(NULL);
							}
						}

						if (!interval) {
							color[0] = clickBtn->holdColor;
							clickBtn->isClicking = 1;
							clickBtn->event(menu, clickBtn);
						}
					} else {
						// i prefer this than setting value two times
						// probably later i cna find something better
						int clicking = clickBtn->isClicking;
						clickBtn->isClicking = 0;
						if (clicking) {
							clickBtn->event(menu, clickBtn);
						}

						color[0] = clickBtn->color;
					}

					drawtile(color, 1, 1, 1, 0x0, 0x0, clickBtnXpos, clickBtnYpos, clickBtn->xSize, clickBtn->ySize, -1);

					int textlen = strlen(clickBtn->text);
					drawText(clickBtnXpos+clickBtn->xSize/2-textlen*3, clickBtnYpos+clickBtn->ySize/2-4, 0x0, clickBtn->text);

					drawSquare(clickBtnXpos, clickBtnYpos, clickBtnXpos+clickBtn->xSize, clickBtnYpos+clickBtn->ySize, 0x0);

					if (clickBtn->xPos >= 0)
						largestX = MAX(largestX, clickBtn->xSize+clickBtn->xPos);

					if (clickBtn->yPos >= 0)
						largestY = MAX(largestY, clickBtn->ySize+2+clickBtnYpos-menu->y);
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
					break;
				case TEXTINPUT_ITEM:
					struct ItemTextInput* input = (struct ItemTextInput*)item;

					int inpXPos = input->xPos+menu->x;
					int inpYPos = menu->y;

					if (input->xPos < 0) {
						inpXPos += menu->xSize;
					}

					if (input->yPos >= 0) {
						inpYPos += (!input->yPos) ? largestY : input->yPos;
					} else {
						inpYPos += input->yPos+menu->ySize;
					}

					color[0] = input->backgroundColor;
					drawtile(color, 1, 1, 1, 0x0, 0x0, inpXPos, inpYPos, input->xSize, input->ySize, -1);
					int displayLen = input->xSize/6;
					char textDisplay[displayLen];

					if (interaction && checkCursorOver(inpXPos, inpYPos,
													   inpXPos+input->xSize,
													   inpYPos+input->ySize))
					{
						activeInputItem = input;
					}

					if (activeInputItem && activeInputItem->id == input->id) {
						int inputlen = strlen(input->input);

						if (inputlen < displayLen) {
							snprintf(textDisplay, displayLen, "%s_", input->input);
						} else {
							snprintf(textDisplay, displayLen, "%s_", &input->input[inputlen-displayLen]);
						}	
					} else {
						if (input->input[0]) {
							strncpy(textDisplay, input->input, displayLen);
						} else {
							strncpy(textDisplay, input->placeholder, displayLen);
						}
					}

					drawText(inpXPos, inpYPos+input->ySize/2-4, 0x505050, textDisplay);

					if (input->xPos >= 0)
						largestX = MAX(largestX, input->xPos+input->xSize);
					if (input->yPos >= 0)
						largestY = MAX(largestY, input->ySize+2+inpYPos-menu->y);
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

		if (!menu->minimized)
			drawSquare(menu->x, menu->y, menu->x+largestX, menu->y+largestY, menu->outlineColor);

		if (interaction && checkCursorOver(menu->x, menu->y, menu->x+largestX, menu->y+8)) {
			int xoffset, yoffset, bst;
			getmousechange(&xoffset, &yoffset, &bst);
			menu->x += xoffset;
			menu->y += yoffset;
		}
	}

	if (showCursor)
		drawCursor();
}
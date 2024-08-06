#include <Menu.h>
#include <stdio.h>
#include <Rendering.h>
#include <windows.h>
#include <Voxlap.h>
#include <AosConfig.h>

#define MAX(x,y) (x>y) ? x : y
#define MIN(x,y) (x<y) ? x : y

struct Menu* menus[MAX_MENU_ENTRIES];

int mouse_x_pos = 0;
int mouse_y_pos = 0;
int show_cursor = 0;

struct Menu* draggingMenu = 0;

// only one can be active per time
struct ItemTextInput* activeInputItem = 0;

int get_next_available_menu_id() {
	int i = 0;
	for (; i <= MAX_MENU_ENTRIES; i++) {
		if (menus[i] == NULL)
			break;
	}

	return i;
}

int get_next_available_item_id(struct Menu* menu) {
	int i = 0;
	for (; i <= MAX_MENU_ITEMS; i++) {
		if (menu->items[i] == NULL)
			break;
	}

	return i;
}

void minimize_button_handler(struct Menu* menu, struct ItemClickableButton* btn) {
	if (!btn->is_clicking)
		return;

	menu->minimized = !menu->minimized;
	if (menu->minimized) {
		strncpy(btn->text, "+", 32);
	} else {
		strncpy(btn->text, "-", 32);
	}
}

void pin_button_handler(struct Menu* menu, struct ItemClickableButton* btn) {
	if (!btn->is_clicking)
		return;

	menu->pin = !menu->pin;
	if (menu->pin) {
		strncpy(btn->text, "X", 32);
	} else {
		strncpy(btn->text, " ", 32);
	}
}

struct ItemText* create_text(struct Menu* menu, int font_id, int color, char* text) {
	int id = get_next_available_item_id(menu);

	struct ItemText* txtItem = calloc(sizeof(struct ItemText)+32, 1);
	txtItem->type = TEXT_ITEM;
	txtItem->id = id;
	txtItem->color = color;
	txtItem->font_id = font_id;
	txtItem->x_pos = 0;
	txtItem->y_pos = 0;
	strncpy(txtItem->text, text, 32);

	menu->items[id] = txtItem;
	menus[menu->id] = menu;

	return txtItem;
}

void add_new_text(struct ItemMultitext* multitext, char* text) {
	int strLen = strlen(text);
	struct MultitextNode* node = calloc(sizeof(struct MultitextNode) + (strLen < 128 ? strLen : 128), 1);

	if (multitext->current_pos != 0)
		multitext->current_pos += 1;

	node->next = 0;
	strncpy(node->text, text, 128);

	if (multitext->last_node == 0) {
		node->previous = 0;

		multitext->first_node = node;
		multitext->last_node = node;
	} else {
		node->previous = multitext->last_node;

		multitext->last_node->next = node;
		multitext->last_node = node;
	}
}

struct ItemMultitext* create_multitext(struct Menu* menu, int color) {
	int id = get_next_available_item_id(menu);

	struct ItemMultitext* multitextItem = calloc(sizeof(struct ItemMultitext), 1);
	multitextItem->type = MULTITEXT_ITEM;
	multitextItem->id = id;
	multitextItem->color = color;
	multitextItem->x_size = 0;
	multitextItem->y_size = 0;
	multitextItem->x_pos = 0;
	multitextItem->y_pos = 0;
	multitextItem->current_pos = 0;
	multitextItem->selected = 0;
	multitextItem->first_node = 0;
	multitextItem->last_node = 0;

	menu->items[id] = multitextItem;
	menus[menu->id] = menu;

	return multitextItem;
}

struct ItemClickableButton* create_clickable_button(struct Menu* menu, char* text, void (*func)()) {
	int id = get_next_available_item_id(menu);

	struct ItemClickableButton* btn = calloc(sizeof(struct ItemClickableButton) + 32, 1);
	btn->type = CLICKABLE_BUTTON_ITEM;
	btn->id = id;
	btn->color = 0xffff0000;
	btn->hold_color = 0xff00ff00;
	btn->x_size = 60;
	btn->y_size = 20;
	btn->x_pos = 0;
	btn->y_pos = 0;
	btn->is_toolbar = 0;
	btn->interval = 0;
	btn->last_interaction = 0;
	btn->event = func;
	strncpy(btn->text, text, 32);

	menu->items[id] = btn;
	menus[menu->id] = menu;

	return btn;
}

struct ItemTextInput* create_text_input(struct Menu* menu, int x_size, int y_size, long background_color, char* placeholder) {
	int id = get_next_available_item_id(menu);

	struct ItemTextInput* input = calloc(sizeof(struct ItemTextInput) + 256, 1);
	input->type = TEXTINPUT_ITEM;
	input->id = id;
	input->x_size = x_size;
	input->y_size = y_size;
	input->x_pos = 0;
	input->y_pos = 0;
	input->background_color = background_color;
	input->max_length = 128;
	strncpy(input->placeholder, placeholder, 128);

	menu->items[id] = input;
	menus[menu->id] = menu;

	return input;
}

struct ItemSlide* create_slide(struct Menu* menu, int min_value, int max_value, int* interact) {
	int id = get_next_available_item_id(menu);

	struct ItemSlide* slide = calloc(sizeof(struct ItemSlide), 1);
	slide->type = SLIDE_ITEM;
	slide->id = id;
	slide->x_size = 90;
	slide->y_size = 30;
	slide->x_pos = 0;
	slide->y_pos = 0;
	slide->slider_size = 10;
	slide->slider_color = 0xff858585;
	slide->background_color = 0x00;
	slide->max_value = max_value;
	slide->min_value = min_value;
	slide->show_status = 0;
	slide->interact_int = interact;

	menu->items[id] = slide;
	menus[menu->id] = menu;

	return slide;
}

struct ItemSwitchButton* create_switch_button(struct Menu* menu, char* label, char* config_entry, void (*func)(), int* enabled) {
	int id = get_next_available_item_id(menu);

	struct ItemSwitchButton* btn = calloc(sizeof(struct ItemSwitchButton) + 32, 1);
	btn->type = SWITCH_BUTTON_ITEM;
	btn->id = id;
	btn->enabled = enabled;
	// colors in ARGB!!!
	btn->enabled_color = 0xff00ff00;
	btn->disabled_color = 0xffff0000;
	btn->hold_color = 0xfff1a42e;
	btn->color = 0xff757575;
	btn->x_size = 40;
	btn->y_size = 15;
	btn->x_pos = 0;
	btn->y_pos = 0;
	btn->interval = 0;
	btn->last_interaction = 0;
	btn->switch_event = func;
	strncpy(btn->label, label, 32);
	strncpy(btn->config_entry, config_entry, 32);

	menu->items[id] = btn;
	menus[menu->id] = menu;

	return btn;
}

struct Menu* create_menu(int x, int y, int outline, char* title) {
	int menu_id = get_next_available_menu_id();
	struct Menu* menu = calloc(sizeof(struct Menu)+32, 1);	

	menu->id = menu_id;
	menu->outline_color = outline;
	menu->x_pos = x;
	menu->y_pos = y;
	menu->x_size = 150;
	menu->y_size = 100;
	menu->fixed_size = 1;
	menu->hidden = 1;
	menu->pin = 0;
	menu->minimized = 0;
	menu->always_hidden = 0;

	menu->buffer_x = menu->x_size; // use this to detect size changes for realloc
	menu->buffer_y = menu->y_size;
	menu->buffer = calloc(menu->x_size*menu->y_size, 4);

	strncpy(menu->title, title, 32);

	menus[menu_id] = menu;

	struct ItemClickableButton* minbtn = create_clickable_button(menu, "-", &minimize_button_handler);
	minbtn->x_size = 8;
	minbtn->y_size = 8;
	minbtn->x_pos = -10;
	minbtn->y_pos = 1;
	minbtn->is_toolbar = 1;
	minbtn->interval = 1;

	struct ItemClickableButton* pinbtn = create_clickable_button(menu, " ", &pin_button_handler);
	pinbtn->x_size = 8;
	pinbtn->y_size = 8;
	pinbtn->x_pos = -20;
	pinbtn->y_pos = 1;
	pinbtn->is_toolbar = 1;
	pinbtn->interval = 1;

	return menu;
}

void show_all_menus() {
	show_cursor = 1;
	int menusLen = get_next_available_menu_id();

	for (int menu_id = 0; menu_id < menusLen; menu_id++) {
		struct Menu* menu = (struct Menu*)menus[menu_id];
		if (menu->always_hidden)
			continue;

		menu->hidden = 0;
	}
}

void hide_all_menus() {
	show_cursor = 0;
	int menusLen = get_next_available_menu_id();

	for (int menu_id = 0; menu_id < menusLen; menu_id++) {
		struct Menu* menu = (struct Menu*)menus[menu_id];
		menu->hidden = 1;
	}
}


// should we move this to voxlap?
int handle_wheel() {
	return *(int*)(client_base+0x12b1b58);
}

//returns 1 if an interaction happened
int handle_cursor() {
	int mx;
	int my;
	int status;

	struct WindowSize winsize = get_config_window_size();
	getmousechange(&mx, &my, &status);

	mouse_x_pos += mx;
	mouse_y_pos += my;

	if (mouse_x_pos < 0) {
		mouse_x_pos = 0;
	} else {
		mouse_x_pos = MIN(mouse_x_pos, winsize.width);
	}

	if (mouse_y_pos < 0) {
		mouse_y_pos = 0;
	} else {
		mouse_y_pos = MIN(mouse_y_pos, winsize.height);
	}

	return status;
}

void handle_keyboard() {
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

	if (currentLen >= activeInputItem->max_length)
		return;

	activeInputItem->input[currentLen] = key;

}

int check_cursor_over(int areaX1, int areaY1, int areaX2, int areaY2) {
	return (mouse_x_pos >= areaX1 && mouse_x_pos <= areaX2 && mouse_y_pos >= areaY1 && mouse_y_pos <= areaY2);
}

void draw_cursor() {
	// todo load mouse cursor image
	long color[] = {0xffffffff};
	drawtile((long)color, 1, 1, 1, 0x0, 0x0, mouse_x_pos, mouse_y_pos, 15, 15, -1);
}

int word_length(int wordpos, int maxWordlen, char* buffer) {
	int len = 0;

	for (; len < maxWordlen; len++) {
		if (buffer[wordpos+len] == ' ' || buffer[wordpos+len] == '\0')
			break;
	}

	return len;
}

int its_new_line_time(int position, int currentLinePos, int maxLine, char* buffer) {
	int newline = 0;

	for (int i = currentLinePos; i < maxLine; i++) {
		if (buffer[position] == ' ' || buffer[position] == '\0')
			break;

		position++;
		if (i+1 >= maxLine) {
			newline = 1;
			break;
		}
	}

	return newline;
}

int testbuf[10*20] = {
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff, 0xffff00ff, 0xffff00ff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		};

void draw_to_buffer(struct Menu* menu, int* copy_buff, int offset_x, int offset_y, int size_x, int size_y) {
	if (menu->x_size != menu->buffer_x || menu->y_size != menu->buffer_y) {
		free(menu->buffer);
		menu->buffer_x = menu->x_size;
		menu->buffer_y = menu->y_size;
		menu->buffer = calloc(menu->buffer_x*menu->buffer_y, 4);
	}

	int true_max_size_x = MIN(size_x, menu->buffer_x);
	int true_max_size_y = MIN(size_y, menu->buffer_y);

	for (int x = offset_x; x < true_max_size_x; x++) {
		for (int y = offset_y; y < true_max_size_y; y++) {
			y+=offset_y;
			*((menu->buffer)+(x+(y*menu->buffer_x))) = *(copy_buff+(x-offset_x+(y-offset_y)*size_x));
		}
	}

	drawtile((long)menu->buffer, menu->buffer_x, menu->buffer_x, menu->buffer_y, 0, 0, 30, 30, 1, 1, -1);
}

void draw_menus() {
	int menusLen = get_next_available_menu_id();

	int interaction = 0;
	if (show_cursor)
		interaction = handle_cursor();

	for (int menu_id = 0; menu_id < menusLen; menu_id++) {
		struct Menu* menu = (struct Menu*)menus[menu_id];
		if (menu->hidden && !menu->pin)
			continue;

		int itemsLen = get_next_available_item_id(menu);


		
		if (menu_id == 1) {
			draw_to_buffer(menu, testbuf, 0, 0, 10, 20);
			draw_to_buffer(menu, *(int**)(client_base+0x51758), 10, 0, 800, 600);
		}

		int largestX = MAX(menu->x_size, (int)strlen(menu->title)*8);
		int largestY = 8;

		// title background
		long color[] = {0xe0000000};
		long secondary_color[] = {0x00000000};
		drawtile((long)color, 1, 1, 1, 0x0, 0x0, menu->x_pos, menu->y_pos, menu->x_size, largestY, -1);
		draw_text(menu->x_pos, menu->y_pos, 0xffffff, menu->title);

		if (!menu->minimized) {
			// content background
			color[0] = 0xc0000000;
			drawtile((long)color, 1, 1, 1, 0x0, 0x0, menu->x_pos, menu->y_pos+8, menu->x_size, menu->y_size-8, -1);
		}

		for (int itemId = 0; itemId < itemsLen; itemId++) {
			void* item = menu->items[itemId];
			int itemType = ((struct Item*)item)->type;

			if (menu->minimized && itemType != CLICKABLE_BUTTON_ITEM)
				continue;

			switch(itemType) {
				case TEXT_ITEM:
					struct ItemText* txtItem = (struct ItemText*)item;

					int txtSizeX = 6;
					int txtSizeY = 8;

					if (txtItem->font_id >= 0) {
						txtSizeX = get_custom_font_size(txtItem->font_id, txtItem->text);
						txtSizeY = (txtItem->font_id+1)*8;
						if (menu->fixed_size && txtSizeX > menu->x_size) {
							int singleCharSize = txtSizeX/strlen(txtItem->text);
							for (int i = txtSizeX; i > menu->x_size; i--) {
								txtItem->text[i/singleCharSize] = '\0';
							}
						}
					}

					int x_pos = menu->x_pos+txtItem->x_pos;
					int ypos = menu->y_pos;

					if (txtItem->x_pos < 0) {
						x_pos += menu->x_size;
					}

					if (txtItem->y_pos >= 0) {
						ypos += (!txtItem->y_pos) ? largestY : txtItem->y_pos;
					} else {
						ypos += menu->y_size+txtItem->y_pos;
					}

					if (txtItem->font_id >= 0) {
						draw_custom_font_text(x_pos, ypos, txtItem->color, txtItem->font_id, txtItem->text);
					} else {
						draw_text(x_pos, ypos, txtItem->color, txtItem->text);
					}
					if (txtItem->x_pos >= 0)
						largestX = MAX(largestX, txtSizeX+txtItem->x_pos);
					if(txtItem->y_pos >= 0)
						largestY = MAX(largestY, txtSizeY+8+ypos-menu->y_pos);
					break;
				case CLICKABLE_BUTTON_ITEM:
					struct ItemClickableButton* click_btn = (struct ItemClickableButton*)item;

					if (menu->minimized && !click_btn->is_toolbar)
						break;

					int clickBtnXpos = menu->x_pos+click_btn->x_pos;
					int clickBtnYpos = menu->y_pos;

					if (click_btn->x_pos < 0) {
						clickBtnXpos += menu->x_size;
					}

					if (click_btn->y_pos >= 0) {
						clickBtnYpos += (!click_btn->y_pos) ? largestY : click_btn->y_pos;
					} else {
						clickBtnYpos += menu->y_size+click_btn->y_pos;
					}

					if (interaction && check_cursor_over(clickBtnXpos, clickBtnYpos,
													   clickBtnXpos+click_btn->x_size,
													   clickBtnYpos+click_btn->y_size))
					{
						int interval = 0;

						if (click_btn->interval > 0) {
							if (time(NULL)-click_btn->last_interaction < click_btn->interval) {
								interval = 1;
							} else {
								click_btn->last_interaction = time(NULL);
							}
						}

						if (!interval) {
							color[0] = click_btn->hold_color;
							click_btn->is_clicking = 1;
							click_btn->event(menu, click_btn);
						}
					} else {
						// i prefer this than setting value two times
						// probably later i cna find something better
						int clicking = click_btn->is_clicking;
						click_btn->is_clicking = 0;
						if (clicking) {
							click_btn->event(menu, click_btn);
						}

						color[0] = click_btn->color;
					}

					drawtile((long)color, 1, 1, 1, 0x0, 0x0, clickBtnXpos, clickBtnYpos, click_btn->x_size, click_btn->y_size, -1);

					int textlen = strlen(click_btn->text);
					draw_text(clickBtnXpos+click_btn->x_size/2-textlen*3, clickBtnYpos+click_btn->y_size/2-4, 0x0, click_btn->text);

					draw_square(clickBtnXpos, clickBtnYpos, clickBtnXpos+click_btn->x_size, clickBtnYpos+click_btn->y_size, 0x0);

					if (click_btn->x_pos >= 0)
						largestX = MAX(largestX, click_btn->x_size+click_btn->x_pos);

					if (click_btn->y_pos >= 0)
						largestY = MAX(largestY, click_btn->y_size+2+clickBtnYpos-menu->y_pos);
					break;
				case MULTITEXT_ITEM:
					struct ItemMultitext* multitext = (struct ItemMultitext*)item;
					struct MultitextNode* lastNode = multitext->last_node;

					int mtxPos = multitext->x_pos+menu->x_pos;
					int mtyPos = multitext->y_pos+menu->y_pos+10; // this +10 is bc menu bar

					if (mtyPos == 0)
						mtyPos += largestY;

					int mtxSize = multitext->x_size;
					int mtySize = multitext->y_size;

					if (mtxSize <= 0)
						mtxSize = menu->x_size;
					if (mtySize <= 0)
						mtySize = menu->y_size-10;

					if (check_cursor_over(mtxPos, mtyPos, mtxPos+mtxSize, mtyPos+mtySize)) {
						int wheel_status = handle_wheel();
						if (wheel_status > 0 && multitext->current_pos > 0) {
							multitext->current_pos -= 1;
						} else if (wheel_status < 0) {
							multitext->current_pos += 1;
						}
					}

					int availableLines = mtySize/8;
					int currentNodeId = 0; // reverse
					for (int i = 0; i < availableLines; i++) {
						if (multitext->current_pos > currentNodeId && lastNode->previous != 0) {
							lastNode = lastNode->previous;
							currentNodeId += 1;
							i = 0;
							continue;
						}

						if (!lastNode)
							continue;

						int txtSizeX = strlen(lastNode->text)*6; // draw_text uses 6x8
						int txtLines = (int)(strlen(lastNode->text)*6/(float)mtxSize+1);

						if (interaction && check_cursor_over(mtxPos, mtyPos+i*8, mtxPos+mtxSize, mtyPos+i*10+txtLines*8)) {
							multitext->selected = lastNode;
						}

						if (multitext->selected) {
							if (multitext->selected->next == lastNode->next && multitext->selected->previous == lastNode->previous) {
								color[0] = 0xff606060;
								drawtile((long)color, 1, 1, 1, 0x0, 0x0, mtxPos, mtyPos+i*8, mtxSize, txtLines*8, -1);
							}
						}

						if (txtSizeX > mtxSize) {
							char copyTxtNode[128];
							int characterInLine = 0;
							int charsPerLine = mtxSize/6;
							memset(copyTxtNode, 0, 128);

							for (int characterNode = 0; characterNode < txtSizeX/6; characterNode++) {
								if ((i+1)*8 >= mtySize)
									continue;

								if (lastNode->text[characterNode] == ' ') {
									copyTxtNode[characterInLine] = ' ';
									characterInLine += 1;

									if (characterInLine >= charsPerLine) {
										characterInLine = 0;
										i+=1;
									}

									continue;
								}

								int wordlen = word_length(characterNode, 128, lastNode->text);
								if (wordlen > charsPerLine) {
									int maxlines = (wordlen+characterInLine)/charsPerLine;
									int lastlineChars = abs(wordlen-charsPerLine*maxlines);
									int copychars = charsPerLine;

									for (int w = 0; w < maxlines; w++) {
										if (characterInLine != 0) {
											copychars = charsPerLine-characterInLine;
										} else {
											copychars = charsPerLine;
										}

										strncpy(&copyTxtNode[characterInLine], lastNode->text+characterNode, copychars);
										draw_text(mtxPos, mtyPos+i*8, multitext->color, copyTxtNode);
										memset(copyTxtNode, 0, sizeof copyTxtNode);
										i+=1;

										characterNode+=copychars;
										characterInLine = 0;
									}

									wordlen = lastlineChars;

								} else if(its_new_line_time(characterNode, characterInLine, charsPerLine, lastNode->text)) {
									draw_text(mtxPos, mtyPos+i*8, multitext->color, copyTxtNode);
									memset(copyTxtNode, 0, sizeof copyTxtNode);
									characterInLine = 0;
									i+=1;
								}

								strncpy(&copyTxtNode[characterInLine], lastNode->text+characterNode, wordlen);
								characterInLine+=wordlen;
								characterNode+=wordlen-1;
							}

							if ((i+1)*8 < mtySize)
								draw_text(mtxPos, mtyPos+i*8, multitext->color, copyTxtNode);
						} else {
							draw_text(mtxPos, mtyPos+i*8, multitext->color, lastNode->text);
						}

						largestX = MAX(largestX, mtxSize);
						largestY += mtySize;

						if (lastNode->previous == 0)
							break;

						lastNode = lastNode->previous;
					}
					break;
				case TEXTINPUT_ITEM:
					{
						struct ItemTextInput* input = (struct ItemTextInput*)item;

						int inpXPos = input->x_pos+menu->x_pos;
						int inpYPos = menu->y_pos;

						if (input->x_pos < 0) {
							inpXPos += menu->x_size;
						}

						if (input->y_pos >= 0) {
							inpYPos += (!input->y_pos) ? largestY : input->y_pos;
						} else {
							inpYPos += input->y_pos+menu->y_size;
						}

						color[0] = input->background_color;
						drawtile((long)color, 1, 1, 1, 0x0, 0x0, inpXPos, inpYPos, input->x_size, input->y_size, -1);
						int displayLen = input->x_size/6;
						char textDisplay[displayLen];

						if (interaction && check_cursor_over(inpXPos, inpYPos,
														   inpXPos+input->x_size,
														   inpYPos+input->y_size))
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

						draw_text(inpXPos, inpYPos+input->y_size/2-4, 0x505050, textDisplay);

						if (input->x_pos >= 0)
							largestX = MAX(largestX, input->x_pos+input->x_size);
						if (input->y_pos >= 0)
							largestY = MAX(largestY, input->y_size+2+inpYPos-menu->y_pos);
					}
					break;
				case SLIDE_ITEM:
					struct ItemSlide* slide = (struct ItemSlide*)item;

					int slideXPos = slide->x_pos+menu->x_pos;
					int slideYPos = slide->y_pos+menu->y_pos;

					color[0] = slide->background_color;
					drawtile((long)color, 1, 1, 1, 0x0, 0x0, slideXPos, slideYPos, slide->x_size, slide->y_size, -1);

					int sliderPosX = slideXPos;
					int sliderPosY = slideYPos;
					int sliderSizeX = slide->x_size;
					int sliderSizeY = slide->y_size;

					int slideToX = 0;
					int slideToY = 0;
					int _fds = 0;
					if (interaction && check_cursor_over(slideXPos, slideYPos, slideXPos+slide->x_size, slideYPos+slide->y_size)) {
						int porc = 1;
						int value = 0;
						if (slide->x_size >= slide->y_size) {
							int offset = mouse_x_pos-slideXPos;
							porc = offset*100/slide->x_size;
						} else {
							int offset = mouse_y_pos-slideYPos;
							porc = 100-offset*100/slide->y_size;
						}

						value += porc*(slide->max_value-slide->min_value)/100+slide->min_value;
						*slide->interact_int = value;
						getmousechange(&slideToX, &slideToY, &_fds);
					}

					int pos = (*slide->interact_int-slide->min_value)*100.0/(slide->max_value-slide->min_value);
					if (slide->x_size >= slide->y_size) {
						sliderSizeX = slide->slider_size;

						if (slideToX > 0 && *slide->interact_int < slide->max_value)
							*slide->interact_int+=1;
						else if(slideToX < 0 && *slide->interact_int > slide->min_value)
							*slide->interact_int-=1;

						sliderPosX += (int)((slide->x_size-sliderSizeX)*pos/100.0);
						sliderPosY = slideYPos;
					} else {
						sliderSizeY = slide->slider_size;

						if (slideToY < 0 && *slide->interact_int < slide->max_value)
							*slide->interact_int+=1;
						else if(slideToY > 0 && *slide->interact_int > slide->min_value)
							*slide->interact_int-=1;

						sliderPosY -= (int)((slide->y_size-sliderSizeY)*pos/100.0);
						sliderPosY += slide->y_size-sliderSizeY;

						sliderPosX = slideXPos;
					}

					color[0] = slide->slider_color;
					drawtile((long)color, 1, 1, 1, 0x0, 0x0, sliderPosX, sliderPosY, sliderSizeX, sliderSizeY, -1);

					if (slide->show_status) {
						char cool[10];
						sprintf(cool, "%i", *slide->interact_int);
						draw_text(slideXPos, slideYPos+slide->y_size+6, 0xff0000, cool);
					}
					draw_square(slideXPos, slideYPos, slideXPos+slide->x_size, slideYPos+slide->y_size, 0x0);

					break;
					//todo
				case SWITCH_BUTTON_ITEM:
					struct ItemSwitchButton* switch_btn = (struct ItemSwitchButton*)item;

					if (menu->minimized)
						break;

					int switchBtnXpos = menu->x_pos + switch_btn->x_pos;
					int switchBtnYpos = menu->y_pos + switch_btn->y_pos;

					if (switch_btn->x_pos < 0) {
						switchBtnXpos += menu->x_size;
					}

					if (switch_btn->y_pos >= 0) {
						switchBtnYpos += (!switch_btn->y_pos) ? largestY : switch_btn->y_pos;
					} else {
						switchBtnYpos += menu->y_size+switch_btn->y_pos;
					}

					color[0] = switch_btn->color;

					if (interaction && check_cursor_over(switchBtnXpos, switchBtnYpos,
													   switchBtnXpos+switch_btn->x_size,
													   switchBtnYpos+switch_btn->y_size))
					{
						secondary_color[0] = switch_btn->hold_color;
						if (!switch_btn->is_holding) {
							switch_btn->switch_event(menu, switch_btn);
							switch_btn->is_holding = 1;
						}
					}
					else {
						switch_btn->is_holding = 0;
						if (*(switch_btn->enabled)) {
							secondary_color[0] = switch_btn->enabled_color;
						}
						else {
							secondary_color[0] = switch_btn->disabled_color;
						}
					}

					drawtile((long)color, 1, 1, 1, 0x0, 0x0, switchBtnXpos, switchBtnYpos, switch_btn->x_size, switch_btn->y_size, -1);
					if (*(switch_btn->enabled)) 
						drawtile((long)secondary_color, 1, 1, 1, 0x0, 0x0, switchBtnXpos + (switch_btn->x_size)/2, switchBtnYpos, (switch_btn->x_size)/2, switch_btn->y_size, -1);
					else 
						drawtile((long)secondary_color, 1, 1, 1, 0x0, 0x0, switchBtnXpos, switchBtnYpos, (switch_btn->x_size)/2, switch_btn->y_size, -1);
					
					// label
					int textlength = strlen(switch_btn->label);
					draw_text(switchBtnXpos - textlength*7, switchBtnYpos, 0xFFFFFF, switch_btn->label);

					draw_square(switchBtnXpos, switchBtnYpos, switchBtnXpos+switch_btn->x_size, switchBtnYpos+switch_btn->y_size, 0x0);

					if (switch_btn->x_pos >= 0)
						largestX = MAX(largestX, switch_btn->x_size+switch_btn->x_pos);

					if (switch_btn->y_pos >= 0)
						largestY = MAX(largestY, switch_btn->y_size+2+switchBtnYpos-menu->y_pos);
					break;
					//todo
			}
		}

		largestY = MAX(menu->y_size, largestY);
		if (menu->fixed_size) {
			largestX = menu->x_size;
			largestY = menu->y_size;
		} else {
			menu->x_size = largestX;
			menu->y_size = largestY;
		}

		// separator title | content
		drawline2d(menu->x_pos, menu->y_pos+8, menu->x_pos+largestX, menu->y_pos+8, menu->outline_color);

		if (!menu->minimized)
			draw_square(menu->x_pos, menu->y_pos, menu->x_pos+largestX, menu->y_pos+largestY, menu->outline_color);

		if (draggingMenu == menu && !interaction) {
			draggingMenu = 0;
		}

		if (interaction && check_cursor_over(menu->x_pos, menu->y_pos, menu->x_pos+largestX, menu->y_pos+8)) {
			draggingMenu = menu;
		}

		if (draggingMenu == menu) {
			int xoffset, yoffset, bst;
			getmousechange(&xoffset, &yoffset, &bst);
			menu->x_pos += xoffset;
			menu->y_pos += yoffset;
		}
	}

	if (show_cursor)
		draw_cursor();
}

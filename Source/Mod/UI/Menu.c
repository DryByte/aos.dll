#include <Menu.h>
#include <stdio.h>
#include <Rendering.h>
#include <windows.h>
#include <Voxlap.h>
#include <AosConfig.h>
#include <math.h>
#include <ft2build.h>
#include <freetype/ftbitmap.h>
#include FT_FREETYPE_H

#define MAX(x,y) (x>y) ? x : y
#define MIN(x,y) (x<y) ? x : y

FT_Library flibrary;
FT_Face ftface;

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

	menu->update = 1;
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

	menu->update = 1;
	menu->pin = !menu->pin;
	if (menu->pin) {
		strncpy(btn->text, "X", 32);
	} else {
		strncpy(btn->text, " ", 32);
	}
}

struct ItemText* create_text(struct Menu* menu, int font_size, int color, char* text) {
	int id = get_next_available_item_id(menu);

	struct ItemText* txtItem = calloc(sizeof(struct ItemText)+32, 1);
	txtItem->type = TEXT_ITEM;
	txtItem->id = id;
	txtItem->color = color;
	txtItem->font_size = font_size;
	txtItem->x_pos = 0;
	txtItem->y_pos = 0;
	txtItem->menu = menu;
	strncpy(txtItem->text, text, 32);

	menu->update = 1;
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
	multitext->menu->update = 1;
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
	multitextItem->menu = menu;

	menu->update = 1;
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
	btn->font_size = 14;
	btn->x_pos = 0;
	btn->y_pos = 0;
	btn->is_toolbar = 0;
	btn->interval = 0;
	btn->last_interaction = 0;
	btn->event = func;
	btn->menu = menu;
	strncpy(btn->text, text, 32);

	menu->update = 1;
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
	input->menu = menu;
	strncpy(input->placeholder, placeholder, 128);

	menu->update = 1;
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
	slide->background_color = 0xff000000;
	slide->max_value = max_value;
	slide->min_value = min_value;
	slide->show_status = 0;
	slide->interact_int = interact;
	slide->menu = menu;

	menu->update = 1;
	menu->items[id] = slide;
	menus[menu->id] = menu;

	return slide;
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
	menu->update = 1;
	menu->is_interacting = 0;

	menu->buffer_x = menu->x_size;
	menu->buffer_y = menu->y_size;
	menu->draw_buffer = calloc(menu->buffer_x*menu->buffer_y, 4); // we gonna draw the menu here

	menu->display_x = menu->x_size; // use this to detect size changes for realloc
	menu->display_y = menu->y_size;
	menu->display_buffer = calloc(menu->display_x*menu->display_y, 4); // and here we display the menu

	menu->max_y = menu->y_size;
	menu->offset_x = 0;
	menu->offset_y = 0;

	strncpy(menu->title, title, 32);

	menus[menu_id] = menu;

	struct ItemClickableButton* minbtn = create_clickable_button(menu, "-", &minimize_button_handler);
	minbtn->x_size = 8;
	minbtn->y_size = 8;
	minbtn->x_pos = -10;
	minbtn->y_pos = 1;
	minbtn->is_toolbar = 1;
	minbtn->interval = 1;
	minbtn->font_size = 6;

	struct ItemClickableButton* pinbtn = create_clickable_button(menu, " ", &pin_button_handler);
	pinbtn->x_size = 8;
	pinbtn->y_size = 8;
	pinbtn->x_pos = -20;
	pinbtn->y_pos = 1;
	pinbtn->is_toolbar = 1;
	pinbtn->interval = 1;
	pinbtn->font_size = 6;

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

void draw_to_buffer(struct Menu* menu, int* copy_buff, int offset_x, int offset_y, int size_x, int size_y) {
	if (menu->x_size != menu->buffer_x || menu->y_size > menu->buffer_y || offset_y+size_y > menu->buffer_y) {
		//free(menu->draw_buffer);
		menu->buffer_x = menu->x_size;
		menu->buffer_y = MAX(offset_y+size_y, menu->buffer_y);
		menu->buffer_y = MAX(menu->y_size, menu->buffer_y);
		menu->draw_buffer = realloc(menu->draw_buffer, menu->buffer_x*menu->buffer_y*4);
	}

	int true_max_size_x = MIN(size_x+offset_x, menu->buffer_x);
	int true_max_size_y = MIN(size_y+offset_y, menu->buffer_y);

	for (int x = offset_x; x < true_max_size_x; x++) {
		for (int y = offset_y; y < true_max_size_y; y++) {
			if (x < 0 || y < 0)
				continue;

			int target_color = *(copy_buff+(x-offset_x+(y-offset_y)*size_x));
			int source_color = *((menu->draw_buffer)+(x+(y*menu->buffer_x)));

			if ((target_color >> 24&255) == 0x0)
				continue;

			if ((target_color >> 24&255) < 0xff && source_color != 0) {
				target_color = target_color+source_color;
			}

			*((menu->draw_buffer)+(x+(y*menu->buffer_x))) = target_color;
		}
	}
}

void draw_line(struct Menu* menu, int color, int x1, int y1, int x2, int y2) {
	int dx = x2-x1;
	int dy = y2-y1;

    int* tempbuf = calloc(dx*dy, 4);

	if (x2 >= y2) {
		int D = 2*dy - dx;
		int y = 0;

		for (int x = 0; x < x2-x1; x++) {
			*(tempbuf+x+y*dx) = color;
			if (D > 0) {
				y = y+1;
				D = D-2*dx;
			}

			D = D+2*dy;
		}
	} else {
		int D = 2*dx - dy;
		int x = 0;

		for (int y = 0; y < y2-y1; y++) {
			*(tempbuf+x+y*dx) = color;
			if (D > 0) {
				x = x+1;
				D = D-2*dy;
			}

			D = D+2*dx;
		}
	}

	draw_to_buffer(menu, tempbuf, x1, y1, dx, dy);
	free(tempbuf);
}

void draw_text(struct Menu* menu, int x, int y, int width, int color, char* font, char* text) {
	int leng = strlen(text);

	int posX = x;
	int posY = y+width;
	FT_Set_Pixel_Sizes(ftface, width, width+8);

	for (int i = 0; i < leng; i++) {
		FT_Load_Glyph(ftface, FT_Get_Char_Index( ftface, text[i] ), FT_LOAD_DEFAULT);
		FT_Render_Glyph( ftface->glyph, FT_RENDER_MODE_NORMAL );

		int* temp_buff = calloc(ftface->glyph->bitmap.width*ftface->glyph->bitmap.rows, 4);

		for (int x = 0; x < ftface->glyph->bitmap.width; x++) {
			for (int y = 0; y<ftface->glyph->bitmap.rows; y++) {
				if(*(ftface->glyph->bitmap.buffer+x+(ftface->glyph->bitmap.width*y)))
					*(temp_buff+x+(ftface->glyph->bitmap.width*y)) = color;


			}
		}

		draw_to_buffer(menu, temp_buff, 0+posX, posY-ftface->glyph->bitmap_top, ftface->glyph->bitmap.width, ftface->glyph->bitmap.rows);
		free(temp_buff);
		posX += ftface->glyph->advance.x>>6;
		posY += ftface->glyph->advance.y >> 6;
	}
}

void draw_rectangle(struct Menu* menu, int color, int x1, int y1, int x2, int y2) {
	int xs = x2-x1;
	int ys = y2-y1;

	int* temp_buff = malloc(xs*ys*4);

	for (int i = 0; i < xs*ys; i++) {
		*(temp_buff+i) = color;
	}

	draw_to_buffer(menu, temp_buff, x1, y1, xs, ys);
	free(temp_buff);
}

void draw_outlines(struct Menu* menu, int color, int x1, int y1, int x2, int y2) {
	draw_line(menu, color, x1, y1, x2, y1);
	draw_line(menu, color, x1, y2, x2, y2);
	draw_line(menu, color, x1, y1, x1, y2);
	draw_line(menu, color, x2, y1, x2, y2);
}

void clean_draw_buffer(struct Menu* menu) {
	memset(menu->draw_buffer, 0, menu->buffer_x*menu->buffer_y*4);
}

void clean_display_buffer(struct Menu* menu) {
	memset(menu->display_buffer, 0, menu->display_x*menu->display_y*4);
}

void draw_menu(struct Menu* menu, int interaction) {
	clean_draw_buffer(menu);

	int itemsLen = get_next_available_item_id(menu);

	/*if (menu_id == 1) {

		if (fsrun) {
			

			fsrun = 0;

			draw_text(menu, 0, 0, 32, 0xff0000ff, "./Monocraft.otf", "hello my boi");
			draw_line(menu, 10, 5, 25, 30);
		}
		drawtile((long)menu->buffer, menu->buffer_x, menu->buffer_x, menu->buffer_y, 0, 0, 30, 30, 1, 1, -1);
	}*/

	int largestX = MAX(menu->x_size, (int)strlen(menu->title)*8);
	int largestY = menu->y_size;

	// title background
	draw_rectangle(menu, 0xe0000000, 1, 1, menu->x_size, largestY);
	draw_text(menu, 0, 0, 16, 0xffffffff, "Monocraft.otf", menu->title);

	if (!menu->minimized) {
		// content background
		draw_rectangle(menu, 0xc0000000, 0, 8, menu->x_size, menu->max_y);
	}

	printf("foudase? %i\n", itemsLen);
	for (int itemId = 0; itemId < itemsLen; itemId++) {
		void* item = menu->items[itemId];
		int itemType = ((struct Item*)item)->type;

		printf("Item type: %i\n", itemType);
		if (menu->minimized && itemType != CLICKABLE_BUTTON_ITEM)
			continue;

		printf("Item type: %i\n", itemType);

		switch(itemType) {
			case TEXT_ITEM:
				struct ItemText* txtItem = (struct ItemText*)item;

				/*int txtSizeX = 6;
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
				}*/

				int x_pos = txtItem->x_pos;
				int ypos = menu->y_pos;

				if (txtItem->x_pos < 0) {
					x_pos += menu->x_size;
				}

				if (txtItem->y_pos >= 0) {
					ypos += (!txtItem->y_pos) ? largestY : txtItem->y_pos;
				} else {
					ypos += menu->y_size+txtItem->y_pos;
				}

				draw_text(menu, txtItem->x_pos, txtItem->y_pos, txtItem->font_size, txtItem->color, "Monocraft.otf", txtItem->text);

				menu->max_y = MAX(txtItem->y_pos+txtItem->font_size, menu->max_y);
				/*if (txtItem->x_pos >= 0)
					largestX = MAX(largestX, txtSizeX+txtItem->x_pos);
				if(txtItem->y_pos >= 0)
					largestY = MAX(largestY, txtSizeY+8+ypos-menu->y_pos);*/
				break;
			case CLICKABLE_BUTTON_ITEM:
				struct ItemClickableButton* click_btn = (struct ItemClickableButton*)item;

				/*if (menu->minimized && !click_btn->is_toolbar)
					break;*/

				int clickBtnXpos = click_btn->x_pos;
				int clickBtnYpos = 0;

				if (click_btn->x_pos < 0) {
					clickBtnXpos += menu->x_size;
				}

				if (click_btn->y_pos >= 0) {
					clickBtnYpos += (!click_btn->y_pos) ? largestY : click_btn->y_pos;
				} else {
					clickBtnYpos += menu->y_size+click_btn->y_pos;
				}

				int btn_color = click_btn->color;
				if (interaction && check_cursor_over(menu->x_pos+clickBtnXpos, menu->y_pos+clickBtnYpos-menu->offset_y,
												   menu->x_pos+clickBtnXpos+click_btn->x_size,
												   menu->y_pos+clickBtnYpos+click_btn->y_size-menu->offset_y))
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
						btn_color = click_btn->hold_color;
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

					btn_color = click_btn->color;
				}

				draw_rectangle(menu, btn_color, clickBtnXpos, clickBtnYpos, clickBtnXpos+click_btn->x_size, clickBtnYpos+click_btn->y_size);

				int textlen = strlen(click_btn->text);
				draw_text(menu, clickBtnXpos+click_btn->x_size/2-(textlen*4), clickBtnYpos, click_btn->font_size, 0xff000000, "Monocraft.otf", click_btn->text);

				draw_outlines(menu, 0xff000000, clickBtnXpos, clickBtnYpos, clickBtnXpos+click_btn->x_size, clickBtnYpos+click_btn->y_size);

				if (click_btn->x_pos >= 0)
					largestX = MAX(largestX, click_btn->x_size+click_btn->x_pos);

				if (click_btn->y_pos >= 0) {
					menu->max_y = MAX(clickBtnYpos+click_btn->y_size, menu->max_y);
					largestY = MAX(largestY, click_btn->y_size+2+clickBtnYpos-menu->y_pos);
				}
				break;
			case MULTITEXT_ITEM:
				struct ItemMultitext* multitext = (struct ItemMultitext*)item;
				struct MultitextNode* lastNode = multitext->last_node;

				int mtxPos = multitext->x_pos;
				int mtyPos = multitext->y_pos+10; // this +10 is bc menu bar

				if (mtyPos == 0)
					mtyPos += largestY;

				int mtxSize = multitext->x_size;
				int mtySize = multitext->y_size;

				if (mtxSize <= 0)
					mtxSize = menu->x_size;
				if (mtySize <= 0)
					mtySize = menu->y_size-10;

				if (check_cursor_over(menu->x_pos+mtxPos, menu->y_pos+mtyPos, menu->x_pos+mtxPos+mtxSize, menu->y_pos+mtyPos+mtySize)) {
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

					if (interaction && check_cursor_over(menu->x_pos+mtxPos, menu->y_pos+mtyPos+i*12, menu->x_pos+mtxPos+mtxSize, menu->y_pos+mtyPos+i*12+txtLines*10)) {
						multitext->selected = lastNode;
					}

					if (multitext->selected) {
						if (multitext->selected->next == lastNode->next && multitext->selected->previous == lastNode->previous) {
							draw_rectangle(menu, 0xff606060, mtxPos, mtyPos+i*12, mtxPos+mtxSize, mtyPos+i*12+txtLines*10);
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
									draw_text(menu, mtxPos, mtyPos+i*12, 8, multitext->color, "Monocraft.otf", copyTxtNode);
									memset(copyTxtNode, 0, sizeof copyTxtNode);
									i+=1;

									characterNode+=copychars;
									characterInLine = 0;
								}

								wordlen = lastlineChars;

							} else if(its_new_line_time(characterNode, characterInLine, charsPerLine, lastNode->text)) {
								draw_text(menu, mtxPos, mtyPos+i*12, 8, multitext->color, "Monocraft.otf", copyTxtNode);
								memset(copyTxtNode, 0, sizeof copyTxtNode);
								characterInLine = 0;
								i+=1;
							}

							strncpy(&copyTxtNode[characterInLine], lastNode->text+characterNode, wordlen);
							characterInLine+=wordlen;
							characterNode+=wordlen-1;
						}

						if ((i+1)*8 < mtySize)
							draw_text(menu, mtxPos, mtyPos+i*12, 8, multitext->color, "Monocraft.otf", copyTxtNode);
					} else {
						draw_text(menu, mtxPos, mtyPos+i*12, 8, multitext->color, "Monocraft.otf", lastNode->text);
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

					int inpXPos = input->x_pos;
					int inpYPos = 0;

					if (input->x_pos < 0) {
						inpXPos += menu->x_size;
					}

					if (input->y_pos >= 0) {
						inpYPos += (!input->y_pos) ? largestY : input->y_pos;
					} else {
						inpYPos += input->y_pos+menu->y_size;
					}

					draw_rectangle(menu, input->background_color, inpXPos, inpYPos, inpXPos+input->x_size, inpYPos+input->y_size);

					int displayLen = input->x_size/6;
					char textDisplay[displayLen];

					if (interaction && check_cursor_over(menu->x_pos+inpXPos, menu->y_pos+inpYPos-menu->offset_y,
													   menu->x_pos+inpXPos+input->x_size,
													   menu->y_pos+inpYPos+input->y_size-menu->offset_y))
					{
						activeInputItem = input;
					}

					if (activeInputItem && activeInputItem->id == input->id) {
						int inputlen = strlen(input->input);

						if (inputlen !=  displayLen)
							menu->is_interacting = 1;

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

					draw_text(menu, inpXPos, inpYPos+input->y_size/2-4, 5, 0xff505050, "Monocraft.otf", textDisplay);

					if (input->x_pos >= 0)
						largestX = MAX(largestX, input->x_pos+input->x_size);
					if (input->y_pos >= 0) {
						menu->max_y = MAX(inpYPos+input->y_size, menu->max_y);
						largestY = MAX(largestY, input->y_size+2+inpYPos-menu->y_pos);
					}
				}
				break;
			case SLIDE_ITEM:
				struct ItemSlide* slide = (struct ItemSlide*)item;

				int slideXPos = slide->x_pos;
				int slideYPos = slide->y_pos;

				draw_rectangle(menu, slide->background_color, slideXPos, slideYPos, slideXPos+slide->x_size, slideYPos+slide->y_size);

				int sliderPosX = slideXPos;
				int sliderPosY = slideYPos;
				int sliderSizeX = slide->x_size;
				int sliderSizeY = slide->y_size;

				int slideToX = 0;
				int slideToY = 0;
				int _fds = 0;
				if (interaction && check_cursor_over(menu->x_pos+slideXPos, menu->y_pos+slideYPos-menu->offset_y, menu->x_pos+slideXPos+slide->x_size, menu->y_pos+slideYPos+slide->y_size-menu->offset_y)) {
					int porc = 1;
					int value = 0;
					if (slide->x_size >= slide->y_size) {
						int offset = mouse_x_pos-slideXPos-menu->x_pos;
						porc = offset*100/slide->x_size;
					} else {
						int offset = mouse_y_pos-slideYPos-menu->y_pos;
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

				draw_rectangle(menu, slide->slider_color, sliderPosX, sliderPosY, sliderPosX+sliderSizeX, sliderPosY+sliderSizeY);

				if (slide->show_status) {
					char cool[10];
					sprintf(cool, "%i", *slide->interact_int);
					draw_text(menu, slideXPos, slideYPos+slide->y_size+6, 16, 0xff000000, "Monocraft.otf", cool);
				}
				draw_outlines(menu, 0xff000000, slideXPos, slideYPos, slideXPos+slide->x_size, slideYPos+slide->y_size);

				menu->max_y = MAX(slideYPos+slide->y_size, menu->max_y);
				break;
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
	draw_line(menu, menu->outline_color, menu->x_pos, menu->y_pos+8, menu->x_pos+largestX, menu->y_pos+8);

	if (!menu->minimized)
		draw_outlines(menu, menu->outline_color, menu->x_pos, menu->y_pos, menu->x_pos+largestX, menu->y_pos+largestY);

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

void draw_to_display(struct Menu* menu) {
	if (menu->x_size != menu->display_x || menu->y_size != menu->display_y) {
		free(menu->display_buffer);
		menu->display_x = menu->x_size;
		menu->display_y = menu->y_size;
		menu->display_buffer = calloc(menu->display_x*menu->display_y, 4);
	}

	clean_display_buffer(menu);

	for (int x = 0; x < menu->display_x; x++) {
		for (int y = 0; y < menu->display_y; y++) {
			*((menu->display_buffer)+(x+(y*menu->display_x))) = *((menu->draw_buffer)+(x+(y+menu->offset_y)*menu->buffer_x));
		}
	}
}

int fsrun = 0;
void draw_menus() {
	int menusLen = get_next_available_menu_id();

	int interaction = 0;
	int wheel_status = 0;
	if (show_cursor) {
		interaction = handle_cursor();
		wheel_status = handle_wheel();
	}

	for (int menu_id = 0; menu_id < menusLen; menu_id++) {
		struct Menu* menu = (struct Menu*)menus[menu_id];

		if ((menu->update || ((interaction || wheel_status) &&
							 check_cursor_over(menu->x_pos, menu->y_pos, menu->x_pos+menu->x_size, menu->y_pos+menu->y_size)
							) || menu->is_interacting) && (!menu->hidden || menu->pin)
			) {
			if (wheel_status && menu->max_y > menu->y_size) {
				if (wheel_status > 0) {
					if (menu->offset_y > 0)
						menu->offset_y -= 1;
				} else {
					if (menu->max_y > menu->offset_y+menu->y_size)
						menu->offset_y += 1;
				}
			}

			menu->is_interacting = interaction;
			draw_menu(menu, interaction);
			draw_to_display(menu);
			menu->update = 0;
		}

		if (!menu->hidden || menu->pin){
			drawtile((long)menu->display_buffer, menu->display_x, menu->display_x, menu->display_y, 0, 0, menu->x_pos, menu->y_pos, 1, 1, -1);
		}
	}

	if (show_cursor)
		draw_cursor();
}

void init_menu() {
	if (FT_Init_FreeType( &flibrary ))
		printf("Could not init freetype\n");

	int err = FT_New_Face(flibrary, "Fixedsys.ttf", 0, &ftface);
	if ( err == FT_Err_Unknown_File_Format ) {
		printf("cant open file\n");
		return;
	} else if ( err ) {
		printf("other erro opening file\n");
		return;
	}
}
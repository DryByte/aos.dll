#include <winsock2.h>
#include <Macro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Menu.h>
#include <Voxlap.h>
#include <Packets.h>
#include <Config.h>

extern int client_base;
config_entry* macros_entry;

// Only alphabet is mapped
const char SCANCODES[26] = {
	30,
	48,
	46,
	32,
	18,
	33,
	34,
	35,
	23,
	36,
	37,
	38,
	50,
	49,
	24,
	25,
	16,
	19,
	31,
	20,
	22,
	47,
	17,
	45,
	21,
	44
};

struct macro_entry* macro_array[256];

struct Menu* edit_menu;
struct ItemMultitext* macro_list;

struct macro_entry* editing_macro = 0;

int get_available_macro_id() {
	for(unsigned int i = 0; i < sizeof(macro_array)/sizeof(macro_array[0]); i++) {
		if (!macro_array[i])
			return i;
	}

	return -1;
}

int get_id_by_key(unsigned char key) {
	for(unsigned int i = 0; i < sizeof(macro_array)/sizeof(macro_array[0]); i++) {
		if(!macro_array[i])
			continue;

		if (macro_array[i]->key == key)
			return i;
	}

	return -1;
}

void handle_macros() {
	if (*(int*)(client_base+0x84660))
		return;

	int array_size = get_available_macro_id();

	for (int i = 0; i < array_size; i++) {
		if (!macro_array[i])
			continue;

		struct macro_entry* entry = macro_array[i];
		if (time(NULL)-entry->last_interaction < 2)
			continue;

		char scancode = SCANCODES[entry->key-97];

		char key = *(char*)(client_base+0x864a0+scancode);
		if (!key)
			continue;

		entry->last_interaction = time(NULL);
		send_msg(entry->msg);
	}
}

void new_macro_entry(unsigned char key, char* macro_name, char* msg) {
	int macro_id = get_id_by_key(key);
	if (!editing_macro && get_id_by_key(key) > -1)
		return;

	int m_id = get_available_macro_id();
	if (m_id < 0)
		return;

	if (editing_macro)
		m_id = macro_id;

	struct macro_entry* m_entry;
	if (!editing_macro) {
		m_entry = malloc(sizeof(struct macro_entry));
	} else {
		m_entry = editing_macro;
	}

	m_entry->key = key;
	strcpy(m_entry->macro_name, macro_name);
	strcpy(m_entry->msg, msg);

	char macrostr[36];
	sprintf(macrostr, "%c | %s", m_entry->key, m_entry->macro_name);

	if (!editing_macro) {
		add_new_text(macro_list, macrostr);
		m_entry->macro_text = macro_list->last_node;
	} else {
		strcpy(m_entry->macro_text->text, macrostr);
	}

	macro_array[m_id] = m_entry;
	editing_macro = 0;
}

void clean_inputs() {
	struct ItemTextInput* key = (struct ItemTextInput*)edit_menu->items[3];
	struct ItemTextInput* name = (struct ItemTextInput*)edit_menu->items[5];
	struct ItemTextInput* message = (struct ItemTextInput*)edit_menu->items[7];
	
	key->input[0] = 0;
	memset(name->input, 0x0, 32);
	memset(message->input, 0x0, 128);
}

void save_macros_to_config() {
	int array_size = get_available_macro_id();
	int config_length = config_array_get_length(macros_entry);

	for (int i = 0; i < array_size; i++) {
		struct macro_entry* entry = macro_array[i];

		config_entry* macro_obj = json_object_new_object(); // we should change this to config somehow

		config_set_string_entry(macro_obj, "key", (char*)&entry->key);
		config_set_string_entry(macro_obj, "name", entry->macro_name);
		config_set_string_entry(macro_obj, "message", entry->msg);

		config_array_insert_entry(macros_entry, i, macro_obj);
	}

	if (config_length > array_size) {
		for (int i = array_size; i < config_length; i++) {
			config_array_del_entry(macros_entry, i);
		}
	}

	save_config();
}

void load_macros_from_config() {
	// the macro array is directly as a general, thats why NULL as section
	macros_entry = config_get_array_entry(NULL, "macros");

	for (int i = 0; i < config_array_get_length(macros_entry); i++) {
		config_entry* obj = config_array_get_entry(macros_entry, i);
		unsigned char key = config_get_string_entry(obj, "key", "f")[0];
		char* macro_name = config_get_string_entry(obj, "name", "example_macro");
		char* msg = config_get_string_entry(obj, "message", "example");

		new_macro_entry(key, macro_name, msg);
	}
}

void btn_new_handler(struct Menu* menu, struct ItemClickableButton* btn) {
	if (!btn->is_clicking)
		return;

	edit_menu->hidden = 0;
	edit_menu->x_pos = menu->x_pos+menu->x_size;
	edit_menu->y_pos = menu->y_pos;
}

void btn_edit_handler(struct Menu* menu, struct ItemClickableButton* btn) {
	if (!btn->is_clicking)
		return;

	if (!macro_list->selected)
		return;

	int macro_id = get_id_by_key(macro_list->selected->text[0]);
	if (macro_id < 0)
		return;

	edit_menu->hidden = 0;
	clean_inputs();

	struct ItemTextInput* key = (struct ItemTextInput*)edit_menu->items[3];
	struct ItemTextInput* name = (struct ItemTextInput*)edit_menu->items[5];
	struct ItemTextInput* message = (struct ItemTextInput*)edit_menu->items[7];

	struct macro_entry* entry = macro_array[macro_id];
	editing_macro = entry;

	key->input[0] = entry->key;
	strcpy(name->input, entry->macro_name);
	strcpy(message->input, entry->msg);
}

// this should be a function in menu.c for deleting multitext nodes
void btn_del_handler(struct Menu* menu, struct ItemClickableButton* btn) {
	int id = get_id_by_key(macro_list->selected->text[0]);
	if (id < 0)
		return;

	struct macro_entry* entry = macro_array[id];

	if (entry->macro_text == macro_list->first_node)
		macro_list->first_node = entry->macro_text->next;

	if (entry->macro_text == macro_list->last_node)
		macro_list->last_node = entry->macro_text->previous;

	if (entry->macro_text->previous) {
		struct MultitextNode* previous_node = (struct MultitextNode*)entry->macro_text->previous;
		previous_node->next = entry->macro_text->next;
	}

	if (entry->macro_text->next) {
		struct MultitextNode* next_node = (struct MultitextNode*)entry->macro_text->next;
		next_node->previous = entry->macro_text->previous;
	}

	memset(entry->macro_text->text, 0, 128);
	free(entry->macro_text);
	macro_array[id] = 0;
	save_macros_to_config();
}

void btn_save_macro(struct Menu* menu, struct ItemClickableButton* btn) {
	if (!btn->is_clicking)
		return;

	struct ItemTextInput* key = (struct ItemTextInput*)menu->items[3];
	struct ItemTextInput* name = (struct ItemTextInput*)menu->items[5];
	struct ItemTextInput* message = (struct ItemTextInput*)menu->items[7];

	if (!key->input[0] || !name->input[0] || !message->input[0])
		return;

	new_macro_entry((unsigned char)key->input[0],(char*)name->input, (char*)message->input);
	save_macros_to_config();
	clean_inputs();
	
	menu->hidden = 1;
}

void btn_cancel_macro(struct Menu* menu, struct ItemClickableButton* btn) {
	if (!btn->is_clicking)
		return;

	clean_inputs();
	menu->hidden = 1;
}

void create_edit_macro_menu() {
	edit_menu = create_menu(200, 100, 0, "Edit Macro");
	edit_menu->x_size = 200;
	edit_menu->y_size = 100;
	edit_menu->always_hidden = 1;

	struct ItemText* key_text = create_text(edit_menu, -1, 0xffffff, "Key:");
	key_text->x_pos = 5;
	key_text->y_pos = 15;

	struct ItemTextInput* key_input = create_text_input(edit_menu, 12, 10, 0xffffff, "");
	key_input->background_color = 0xffaaaaaa;
	key_input->x_pos = 60;
	key_input->y_pos = 14;
	key_input->max_length = 1;

	struct ItemText* name_text = create_text(edit_menu, -1, 0xffffff, "Name:");
	name_text->x_pos = 5;
	name_text->y_pos = 28;

	struct ItemTextInput* name_input = create_text_input(edit_menu, 100, 10, 0xffffff, "Macro name");
	name_input->background_color = 0xffaaaaaa;
	name_input->x_pos = 60;
	name_input->y_pos = 27;
	name_input->max_length = 32;

	struct ItemText* msg_text = create_text(edit_menu, -1, 0xffffff, "Message:");
	msg_text->x_pos = 5;
	msg_text->y_pos = 41;

	struct ItemTextInput* msg_input = create_text_input(edit_menu, 100, 10, 0xffffff, "My cool message");
	msg_input->background_color = 0xffaaaaaa;
	msg_input->x_pos = 60;
	msg_input->y_pos = 40;
	msg_input->max_length = 128;

	struct ItemClickableButton* save_btn = create_clickable_button(edit_menu, "SAVE", &btn_save_macro);
	save_btn->x_pos = 5;
	save_btn->y_pos = 65;
	save_btn->x_size = 60;
	save_btn->y_size = 20;
	save_btn->interval = 2;

	struct ItemClickableButton* cancel_btn = create_clickable_button(edit_menu, "CANCEL", &btn_cancel_macro);
	cancel_btn->x_pos = 70;
	cancel_btn->y_pos = 65;
	cancel_btn->x_size = 60;
	cancel_btn->y_size = 20;
	cancel_btn->interval = 2;
}

void create_macro_menu() {
	struct Menu* macromenu = create_menu(100, 100, 0, "Macros");
	macromenu->x_size = 100;
	macromenu->y_size = 150;

	macro_list = create_multitext(macromenu, 0xffffff);
	macro_list->y_size = 150;

	struct ItemClickableButton* btn_new = create_clickable_button(macromenu, "NEW", &btn_new_handler);
	btn_new->x_size = 30;
	btn_new->y_size = 20;
	btn_new->y_pos = 125;

	struct ItemClickableButton* btn_edit = create_clickable_button(macromenu, "EDIT", &btn_edit_handler);
	btn_edit->x_size = 30;
	btn_edit->y_size = 20;
	btn_edit->x_pos = 35;
	btn_edit->y_pos = 125;

	struct ItemClickableButton* btn_del = create_clickable_button(macromenu, "DEL", &btn_del_handler);
	btn_del->x_size = 30;
	btn_del->y_size = 20;
	btn_del->x_pos = 70;
	btn_del->y_pos = 125;
}

void initmacro() {
	create_macro_menu();
	create_edit_macro_menu();
	load_macros_from_config();
}
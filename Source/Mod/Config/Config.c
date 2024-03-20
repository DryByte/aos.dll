#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Config.h>
#include <windows.h>

config_entry* config_object;

config_entry* get_entry(config_entry* section, char* entry) {
	if (section == NULL)
		section = config_object;

	return json_object_object_get(section, entry);
}

int config_array_get_length(config_entry* array) {
	return json_object_array_length(array);
}

int config_array_insert_entry(config_entry* array, char* entry, int index, config_entry* value) {
	return json_object_array_put_idx(array, index, value);
}

int config_array_del_entry(config_entry* array, char* entry, int index) {
	return json_object_array_del_idx(array, index, 1);
}

int config_array_add_entry(config_entry* array, config_entry* value) {
	return json_object_array_add(array, value);
}

config_entry* config_array_get_entry(config_entry* array, int index) {
	return json_object_array_get_idx(array, index);
}

int config_set_bool_entry(config_entry* section, char* entry, int value) {
	config_entry* bool_obj = get_entry(section, entry);
	return json_object_set_boolean(bool_obj, value);
}

int config_set_string_entry(config_entry* section, char* entry, const char* value) {
	config_entry* str_obj = get_entry(section, entry);
	return json_object_set_string(str_obj, value);
}

config_entry* config_get_array_entry(config_entry* section, char* entry) {
	config_entry* array_obj = get_entry(section, entry);

	if (!array_obj) {
		array_obj = json_object_new_array();
		json_object_object_add(section, entry, array_obj);
	}

	return array_obj;
}

int config_get_bool_entry(config_entry* section, char* entry, int default_value) {
	config_entry* bool_obj = get_entry(section, entry);

	if (!bool_obj) {
		bool_obj = json_object_new_boolean(default_value);
		json_object_object_add(section, entry, bool_obj);

		return default_value;
	}


	return json_object_get_boolean(bool_obj);
}

char* config_get_string_entry(config_entry* section, char* entry, char* default_value) {
	config_entry* str_obj = get_entry(section, entry);

	if (!str_obj) {
		str_obj = json_object_new_string(default_value);
		json_object_object_add(section, entry, str_obj);

		return default_value;
	}

	return (char*)json_object_get_string(str_obj);
}

config_entry* config_get_section(char* section_name) {
	config_entry* section;
	json_bool found = json_object_object_get_ex(config_object, section_name, &section);

	if(!found) {
		section = json_object_new_object();
		json_object_object_add(config_object, section_name, section);
	}

	return section;
}

void save_config() {
	json_object_to_file("./config.json", config_object);
}

void init_config() {
	if (GetFileAttributes("./config.json") == INVALID_FILE_ATTRIBUTES) {
		config_object = json_object_new_object();
	} else {
		config_object = json_object_from_file("./config.json");
	}
}
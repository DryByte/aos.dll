#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Config.h>
#include <windows.h>

json_object* config_object;

json_object* get_entry(json_object* section, char* entry) {
	if (section == NULL)
		section = config_object;

	return json_object_object_get(section, entry);
}

int config_array_insert_entry(json_object* section, char* entry, int index, json_object* value) {
	json_object* arr_obj = get_entry(section, entry);
	return json_object_array_put_idx(arr_obj, index, value);
}

int config_array_del_entry(json_object* section, char* entry, int index) {
	json_object* arr_obj = get_entry(section, entry);
	return json_object_array_del_idx(arr_obj, index, 1);
}

int config_array_add_entry(json_object* section, char* entry, json_object* value) {
	json_object* arr_obj = get_entry(section, entry);
	return json_object_array_add(arr_obj, value);
}

int config_set_bool_entry(json_object* section, char* entry, int value) {
	json_object* bool_obj = get_entry(section, entry);
	return json_object_set_boolean(bool_obj, value);
}

int config_set_string_entry(json_object* section, char* entry, const char* value) {
	json_object* str_obj = get_entry(section, entry);
	return json_object_set_string(str_obj, value);
}

array_list* config_get_array_entry(json_object* section, char* entry) {
	json_object* array_obj = get_entry(section, entry);

	if (!array_obj) {
		array_obj = json_object_new_array();
		json_object_object_add(section, entry, array_obj);
	}

	return json_object_get_array(array_obj);
}

int config_get_bool_entry(json_object* section, char* entry, int default_value) {
	json_object* bool_obj = get_entry(section, entry);

	if (!bool_obj) {
		bool_obj = json_object_new_boolean(default_value);
		json_object_object_add(section, entry, bool_obj);

		return default_value;
	}


	return json_object_get_boolean(bool_obj);
}

const char* config_get_string_entry(json_object* section, char* entry, const char* default_value) {
	json_object* str_obj = get_entry(section, entry);

	if (!str_obj) {
		str_obj = json_object_new_string(default_value);
		json_object_object_add(section, entry, str_obj);

		return default_value;
	}

	return json_object_get_string(str_obj);
}

json_object* config_get_section(char* section_name) {
	json_object* section;
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
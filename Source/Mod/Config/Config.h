#include <json.h>

int config_array_insert_entry(json_object* section, char* entry, int index, json_object* value);
int config_array_del_entry(json_object* section, char* entry, int index);
int config_array_add_entry(json_object* section, char* entry, json_object* value);
int config_set_bool_entry(json_object* section, char* entry, int value);
int config_set_string_entry(json_object* section, char* entry, const char* value);
array_list* config_get_array_entry(json_object* section, char* entry);
int config_get_bool_entry(json_object* section, char* entry, int default_value);
const char* config_get_string_entry(json_object* section, char* entry, const char* default_value);
json_object* config_get_section(char* section_name);

void save_config();
void init_config();
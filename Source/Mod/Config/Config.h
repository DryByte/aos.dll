#include <json.h>

typedef json_object config_entry;

int config_array_get_length(config_entry* array);
int config_array_insert_entry(config_entry* array, char* entry, int index, config_entry* value);
int config_array_del_entry(config_entry* array, char* entry, int index);
int config_array_add_entry(config_entry* array, config_entry* value);
config_entry* config_array_get_entry(config_entry* array, int index);
int config_set_bool_entry(config_entry* section, char* entry, int value);
int config_set_string_entry(config_entry* section, char* entry, const char* value);
config_entry* config_get_array_entry(config_entry* section, char* entry);
int config_get_bool_entry(config_entry* section, char* entry, int default_value);
char* config_get_string_entry(config_entry* section, char* entry, char* default_value);
config_entry* config_get_section(char* section_name);

void save_config();
void init_config();
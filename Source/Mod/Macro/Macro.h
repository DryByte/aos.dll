#include <time.h>

struct macro_entry {
	char macro_name[32];
	char msg[128];
	unsigned char key;
	struct MultitextNode* macro_text;
	time_t last_interaction;
};

void handle_macros();
void initmacro();
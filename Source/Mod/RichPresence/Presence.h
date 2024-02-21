#include <discord_rpc.h>

#define MAX_TEXT 100

extern int presence_enabled;
extern int client_base;

void initrichpresence();
void update_presence();

typedef struct game_state {
    int current_tool;
    int current_weapon;
    int current_team;
    int intel_holder_t1;
    int intel_holder_t2;
} game_state;
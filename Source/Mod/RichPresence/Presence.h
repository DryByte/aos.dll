#include <discord_rpc.h>

extern int presence_enabled;
extern int client_base;

void init_rich_presence();
void update_presence();

void validate_player_count();
void decrement_player_count();
void get_server_info();

typedef struct game_state {
    // offline
    int current_tool;
    int current_weapon;
    int current_team;
    int intel_holder_t1;
    int intel_holder_t2;

    // online - state data trigger
    char server_name[32];
    char map_name[32];
    int max_players;
    int64_t playtime_start;
} game_state;

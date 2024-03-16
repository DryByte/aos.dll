//#include <time.h>
#include <string.h>
#include <stdio.h>

#include <Presence.h>

game_state state;
game_state old_state;

const char* app_id = "699358451494682714";
int player_id = 0;
int validator_enabled = 0;

int player_count = 1;
int old_player_count = 1;

const char* tool_descriptions[4] = {
    "Digging",
    "Building",
    "",
    "Throwing nades"
};
const char* weapon_descriptions[3]= {
    "Rifle",
    "SMG",
    "Shotgun"
};
const char* tool_images[4] = {
    "largeimagekey_spade",
    "largeimagekey_block",
    "",
    "largeimagekey_grenade"
};
const char* weapon_images[3] = {
    "largeimagekey_rifle",
    "largeimagekey_smg",
    "largeimagekey_shotgun"
};

// handlers that inform about something may be good for logger module perhaps?
// void handle_discord_ready(const DiscordUser* user) { return; }
// void handle_discord_disconnected(int error_code, const char* message) { return; }
// void handle_discord_error(int error_code, const char* message) { return; }
// void handle_discord_join(const char* join_secret) { return; }
// void handle_discord_spectate(const char* spectate_secret) { return; }
// void handle_discord_join_request(const DiscordUser* request) { return; }

void discord_init() {
    // DiscordEventHandlers handlers;
    // handlers.ready = handle_discord_ready;
    // handlers.disconnected = handle_discord_disconnected;
    // handlers.errored = handle_discord_error;
    // handlers.joinGame = handle_discord_join;
    // handlers.spectateGame = handle_discord_spectate;
    // handlers.joinRequest = handle_discord_join_request;

    Discord_Initialize(app_id, NULL, 1, NULL);
}

void discord_shutdown() {
    Discord_Shutdown();
}

void validate_player_count() {
    if (validator_enabled) {
        int ply_count = 0;
        char* player_connected_addr = (char*)(client_base + 0x7ce94);
        
        for(int i = 0; i < 32; i++) {
            if (*(int*)(player_connected_addr + (i * 0x3a8)) == 1) ply_count++;
        }

        player_count = ply_count;
    }
    else return;
}
void decrement_player_count() {
    player_count--;
}

void get_current_game_state() {
    memset(&state, 0, sizeof(game_state));

    state.current_tool = *((int*)(client_base + 0x13cf808));
    state.current_weapon = *((int*)((client_base + 0x7ce5c) + (player_id * 0x3a8)));
    state.current_team = *((int*)((client_base + 0x7ce58) + (player_id * 0x3a8)));
    state.intel_holder_t1 = *((int*)(client_base + 0x13cf958));
    state.intel_holder_t2 = *((int*)(client_base + 0x13cf924));
}

void update_presence(){
    if (presence_enabled)
    {
        player_id = *((int*)(client_base+0x13b1cf0));
        // int64_t playtime_start = time(0); // map change
        get_current_game_state();

        if ((state.current_team == old_state.current_team) &&
            (state.current_tool == old_state.current_tool) &&
            (state.current_weapon == old_state.current_weapon) &&
            (state.intel_holder_t1 == old_state.intel_holder_t1) &&
            (state.intel_holder_t2 == old_state.intel_holder_t2) && 
            (player_count == old_player_count)) 
        { return; }

        old_state = state;
        old_player_count = player_count;

        DiscordRichPresence presence;
        memset(&presence, 0, sizeof(DiscordRichPresence));
        presence.details = "Server placeholder";
        presence.state = "Map placeholder";
        // presence.startTimestamp = playtime_start;

        if (player_id == -1) {
            presence.largeImageKey = "largeimagekey_loading";
            presence.largeImageText = "Loading map...";
        }
        else {
            if (state.current_team == -2) {
                presence.largeImageKey = "largeimagekey_teamselection";
                presence.largeImageText = "Choosing team";
            }
            else if (state.current_team == -1) {
                validator_enabled = 1;
                presence.largeImageKey = "largeimagekey_spectating";
                presence.largeImageText = "Spectating";
            }
            else {
                validator_enabled = 1;
                if (state.current_tool != 2) {
                    presence.largeImageKey = tool_images[state.current_tool];
                    presence.largeImageText = tool_descriptions[state.current_tool];
                }
                else {
                    presence.largeImageKey = weapon_images[state.current_weapon];
                    presence.largeImageText = weapon_descriptions[state.current_weapon];
                }
                
                if (state.intel_holder_t1 == player_id || state.intel_holder_t2 == player_id) {
                    presence.smallImageKey = "smallimagekey_intel";
                    presence.smallImageText = "Holds enemy intel!";
                }
                else {
                    presence.smallImageKey = "ace_of_spades";
                    char ply_count_buf[128];
                    sprintf(ply_count_buf, "Players: %d", player_count);
                    presence.smallImageText = ply_count_buf;
                }
            }
        }


        Discord_UpdatePresence(&presence);
    }
    else {
        Discord_ClearPresence();
    }
}

void init_rich_presence() {
    discord_init();
    update_presence();

    //create_richpresence_menu();
}

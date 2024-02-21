#include <time.h>
#include <string.h>

#include <Presence.h>

const char* app_id = "699358451494682714";
int player_id = 0;
game_state old_state;

const char tool_descriptions[4][MAX_TEXT] = {
    "Digging",
    "Building",
    "",
    "Throwing nades"
};
const char weapon_descriptions[3][MAX_TEXT] = {
    "Rifle",
    "SMG",
    "Shotgun"
};
const char tool_images[4][MAX_TEXT] = {
    "largeimagekey_spade",
    "largeimagekey_block",
    "",
    "largeimagekey_grenade"
};
const char weapon_images[3][MAX_TEXT] = {
    "largeimagekey_rifle",
    "largeimagekey_smg",
    "largeimagekey_shotgun"
};

// handlers that inform about something may be good for logger module perhaps?
void handle_discord_ready(const DiscordUser* user) { return; }
void handle_discord_disconnected(int error_code, const char* message) { return; }
void handle_discord_error(int error_code, const char* message) { return; }
void handle_discord_join(const char* join_secret) { return; }
void handle_discord_spectate(const char* spectate_secret) { return; }
void handle_discord_join_request(const DiscordUser* request) { return; }

void discord_init() {
    DiscordEventHandlers handlers;
    handlers.ready = handle_discord_ready;
    handlers.disconnected = handle_discord_disconnected;
    handlers.errored = handle_discord_error;
    handlers.joinGame = handle_discord_join;
    handlers.spectateGame = handle_discord_spectate;
    handlers.joinRequest = handle_discord_join_request;

    Discord_Initialize(app_id, &handlers, 1, NULL);
}

void discord_shutdown() {
    Discord_Shutdown();
}

game_state get_current_game_state() {
    game_state state;
    memset(&state, 0, sizeof(game_state));

    state.current_tool = *((int*)((client_base + 0x13cf808) + (player_id * 0x3a8)));
    state.current_weapon = *((int*)((client_base + 0x7ce5c) + (player_id * 0x3a8)));
    state.current_team = *((int*)((client_base + 0x7ce58) + (player_id * 0x3a8)));
    state.intel_holder_t1 = *((int*)(client_base + 0x13cf958));
    state.intel_holder_t2 = *((int*)(client_base + 0x13cf924));

    return state;
}

void update_presence(){
    if (presence_enabled)
    {
        player_id = *((int*)(client_base+0x13b1cf0));
        // int64_t playtime_start = time(0); // map change
        game_state state = get_current_game_state();

        if ((state.current_team == old_state.current_team) &&
            (state.current_tool == old_state.current_tool) &&
            (state.current_weapon == old_state.current_weapon) &&
            (state.intel_holder_t1 == old_state.intel_holder_t1) &&
            (state.intel_holder_t2 == old_state.intel_holder_t2)) 
        { return; }

        old_state = state;
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
                presence.largeImageKey = "largeimagekey_spectating";
                presence.largeImageText = "Spectating";
            }
            else {
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
                    presence.smallImageText = "Ace Of Spades";
                }
            }
        }


        Discord_UpdatePresence(&presence);
    }
    else {
        Discord_ClearPresence();
    }
}

void initrichpresence() {
    discord_init();
    update_presence();

    //create_richpresence_menu();
}

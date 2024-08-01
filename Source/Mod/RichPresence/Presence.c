#include <time.h>
#include <stdio.h>
#include <string.h>
#include <json.h>
#include <windows.h>
#include <winhttp.h>
#include <processenv.h>

#include <Presence.h>
#include <Config.h>

#define MAX_RESPONSE 65535

game_state state;
game_state old_state;
config_entry* presence_config;

presence_button serverlist_button = {
    "Serverlist",
    "https://www.buildandshoot.com/servers/"
};

const char* app_id = "699358451494682714";

int player_id = 0;
int reset_timer_on_map_change = 1;
int show_join_button = 1;

int state_data_sleep_count = 1,
    valid_state_data = 0;

int player_count = 0, 
    old_player_count = 0;

int ply_count_sleep_count = 0,
    ply_count_delay = 0;

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


void trigger_player_count_validation() {
    ply_count_delay = 1;
}

void validate_player_count() {
    int ply_count = 0;
    char* player_connected_addr = (char*)(client_base + 0x7ce94);
    
    for(int i = 0; i < 32; i++) {
        if (*(int*)(player_connected_addr + (i * 0x3a8)) == 1) ply_count++;
    }

    player_count = ply_count;
}
void decrement_player_count() {
    player_count--;
}

void get_current_game_state() {
    state.is_alive = *((int*)((client_base + 0x7ce90) + (player_id * 0x3a8)));
    state.current_tool = *((int*)(client_base + 0x13cf808));
    state.current_weapon = *((int*)((client_base + 0x7ce5c) + (player_id * 0x3a8)));
    state.current_team = *((int*)((client_base + 0x7ce58) + (player_id * 0x3a8)));
    state.intel_holder_t1 = *((int*)(client_base + 0x13cf958));
    state.intel_holder_t2 = *((int*)(client_base + 0x13cf924));
}

void get_server_info(int triggered_by_packet, uint8_t game_mode_id) {
    if (triggered_by_packet) { 
        valid_state_data = 0; 
        state.game_mode_id = game_mode_id;
        if (reset_timer_on_map_change) state.playtime_start = time(0); 
    }
    // request buildandshoot for serverlist (https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpquerydataavailable#examples)
    DWORD bytes_available = 0;
    DWORD bytes_read = 0;
    LPSTR http_data_buffer;
    int is_successful = FALSE;
    HINTERNET session = NULL, connection = NULL, request = NULL;

    LPSTR full_response = (LPSTR)malloc(sizeof(char) * MAX_RESPONSE);
    int index_to_copy = 0;

    session = WinHttpOpen( L"Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20100101 Firefox/10.0",  
                            WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                            WINHTTP_NO_PROXY_NAME, 
                            WINHTTP_NO_PROXY_BYPASS, 0 );

    if(session)
        connection = WinHttpConnect( session, L"services.buildandshoot.com",
                                INTERNET_DEFAULT_HTTPS_PORT, 0 );
    else
        printf("no session\n");

    if(connection)
        request = WinHttpOpenRequest( connection, L"GET", L"/serverlist.json",
                                    NULL, WINHTTP_NO_REFERER, 
                                    WINHTTP_DEFAULT_ACCEPT_TYPES, 
                                    WINHTTP_FLAG_SECURE );
    else
        printf("no connection (variable)\n");

    if(request)
        is_successful = WinHttpSendRequest( request,
                                    WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                    WINHTTP_NO_REQUEST_DATA, 0, 
                                    0, 0 );
    else 
        printf("no request\n");


    if(is_successful)
        is_successful = WinHttpReceiveResponse( request, NULL );
    else 
        printf("no results\n");

    if(is_successful)
    {
        do 
        {
            bytes_available = 0;
            if( !WinHttpQueryDataAvailable( request, &bytes_available ) )
                printf( "Error %lu in WinHttpQueryDataAvailable.\n",
                        GetLastError( ) );

            http_data_buffer = malloc(bytes_available+1);
            if( !http_data_buffer )
            {
                printf( "Out of memory\n" );
                bytes_available=0;
            }
            else
            {
                ZeroMemory( http_data_buffer, bytes_available+1 );

                if( !WinHttpReadData( request, (LPVOID)http_data_buffer, 
                                    bytes_available, &bytes_read ) )
                    printf( "Error %lu in WinHttpReadData.\n", GetLastError( ) );
                else {
                    for(size_t i = 0; i < bytes_read; i++) {
                        full_response[i + index_to_copy] = http_data_buffer[i];
                    }
                }
                index_to_copy += bytes_read;
                free(http_data_buffer);
            }
        } while( bytes_available > 0 );
    }


    if(!is_successful) 
        printf( "Error %ld has occurred.\n", GetLastError( ) );

    if(request) WinHttpCloseHandle(request);
    if(connection) WinHttpCloseHandle(connection);
    if(session) WinHttpCloseHandle(session);


    int serverlist_len = strlen((const char*)full_response) + 1;
    json_tokener* tokener = json_tokener_new();
    json_object* serverlist = json_tokener_parse_ex(tokener, (const char*)full_response, serverlist_len);
    json_tokener_free(tokener);

    LPSTR command_line = GetCommandLineA();
    char* identifier = strstr(command_line, "aos://");

    int expected_quote_pos = strlen(identifier) - 1;
    if (identifier[expected_quote_pos] == '"' || identifier[expected_quote_pos] == '/') // FKN 2 hours to find out that on windows, when the game is not launched directly from a debugger,
                                                                                        // it just adds a slash to the end of aos identifier, because why not... WINDOWS MOMENT
        identifier[expected_quote_pos] = '\0'; // remove useless quote/slash
    strcpy(state.identifier, identifier);

    int server_found = 0;
    for (size_t index = 0; index < json_object_array_length(serverlist); index++) {
        json_object* server_instance = json_object_array_get_idx(serverlist, index);
        json_object* server_identifier = json_object_object_get(server_instance, "identifier");

        if (!strcmp(identifier, json_object_get_string(server_identifier))) {
            server_found = 1;
            
            json_object* map = json_object_object_get(server_instance, "map");

            if (strcmp(json_object_get_string(map), state.map_name)) { // if maps not the same
                json_object* name_obj = json_object_object_get(server_instance, "name");
                json_object* game_mode = json_object_object_get(server_instance, "game_mode");
                json_object* players_max = json_object_object_get(server_instance, "players_max");

                strcpy(state.server_name, json_object_get_string(name_obj));
                strcpy(state.map_name, json_object_get_string(map));
                strcpy(state.game_mode, json_object_get_string(game_mode));
                state.max_players = json_object_get_int(players_max);

                valid_state_data = 1;
                break;
            }
        }
    }
    if (!server_found) {
        strcpy(state.server_name, "(Local/Private server)");
        strcpy(state.map_name, "(none)");
        strcpy(state.game_mode, "(none)");
        state.max_players = 0;
        valid_state_data = 1;
    }
}

void update_presence() {
    if (ply_count_delay) {
        ply_count_sleep_count++;

        if (ply_count_sleep_count > 4) {
            ply_count_delay = 0;
            ply_count_sleep_count = 0;
            validate_player_count();
        }
    }

    if (!valid_state_data) {
        if (state_data_sleep_count > 240) {
            state_data_sleep_count = 1; 
            valid_state_data = 1;
        }
        if (state_data_sleep_count % 40 == 0) get_server_info(0, -1);
        state_data_sleep_count++;
    }

    player_id = *((int*)(client_base+0x13b1cf0));
    get_current_game_state();

    if ((state.is_alive == old_state.is_alive) &&
        (!strcmp(state.map_name, old_state.map_name)) &&
        (state.current_team == old_state.current_team) &&
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
    presence.startTimestamp = state.playtime_start;

    char s_name_buf[128], m_name_buf[128], ply_count_buf[128]; 
    sprintf(s_name_buf, "%s", state.server_name);
    sprintf(m_name_buf, "Map: %s", state.map_name);
    presence.details = s_name_buf;
    presence.state = m_name_buf;

    if (show_join_button)
    {
        presence_button join_button;
        sprintf(join_button.label, "%s", "Join");
        sprintf(join_button.url, "%s", state.identifier);
        presence.buttonLabels[0] = join_button.label;
        presence.buttonUrls[0] = join_button.url;
    }
    presence.buttonLabels[1] = serverlist_button.label;
    presence.buttonUrls[1] = serverlist_button.url;

    if (player_id == -1) {
        presence.largeImageKey = "largeimagekey_loading";
        presence.largeImageText = "Loading map...";
    }
    else {
        if (state.current_team == -2) {
            presence.largeImageKey = "largeimagekey_teamselection";
            presence.largeImageText = "Choosing team";
            Discord_UpdatePresence(&presence);
            return;
        }
        else if (state.current_team == -1) {
            presence.largeImageKey = "largeimagekey_spectating";
            presence.largeImageText = "Spectating";
        }

        if (state.game_mode_id) {
            presence.smallImageKey = "smallimagekey_tc";
            sprintf(ply_count_buf, "Players: %d/%d | Game mode: %s", player_count, state.max_players, state.game_mode);
            presence.smallImageText = ply_count_buf;
        }
        else if (state.game_mode_id == 0 && (state.intel_holder_t1 == player_id || state.intel_holder_t2 == player_id)) {
            presence.smallImageKey = "smallimagekey_intel";
            sprintf(ply_count_buf, "Holds enemy intel! | Players: %d/%d | Game mode: %s", player_count, state.max_players, state.game_mode);
            presence.smallImageText = ply_count_buf;
        }
        else {
            presence.smallImageKey = "ace_of_spades";
            sprintf(ply_count_buf, "Players: %d/%d | Game mode: %s", player_count, state.max_players, state.game_mode);
            presence.smallImageText = ply_count_buf;
        }

        if (state.current_team > -1) {
            if (!state.is_alive) {
                presence.largeImageKey = "largeimagekey_dead";
                presence.largeImageText = "Dead";
                Discord_UpdatePresence(&presence);
                return;
            }

            if (state.current_tool != 2) {
                presence.largeImageKey = tool_images[state.current_tool];
                presence.largeImageText = tool_descriptions[state.current_tool];
            }
            else {
                presence.largeImageKey = weapon_images[state.current_weapon];
                presence.largeImageText = weapon_descriptions[state.current_weapon];
            }
        }
    }

    Discord_UpdatePresence(&presence);
}

void init_rich_presence() {
    presence_config = config_get_section("richpresence");
    presence_enabled = config_get_bool_entry(presence_config, "enabled", 1);
    if (!presence_enabled) { return; }

    discord_init();
    show_join_button = config_get_bool_entry(presence_config, "show_join_button", 1);
    // intentional 2 calls, it works only this way on map loading stage idk why (again on WINDOWS)
    get_server_info(0, -1);
    get_server_info(0, -1);
    update_presence();

    reset_timer_on_map_change = config_get_bool_entry(presence_config, "reset_timer_on_map_change", 1);
    state.playtime_start = time(0);

    save_config();
}

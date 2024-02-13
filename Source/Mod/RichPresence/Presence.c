#include <time.h>
#include <string.h>

#include <Presence.h>

const char* app_id = "699358451494682714";

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

void update_presence(){
    if (presence_enabled)
    {
        int64_t playtime_start = time(0);

        DiscordRichPresence presence;
        memset(&presence, 0, sizeof(DiscordRichPresence));
        presence.details = "Server placeholder";
        presence.state = "Map placeholder";
        presence.startTimestamp = playtime_start;
        presence.largeImageKey = "ace_of_spades";
        presence.smallImageKey = "smallimagekey_intel";
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

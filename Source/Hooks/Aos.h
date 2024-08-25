#include <windows.h>

extern int client_base;

enum KV6TYPE {
	SPADE_3 = 0,
	BLOCK_3 = 4,
	SEMI_3 = 8,
	SMG_3 = 12,
	SHOTGUN_3 = 16,
	GRENADE_3 = 20,

	SPADE = 24,
	BLOCK = 28,
	SEMI = 32,
	SMG = 36,
	SHOTGUN = 40,
	GRENADE = 44,

	PLAYERHEAD_B = 48,
	PLAYERTORSO_B = 52,
	PLAYERTORSOC_B = 56,
	PLAYERARMS_B = 60,
	PLAYERLEG_B = 64,
	PLAYERLEGC_B = 68,
	PLAYERDEAD_B = 72,

	PLAYERHEAD_G = 76,
	PLAYERTORSO_G = 80,
	PLAYERTORSOC_G = 84,
	PLAYERARMS_G = 88,
	PLAYERLEG_G = 92,
	PLAYERLEGC_G = 96,
	PLAYERDEAD_G = 100,

	INTEL_B = 104,
	INTEL_G = 108,
	CP_B = 112,
	CP_G = 116,
	CP_N = 120,
	SEMITRACER = 124,
	SMGTRACER = 128,
	SHOTGUNTRACER = 132,
	SEMICASING = 136,
	SMGCASING = 140,
	SHOTGUNCASING = 144
};

// entries starting with _ is unknown values
struct player_struct {
	float x_pos_visible; // this is only updated when you can see the other player 0x7cb70 - 0x4
	float y_pos_visible; // this is only updated when you can see the other player 0x7cb74 - 0x8
	float z_pos_visible; // this is only updated when you can see the other player 0x7cb78 - 0xc
	char _unk[0x44]; // unknown yet 0x7cb7c->0x7cbbf- 0x50
	char _kv6related[0x10]; // probably kv6 related 0x7cbc0->0x7cbcf - 0x60
	float x_dead_body_size; // KV6 0x7cbd0 - 0x64
	float y_dead_body_size; // KV6 0x7cbd4 - 0x68
	float z_dead_body_size; // KV6 0x7cbd8 - 0x6c
	float x_dead_body_rot; // KV6 0x7cbdc - 0x70
	float y_dead_body_rot; // KV6 0x7cbe0 - 0x74
	float z_dead_body_rot; // KV6 0x7cbe4 - 0x78
	char _kv6body[0x30]; // 0x7cbe8->0x7cc17 - 0xA8
	char _moreunk[0x50]; // 0x7cc18->0x7cc67 - 0xF8
	int* kv6_player_torso_ptr; // pointer to the current kv6 for torso (blue/green crouch/normal) - 0x7cc68 - 0xFC
	char _munk[0x54]; // prob related to rotation/size - 0x7cc6c->0x7ccbf - 0x150
	char _anunk[0x50]; // 0x7ccc0->7cd0f - 0x1A0
	int* kv6_player_left_leg_ptr; // pointer to the current kv6 for leg (blue/green crouch/normal) - 0x7cd10 - 0x1A4
	char _unkagain[0x50]; // prob related to rotation/size - 0x7cd14->0x7cd63 - 0x1F4
	int* kv6_player_right_leg_ptr; // pointer to the current kv6 for leg (blue/green crouch/normal) - 0x7cd64 - 0x1F8
	char _unkidk[0xA8]; // prob related to rotation/size - 0x7cd68->0x7ce0f - 0x2A0
	float x_pos_velocity_calc; // this is used with velocity before setting the real player position - 0x7ce10 - 0x2A4
	float y_pos_velocity_calc; // this is used with velocity before setting the real player position - 0x7ce14 - 0x2A8
	float z_pos_velocity_calc; // this is used with velocity before setting the real player position - 0x7ce18 - 0x2AC
	float x_pos; // real x pos - 0x7ce1c - 0x2B0;
	float y_pos; // real y pos - 0x7ce20 - 0x2B4;
	float z_pos; // real z pos - 0x7ce24 - 0x2B8;
	float x_velocity; // 0x7ce28 - 0x2BC
	float y_velocity; // 0x7ce2c - 0x2C0;
	float z_velocity; // 0x7ce30 - 0x2C4;
	float x_ori; // 0x7ce34 - 0x2C8
	float y_ori; // 0x7ce38 - 0x2CC
	float z_ori; // 0x7ce3c - 0x2D0
	float _unkown_ori_x; // 0x7ce40 - 0x2D4
	float _unkown_ori_y; // 0x7ce44 - 0x2D8
	float _unkown_ori_z; // 0x7ce48 - 0x2DC
	char _unkown_oris[0xC]; // 0x7ce4c - 0x2E8
	int team_id; // 0x7ce58 - 0x2EC
	int weapon_id; // 0x7ce5c - 0x2F0
	int input_key_up; // 0x7ce60 - 0x2F4
	int input_key_down; // 0x7ce64 - 0x2F8
	int input_key_left; // 0x7ce68 - 0x2FC
	int input_key_right; // 0x7ce6c - 0x300
	int input_key_jump; // 0x7ce70 - 0x304
	int input_key_crouch; // 0x7ce74 - 0x308
	int input_key_sneak; // 0x7ce78 - 0x30C
	int input_key_sprint; // 0x7ce7c - 0x310
	int primary_fire; // 0x7ce80 - 0x314
	int secondary_fire; // 0x7ce84 - 0x318
	int tool_id; // 0x7ce88 - 0x31C
	int block_color; // 0x7ce8c - 0x320
	int is_alive; // 0x7ce90 - 0x324
	int connected; // 0x7ce94 - 0x328
	char _unk_inputs[0xC]; // 0x7ce98 - 0x334
	int block_count; // 0x7cea4 - 0x338
	int weapon_ammo; // 0x7cea8 - 0x33C
	int weapon_stock; // 0x7ceac - 0x340
	int grenades; // 0x7ceb0 - 0x344
	int hp; // 0x7ceb4 - 0x348
	float tool_interval_ts; // this is used to animate tools and to interval tools (like shooting), it uses the global time 0x7ceb8 - 0x34C
	float grenade_spade_interval_ts; // interval for spade right click and grenade holding (spadenade kekw) 0x7cebc - 0x350
	float _unk_interval_spade; // 0x7cec0 - 0x354
	float reload_ts; // reload start ts 0x7cec4 - 0x358
	float _movement_ts; // idk whats this exactly, it increments as ts but for movement... 0x7cec8 - 0x35c
	float _movement_ts2; // idk whats this exactly, it increments as ts but for movement... 0x7cec8 - 0x360
	int death_ts; // stores the time when you died 0x7ced0 - 0x364
	int _death_modifier; // idk, it can modify how death_ts is rendered 0x7ced4 - 0x360
	char _more_death_modfier[0x10]; // 0x7ced8 - 0x370
	char _boolean_stuff_related_to_death[0xC]; // 0x7cee8 - 0x37c
	int player_id; // 0x7cef4 - 0x380
	char _fill[0xc]; // weird bytes after player_id 0x7cef8 - 0x38c
	char player_name[0x14]; // 0x7cf04 - 0x3a0
};

void load_kv6_files();
void load_world_objects();
void load_player_team_skin(int player_id);
void load_player_weapon_skin(int player_id);
void load_player_skins();
void load_image_files();
void spectator_movement_hook();
void handle_spectate_input();
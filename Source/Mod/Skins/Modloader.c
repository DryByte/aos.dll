#include <Modloader.h>
#include <Menu.h>
#include <stdio.h>
#include <Voxlap.h>
#include <Aos.h>
#include <Config.h>

HANDLE file_handler;
WIN32_FIND_DATAA file_desc;

struct ItemMultitext* mt;
config_entry* modloader_config;

extern int client_base;

const char kv6filenames[37][15] = {
	"SPADE_3","BLOCK_3","SEMI_3","SMG_3","SHOTGUN_3","GRENADE_3","SPADE","BLOCK","SEMI","SMG","SHOTGUN","GRENADE",
	"PLAYERHEAD_B","PLAYERTORSO_B","PLAYERTORSOC_B","PLAYERARMS_B","PLAYERLEG_B","PLAYERLEGC_B","PLAYERDEAD_B",
	"PLAYERHEAD_G","PLAYERTORSO_G","PLAYERTORSOC_G","PLAYERARMS_G","PLAYERLEG_G","PLAYERLEGC_G","PLAYERDEAD_G",
	"INTEL_B","INTEL_G","CP_B","CP_G","CP_N","SEMITRACER","SMGTRACER","SHOTGUNTRACER","SEMICASING","SMGCASING","SHOTGUNCASING"
};

void word_to_lower_case(char* s) {
	for (unsigned int c = 0; c < strlen(s); c++) {
		*(s+c) = tolower(s[c]);
	}
}

void load_skin_images(char* skin_name) {
	long* SEMI_BUFFER_PNG = (long*)(client_base+0x51758);
	long* SMG_BUFFER_PNG = (long*)(client_base+0x517b0);
	long* SHOTGUN_BUFFER_PNG = (long*)(client_base+0x51808);
	long* SPLASH_BUFFER_PNG = (long*)(client_base+0x13b1e08);
	long* TARGET_BUFFER_PNG = (long*)(client_base+0x12b1c00);

	char directory[256];
	sprintf(directory, "./modloader/%s/png/*", skin_name);
	HANDLE png_handle = FindFirstFileA(directory, &file_desc);

	if (png_handle == INVALID_HANDLE_VALUE) {
		return;
	}

	WIN32_FIND_DATAA png_desc;
	do {
		if (!(png_desc.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE))
			continue;

		char* file_ext = strrchr(png_desc.cFileName, '.');
		if (!file_ext)
			continue;

		char path[256];
		sprintf(path, "./modloader/%s/png/%s", skin_name, png_desc.cFileName);

		if (!strcmp(file_ext, ".png")) {
			if (!strcmp(png_desc.cFileName, "semi.png")) {
				kpzload(path, SEMI_BUFFER_PNG, 800, 600);
			} else if(!strcmp(png_desc.cFileName, "smg.png")) {
				kpzload(path, SMG_BUFFER_PNG, 800, 600);
			} else if(!strcmp(png_desc.cFileName, "shotgun.png")) {
				kpzload(path, SHOTGUN_BUFFER_PNG, 800, 600);
			} else if(!strcmp(png_desc.cFileName, "splash.png")) {
				kpzload(path, SPLASH_BUFFER_PNG, 800, 600);
			} else if(!strcmp(png_desc.cFileName, "target.png")) {
				kpzload(path, TARGET_BUFFER_PNG, 16, 16);
			}

		} else if (!strcmp(file_ext, ".bmp")) {
			HANDLE bmp_file = LoadImageA(NULL, path, IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION|LR_LOADFROMFILE);

			if (!bmp_file)
				continue;

			int offset = 0;
			if (!strcmp(png_desc.cFileName, "indicator.bmp")) {
				offset = 0x13b75c8;
			} else if (!strcmp(png_desc.cFileName, "player.bmp")) {
				offset = 0x13cf838;
			} else if (!strcmp(png_desc.cFileName, "intel.bmp")) {
				offset = 0x13b1f24;
			} else if (!strcmp(png_desc.cFileName, "command.bmp")) {
				offset = 0x13b2040;
			} else if (!strcmp(png_desc.cFileName, "medical.bmp")) {
				offset = 0x13cf804;
			} else if (!strcmp(png_desc.cFileName, "tracer.bmp")) {
				offset = 0x13b1ca8;
			} else if (!strcmp(png_desc.cFileName, "health.bmp")) {
				offset = 0x12b1c0c;
			} else if (!strcmp(png_desc.cFileName, "block.bmp")) {
				offset = 0x13b75a4;
			} else if (!strcmp(png_desc.cFileName, "semi.bmp")) {
				offset = 0x13b1c20;
			} else if (!strcmp(png_desc.cFileName, "shotgun.bmp")) {
				offset = 0x13cf7e0;
			} else if (!strcmp(png_desc.cFileName, "smg.bmp")) {
				offset = 0x13b1e10;
			} else if (!strcmp(png_desc.cFileName, "grenade.bmp")) {
				offset = 0x13b1ca0;
			}

			if (!offset)
				continue;

			*((long*)(client_base+offset)) = (int)bmp_file;
		}
	} while(FindNextFile(png_handle, &png_desc));
}

void load_skin(char* skin_name) {
	char directory[256];
	sprintf(directory, "./modloader/%s/kv6", skin_name);

	char kv6filenamesCopy[37][15];
	memcpy(kv6filenamesCopy, kv6filenames, 37*15);
	for (int i = 0; i < 37; i++) {
		word_to_lower_case(kv6filenamesCopy[i]);

		char tmpdir[256];
		sprintf(tmpdir, "%s/%s.kv6", directory, kv6filenamesCopy[i]);
		free((struct aoskv6data*)(client_base+0x13cf840+i*4)); // im not sure if this is right...

		int firsttry = 0;
		unsigned char color = 0;
		if (GetFileAttributesA(tmpdir) == INVALID_FILE_ATTRIBUTES) {
			char* firstpart = strrchr(kv6filenamesCopy[i], '_');
			if (!firstpart) {
				sprintf(tmpdir, "./kv6/%s.kv6", kv6filenamesCopy[i]);
			} else {
				int leng = strlen(firstpart);
				int anotherleng = strlen(kv6filenamesCopy[i]);

				color = firstpart[1];
				kv6filenamesCopy[i][anotherleng-leng] = '\0';

				sprintf(tmpdir, "%s/%s.kv6", directory, kv6filenamesCopy[i]);
			}
		} else {
			char* _skin = strrchr(kv6filenamesCopy[i], '_');
			if (_skin && (_skin[1] == 'b' || _skin[1] == 'g'))
				color = _skin[1];

			firsttry = 1;
		}

		if (!firsttry && GetFileAttributesA(tmpdir) == INVALID_FILE_ATTRIBUTES)
			continue;

		struct aoskv6data* kv6 = loadkv6(tmpdir);

		if (color && kv6->numvoxs) {
			for (int voxel = 0; voxel < (int)kv6->numvoxs; voxel++) {
				if ((kv6->vox[voxel].col & 0xffffff) == 0) {
					if (color == 'b')
						kv6->vox[voxel].col = *(int*)(client_base+0x13cfcec);
					else
						kv6->vox[voxel].col = *(int*)(client_base+0x13cfcfc);
				}
			}
		}

		switch(i) {
			case 0:
				kv6->xpiv += 6.0;
				kv6->ypiv -= 12.0;
				kv6->zpiv += 2.0;
				break;
			case 1:
			case 5:
				kv6->xpiv += 5.0;
				kv6->ypiv -= 13.0;
				break;
			case 2:
			case 3:
			case 4:
				kv6->xpiv += 6.0;
				kv6->ypiv -= 18.0;
				break;
		}

		*(int*)(client_base+0x13cf840+i*4) = (int)kv6;
	}
}

void update_skin(char* skin_name) {
	// load default skins
	load_kv6_files();
	load_image_files();

	if (!skin_name) {
		char* skin = config_get_string_entry(modloader_config, "selected_skin", "(none)");

		if (strcmp(skin, "(none)"))
			skin_name = skin;
	}

	if (skin_name) {
		load_skin(skin_name);
		load_skin_images(skin_name);
	}

	load_player_skins();
	load_world_objects();
}

void update_skin_btn(struct Menu* skinMenu, struct ItemClickableButton* btn) {
	if (!btn->is_clicking)
		return;

	if (!strcmp(mt->selected->text, "(none)")) {
		load_kv6_files();
		load_image_files();

		load_player_skins();
		load_world_objects();
	} else {
		update_skin(mt->selected->text);
	}

	config_set_string_entry(modloader_config, "selected_skin", mt->selected->text);
	save_config();
}

void create_modloader_menu() {
	struct Menu* skin_menu = create_menu(100, 300, 0, "Skins");
	skin_menu->x_size = 100;
	skin_menu->y_size = 150;

	mt = create_multitext(skin_menu, 0xffffffff);
	mt->x_size = 100;
	mt->y_size = 100;

	struct ItemClickableButton* save_btn = create_clickable_button(skin_menu, "SAVE", &update_skin_btn);
	save_btn->x_size = 100;
	save_btn->y_size = 25;
	save_btn->x_pos = 0;
	save_btn->y_pos = -30;
	save_btn->interval = 1;

	do {
		if (!(file_desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		if (!strcmp(file_desc.cFileName, ".") || !strcmp(file_desc.cFileName, ".."))
			continue;
		add_new_text(mt, file_desc.cFileName);
	} while(FindNextFile(file_handler, &file_desc));
	add_new_text(mt, "(none)");
}

__declspec(naked) void load_skin_hook() {
	update_skin(NULL);

	asm volatile ("jmp *%0":: "r"(client_base+0x33dfb));
}

void init_mod_loader() {
	file_handler = FindFirstFileA("./modloader/*", &file_desc);

	if (file_handler == INVALID_HANDLE_VALUE)
		return;

	create_modloader_menu();
	modloader_config = config_get_section("modloader");
}
#include <Modloader.h>
#include <Menu.h>
#include <stdio.h>
#include <Voxlap.h>
#include <Aos.h>

HANDLE fileHandler;
WIN32_FIND_DATAA fileDesc;

struct ItemMultitext* mt;

const char kv6filenames[37][15] = {
	"SPADE_3","BLOCK_3","SEMI_3","SMG_3","SHOTGUN_3","GRENADE_3","SPADE","BLOCK","SEMI","SMG","SHOTGUN","GRENADE",
	"PLAYERHEAD_B","PLAYERTORSO_B","PLAYERTORSOC_B","PLAYERARMS_B","PLAYERLEG_B","PLAYERLEGC_B","PLAYERDEAD_B",
	"PLAYERHEAD_G","PLAYERTORSO_G","PLAYERTORSOC_G","PLAYERARMS_G","PLAYERLEG_G","PLAYERLEGC_G","PLAYERDEAD_G",
	"INTEL_B","INTEL_G","CP_B","CP_G","CP_N","SEMITRACER","SMGTRACER","SHOTGUNTRACER","SEMICASING","SMGCASING","SHOTGUNCASING"
};

void wordToLowerCase(char* s) {
	for (unsigned int c = 0; c < strlen(s); c++) {
		*(s+c) = tolower(s[c]);
	}
}

void loadSkinImages(char* skin_name) {
	long* SEMI_BUFFER_PNG = (long*)(clientBase+0x51758);
	long* SMG_BUFFER_PNG = (long*)(clientBase+0x517b0);
	long* SHOTGUN_BUFFER_PNG = (long*)(clientBase+0x51808);
	long* SPLASH_BUFFER_PNG = (long*)(clientBase+0x13b1e08);
	long* TARGET_BUFFER_PNG = (long*)(clientBase+0x12b1c00);

	char directory[256];
	sprintf(directory, "./modloader/%s/png/*", skin_name);
	HANDLE pngHandle = FindFirstFileA(directory, &fileDesc);

	if (pngHandle == INVALID_HANDLE_VALUE) {
		return;
	}

	WIN32_FIND_DATAA pngDesc;
	do {
		if (!(pngDesc.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE))
			continue;

		char* fileExt = strrchr(pngDesc.cFileName, '.');
		if (!fileExt)
			continue;

		char path[256];
		sprintf(path, "./modloader/%s/png/%s", skin_name, pngDesc.cFileName);

		if (!strcmp(fileExt, ".png")) {
			if (!strcmp(pngDesc.cFileName, "semi.png")) {
				kpzload(path, SEMI_BUFFER_PNG, 800, 600);
			} else if(!strcmp(pngDesc.cFileName, "smg.png")) {
				kpzload(path, SMG_BUFFER_PNG, 800, 600);
			} else if(!strcmp(pngDesc.cFileName, "shotgun.png")) {
				kpzload(path, SHOTGUN_BUFFER_PNG, 800, 600);
			} else if(!strcmp(pngDesc.cFileName, "splash.png")) {
				kpzload(path, SPLASH_BUFFER_PNG, 800, 600);
			} else if(!strcmp(pngDesc.cFileName, "target.png")) {
				kpzload(path, TARGET_BUFFER_PNG, 16, 16);
			}

		} else if (!strcmp(fileExt, ".bmp")) {
			HANDLE bmpfile = LoadImageA(NULL, path, IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION|LR_LOADFROMFILE);

			if (!bmpfile)
				continue;

			int offset = 0;
			if (!strcmp(pngDesc.cFileName, "indicator.bmp")) {
				offset = 0x13b75c8;
			} else if (!strcmp(pngDesc.cFileName, "player.bmp")) {
				offset = 0x13cf838;
			} else if (!strcmp(pngDesc.cFileName, "intel.bmp")) {
				offset = 0x13b1f24;
			} else if (!strcmp(pngDesc.cFileName, "command.bmp")) {
				offset = 0x13b2040;
			} else if (!strcmp(pngDesc.cFileName, "medical.bmp")) {
				offset = 0x13cf804;
			} else if (!strcmp(pngDesc.cFileName, "tracer.bmp")) {
				offset = 0x13b1ca8;
			} else if (!strcmp(pngDesc.cFileName, "health.bmp")) {
				offset = 0x12b1c0c;
			} else if (!strcmp(pngDesc.cFileName, "block.bmp")) {
				offset = 0x13b75a4;
			} else if (!strcmp(pngDesc.cFileName, "semi.bmp")) {
				offset = 0x13b1c20;
			} else if (!strcmp(pngDesc.cFileName, "shotgun.bmp")) {
				offset = 0x13cf7e0;
			} else if (!strcmp(pngDesc.cFileName, "smg.bmp")) {
				offset = 0x13b1e10;
			} else if (!strcmp(pngDesc.cFileName, "grenade.bmp")) {
				offset = 0x13b1ca0;
			}

			if (!offset)
				continue;

			*((long*)(clientBase+offset)) = (int)bmpfile;
		}
	} while(FindNextFile(pngHandle, &pngDesc));
}

void loadSkin(char* skin_name) {
	char directory[256];
	sprintf(directory, "./modloader/%s/kv6", skin_name);

	char kv6filenamesCopy[37][15];
	memcpy(kv6filenamesCopy, kv6filenames, 37*15);
	for (int i = 0; i < 37; i++) {
		wordToLowerCase(kv6filenamesCopy[i]);

		char tmpdir[256];
		sprintf(tmpdir, "%s/%s.kv6", directory, kv6filenamesCopy[i]);
		free((struct aoskv6data*)(clientBase+0x13cf840+i*4)); // im not sure if this is right...

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
			firsttry = 1;
		}

		if (!firsttry && GetFileAttributesA(tmpdir) == INVALID_FILE_ATTRIBUTES)
			continue;

		struct aoskv6data* kv6 = loadkv6(tmpdir);

		if (color && kv6->numvoxs) {
			for (int voxel = 0; voxel < (int)kv6->numvoxs; voxel++) {
				if ((kv6->vox[voxel].col & 0xffffff) == 0) {
					if (color == 'b')
						kv6->vox[voxel].col = *(int*)(clientBase+0x13cfcec);
					else
						kv6->vox[voxel].col = *(int*)(clientBase+0x13cfcfc);
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

		*(int*)(clientBase+0x13cf840+i*4) = (int)kv6;
	}
}

void updateSkinBtn(struct Menu* skinMenu, struct ItemClickableButton* btn) {
	if (!btn->isClicking)
		return;

	if (!strcmp(mt->selected->text, "(none)")) {
		loadkv6Files();
		loadimageFiles();
	} else {
		loadSkin(mt->selected->text);
		loadSkinImages(mt->selected->text);
	}

	loadplayerSkins();
	loadworldObjects();
}

void createModloaderMenu() {
	struct Menu* skinMenu = createMenu(100, 300, 0, "Skins");
	skinMenu->xSize = 100;
	skinMenu->ySize = 150;

	mt = createMultitext(skinMenu, 0xffffff);
	mt->xSize = 100;
	mt->ySize = 100;

	struct ItemClickableButton* saveBtn = createClickableButton(skinMenu, "SAVE", &updateSkinBtn);
	saveBtn->xSize = 100;
	saveBtn->ySize = 25;
	saveBtn->xPos = 0;
	saveBtn->yPos = -30;
	saveBtn->interval = 1;

	do {
		if (!(fileDesc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		if (!strcmp(fileDesc.cFileName, ".") || !strcmp(fileDesc.cFileName, ".."))
			continue;
		addNewText(mt, fileDesc.cFileName);
	} while(FindNextFile(fileHandler, &fileDesc));
	addNewText(mt, "(none)");
}

void initmodloader() {
	fileHandler = FindFirstFileA("./modloader/*", &fileDesc);

	if (fileHandler == INVALID_HANDLE_VALUE)
		return;

	createModloaderMenu();
}
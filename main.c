#include "windows.h"
#include "stdio.h"
#include <stdint.h>
#include <enet/enet.h>
#include <hook.h>
#include <rendering.h>
#include <packets.h>
#include <aos_config.h>
#include <menu.h>
#include <inputs.h>
#include <voxlap.h>
#include <aos_utils.h>

HANDLE clientBase;
struct ItemMultitext* LoggerMultitext;

void testBtnEventHandler(struct Menu* menu, struct ItemClickableButton* btn) {
	if (btn->isClicking) {
		strncpy(btn->text, "YOU MADE IT!", 32);
	} else {
		strncpy(btn->text, "CLICK ME!", 32);
	}
}

DWORD WINAPI LoopFunction(LPVOID lpParam)
{
	HANDLE aos = GetCurrentProcess();
	clientBase = GetModuleHandle(NULL);

	if(clientBase == NULL) {
		printf("Cant load the module\n");
		return 0;
	}

	printf("\n----------------\n");
	printf("DLL Injected :D\n");
	printf("----------------\n");

	setMaxFPS(120);

	//83 7e 0c 02
	// remove the block for packets of length less than 2 so we receive use versionget
	DWORD _old_protect;
	uint8_t* lengthLimit = (uint8_t*)(clientBase+0x343e1);
	VirtualProtect((void*)lengthLimit, 1, PAGE_EXECUTE_READWRITE, &_old_protect);
	*lengthLimit = 1;

	createHook(clientBase, 0x343e4, packetHandler);
	createHook(clientBase, 0x32f00, renderingHookBI);
	createHook(clientBase, 0x334aa, renderingHookAI);
	createHook(clientBase, 0x3126d, hookInputs);

	struct Menu* mfds = createMenu(150, 20, 0, "Just a test");
	mfds->fixedSize = 0;

	struct Menu* meirrita = createMenu(400, 30, 0, "Sliders menu");
	meirrita->fixedSize = 0;

	int randomintfds = 35;
	struct ItemSlide* slidefds = createSlide(meirrita, -35, 150, &randomintfds);
	slidefds->xPos = 5;
	slidefds->yPos = 10;
	slidefds->xSize = 15;
	slidefds->ySize = 50;
	slidefds->showStatus = 1;

	struct ItemText* hptxt = createText(meirrita, -1, 0x30ff30, "Change your health:");
	hptxt->xPos = 25;
	hptxt->yPos = 10;

	struct ItemSlide* slidehp = createSlide(meirrita, 0, 255, (clientBase+0x7ceb4+(*(int*)(clientBase+0x13b1cf0))*0x3a8));
	slidehp->xPos = 25;
	slidehp->yPos = 20;
	slidehp->xSize = 50;
	slidehp->ySize = 15;
	slidehp->sliderColor = 0xffff2020;
	slidehp->backgroundColor = 0x3030ff00;

	struct ItemText* itemfds = createText(mfds, 0, 0xffffff, "muito dasdasd");
	itemfds->xPos = -50;
	itemfds->yPos = -20;

	struct ItemText* itemfds2 = createText(mfds, 1, 0xff55ff, "outro texto sla tlg?");
	struct ItemClickableButton* btnfds = createClickableButton(mfds, "CLICK ME!", &testBtnEventHandler);
	btnfds->xPos = -90;
	btnfds->yPos = -15;

	createTextInput(mfds, 90, 15, 0xffffffff, "Type here ma friend");
	struct ItemTextInput* inpfds = createTextInput(mfds, 90, 15, 0xffffffff, "here too");
	inpfds->xPos = 15;
	inpfds->yPos = -15;

	struct Menu* LoggerMenu = createMenu(300, 200, 0, "Logger");
	LoggerMultitext = createMultitext(LoggerMenu, 0xffffff);
	addNewText(LoggerMultitext, "tf is going on? is it working");
	addNewText(LoggerMultitext, "another item kekw");
	addNewText(LoggerMultitext, "one more");

	loadAoSConfig();
	struct WindowSize fds = getConfigWindowSize();
	printf("%i\n", fds.width);
	printf("%i\n", fds.height);
	printf("%i\n", getConfigVolume());
	printf("%.2f\n", getConfigMouseSensitivity());

	int isMenuHidden = 0;
	while (1) {
		if (GetAsyncKeyState(VK_MENU)) {
			struct aoskv6data* kv6 = loadkv6("./kv6/playerleg.kv6");
			*(int*)(clientBase+0x13cf840+SEMI) = kv6;

			if (isMenuHidden) {
				isMenuHidden = 0;
				showAllMenus();
			} else {
				isMenuHidden = 1;
				hideAllMenus();
			}
		} else if (GetAsyncKeyState(VK_NEXT)) {
			LoggerMultitext->currentPos+=1;
		} else if (GetAsyncKeyState(VK_PRIOR)) {
			if (LoggerMultitext->currentPos > 0)
				LoggerMultitext->currentPos-=1;
		}

		Sleep(250);
	}

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwAttached, LPVOID lpvReserved)
{
	if (dwAttached == DLL_PROCESS_ATTACH) {
		CreateThread(NULL,0,&LoopFunction,NULL,0,NULL);
	}

	return 1;
}
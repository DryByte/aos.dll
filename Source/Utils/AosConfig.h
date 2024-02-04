#include <windows.h>
#include <stdint.h>

extern int clientBase;

struct WindowSize {
	int width;
	int height;
};

struct WindowSize getConfigWindowSize();
int getConfigVolume();
float getConfigMouseSensitivity();
void loadAoSConfig();

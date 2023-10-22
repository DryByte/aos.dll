#include <windows.h>
#include <window.h>
#include <time.h>

extern HANDLE clientBase;

struct customMessage {
	time_t timestamp;
	char msg[255];
};

enum CustomMessageTypes
{
	MESSAGE_STATUS = 3,
	MESSAGE_NOTICE = 4,
	MESSAGE_WARNING = 5,
	MESSAGE_ERROR = 6
};

int getCustomFontSize(int fontid, char* msg);
void drawText(int x, int y, int color, char* msg);
void drawCustomFontText(int x, int y, int color, int fontid, char *msg);
void drawProgressBar(float progress, int progressColor, int backgroundColor);
void addCustomMessage(int type, char* msg);
void drawSquare(int x1, int y1, int x2, int y2, int color);
void renderingHookBI(void);
void renderingHookAI(void);
void setMaxFPS(int fps);
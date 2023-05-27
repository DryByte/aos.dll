#include <windows.h>

extern HANDLE clientBase;

void drawText(int x, int y, int color, char* msg);
void drawCustomFontText(int x, int y, int color, int fontid, char *msg);
void drawLine(int x1, int y1, int x2, int y2, int color);
void addInfoMessage(char* msg);
void renderingHook(void);
void setMaxFPS(int fps);
#include <windows.h>
extern HANDLE clientBase;

	//Screen related functions:
void drawline2d(int x1, int y1, int x2, int y2, int color);
void drawtile(long tf, long tp, long tx, long ty, long tcx, long tcy, long sx, long sy, long xz, long yz, long black);
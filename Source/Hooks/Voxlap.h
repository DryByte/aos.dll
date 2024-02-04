#include <windows.h>
extern int client_base;

	//Screen related functions:
void drawline2d(int x1, int y1, int x2, int y2, int color);
void drawtile(long tf, long tp, long tx, long ty, long tcx, long tcy, long sx, long sy, long xz, long yz, long black);
	
	// sound related functions:
	// these functions actually comes from sysmain.h
void playsound2d(char *filnam, long volperc);

void readmouse(int *fmousx, int *fmousy, int* bstatus);
void getmousechange(int *fmousx, int *fmousy, int* bstatus); // non oficial sysmain.h

long keyread();

struct kv6voxtype { long col; unsigned short z; char vis, dir; };
// this is a custom version of the kv6 data created by ken
struct aoskv6data
{
	long leng, xsiz, ysiz;
	float xpiv, ypiv, zpiv;
	unsigned long numvoxs;
	struct kv6voxtype *vox;
	unsigned long *xlen;
	unsigned short *ylen;
};

struct aoskv6data* loadkv6(char* filnam);
void kpzload(char* filepath, long *pic, int xsiz, int ysiz);
long getcube(long x, long y, long z);
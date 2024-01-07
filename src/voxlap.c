#include <voxlap.h>

void drawline2d(int x1, int y1, int x2, int y2, int color) {
	asm volatile(
		"mov %0, %%edi\n\t"
		"push %5\n\t"
		"push %4\n\t" 
		"push %3\n\t"
		"push %1\n\t"
		"mov %2, %%eax\n\t"
		"add $0x1ef80, %%edi\n\t"
		"call *%%edi\n\t"
		"add $0x10, %%esp"
	:: "r" (clientBase), "g" (x1), "g" (y1), "g" (x2), "g" (y2), "g" (color));
}

//(tf,tp,tx,ty,tcx,tcy): Tile source, (tcx&tcy) is texel (<<16) at (sx,sy)
//(sx,sy,xz,yz) screen coordinates and x&y zoom, all (<<16)
//(black,white): black & white shade scale (ARGB format)
//   Note: if alphas of black&white are same, then alpha channel ignored
void drawtile(long tf, long tp, long tx, long ty, long tcx, long tcy, long sx, long sy, long xz, long yz, long black) {
	asm volatile(
		"push %11\n\t" //black
		"shll $0x10, %10\n\t"
		"push %10\n\t" // zoom
		"shll $0x10, %9\n\t"
		"push %9\n\t" // zoom

		"mov %8, %%edi\n\t" //sy
		"shll $0x10, %%edi\n\t"
		"push %%edi\n\t" 
		"mov %7, %%esi\n\t" // sx
		"shl $0x10, %%esi\n\t"

		"push %6\n\t" // tcy
		"push %5\n\t" // tcx

		"mov %4, %%ecx\n\t" // ty
		"push %3\n\t" // tx
		
		"push %2\n\t" // tp

		"push %1\n\t" // buffer
		"mov %0, %%edi\n\t"
		"mov %%esi, %%eax\n\t"

		"add $0x22390, %%edi\n\t"
		"call *%%edi\n\t"
		"add $0x24, %%esp\n\t"
	:: "r" (clientBase), "g" (tf), "g" (tp), "g" (tx), "g" (ty), "g" (tcx), "g" (tcy), "g" (sx), "g" (sy), "g" (xz), "g" (yz), "g" (black));
}

void playsound2d(char *filnam, long volperc) {
	asm volatile(
		"push %1\n\t"
		"mov %2, %%edi\n\t"
		"add $0x19c30, %0\n\t"
		"call *%0"
	:: "r" (clientBase), "r" (filnam), "r" (volperc));
}

// this function actually comes from winmain.cpp
// in aos, probably the compiler though this is just used once,
// with same address so it managed to set it in the offsets:
//  fmousx  ,  fmousy  ,  fmousz  ,  bstatus
// 0x13b1e14, 0x13cf80c, 0x12b1b58, 0x13b75b0
// so this function will actually do what it supposed to do using
// the parameters
// probably this function is useless, since this is called
// everytime in the main loop
// TODO: figure out why floats doesnt work
void readmouse(int *fmousx, int *fmousy, int* bstatus) {
	asm volatile(
		"mov $0x4192d0, %esi\n\t"
		"call *%esi"
	);

	*fmousx = *(int*)(clientBase+0x13b1e14);
	*fmousy = *(int*)(clientBase+0x13cf80c);
	*bstatus = *(int*)(clientBase+0x13b75b0);
}

// a replacer to readmouse() so this can be used through the
// aos original loop
void getmousechange(int *fmousx, int *fmousy, int* bstatus) {
	*fmousx = *(int*)(clientBase+0x13b1e14);
	*fmousy = *(int*)(clientBase+0x13cf80c);
	*bstatus = *(int*)(clientBase+0x13b75b0);
}

long keyread() {
	long keybuf = (long)clientBase+0x7c770;
	long keybufw = *(long*)(clientBase+0x84acc);
	long* keybufr = (long*)(clientBase+0x84ac8);

	if (*keybufr == keybufw)
		return 0;

	long tores = *(long*)(keybuf+*keybufr*4);
	*keybufr = (*keybufr+1)&255;
	return tores;
}

struct aoskv6data* loadkv6(char* filename) {
	asm volatile(
		"mov %0, %%eax\n\t"
		"call *%1"
	:: "r"(filename), "r"(clientBase+0x257e0));
}
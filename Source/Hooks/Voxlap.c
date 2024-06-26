#include <Voxlap.h>
#include <stdio.h>
#include <io.h>

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
	:: "r" (client_base), "g" (x1), "g" (y1), "g" (x2), "g" (y2), "g" (color));
}

//Draws a 32-bit color texture from memory to the screen. This is the
//   low-level function used to draw text loaded from a PNG,JPG,TGA,GIF.
//     tf: pointer to top-left corner of SOURCE picture
//     tp: pitch (bytes per line) of the SOURCE picture
//  tx,ty: dimensions of the SOURCE picture
//tcx,tcy: texel (<<16) at (sx,sy). Set this to (0,0) if you want (sx,sy)
//            to be the top-left corner of the destination
//  sx,sy: screen coordinate (matches the texture at tcx,tcy)
//  xz,yz: x&y zoom, all (<<16). Use (65536,65536) for no zoom change
//black,white: shade scale (ARGB format). For no effects, use (0,-1)
//   NOTE: if alphas of black&white are same, then alpha channel ignored
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
		
		"mov %2, %%edi\n\t"
		"shll $0x2, %%edi\n\t"
		"push %%edi\n\t" // tp, multiply by 4, since it will always be an int

		"push %1\n\t" // buffer
		"mov %0, %%edi\n\t"
		"mov %%esi, %%eax\n\t"

		"add $0x22390, %%edi\n\t"
		"call *%%edi\n\t"
		"add $0x24, %%esp\n\t"
	:: "r" (client_base), "g" (tf), "g" (tp), "g" (tx), "g" (ty), "g" (tcx), "g" (tcy), "g" (sx), "g" (sy), "g" (xz), "g" (yz), "g" (black));
}

void playsound2d(char *filnam, long volperc) {
	asm volatile(
		"push %1\n\t"
		"mov %2, %%edi\n\t"
		"add $0x19c30, %0\n\t"
		"call *%0"
	:: "r" (client_base), "r" (filnam), "r" (volperc));
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

	*fmousx = *(int*)(client_base+0x13b1e14);
	*fmousy = *(int*)(client_base+0x13cf80c);
	*bstatus = *(int*)(client_base+0x13b75b0);
}

// a replacer to readmouse() so this can be used through the
// aos original loop
void getmousechange(int *fmousx, int *fmousy, int* bstatus) {
	*fmousx = *(int*)(client_base+0x13b1e14);
	*fmousy = *(int*)(client_base+0x13cf80c);
	*bstatus = *(int*)(client_base+0x13b75b0);
}

long keyread() {
	long keybuf = (long)client_base+0x7c770;
	long keybufw = *(long*)(client_base+0x84acc);
	long* keybufr = (long*)(client_base+0x84ac8);

	if (*keybufr == keybufw)
		return 0;

	long tores = *(long*)(keybuf+*keybufr*4);
	*keybufr = (*keybufr+1)&255;
	return tores;
}

struct aoskv6data* loadkv6(char* filename) {
	struct aoskv6data* ret;

	asm volatile(
		"mov %1, %%eax\n\t"
		"call *%2\n\t"
		"mov %%eax, %0"
	: "=r" (ret) : "r"(filename), "r"(client_base+0x257e0));

	return ret;
}

// this comes from kplib, but its bundledwith voxlap
// the original function doesn't work, i guess its because
// fileno function needs to be called with a file descriptor
// opened by the process, not by the dll... So we just rewrite
// what that function does ;)
void kpzload(char* filepath, long *pic, int xsiz, int ysiz) {
	FILE* fp = fopen(filepath, "rb");

	if (!fp)
		return;

	if (pic)
		free(pic);


	int desc = _fileno(fp);
	long len = _filelength(desc);

	int* coolbuf = malloc(len);
	fread(coolbuf, len, 1, fp);
	fclose(fp);

	long loadbuf = (long)malloc(xsiz*4*ysiz);
	*pic = loadbuf;

	int nsize = xsiz*4;

	int result = 0;
	asm volatile(
		"push %5\n\t"
		"push %4\n\t"
		"push %3\n\t"
		"push %2\n\t"
		"push %1\n\t"
		"call *%6\n\t"
		"mov %%eax, %0"
	: "=r" (result) : "g" (coolbuf), "g" (loadbuf), "g" (nsize), "g"(xsiz), "g"(ysiz), "r" (client_base+0x26fd0));

	free(coolbuf);

	if (result < 0) {
		*pic = 0;
		free((long*)loadbuf);
	}
}

// returns 0 if hidden or air
// or returns memory address for cube
long getcube(long x, long y, long z) {
	long ret = 0;

	asm volatile(
		"mov %4, %%edi\n\t"
		"mov %1, %%ecx\n\t"
		"mov %2, %%eax\n\t"
		"push %3\n\t"
		"call *%%edi\n\t"
		"mov %%eax, %0\n\t"
		"add $0x4, %%esp"
		: "=r" (ret) : "g" (x), "g" (y), "g" (z), "r" (client_base+0x1be20));

	return ret;
}
#include <Voxlap.h>
#include <stdio.h>
#include <io.h>

void drawline2d(int x1, int y1, int x2, int y2, int color) {
	register int _y1 asm("eax") = y1;
	register int func_addr asm("edi") = client_base+0x1ef80;

	asm volatile(
		"push 0x18(%%esp)\n\t" // %5
		"push 0x18(%%esp)\n\t" // %4
		"push 0x18(%%esp)\n\t" // %3
		"push 0x14(%%esp)\n\t" // %1
		"call *%0\n\t"
		"add $0x10, %%esp"
	:: "r" (func_addr), "g" (x1), "r" (_y1), "g" (x2), "g" (y2), "g" (color));
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
	register long _sx asm("eax") = sx;
	register long _ty asm("ecx") = ty;

	asm volatile(
		"shl $0x10, %%eax\n\t"

		"push %11\n\t" //black

		"shll $0x10, 0x2C(%%esp)\n\t" // %10
		"push 0x2C(%%esp)\n\t" // zoom

		"shll $0x10, 0x2C(%%esp)\n\t" // %9
		"push 0x2C(%%esp)\n\t" // zoom

		"shll $0x10, 0x2C(%%esp)\n\t" // %8
		"push 0x2C(%%esp)\n\t"

		"push 0x28(%%esp)\n\t" // tcy %6
		"push 0x28(%%esp)\n\t" // tcx %5

		"push 0x24(%%esp)\n\t" // tx %3
		
		"shll $0x2, 0x24(%%esp)\n\t" // %2
		"push 0x24(%%esp)\n\t" // tp, multiply by 4, since it will always be an int

		"push 0x24(%%esp)\n\t" // buffer %1
		"mov %0, %%edi\n\t"

		"add $0x22390, %%edi\n\t"
		"call *%%edi\n\t"
		"add $0x24, %%esp\n\t"
	:: "r" (client_base), "g" (tf), "g" (tp), "g" (tx), "r" (_ty), "g" (tcx), "g" (tcy), "r" (_sx), "g" (sy), "g" (xz), "g" (yz), "g" (black));
}

void playsound2d(char *filnam, long volperc) {
	register long _volperc asm("edi") = volperc;

	asm volatile(
		"push %1\n\t"
		"add $0x19c30, %0\n\t"
		"call *%0\n\t"
		"add $4, %%esp\n\t"
	:: "r" (client_base), "r" (filnam), "r" (_volperc));
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
	register FILE* fp asm("eax") = fopen(filepath, "rb");

	if (!fp)
		return;

	if (pic)
		free(pic);


	int desc = _fileno(fp);
	long len = _filelength(desc);

	int* coolbuf = malloc(len);
	fread(coolbuf, len, 1, fp);
	fclose(fp);

	*pic = (long)malloc(xsiz*4*ysiz);

	int nsize = xsiz*4;

	int result = 0;
	asm volatile(
		"push %5\n\t"
		"push %4\n\t"
		"push %3\n\t"
		"push %2\n\t"
		"push %1\n\t"
		"call *%6\n\t"
		"add $0x14, %%esp\n\t"
	: "=eax" (result) : "r" (coolbuf), "r" (*pic), "g" (nsize), "m"(xsiz), "m"(ysiz), "r" (client_base+0x26fd0));

	free(coolbuf);

	if (result < 0) {
		free((long*)pic);
	}
}

// returns 0 if hidden or air
// or returns memory address for cube
long getcube(long x, long y, long z) {
	register long _x asm("ecx") = x;
	register long _y asm("eax") = y;
	register int func_addr asm("edi") = client_base+0x1be20;

	long ret = 0;

	asm volatile(
		"push %3\n\t"
		"call *%%edi\n\t"
		"add $0x4, %%esp"
		: "=eax" (ret) : "r" (_x), "r" (_y), "g" (z), "r" (func_addr));

	return ret;
}
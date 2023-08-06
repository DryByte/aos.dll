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
		"shl $0x10, %10\n\t"
		"push %10\n\t" // zoom
		"shl $0x10, %9\n\t"
		"push %9\n\t" // zoom

		"mov %8, %%edi\n\t" //sy
		"shl $0x10, %%edi\n\t"
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
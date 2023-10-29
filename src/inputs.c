#include <inputs.h>
#include <windows.h>
#include <menu.h>

extern HANDLE clientBase;

__declspec(naked) void hookInputs() {
	handleKeyboard();

	asm volatile(
		"push %%edi\n\t"
		"movl (0x17b75b0), %%edi\n\t"
		"movl %%edi, (0x17cf6d0)\n\t"
		"add $0x31273, %0\n\t"
		"jmp *%0"
		:: "r" (clientBase));
}
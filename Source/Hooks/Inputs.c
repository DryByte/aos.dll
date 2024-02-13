#include <Inputs.h>
#include <windows.h>
#include <Menu.h>
#include <Macro.h>

extern int client_base;

__declspec(naked) void hook_inputs() {
	handle_keyboard();
	handle_macros();

	asm volatile(
		"push %%edi\n\t"
		"mov %0, %%eax\n\t"
		"mov %%eax, %%edi\n\t"
		"add $0x13b75b0, %%edi\n\t"
		"movl (%%edi), %%edi\n\t"
		"mov %%eax, %%esi\n\t"
		"add $0x13cf6d0, %%esi\n\t"// bstatus
		"movl %%edi, (%%esi)\n\t"
		"add $0x31273, %0\n\t"
		"jmp *%0"
		:: "r" (client_base));
}
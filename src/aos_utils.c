#include <aos_utils.h>

void loadkv6Files() {
	asm volatile(
		"call *%0"
	:: "r"(clientBase+0x33640));
}

// this function from what i saw so far
// is used to assign kv6 files to the right spot
// used always after loadkv6files in statedata
void loadworldObjects() {
	asm volatile(
		"call *%0"
	:: "r"(clientBase+0x292e0));
}
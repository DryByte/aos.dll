file(GLOB HOOK_FILES "./Hooks/*.c")
file(GLOB MACRO_FILES "./Mod/Macro/*.c")
file(GLOB SKINS_FILES "./Mod/Skins/*.c")
file(GLOB UI_FILES "./Mod/UI/*.c")
file(GLOB UTILS_FILES "./Utils/*.c")

set(HOOK_INCLUDE "./Hooks/")
set(MACRO_INCLUDE "./Mod/Macro/")
set(SKINS_INCLUDE "./Mod/Skins/")
set(UI_INCLUDE "./Mod/UI/")
set(UTILS_INCLUDE "./Utils/")

target_sources(aos PRIVATE ${HOOK_FILES} ${MACRO_FILES} ${SKINS_FILES} ${UI_FILES} ${UTILS_FILES} Main.c)
target_include_directories(aos PUBLIC ${HOOK_INCLUDE} ${MACRO_INCLUDE} ${SKINS_INCLUDE} ${UI_INCLUDE} ${UTILS_INCLUDE})
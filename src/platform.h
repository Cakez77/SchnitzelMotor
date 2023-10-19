#pragma once
#include "input.h"
#include "schnitzel_lib.h"

// #############################################################################
//                           Platform Constants
// #############################################################################
constexpr int TRANSIENT_STORAGE_SIZE = MB(128);
constexpr int PERSISTENT_STORAGE_SIZE = MB(256);

// #############################################################################
//                           Platform Globals
// #############################################################################
static bool running = true;
static KeyCodeID KeyCodeLookupTable[MAX_KEYCODES];
static BumpAllocator transientStorage;
static BumpAllocator persistentStorage;
static float musicVolume = 0.25f;

// #############################################################################
//                           Platform Functions
// #############################################################################
void platform_fill_keycode_lookup_table();
bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_func(char* funName);
void platform_swap_buffers();
void platform_set_vsync(bool vSync);
void* platform_load_dynamic_library(const char* dll);
void* platform_load_dynamic_function(void* dll, const char* funName);
bool platform_free_dynamic_library(void* dll);
bool platform_init_audio();
void platform_update_audio(float dt);
void platform_sleep(unsigned int ms);
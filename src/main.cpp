#include "schnitzel_lib.h"
#include "input.h"
#include "sound.h"
#include "game.h"
#include "render_interface.h"


// This is so glcorearb does not include windows.h on Windows
#define APIENTRY
#define GL_GLEXT_PROTOTYPES // This is so we get the function declarations
#include "glcorearb.h"

// #############################################################################
//                           Platform defines
// #############################################################################
// This is the function pointer to update_game in game.cpp
typedef decltype(update_game) update_game_type;

// #############################################################################
//                           Platform Globals
// #############################################################################
bool running = true;
static KeyCodes KeyCodeLookupTable[256];
static update_game_type* update_game_ptr;


// #############################################################################
//                           Platform Functions
// #############################################################################
void platform_fill_keycode_lookup_table();
bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_func(char* funName);
void platform_swap_buffers();
void platform_reaload_dynamic_library();

// #############################################################################
//                           Windows Platform
// #############################################################################
#ifdef _WIN32
#include "win32_platform.cpp"
#elif defined(__APPLE__)
#include "mac_platform.cpp"
#else // Linux
#include "linux_platform.cpp"
#endif

// #############################################################################
//                           Renderer
// #############################################################################
#include "gl_renderer.cpp"

int main()
{
  BumpAllocator allocator = make_bump_allocator(MB(256), true);

  GameState* gameState = (GameState*)bump_alloc(&allocator, sizeof(GameState));

  // Defined in the file render_interface.h
  renderData = (RenderData*)bump_alloc(&allocator, sizeof(RenderData));

  // Defiend in "input.h"
  input = (Input*)bump_alloc(&allocator, sizeof(Input));

  if(!platform_create_window(1200, 720, "Schnitzel Motor"))
  {
    SM_ERROR("Failed to create Windows Window");
    return -1;
  }

  if(!platform_init_sound())
  {
    SM_ERROR("Failed to initialize sound");
  }

  platform_fill_keycode_lookup_table();

  if(!gl_init())
  {
    SM_ERROR("Failed to initialize OpenGL");
    return -1;
  }

  while(running)
  {
    // Load the update_game function pointer from the DLL
    platform_reaload_dynamic_library();

    // Reset Input
    for(int keyIdx = 0; keyIdx < 512; keyIdx++)
    {
      input->keys[keyIdx].justReleased = false;
      input->keys[keyIdx].justPressed = false;
      input->keys[keyIdx].halfTransitionCount = 0;
    }

    platform_update_window();
    update_game(gameState, input, renderData, &allocator, play_sound);
    gl_render();

    // This is platform specific!
    platform_swap_buffers();
  }

  return 0;
}

void update_game(GameState* gameState, Input* inputIn, RenderData* renderDataIn, BumpAllocator* allocator, PlaySoundFunc play_sound)
{
  update_game_ptr(gameState, inputIn, renderDataIn, allocator, play_sound);
}
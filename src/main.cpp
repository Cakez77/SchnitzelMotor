#include "schnitzel_lib.h"
#include "input.h"
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
  int allocationByteOffset = 0;
  char* gameMemory = (char*)malloc(MB(256));
  memset(gameMemory, 0, MB(256));

  GameState * gameState = (GameState*)&gameMemory[allocationByteOffset];
  allocationByteOffset += sizeof(GameState);

  // Defined in the file render_interface.h
  renderData = (RenderData*)&gameMemory[allocationByteOffset];
  allocationByteOffset += sizeof(RenderData);

  // Defiend in "input.h"
  input = (Input*)&gameMemory[allocationByteOffset];
  allocationByteOffset += sizeof(Input);

  if(!platform_create_window(1200, 720, "Schnitzel Motor"))
  {
    SM_ERROR("Failed to create Windows Window");
    return -1;
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
      input->keys[keyIdx].halfTransitionCount = 0;
    }

    platform_update_window();
    update_game(gameState, input, renderData);
    gl_render();

    // This is platform specific!
    platform_swap_buffers();
  }

  return 0;
}

void update_game(GameState* gameState, Input* inputIn, RenderData* renderDataIn)
{
  update_game_ptr(gameState, inputIn, renderDataIn);
}
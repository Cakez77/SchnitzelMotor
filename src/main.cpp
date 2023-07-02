#include "defines.h"
#include "schnitzel_lib.h"
#include "game.cpp"
#include <stdio.h>

// This is so glcorearb does not include windows.h on Windows
#define APIENTRY
#define GL_GLEXT_PROTOTYPES // This is so we get the function declarations
#include "glcorearb.h"

// #############################################################################
//                           Platform Globals
// #############################################################################
bool running = true;

// #############################################################################
//                           Platform Functions
// #############################################################################
bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_func(char* funName);
void platform_swap_buffers();

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
  if(!platform_create_window(1200, 720, "Schnitzel Motor"))
  {
    SM_ERROR("Failed to create Windows Window");
    return -1;
  }

  if(!gl_init())
  {
    SM_ERROR("Failed to initialize OpenGL");
    return -1;
  }

  while(running)
  {
    platform_update_window();
    update_game();
    gl_render();

    // This is platform specific!
    platform_swap_buffers();
  }

  return 0;
}
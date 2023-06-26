#include "defines.h"
#include "schnitzel_lib.h"
#include "game.cpp"

// #############################################################################
//                           Platform Globals
// #############################################################################
extern bool running; 

// #############################################################################
//                           Platform Functions
// #############################################################################
#if defined(_WIN32) || defined(__linux__)
bool platform_create_window(int width, int height, char* title);
void platform_update_window();
#elif defined(__APPLE__)
  #include "mac_platform.h"
#endif

int main()
{
  platform_create_window(1200, 720, "Schnitzel Motor");

  while(running)
  {
    platform_update_window();
    update_game();
  }

  return 0;
}
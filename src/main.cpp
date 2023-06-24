#include "defines.h"
#include <stdio.h>

// #############################################################################
//                           Platform Globals
// #############################################################################
static bool running = true;


// #############################################################################
//                           Platform Functions
// #############################################################################
bool platform_create_window(int width, int height, char* title);
void platform_update_window();


// #############################################################################
//                           Windows Platform
// #############################################################################
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// #############################################################################
//                           Windows Globals
// #############################################################################
static HWND window;

// #############################################################################
//                           Platform Implementations
// #############################################################################
LRESULT CALLBACK windows_window_callback(HWND window, UINT msg,
                                         WPARAM wParam, LPARAM lParam)
{
  LRESULT result = 0;
  
  switch(msg)
  {
    case WM_CLOSE:
    {
      running = false;
      
      break;
    }
    
    default:
    {
      // Let windows handle the default input for now
      result = DefWindowProcA(window, msg, wParam, lParam);
    }
  }
  
  return result;
}

bool platform_create_window(int width, int height, char* title)
{
  HINSTANCE instance = GetModuleHandleA(0);
  
  // Setup and register window class
  HICON icon = LoadIcon(instance, IDI_APPLICATION);
  WNDCLASS wc = {};
  wc.hInstance = instance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = title;
  wc.lpfnWndProc = windows_window_callback;  // Callback for Input into the window
  
  if (!RegisterClassA(&wc))
  {
    return false;
  }
  
  // Ex Style
  int exStyle = WS_EX_APPWINDOW;
  
  // Dw Style (window Styles), 
  // (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
  int dwStyle = WS_OVERLAPPEDWINDOW;
  
  
  window = CreateWindowExA(exStyle,
                           title,       // Class Name, reference window class
                           title,       // acutal Title
                           dwStyle,
                           100,         // x - Pos
                           100,         // y - Pos
                           width,       // width
                           height,      // height
                           null,        // parent
                           null,        // menu
                           instance,    
                           null);       // lpParam
  
  if(window == null)
  {
    return false;
  }
  
  ShowWindow(window, SW_SHOW);
  
  return true;
}

void platform_update_window()
{
  MSG msg;
  
  while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg); // Calls the callback specified when creating the window
  }
}

#else

#endif

int main()
{
  platform_create_window(1200, 720, "Schnitzel Motor");
  
  while(running)
  {
    platform_update_window();
  }
  
  return 0;
}



















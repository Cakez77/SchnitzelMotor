
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "wglext.h"

// #############################################################################
//                           Windows Globals
// #############################################################################
static HWND window;
static HDC dc;

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

  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

  // Windows specific OpenGL function loading
  {
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
      SM_ASSERT(0, "Failed to create Windows Window");
      return false;
    }

    HDC fakeDC = GetDC(window);
    if(!fakeDC)
    {
      SM_ASSERT(0, "Failed to get HDC");
      return false;
    }

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;

    int pixelFormat = ChoosePixelFormat(fakeDC, &pfd);
    if(!pixelFormat)
    {
      SM_ASSERT(0, "Failed to ChoosePixelFormat");
      return false;
    }

    if(!SetPixelFormat(fakeDC, pixelFormat, &pfd))
    {
      SM_ASSERT(0, "Failed to SetPixelFormat");
      return false;
    }

    // Create OpenGL Render Context
    HGLRC fakeRC = wglCreateContext(fakeDC);
    if(!fakeRC)
    {
      SM_ASSERT(0, "Failed to wglCreateContext");
      return false;
    }

    if(!wglMakeCurrent(fakeDC, fakeRC))
    {
      SM_ASSERT(0, "Failed to wglMakeCurrent");
      return false;
    }

    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_func("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_func("wglCreateContextAttribsARB");

    // Clean up the fake stuff
    wglMakeCurrent(fakeDC, 0);
    wglDeleteContext(fakeRC);
    ReleaseDC(window, fakeDC);

    // Can't add reuse the same (Device)Context, we already SetPixelFormat, etc.
    DestroyWindow(window);
  }

  // Setup window and initialize Windows specific OpenGL
  {
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
      SM_ASSERT(0, "Failed to create Windows Window");
      return false;
    }

    // We need this globally, because we will need to call SwapBuffers(DC)!
    dc = GetDC(window);
    if(!dc)
    {
      SM_ASSERT(0, "Failed to get HDC");
      return false;
    }

    const int pixelAttribs[] =
    {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
      WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
      WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
      WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
      WGL_COLOR_BITS_ARB,     32,
      WGL_ALPHA_BITS_ARB,     8,
      WGL_DEPTH_BITS_ARB,     24,
      0 // Terminate with 0, otherwise OpenGL will throw an Error!
    };

    UINT numPixelFormats;
    int pixelFormat = 0;
    if(!wglChoosePixelFormatARB(dc, pixelAttribs,
                                0, // Float List
                                1, // Max Formats
                                &pixelFormat,
                                &numPixelFormats))
    {
      SM_ASSERT(0, "Failed to wglChoosePixelFormatARB");
      return false;
    }

    PIXELFORMATDESCRIPTOR pfd = {0};
    DescribePixelFormat(dc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if(!SetPixelFormat(dc, pixelFormat, &pfd))
    {
      SM_ASSERT(0, "Failed to SetPixelFormat");
      return true;
    }

    const int contextAttribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 3,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0 // Terminate the Array
    };

    HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
    if(!rc)
    {
      SM_ASSERT(0, "Failed to crate Render Context for OpenGL");
      return false;
    }

    if(!wglMakeCurrent(dc, rc))
    {
      SM_ASSERT(0, "Faield to wglMakeCurrent");
      return false;
    }
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

void* platform_load_gl_func(char* funName)
{
  PROC proc = wglGetProcAddress(funName);
  if(!proc)
  {
    SM_ASSERT(0, "Failed to load OpenGL Function: %s", funName);
  }

  return (void*)proc;
}

void platform_swap_buffers()
{
  SwapBuffers(dc);
}

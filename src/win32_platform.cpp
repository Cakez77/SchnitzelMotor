
// #############################################################################
//                           Windows Globals
// #############################################################################
static HWND window;
extern bool running;  // <- declare, but do not define

static PIXELFORMATDESCRIPTOR gldesc;
static int glformat;
static HGLRC glcontext;


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


bool platform_init_opengl(void) 
{
 char *err = 0;

 HINSTANCE instance = GetModuleHandle(0);

 WNDCLASS wc = {0};
 wc.hInstance = instance;
 wc.lpszClassName = "Dummy window class for opengl creation";
 wc.lpfnWndProc = DefWindowProcA;
 if (!RegisterClassA(&wc))
 {
  err = "Failed to register dummy window class";
  DEBUG_BREAK();
 }

 //- Create old opengl context
 {
  HWND dwindow;
  if (!err) 
  {
   dwindow = CreateWindowA(wc.lpszClassName, "Dummy window for opengl", 0,
     0, 0, 0, 0, null, null, instance, null);
   if (dwindow == null) 
   {
    err = "Failed to create a dummy window for creating an old opengl context";
    DEBUG_BREAK();
   }
  }

  HDC dc = null;
  if (!err) 
  {
   dc = GetDC(dwindow);
   if (dc == null) 
   {
    err = "Failed to get a device context from the dummy window";
    DEBUG_BREAK();
   }
  }

  PIXELFORMATDESCRIPTOR desc =
  {
   .nSize = sizeof(desc),
   .nVersion = 1,
   .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
   .iPixelType = PFD_TYPE_RGBA,
   .cColorBits = 24,
  };

  int format = 0;
  if (!err) 
  {
   format = ChoosePixelFormat(dc, &desc);
   if (!format) 
   {
    err ="Failed to choose a pixel format for the dummy window";
    DEBUG_BREAK();
   }
  }

  if (!err) 
  {
   if (!DescribePixelFormat(dc, format, sizeof(desc), &desc))
   {
    err = "Failed to describe the pixel format for the dummy window";
    DEBUG_BREAK();
   }
  }

  if (!err) 
  {
   if (!SetPixelFormat(dc, format, &desc))
   {
    err = "Failed to set pixel format for the dummy window";
    DEBUG_BREAK();
   }
  }

  HGLRC old_rc = null;
  if (!err) 
  {
   old_rc = wglCreateContext(dc);
   if (old_rc == null) 
   {
    err = "Failed to create a dummy opengl render context";
    DEBUG_BREAK();
   }
  }

  if (!err) 
  {
   if (!wglMakeCurrent(dc, old_rc)) 
   {
    err = "Failed to bind the old gl render context to the dummy window";
    DEBUG_BREAK();
   }
  }

#define X(name, type) \
  if (!err) { \
   *(void **)(&name) = (void *)wglGetProcAddress(#name); \
   if (name == 0) \
   { \
    err = "Failed to load wgl arb function: " #name; \
    DEBUG_BREAK(); \
   } \
  }

  wgl_arb_funcs
#undef X

  //- allowing err to disable codepaths by if(!err) makes the clean up code way easier 
  // at the cost of some readibility and performance. 
  if (old_rc) 
  {
   if (!wglDeleteContext(old_rc)) 
   {
    err = "Failed to delete the old render context";
    DEBUG_BREAK();
   }
  }

  if (dc) 
  {
   SM_ASSERT(dwindow, "HUH? We got a dc from a null window");
   if (!ReleaseDC(dwindow, dc)) 
   {
    err = "Failed to release the device context for the dummy window";
    DEBUG_BREAK();
   }
  }

  if (dwindow) 
  {
   if (!DestroyWindow(dwindow)) {
    err = "Failed to destroy the dummy window";
    DEBUG_BREAK();
   }
  }
 }

  //- Create a modern opengl context and load the opengl functions
 {
  HWND dwindow;
  if (!err) 
  {
   dwindow = CreateWindowA(wc.lpszClassName, "Dummy window for opengl", 0,
     0, 0, 0, 0, null, null, instance, null);
   if (dwindow == null) 
   {
    err = "Failed to create a dummy window for creating an old opengl context";
    DEBUG_BREAK();
   }
  }

  HDC dc = null;
  if (!err)
  {
   dc = GetDC(dwindow);
   if (dc == null) {
    err = "Failed to get hdc from dummy window for modern opengl context";
    DEBUG_BREAK();
   }
  }

  int pfd_attribs[] = {
   WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
   WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
   WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
   WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
   WGL_COLOR_BITS_ARB,     32,
   WGL_ALPHA_BITS_ARB,     8,
   WGL_DEPTH_BITS_ARB,     24,
   WGL_STENCIL_BITS_ARB,   8,
   0
  };

  unsigned int number_of_formats = 0;
  if (!err) 
  {
   if (!wglChoosePixelFormatARB(dc, pfd_attribs, null, 1, &glformat, &number_of_formats))
   {
    err = "Failed to choose a pixel format for the opengl modern context";
    DEBUG_BREAK();
   }
  }

  if (!err) 
  {
   if (!DescribePixelFormat(dc, glformat, sizeof(gldesc), &gldesc)) 
   {
    err = "Failed to describe the pixel format for modern context";
    DEBUG_BREAK();
   }
  }

  if (!err)
  {
   if (!SetPixelFormat(dc, glformat, &gldesc))
   {
    err = "Failed to set the pixel format on the dummy window for the modern gl context";
    DEBUG_BREAK();
   }
  }

  int context_attribs[] =
  {
   WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_MAJOR,
   WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_MINOR,
   WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef OPENGL_DEBUG
   WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
   0,
  };

  if (!err) 
  {
   glcontext = wglCreateContextAttribsARB(dc, null, context_attribs);
   if (glcontext == null)
   {
    err = "Failed to create a modern opengl context using wglCreateContextAttribsARB";
    DEBUG_BREAK();
   }
  }

  if (!err) 
  {
   if (!wglMakeCurrent(dc, glcontext)) 
   {
    err = "Failed to bind the new gl render context to the dummy window";
    DEBUG_BREAK();
   }
  }

#define X(name, type) \
  if (!err) { \
   *(void **)(&name) = (void *)wglGetProcAddress(#name); \
   if (name == 0) { \
    err = "Failed to load gl func: " #name; \
    DEBUG_BREAK(); \
   } \
  } 
  gl_funcs
#undef X

  if (dc)
  {
   SM_ASSERT(dwindow, "How did we get a dc here from null window?");
   if (!ReleaseDC(dwindow, dc)) {
    err = "Failed to release the device context for dummy window";
   }
  }
  if (dwindow)
  {
   if (!DestroyWindow(dwindow))
   {
    err = "Failed to destroy dummy window for modern opengl context";
    DEBUG_BREAK();
   }
  }
 }

 if (wc.lpszClassName)
 {
  if (!UnregisterClassA(wc.lpszClassName, instance))
  {
   err = "Failed to unregister class";
   DEBUG_BREAK();
  }
 }

 return (bool)(!err);
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
  int exStyle = 0;
  // Dw Style (window Styles), 
  // (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
  int dwStyle = WS_OVERLAPPEDWINDOW;
  SM_ASSERT(window == null, "Already created a window");
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

  HDC dc = GetDC(window);
  if (dc == null) { return false; }

  {
   if (!SetPixelFormat(dc, glformat, &gldesc)) { return false; }
   SM_ASSERT(glcontext, "A modern opengl context hasn't been initialized");
   if (!wglMakeCurrent(dc, glcontext)) { return false; }
  }

  if (!ReleaseDC(window, dc)) { return false; }
  
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

void platform_swap_buffers()
{
 HDC dc = GetDC(window);
 SwapBuffers(dc);
 ReleaseDC(window, dc);
}

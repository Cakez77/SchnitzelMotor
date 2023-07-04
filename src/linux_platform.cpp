
#include <X11/Xlib.h>
#include <GL/glx.h>

// #############################################################################
//                           Windows Globals
// #############################################################################
extern bool running;  // <- declare, but do not define
static Display* display;
static Atom wmDeleteWindow;
static Window window;

void platform_fill_keycode_lookup_table()
{
  // KeyCode lookup Table
  {
    // first; set unmapped keys to KEY_COUNT to detect unmapped, for now
    for (int i = 0; i < 65536; ++i)
    {
      KeyCodeLookupTable[i] = KEY_COUNT;
    }
    // map the ones you need
    KeyCodeLookupTable[XK_space] = KEY_SPACE;
    KeyCodeLookupTable[XK_Escape] = KEY_ESCAPE;
    KeyCodeLookupTable[XK_a] = KEY_A;
    KeyCodeLookupTable[XK_b] = KEY_B;
    KeyCodeLookupTable[XK_c] = KEY_C;
    KeyCodeLookupTable[XK_d] = KEY_D;
    KeyCodeLookupTable[XK_e] = KEY_E;
    KeyCodeLookupTable[XK_f] = KEY_F;
    KeyCodeLookupTable[XK_g] = KEY_G;
    KeyCodeLookupTable[XK_h] = KEY_H;
    KeyCodeLookupTable[XK_i] = KEY_I;
    KeyCodeLookupTable[XK_j] = KEY_J;
    KeyCodeLookupTable[XK_k] = KEY_K;
    KeyCodeLookupTable[XK_l] = KEY_L;
    KeyCodeLookupTable[XK_m] = KEY_M;
    KeyCodeLookupTable[XK_n] = KEY_N;
    KeyCodeLookupTable[XK_o] = KEY_O;
    KeyCodeLookupTable[XK_p] = KEY_P;
    KeyCodeLookupTable[XK_q] = KEY_Q;
    KeyCodeLookupTable[XK_r] = KEY_R;
    KeyCodeLookupTable[XK_s] = KEY_S;
    KeyCodeLookupTable[XK_t] = KEY_T;
    KeyCodeLookupTable[XK_u] = KEY_U;
    KeyCodeLookupTable[XK_v] = KEY_V;
    KeyCodeLookupTable[XK_w] = KEY_W;
    KeyCodeLookupTable[XK_x] = KEY_X;
    KeyCodeLookupTable[XK_y] = KEY_Y;
    KeyCodeLookupTable[XK_z] = KEY_Z;
  }
}

// #############################################################################
//                           Platform Implementations
// #############################################################################
bool platform_create_window(int width, int height, char* title)
{
  display = XOpenDisplay(NULL);
  if (display == NULL)
  {
      // Handle error if display cannot be opened
      return 1;
  }

  int screenId = DefaultScreen(display);
  Screen* screen = DefaultScreenOfDisplay(display);
  Window root = RootWindowOfScreen(screen);

  // Linux specific OpenGL setup
  {
    int pixelAttribs [] =
    {
      GLX_RGBA,         True,
      GLX_DOUBLEBUFFER, True,
      GLX_DEPTH_SIZE,   24,
      GLX_STENCIL_SIZE, 8,
      GLX_RED_SIZE, 		8,
      GLX_GREEN_SIZE,		8,
      GLX_BLUE_SIZE,		8,
      GLX_ALPHA_SIZE,		8,
      None,
    };

    XVisualInfo* visualInfo = glXChooseVisual(display, screenId, pixelAttribs);
    if(!visualInfo)
    {
      SM_ASSERT(0, "Failed to glXChooseVisual");
      return false;
    }

    XSetWindowAttributes windowAttribs;
    windowAttribs.border_pixel = BlackPixel(display, screenId);
    windowAttribs.background_pixel = WhitePixel(display, screenId);
    windowAttribs.override_redirect = True;
    windowAttribs.colormap = XCreateColormap(display, root, visualInfo->visual, AllocNone);
    windowAttribs.event_mask = ExposureMask;
    window = XCreateWindow(display, root, 0, 0, width, height, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask | KeymapStateMask | ExposureMask);

    XStoreName(display, window, title);
    XClearWindow(display, window);
    XMapRaised(display, window);

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB  =
      (PFNGLXCREATECONTEXTATTRIBSARBPROC)platform_load_gl_func("glXCreateContextAttribsARB");

    int contextAttribs [] =
    {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
	    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
      GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
	    None
    };

    int fbCount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), pixelAttribs, &fbCount);
    GLXFBConfig bestFbc = fbc[0];

    GLXContext rc = glXCreateContextAttribsARB(display, bestFbc, 0, true, contextAttribs);
    if(!rc)
    {
      SM_ASSERT(0, "Failed to glXCreateContext");
      return false;
    }

    if(!glXMakeCurrent(display, window, rc))
    {
      SM_ASSERT(0, "Failed to glXMakeCurrent");
      return false;
    }

    // Tell the server to notify us when the window manager attempts to destroy the window
    wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteWindow, 1);
  }

  return true;
}

static void handle_keyevent(KeySym keySym, bool isDown)
{
  KeyCodes keyCode = KeyCodeLookupTable[keySym];
  if (keyCode >= KEY_COUNT)
  {
    SM_TRACE("Pressed unmapped key: %u", keySym);
    return;
  }
  input.keys[keyCode].isDown = isDown;
  input.keys[keyCode].halfTransitionCount++;
}

void platform_update_window()
{
  while (XPending(display))
  {
    XEvent event;
    XNextEvent(display, &event);
    switch (event.type)
    {
      case KeymapNotify:
        // Received new keyboard layout
        XRefreshKeyboardMapping(&event.xmapping);
        break;
      case KeyPress:
      {
        // Lookup key mapping for pressed key
        KeySym keySym = XLookupKeysym(&event.xkey, 0);
        handle_keyevent(keySym, true);
      } break;
      case KeyRelease:
      {
        // Lookup key mapping for released key
        KeySym keySym = XLookupKeysym(&event.xkey, 0);
        handle_keyevent(keySym, false);
      } break;
      case Expose:
      {
        // Screen resized
        input.screenSize.x = event.xexpose.width;
        input.screenSize.y = event.xexpose.height;
      } break;
      case ClientMessage:
      {
        Atom wmProtocol = XInternAtom(display, "WM_PROTOCOLS", False);
        if (event.xclient.message_type == wmProtocol &&
          event.xclient.data.l[0] == wmDeleteWindow)
        {
          // Window close event received
          running = false;
        }
      } break;
    }
  }
}

void* platform_load_gl_func(char* funName)
{
  void* proc = (void*)glXGetProcAddress((const GLubyte*)funName);
  if(!proc)
  {
    SM_ASSERT(0, "Failed to load OpenGL Function: %s", funName);
  }

  return proc;
}

void platform_swap_buffers()
{
  glXSwapBuffers(display, window);
}



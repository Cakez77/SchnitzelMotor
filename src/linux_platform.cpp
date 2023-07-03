
#include <X11/Xlib.h>
#include <GL/glx.h>

// #############################################################################
//                           Windows Globals
// #############################################################################
extern bool running;  // <- declare, but do not define
static Display* display;
static Atom wmDeleteWindow;
static Window window;

// #############################################################################
//                           Platform Implementations
// #############################################################################
bool platform_create_window(int width, int height, char* title)
{
  display = XOpenDisplay(NULL);
  if (display == NULL) {
      // Handle error if display cannot be opened
      return 1;
  }

  int screen = DefaultScreen(display);
  Window root = RootWindow(display, screen);

  XSetWindowAttributes swa;
  swa.event_mask = ExposureMask;

  window = XCreateWindow(display, root, 0, 0, width, height, 0,
                         CopyFromParent, InputOutput, CopyFromParent,
                         CWEventMask, &swa);

  XMapWindow(display, window);
  XStoreName(display, window, title);

  // Linux specific OpenGL setup
  {
    int pixelAttribs [] =
    {
      GLX_RGBA, 
      GLX_DEPTH_SIZE,   24,
      GLX_RED_SIZE, 		8,
      GLX_GREEN_SIZE,		8,
      GLX_BLUE_SIZE,		8,
      GLX_ALPHA_SIZE,		8,

      GLX_DOUBLEBUFFER, 
      None,
    };
    int fbCount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), pixelAttribs, &fbCount);
    // XVisualInfo* visualInfo = glXChooseVisual(display, screen, pixelAttribs);
    // if(!visualInfo)
    // {
      // SM_ASSERT(0, "Failed to glXChooseVisual");
      // return false;
    // }

    GLXFBConfig bestFbc = fbc[0];

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB  =
      (PFNGLXCREATECONTEXTATTRIBSARBPROC)platform_load_gl_func("glXCreateContextAttribsARB");

    int contextAttribs [] =
    {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
	    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
	    //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
	    None
    };

    GLXContext rc = glXCreateContextAttribsARB(display, bestFbc, 0, True, contextAttribs);
    // GLXContext rc = glXCreateContext(display, visualInfo, NULL, true);
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
  }

  XRaiseWindow(display, window); // Raise the window to the top
  XFlush(display); // Flush the output buffer

  // Tell the server to notify us when the window manager attempts to destroy the window
  wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteWindow, 1);

  return true;
}

void platform_update_window()
{
  while (XPending(display))
  {
    XEvent event;
    XNextEvent(display, &event);
    if (event.type == Expose)
    {
      // When the window is resied
    }
    else if (event.type == KeyPress)
    {
        // Handle key press event
        break;
    }
    else if (event.type == ClientMessage)
    {
      // Client message event: Window close event
      Atom wmProtocol = XInternAtom(display, "WM_PROTOCOLS", False);
      if (event.xclient.message_type == wmProtocol &&
          event.xclient.data.l[0] == wmDeleteWindow)
      {
          // Window close event received
          running = false;
          break;
      }
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



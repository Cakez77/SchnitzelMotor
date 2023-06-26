
#include <X11/Xlib.h>

static Display* display;
static Atom wmDeleteWindow;
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

  Window window = XCreateWindow(display, root, 0, 0, width, height, 0,
                                CopyFromParent, InputOutput, CopyFromParent,
                                CWEventMask, &swa);

  XMapWindow(display, window);
  XStoreName(display, window, title);
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
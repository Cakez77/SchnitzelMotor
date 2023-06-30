//- OS specific code for creating a modern opengl context

#include <GL/gl.h>

#if _WIN32
# include "w32/wglext.h"
# include "w32/wgl_arb_funcs.h"
#elif __APPLE__
# error "opengl not implemented on apple yet"
#else
# error "opengl not implemented on linux yet"
#endif

//- OS agnostic code that contains stuff about all the opengl extensions
#include "khronos/glcorearb.h"

//- OS agnostic function pointer declaration and X macro table for desired functions
#include "gl_funcs.h"




#define wgl_arb_funcs \
X(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC) \
X(wglCreateContextAttribsARB,     PFNWGLCREATECONTEXTATTRIBSARBPROC) \

#define X(name, type) type name = 0;
 wgl_arb_funcs
#undef X




//- This file contains a table of X macros that can be used for scuffed code generation.
// If you need a specific opengl function all you have to do is include it into the table
// and it should get loaded on startup. The X macro contains a "name" and a "type", the 
// "name" is the name of the function and the "type" is the function pointer type that's
// defined in the khronos opengl headers inside this directory. The type for any gl function
// is "PFN FUNCNAME PROC"

#define gl_funcs \
 X(glGenBuffers, PFNGLGENBUFFERSPROC) \
 X(glBindBuffer, PFNGLBINDBUFFERPROC) \
 X(glBufferData, PFNGLBUFFERDATAPROC) \
 X(glCreateShader, PFNGLCREATESHADERPROC) \
 X(glDeleteShader, PFNGLDELETESHADERPROC) \
 X(glShaderSource, PFNGLSHADERSOURCEPROC) \
 X(glCompileShader, PFNGLCOMPILESHADERPROC) \
 X(glGetShaderiv, PFNGLGETSHADERIVPROC) \
 X(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC) \
 X(glCreateProgram, PFNGLCREATEPROGRAMPROC) \
 X(glAttachShader, PFNGLATTACHSHADERPROC) \
 X(glLinkProgram, PFNGLLINKPROGRAMPROC) \
 X(glGetProgramiv, PFNGLGETPROGRAMIVPROC) \
 X(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC) \
 X(glUseProgram, PFNGLUSEPROGRAMPROC) \
 X(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC) \
 X(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC) \
 X(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC) \
 X(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC) \
 




#define X(name, type) type name = 0;
 gl_funcs
#undef X

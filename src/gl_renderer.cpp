#include "gl_renderer.h"

struct GLContext
{
  int programID;
};

static GLContext glContext;

static void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                         GLsizei length, const GLchar* message, const void* user)
{
  if(severity == GL_DEBUG_SEVERITY_MEDIUM ||
     severity == GL_DEBUG_SEVERITY_HIGH)
  {
    SM_ASSERT(0, "OpenGL Error: %s", message);
  }
}

bool gl_init()
{
  load_gl_functions();

  glDebugMessageCallback(&gl_debug_callback, 0);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

  glDebugMessageCallback(&gl_debug_callback, 0);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

  int vertShaderID = glCreateShader(GL_VERTEX_SHADER);
  int fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  int fileSize = 0;
  char* vertShaderSource = read_file("assets/shaders/quad.vert", &fileSize);
  if(!vertShaderSource)
  {
    SM_ASSERT(0, "Failed to load Shader: %s", "assets/shaders/quad.vert");
    return false;
  }
  glShaderSource(vertShaderID, 1, &vertShaderSource, &fileSize);
  glCompileShader(vertShaderID);


  char* fragShaderSource = read_file("assets/shaders/quad.frag", &fileSize);
  if(!fragShaderSource)
  {
    SM_ASSERT(0, "Failed to load Shader: %s", "assets/shaders/quad.frag");
    return false;
  }
  glShaderSource(fragShaderID, 1, &fragShaderSource, &fileSize);
  glCompileShader(fragShaderID);

  glContext.programID = glCreateProgram();
  glAttachShader(glContext.programID, vertShaderID);
  glAttachShader(glContext.programID, fragShaderID);
  glLinkProgram(glContext.programID);

  // This is preemtively, because they are still bound
  // They are already marked for deletion tho
  glDetachShader(glContext.programID, vertShaderID);
  glDetachShader(glContext.programID, fragShaderID);
  glDeleteShader(vertShaderID);
  glDeleteShader(fragShaderID);

  // This needs to be bound, otherwise OpenGL doesn't draw anything
  // We won't use it tho!
  int VAO = 0;
  glGenVertexArrays(1, (GLuint*)&VAO);
  glBindVertexArray(VAO);

  glUseProgram(glContext.programID);

  return true;
}

void gl_render()
{
  // Linux Colors, kinda
  glClearColor(0.2f, 0.02f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}








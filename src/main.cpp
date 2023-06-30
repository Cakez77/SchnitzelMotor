#include "defines.h"
#include "schnitzel_lib.h"
#include "game.cpp"

#include <stdio.h>

// #############################################################################
//                           Platform Globals
// #############################################################################
bool running = true; 

// #############################################################################
//                           Platform Functions
// #############################################################################
bool platform_create_window(int width, int height, char* title);
void platform_update_window();

// #############################################################################
//                           Windows Platform
// #############################################################################

#define OPENGL_DEBUG
#define OPENGL_MAJOR 4
#define OPENGL_MINOR 5


#ifdef _WIN32
 #define WIN32_LEAN_AND_MEAN
 #define NOMINMAX
 #include <windows.h>

 #include "opengl/opengl_inc.h"

 #include "win32_platform.cpp"
#elif defined(__APPLE__)
 #error "opengl isn't included for macos"
#include "mac_platform.cpp"
#else // Linux
 #error "opengl isn't included for linux"
 #include "linux_platform.cpp"
#endif

char *vertShaderCode = {
 "#version 330 core\n"
 "layout (location = 0) in vec3 input_pos;\n"
 "\n"
 "void main() {\n"
 " gl_Position = vec4(input_pos.x, input_pos.y, input_pos.z, 1.0);\n"
 "}\n"
};

char *fragShaderCode = {
 "#version 330 core\n"
 "out vec4 frag_color;\n"
 "\n"
 "void main() {\n"
 " frag_color = vec4(1.f, .5f, .2f, 1.f);\n"
 "}\n"
};

int main()
{
  bool initOpengl = platform_init_opengl();
  SM_ASSERT(initOpengl, "Failed to init opengl for platform");

  int windowWidth = 1280, windowHeight = 720;
  bool windowCreated = platform_create_window(windowWidth, windowHeight, "Schnitzel Motor");
  SM_ASSERT(windowCreated, "Failed to create the platform window");


  unsigned int vertShader;
  vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertShader, 1, &vertShaderCode, null);
  glCompileShader(vertShader);

  //- Check for vert  compile errors
  {
   int  success;
   glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
   if (!success)
   {
    char infoLog[512];
    glGetShaderInfoLog(vertShader, 512, null, infoLog);
    SM_ASSERT(false, "Failed to compile vertex shader");
   }
  }

  unsigned int fragShader;
  fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShader, 1, &fragShaderCode, null);
  glCompileShader(fragShader);

  //- Check for frag complie errors
  {
   int  success;
   glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
   if (!success)
   {
    char infoLog[512] = {0};
    glGetShaderInfoLog(fragShader, 512, null, infoLog);
    SM_ASSERT(false, "Failed to compile fragment shader");
   }
  }

  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();

  glAttachShader(shaderProgram, vertShader);
  glAttachShader(shaderProgram, fragShader);
  glLinkProgram(shaderProgram);

  {
   int success;
   glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
   if(!success) {
    char infoLog[512] = {0};
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    SM_ASSERT(false, "Failed to link shader program");
   }
  }

  glUseProgram(shaderProgram);

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  float vertices[] = {
   -0.5f, -0.5f, 0.0f,
   0.5f, -0.5f, 0.0f,
   0.0f,  0.5f, 0.0f
  }; 
  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glViewport(0, 0, windowWidth, windowHeight);
  while(running)
  {
    platform_update_window();
    update_game();

    glClearColor(.2f, .3f, .3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    platform_swap_buffers();
  }


  return 0;
}

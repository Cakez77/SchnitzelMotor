#include "gl_renderer.h"
#include "schnitzel_lib.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct GLContext
{
  char* texturePath;
  int programID;
  int transformSBOID;
  int screenSizeID;
  int textureID;
  long long textureTimestamp;
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

bool gl_init(BumpAllocator* transientStorage)
{
  load_gl_functions();

  glDebugMessageCallback(&gl_debug_callback, 0);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

  int vertShaderID = glCreateShader(GL_VERTEX_SHADER);
  int fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  int fileSize = 0;
  char* vertShaderSource = read_file("assets/shaders/quad.vert", &fileSize, transientStorage);
  if(!vertShaderSource)
  {
    SM_ASSERT(0, "Failed to load Shader: %s", "assets/shaders/quad.vert");
    return false;
  }
  glShaderSource(vertShaderID, 1, &vertShaderSource, &fileSize);
  glCompileShader(vertShaderID);

  // Validate if the Shader works
  {
    int success;
    char shaderLog[1024] = {};
    glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(vertShaderID, 1024, 0, shaderLog);
      SM_ASSERT(0, "Failed to compile shader: %s", shaderLog);
    }
  }

  char* fragShaderSource = read_file("assets/shaders/quad.frag", &fileSize, transientStorage);
  if(!fragShaderSource)
  {
    SM_ASSERT(0, "Failed to load Shader: %s", "assets/shaders/quad.frag");
    return false;
  }
  glShaderSource(fragShaderID, 1, &fragShaderSource, &fileSize);
  glCompileShader(fragShaderID);
  // Validate if the Shader works
  {
    int success;
    char shaderLog[1024] = {};
    glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(fragShaderID, 1024, 0, shaderLog);
      SM_ASSERT(0, "Failed to compile shader: %s", shaderLog);
    }
  }

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

  // Transform Storage Buffer
  {
    glGenBuffers(1, (GLuint*)&glContext.transformSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * MAX_TRANSFORMS,
                 renderData->transforms.elements, GL_DYNAMIC_DRAW);
  }

  // Screen Size Uniform
  {
    glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
  }

  // Load our first Texture
  {
    int width, height, nChannels;
    glContext.texturePath = "assets/textures/Texture_Atlas_01.png";
    char* data = (char*)stbi_load(glContext.texturePath, 
                                  &width, &height, &nChannels, 4);
    int textureSizeInBytes = 4 * width * height;

    glContext.textureTimestamp = get_timestamp(glContext.texturePath);

    if(!data)
    {
      SM_ASSERT(0, "Failed to load Texture!");
      return false;
    }

    glContext.textureTimestamp = get_timestamp(glContext.texturePath);

    glGenTextures(1, (GLuint*)&glContext.textureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glContext.textureID);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // This setting only matters when using the GLSL texture() function
    // When you use texelFetch() this setting has no effect,
    // because texelFetch is designed for this purpose
    // See: https://interactiveimmersive.io/blog/glsl/glsl-data-tricks/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }

  glUseProgram(glContext.programID);

  // sRGB output (even if input texture is non-sRGB -> don't rely on texture used)
  // Your font is not using sRGB, for example (not that it matters there, because no actual color is sampled from it)
  // But this could prevent some future bug when you start mixing different types of textures
  // Of course, you still need to correctly set the image file source format when using glTexImage2D()
  glEnable(GL_FRAMEBUFFER_SRGB);
  glDisable(0x809D); // disable multisampling

  return true;
}

void gl_render()
{
  // Hot reload texure
  long long textureTimestamp = get_timestamp(glContext.texturePath);
  if(textureTimestamp > glContext.textureTimestamp)
  {
    int width, height, nChannels;
    char* data = (char*)stbi_load(glContext.texturePath, 
                                  &width, &height, &nChannels, 4);
    if(data)
    {
      int textureSizeInBytes = 4 * width * height;
    

      glContext.textureTimestamp = textureTimestamp;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    }
  }

  // Linux Colors, kinda
  glClearColor(0.2f, 0.02f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, input->screenSize.x, input->screenSize.y);

  // Copy screenSize to the GPU
  glUniform2fv(glContext.screenSizeID, 1, &input->screenSize.x);

  // Copy transforms to the GPU
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Transform) * MAX_TRANSFORMS,
                  renderData->transforms.elements);
  // Reset for next Frame

  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->transforms.count);
  renderData->transforms.clear();
}
















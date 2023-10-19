#include "gl_renderer.h"
#include "schnitzel_lib.h"
#include "render_interface.h"

// To Load PNG Files
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// To Load TTF Files
#include <ft2build.h>
#include FT_FREETYPE_H

// #############################################################################
//                           OpenGL Constants
// #############################################################################
const char* TEXTURE_PATH = "assets/textures/Texture_Atlas_01.png";
const int ERROR_LOG_SIZE = 2048;

// #############################################################################
//                           OpenGL Structs
// #############################################################################
struct GLContext
{
  int programID;
#ifdef USE_OPENGL410
  int vaoID; // Vertex Array Object
  int vboID; // Vertex Buffer Object
#else
  int transformSBOID; // Shader Storage Buffer Object
#endif
  int screenSizeID;
  int projectionID;
  int textureID;
  int fontAtlasID;
  long long textureTimestamp;
};

// #############################################################################
//                           OpenGL Globals
// #############################################################################
static GLContext glContext;
static char error_log[ERROR_LOG_SIZE] = {};

// #############################################################################
//                           Render Interface Implementations
// #############################################################################
void load_font(char* filePath, int fontSize)
{
  FT_Library fontLibrary;
  FT_Init_FreeType(&fontLibrary);

  FT_Face fontFace;
  FT_New_Face(fontLibrary, filePath, 0, &fontFace);
  FT_Set_Pixel_Sizes(fontFace, 0, fontSize);

  int padding = 2;
  int row = 0;
  int col = padding;

  const int textureWidth = 512;
  char textureBuffer[textureWidth * textureWidth];
  for (FT_ULong glyphIdx = 32; glyphIdx < 127; ++glyphIdx)
  {
    FT_UInt glyphIndex = FT_Get_Char_Index(fontFace, glyphIdx);
    FT_Load_Glyph(fontFace, glyphIndex, FT_LOAD_DEFAULT);
    FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL);

    if (col + fontFace->glyph->bitmap.width + padding >= 512)
    {
      col = padding;
      row += fontSize;
    }

    for (unsigned int y = 0; y < fontFace->glyph->bitmap.rows; ++y)
    {
      for (unsigned int x = 0; x < fontFace->glyph->bitmap.width; ++x)
      {
        textureBuffer[(row + y) * textureWidth + col + x] =
            fontFace->glyph->bitmap.buffer[y * fontFace->glyph->bitmap.width + x];
      }
    }
    // clang-format off
    Glyph* glyph = &renderData->glyphs[glyphIdx];
     glyph->textureCoords = 
    {
      (float)col, 
      (float)row
    };
    glyph->size = 
    { 
      (float)fontFace->glyph->bitmap.width, 
      (float)fontFace->glyph->bitmap.rows 
    };
    glyph->advance = 
    {
      (float)(fontFace->glyph->advance.x >> 6), 
      (float)(fontFace->glyph->advance.y >> 6)
    };
    glyph->offset =
    {
      (float)fontFace->glyph->bitmap_left,
      (float)fontFace->glyph->bitmap_top,
    };
    // clang-format on
    col += fontFace->glyph->bitmap.width + padding;
  }

  FT_Done_Face(fontFace);
  FT_Done_FreeType(fontLibrary);

  // Upload OpenGL Texture
  {
    glGenTextures(1, (GLuint*)&glContext.fontAtlasID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, glContext.fontAtlasID);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_R8, textureWidth, textureWidth, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)textureBuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
}

// #############################################################################
//                           OpenGL Functions
// #############################################################################

// Debug callback is not supported in OpenGL 4.1
#ifndef USE_OPENGL410
static void APIENTRY gl_debug_callback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user)
{
  if (severity == GL_DEBUG_SEVERITY_LOW || 
      severity == GL_DEBUG_SEVERITY_MEDIUM || 
      severity == GL_DEBUG_SEVERITY_HIGH)
  {
    SM_ASSERT(0, "OpenGL Error: %s", message);
  }
  else
  {
    SM_TRACE((char*)message);
  }
}
#endif

void get_shader_info_log(GLuint shader)
{
  GLsizei log_length = 0;
  glGetShaderInfoLog(shader, ERROR_LOG_SIZE, &log_length, error_log);
  if (log_length > 0)
  {
    // Handle or output the error log as needed
    SM_ASSERT(false, "Shader compilation failed, Error: %s", error_log);
  }
}

GLuint gl_create_shader(int shaderType, char* shaderPath, BumpAllocator* transientStorage)
{
  int fileSize = 0;
  char* shaderHeader = read_file("src/shader_header.h", &fileSize, transientStorage);
  char* shaderSource = read_file(shaderPath, &fileSize, transientStorage);
  if (!shaderHeader)
  {
    SM_ASSERT(false, "Failed to load shader_header.h");
    return 0;
  }
  if (!shaderSource)
  {
    SM_ASSERT(false, "Failed to load shader: %s", shaderPath);
    return 0;
  }

#ifdef USE_OPENGL410
  char* shaderSources[] = {"#version 410 core\r\n", shaderHeader, shaderSource};
#else
  char* shaderSources[] = {"#version 430 core\r\n", shaderHeader, shaderSource};
#endif
  GLuint shaderID = glCreateShader(shaderType);
  glShaderSource(shaderID, ArraySize(shaderSources), shaderSources, 0);

  glCompileShader(shaderID);

  // Test if Shader compiled successfully
  {
    int success;

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      get_shader_info_log(shaderID);
      return 0;
    }
  }

  return shaderID;
}

// #############################################################################
//                           OpenGL Initialization
// #############################################################################
const char* get_gl_error_string(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:
        return "No error";
    case GL_INVALID_ENUM:
        return "Invalid enum";
    case GL_INVALID_VALUE:
        return "Invalid value";
    case GL_INVALID_OPERATION:
        return "Invalid operation";
    case GL_STACK_OVERFLOW:
        return "Stack overflow";
    case GL_STACK_UNDERFLOW:
        return "Stack underflow";
    case GL_OUT_OF_MEMORY:
        return "Out of memory";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "Invalid framebuffer operation";
    default:
        return "Unknown error";
    }
}


void check_link_errors(GLuint programID)
{
  GLint success;
  glGetProgramiv(programID, GL_LINK_STATUS, &success);
  if (!success)
  {
    GLint maxLength = 0;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);
    maxLength = min(maxLength, ERROR_LOG_SIZE); 
    glGetProgramInfoLog(programID, maxLength, &maxLength, error_log);
    glDeleteProgram(programID);  // program is useless now
    SM_ASSERT(false, "Shader Linking failed, Error: %s", error_log);
  }
}

#ifdef USE_OPENGL410
// Create a Vertex Array Object (VAO) and a Vertex Buffer Object (VBO)
void create_vertex_array_buffers()
{
  // clear errors
  while (glGetError() != GL_NO_ERROR)
    ;

  // Initialize OpenGL buffers
  glGenVertexArrays(1, (GLuint*)&glContext.vaoID);
  glBindVertexArray(glContext.vaoID);

  GLsizei maxTransforms = MAX_TRANSFORMS;
  GLsizei totalSize = maxTransforms * sizeof(Transform);

  glGenBuffers(1, (GLuint*)&glContext.vboID);
  glBindBuffer(GL_ARRAY_BUFFER, glContext.vboID);
  glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);

  SM_ASSERT(glGetError() == GL_NO_ERROR,
            "Failed to allocate memory for Vertex Buffer: %s",
            get_gl_error_string(glGetError()));
}
#endif

void create_gl_buffers()
{

#ifdef USE_OPENGL410
  create_vertex_array_buffers();
  GLsizei stride = sizeof(Transform);

  // inPosition (location = 0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Transform, pos));
  glEnableVertexAttribArray(0);
  glVertexAttribDivisor(0, 1);

  // inSize (location = 1)
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Transform, size));
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1);

  // inAtlasOffset (location = 2)
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Transform, atlasOffset));
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2, 1);

  // inSpriteSize (location = 3)
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Transform, spriteSize));
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3, 1);

  // inRenderOptions (location = 4)
  glVertexAttribIPointer(4, 1, GL_INT, stride, (const void*)offsetof(Transform, renderOptions));
  glEnableVertexAttribArray(4);
  glVertexAttribDivisor(4, 1);

  // inLayer (location = 5)
  glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Transform, layer));
  glEnableVertexAttribArray(5);
  glVertexAttribDivisor(5, 1);

  // Unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);  

#else
  // This needs to be bound, otherwise OpenGL doesn't draw anything
  // We won't use it tho!
  int VAO = 0;
  glGenVertexArrays(1, (GLuint*)&VAO);
  glBindVertexArray(VAO);

  // Transform Storage Buffer
  {
    glGenBuffers(1, (GLuint*)&glContext.transformSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * MAX_TRANSFORMS, renderData->transforms.elements, GL_DYNAMIC_DRAW);
  }
#endif

  // Uniforms
  {
    glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
    glContext.projectionID = glGetUniformLocation(glContext.programID, "orthoProjection");
  }
}

bool gl_init(BumpAllocator* transientStorage)
{
  load_gl_functions();

  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glEnable(GL_DEBUG_OUTPUT);

#ifdef USE_OPENGL410
  char* vertShaderName = "assets/shaders/quad_41.vert";
  char* fragShaderName = "assets/shaders/quad_41.frag";
#else
  char* vertShaderName = "assets/shaders/quad.vert";
  char* fragShaderName = "assets/shaders/quad.frag";
#endif

  GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, vertShaderName, transientStorage);
  GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, fragShaderName, transientStorage);
  if (!vertShaderID || !fragShaderID)
  {    
    SM_ASSERT(false, "Failed to create Shaders")
    return false;
  }

  glContext.programID = glCreateProgram();
  glAttachShader(glContext.programID, vertShaderID);
  glAttachShader(glContext.programID, fragShaderID);
  glLinkProgram(glContext.programID);
  check_link_errors(glContext.programID);
  
  // This is preemtively, because they are still bound
  // They are already marked for deletion tho
  glDetachShader(glContext.programID, vertShaderID);
  glDetachShader(glContext.programID, fragShaderID);
  glDeleteShader(vertShaderID);
  glDeleteShader(fragShaderID);

  create_gl_buffers();

  // Load our first Texture
  {
    int width, height, nChannels;
    char* data = (char*)stbi_load(TEXTURE_PATH, &width, &height, &nChannels, 4);
    int textureSizeInBytes = 4 * width * height;

    glContext.textureTimestamp = get_timestamp(TEXTURE_PATH);

    if (!data)
    {
      SM_ASSERT(0, "Failed to load Texture!");
      return false;
    }

    glContext.textureTimestamp = get_timestamp(TEXTURE_PATH);

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

  // Load Font Atlas
  {
    load_font("assets/fonts/AtariClassic-gry3.ttf", 8);
  }

  glUseProgram(glContext.programID);

  // sRGB output (even if input texture is non-sRGB -> don't rely on texture used)
  // Your font is not using sRGB, for example (not that it matters there, because no
  // actual color is sampled from it) But this could prevent some future bug when you
  // start mixing different types of textures Of course, you still need to correctly set
  // the image file source format when using glTexImage2D()
  glEnable(GL_FRAMEBUFFER_SRGB);
  glDisable(0x809D); // disable multisampling

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);

  return true;
}

// #############################################################################
//                           Render Interface Implementations
// #############################################################################
template <typename T, int N>
void drawObjects(GLuint buffer, GLenum target, Array<T, N>& transformData, bool isTransparent = false)
{
  if (isTransparent)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  glBindBuffer(target, buffer);
  glBufferSubData(target, 0, transformData.count * sizeof(T), transformData.elements);

  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, transformData.count);
  transformData.clear();

  if (isTransparent)
  {
    glDisable(GL_BLEND);
  }
}

void gl_render()
{
  // Texture Hot Reloading
  {
    long long currentTimestamp = get_timestamp(TEXTURE_PATH);

    if (currentTimestamp > glContext.textureTimestamp)
    {
      glActiveTexture(GL_TEXTURE0);
      int width, height, nChannels;
      char* data = (char*)stbi_load(TEXTURE_PATH, &width, &height, &nChannels, 4);
      if (data)
      {
        glContext.textureTimestamp = currentTimestamp;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
      }
    }
  }

  // Linux Colors, kinda
  glClearColor(renderData->clearColor.r, 
               renderData->clearColor.g, 
               renderData->clearColor.b, 
               renderData->clearColor.a);
  glClearDepth(0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, input->screenSize.x, input->screenSize.y);

  // Copy screenSize to the GPU
  glUniform2fv(glContext.screenSizeID, 1, &input->screenSize.x);

#ifdef USE_OPENGL410
  glBindVertexArray(glContext.vaoID);

  /////////////////////////////////////////////////
  // Draw Game
  glUniformMatrix4fv(glContext.projectionID, 1, GL_FALSE, &renderData->orthoProjectionGame.ax);
  // Opaque Game Objects
  drawObjects(glContext.vboID, GL_ARRAY_BUFFER, renderData->transforms);
  // Transparent Game Objects
  drawObjects(glContext.vboID, GL_ARRAY_BUFFER, renderData->transparentTransforms, true);

  /////////////////////////////////////////////////
  // Draw UI
  glUniformMatrix4fv(glContext.projectionID, 1, GL_FALSE, &renderData->orthoProjectionUI.ax);
  // Opaque UI objects
  drawObjects(glContext.vboID, GL_ARRAY_BUFFER, renderData->uiTransforms);
  // Transparent UI objects
  drawObjects(glContext.vboID, GL_ARRAY_BUFFER, renderData->uiTransparentTransforms, true);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO
  glBindVertexArray(0);             // Unbind the VAO
#else
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.transformSBOID);

  /////////////////////////////////////////////////
  // Draw Game
  glUniformMatrix4fv(glContext.projectionID, 1, GL_FALSE, &renderData->orthoProjectionGame.ax);
  // Opaque Game Objects
  drawObjects(glContext.transformSBOID, GL_SHADER_STORAGE_BUFFER, renderData->transforms);
  // Transparent Game Objects
  drawObjects(glContext.transformSBOID, GL_SHADER_STORAGE_BUFFER, renderData->transparentTransforms, true);

  /////////////////////////////////////////////////
  // Draw UI
  glUniformMatrix4fv(glContext.projectionID, 1, GL_FALSE, &renderData->orthoProjectionUI.ax);
  // Opaque UI objects
  drawObjects(glContext.transformSBOID, GL_SHADER_STORAGE_BUFFER, renderData->uiTransforms);
  // Transparent UI objects
  drawObjects(glContext.transformSBOID, GL_SHADER_STORAGE_BUFFER, renderData->uiTransparentTransforms, true);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the SSBO
#endif
}

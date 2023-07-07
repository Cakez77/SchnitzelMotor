#pragma once
#include <stdio.h>
#include <stdlib.h>

// Used to get the timestamp of a file
#include <sys/stat.h>

// #############################################################################
//                           Defines
// #############################################################################
#define b8 char
#define u8 unsigned char
#define BIT(x) 1 << (x)
#define KB(x) (x) * 1024
#define MB(x) (x) * KB(x)
#define GB(x) (x) * MB(x)

// #############################################################################
//                           Defines
// #############################################################################
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __asm__ volatile ("int3")
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#endif

// render_interface.h
constexpr int MAX_TRANSFORMS = 100;

// #############################################################################
//                           Logging
// #############################################################################
// Aparently works on Windows, Linux and Mac, have not tested on MAC!
#define TEXT_COLOR_BLACK "\x1b[30m"
#define TEXT_COLOR_RED "\x1b[31m"
#define TEXT_COLOR_GREEN "\x1b[32m"
#define TEXT_COLOR_YELLOW "\x1b[33m"
#define TEXT_COLOR_BLUE "\x1b[34m"
#define TEXT_COLOR_MAGENTA "\x1b[35m"
#define TEXT_COLOR_CYAN "\x1b[36m"
#define TEXT_COLOR_WHITE "\x1b[37m"
#define TEXT_COLOR_BRIGHT_BLACK "\x1b[90m"
#define TEXT_COLOR_BRIGHT_RED "\x1b[91m"
#define TEXT_COLOR_BRIGHT_GREEN "\x1b[92m"
#define TEXT_COLOR_BRIGHT_YELLOW "\x1b[93m"
#define TEXT_COLOR_BRIGHT_BLUE "\x1b[94m"
#define TEXT_COLOR_BRIGHT_MAGENTA "\x1b[95m"
#define TEXT_COLOR_BRIGHT_CYAN "\x1b[96m"
#define TEXT_COLOR_BRIGHT_WHITE "\x1b[97m"

// TRACE: Update Game took %.02f seconds, time
template <typename... Args>
void _log(char* prefix, char* msg, Args... args)
{
  char formatBuffer[8192] = {};
  sprintf(formatBuffer, "%s %s \033[0m", prefix, msg);

  static char buffer[8192] = {};
  sprintf(buffer, formatBuffer, args...);
  puts(buffer);
}

#define SM_TRACE(msg, ...) _log(TEXT_COLOR_GREEN "TRACE:", msg, ##__VA_ARGS__);
#define SM_WARN(msg, ...) _log(TEXT_COLOR_YELLOW "WARN:", msg "\033[0m", ##__VA_ARGS__);
#define SM_ERROR(msg, ...) _log(TEXT_COLOR_RED "ERROR:", msg "\033[0m", ##__VA_ARGS__);

#define SM_ASSERT(x, msg, ...)     \
{                                  \
  if(!(x))                         \
  {                                \
    SM_ERROR(msg, ##__VA_ARGS__);  \
    DEBUG_BREAK();                 \
  }                                \
}

// #############################################################################
//                           Array
// #############################################################################
template<typename T, int N>
struct Array
{
  static constexpr int maxElements = N;
  int count = 0;
  T elements[N];

  T& operator[](int idx)
  {
    SM_ASSERT(idx >= 0, "idx negative!");
    SM_ASSERT(idx < count, "Idx out of bounds!");
    return elements[idx];
  }

  int add(T element)
  {
    SM_ASSERT(count < maxElements, "Array Full!");
    elements[count] = element;
    return count++;
  }

  void remove_idx_and_swap(int idx)
  {
    SM_ASSERT(idx >= 0, "idx negative!");
    SM_ASSERT(idx < count, "idx out of bounds!");
    elements[idx] = elements[--count];
  }

  void clear()
  {
    count = 0;
  }

  bool is_full()
  {
    return count == N;
  }
};


// #############################################################################
//                           Math stuff
// #############################################################################
struct Vec2
{
  float x;
  float y;
};

Vec2 operator*(Vec2 a, float scalar)
{
  return Vec2{a.x * scalar, a.y * scalar};
}

struct IVec2
{
  int x;
  int y;
};

Vec2 vec_2(IVec2 ivec2)
{
  return Vec2{(float)ivec2.x, (float)ivec2.y};
}

// #############################################################################
//                           File I/O
// #############################################################################
char* read_file(char* filePath, int* fileSize)
{
  SM_ASSERT(filePath, "No filePath supplied!");
  SM_ASSERT(fileSize, "No fileSize supplied!");

  *fileSize = 0;
  auto file = fopen(filePath, "rb");
  if(!file)
  {
    SM_ERROR("Failed opening File: %s", filePath);
    return nullptr;
  }

  fseek(file, 0, SEEK_END);
  *fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  // TODO: Later we use our own memory allocator
  char* fileBuffer = new char[*fileSize + 1];
  fileBuffer[*fileSize] = 0; // Terminate the String

  fread(fileBuffer, sizeof(char), *fileSize, file);

  fclose(file);

  return fileBuffer;
}

long long get_timestamp(char* file)
{
    struct stat file_stat = {};
    stat(file, &file_stat);
    return file_stat.st_mtime;
}

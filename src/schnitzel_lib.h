#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Used to get the timestamp of a file
#include <sys/stat.h>

// Used to get memset
#include <string.h>

// #############################################################################
//                           Constants
// #############################################################################
// WAV Files
static constexpr int NUM_CHANNELS = 2;
static constexpr int SAMPLE_RATE = 44100;

// #############################################################################
//                           Defines
// #############################################################################
#define b8 char
#define BIT(x) 1 << (x)
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))

#define line_id(index) (size_t)((__LINE__ << 16) | (index))

#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))

#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#define EXPORT_FN
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN
#endif

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
float min(float a, float b)
{
  return (a < b)? a : b;
}

float max(float a, float b)
{
  return (a > b)? a : b;
}

int min(int a, int b)
{
  return (a < b)? a : b;
}

int max(int a, int b)
{
  return (a > b)? a : b;
}

float clamp(float x, float min, float max)
{
  if(x < min)
  {
    return min;
  }

  if(x > max)
  {
    return max;
  }

  return x;
}

int clamp(int x, int min, int max)
{
  if(x < min)
  {
    return min;
  }

  if(x > max)
  {
    return max;
  }

  return x;
}

// speed = 10
// targetSpeed = 100
// speedUP = 12
// speed < targetSpeed? -> speed += speedUp


// speed 200
// targetSpeed = 100
// speedUp = 12
// speed > targetSpeed? -> speed -= speedUp

// speed 100
// targetSpeed = -5
// speedUp = 12
// speed > targetSpeed? -> speed -= speedUp

float approach(float current, float target, float increase)
{
  if(current < target)
  {
    return min(current + increase, target);
  }
  return max(current - increase, target);
}

int sign(int x)
{
  return (x >= 0)? 1 : -1;
}

float sign(float x)
{
  return (x >= 0.0f)? 1.0f : -1.0f;
}

struct Vec2
{
  float x;
  float y;
};

Vec2 operator+(Vec2 a, Vec2 b)
{
  return Vec2{a.x + b.x, a.y + b.y};
}

Vec2 operator+(Vec2 a, float scalar)
{
  return Vec2{a.x + scalar, a.y + scalar};
}

Vec2 operator*(Vec2 a, float scalar)
{
  return Vec2{a.x * scalar, a.y * scalar};
}

Vec2 operator*(Vec2 a, Vec2 b)
{
  return Vec2{a.x * b.x, a.y * b.y};
}

Vec2 operator/(Vec2 a, int scalar)
{
  return Vec2{a.x / (float)scalar, 
              a.y / (float)scalar};
}

Vec2 operator-(Vec2 a)
{
  return Vec2{-a.x, -a.y};
}

Vec2 operator-(Vec2 a, Vec2 b)
{
  return Vec2{a.x - b.x, a.y - b.y};
}

Vec2 operator-(Vec2 a, float scalar)
{
  return Vec2{a.x - scalar, a.y - scalar};
}

bool operator==(Vec2 a, Vec2 b)
{
  return a.x == b.x && a.y == b.y;
}

bool operator!=(Vec2 a, Vec2 b)
{
  return !(a == b);
}

struct IVec2
{
  int x;
  int y;
};

IVec2 operator/(IVec2 a, float scalar)
{
  return IVec2{(int)((float)a.x / scalar), 
               (int)((float)a.y / scalar)};
}

IVec2 operator/(IVec2 a, IVec2 b)
{
  return IVec2{a.x / b.x, a.y / b.y};
}

IVec2 operator-(IVec2 a, IVec2 b)
{
  return IVec2{a.x - b.x, a.y - b.y};
}

IVec2 operator+(IVec2 a, IVec2 b)
{
  return IVec2{a.x + b.x, a.y + b.y};
}

IVec2 operator+(IVec2 a, int scalar)
{
  return IVec2{a.x + scalar, a.y + scalar};
}

IVec2 operator*(IVec2 a, float scalar)
{
  return IVec2{(int)((float)a.x * scalar), (int)((float)a.y * scalar)};
}

Vec2 vec_2(IVec2 ivec2)
{
  return Vec2{(float)ivec2.x, (float)ivec2.y};
}

Vec2 vec_2(float scalar)
{
  return Vec2{scalar, scalar};
}

IVec2 ivec_2(Vec2 ivec2)
{
  return IVec2{(int)ivec2.x, (int)ivec2.y};
}

float lerp(float a, float b, float t)
{
  return a + (b - a) * t;
}

float length(Vec2 v)
{
  return sqrt(v.x * v.x + v.y * v.y);
}

Vec2 normalize(Vec2 v)
{
  Vec2 normalized = {};
  float vecLength = length(v);
  if(vecLength)
  {
    normalized = v / length(v);
  }
  else
  {
    SM_ASSERT(0, "Vector has a length of 0");
  }

  return normalized;
}

Vec2 lerp(Vec2 a, Vec2 b, float t)
{
  Vec2 result;
  result.x = lerp(a.x, b.x, t);
  result.y = lerp(a.y, b.y, t);
  return result;
}

IVec2 lerp(IVec2 a, IVec2 b, float t)
{
  IVec2 result;
  result.x = (int)floorf(lerp((float)a.x, (float)b.x, t));
  result.y = (int)floorf(lerp((float)a.y, (float)b.y, t));
  return result;
}

struct Rect
{
  Vec2 pos;
  Vec2 size;
};

struct IRect
{
  IVec2 pos;
  IVec2 size;
};

bool point_in_rect(Vec2 point, Rect rect)
{
  return (point.x >= rect.pos.x &&
          point.x <= rect.pos.x + rect.size.x &&
          point.y >= rect.pos.y &&
          point.y <= rect.pos.y + rect.size.y);
}

bool point_in_rect(Vec2 point, IRect rect)
{
  return (point.x >= rect.pos.x &&
          point.x <= rect.pos.x + rect.size.x &&
          point.y >= rect.pos.y &&
          point.y <= rect.pos.y + rect.size.y);
}

bool rect_collision(IRect a, IRect b)
{
  return a.pos.x < b.pos.x  + b.size.x && // Collision on Left of a and right of b
         a.pos.x + a.size.x > b.pos.x  && // Collision on Right of a and left of b
         a.pos.y < b.pos.y  + b.size.y && // Collision on Bottom of a and Top of b
         a.pos.y + a.size.y > b.pos.y;    // Collision on Top of a and Bottom of b
}

struct Vec4
{
  union
  {
    float values[4];
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };
    
    struct
    {
      float r;
      float g;
      float b;
      float a;
    };
    
    struct
    {
      float xy[2];
    };
    
    struct
    {
      float xyz[3];
    };
  };
  
  bool operator==(Vec4 other)
  {
    return x == other.x && y == other.y && z == other.z && w == other.w;
  }
  
  Vec4 operator+(Vec4 other) const
  {
    return {x + other.x,
      y + other.y,
      z + other.z,
      w + other.w};
  }
  
  Vec4 operator+(float value) const
  {
    return {x + value,
      y + value,
      z + value,
      w + value};
  }
  
  Vec4 operator*(float value) const
  {
    return {x * value,
      y * value,
      z * value,
      w * value};
  }
  
  Vec4 operator-(float value) const
  {
    return {x - value,
      y - value,
      z - value,
      w - value};
  }
  
  Vec4 operator*(Vec4 other) const
  {
    return {x * other.x,
      y * other.y,
      z * other.z,
      w * other.w};
  }
  
  Vec4& operator*=(float value)
  {
    x *= value;
    y *= value;
    z *= value;
    w *= value;
    
    return *this;
  }
  
  float& operator[](int index)
  {
    return values[index];
  }
};

struct Mat4
{
  union 
  {
    Vec4 values[4];
    struct
    {
      float ax;
      float bx;
      float cx;
      float dx;

      float ay;
      float by;
      float cy;
      float dy;

      float az;
      float bz;
      float cz;
      float dz;
      
      float aw;
      float bw;
      float cw;
      float dw;
    };
  };

  Vec4& operator[](int col)
  {
    return values[col];
  }
 
  Mat4 operator*(Mat4 other)
  {
    Mat4 result = {};
    
    // TODO: think about how to do this in a for loop
    
    result.ax = ax * other.ax + bx * other.ay + cx * other.az + dx * other.aw;
    result.ay = ay * other.ax + by * other.ay + cy * other.az + dy * other.aw;
    result.az = az * other.ax + bz * other.ay + cz * other.az + dz * other.aw;
    result.aw = aw * other.ax + bw * other.ay + cw * other.az + dw * other.aw;
    
    result.bx = ax * other.bx + bx * other.by + cx * other.bz + dx * other.bw;
    result.by = ay * other.bx + by * other.by + cy * other.bz + dy * other.bw;
    result.bz = az * other.bx + bz * other.by + cz * other.bz + dz * other.bw;
    result.bw = aw * other.bx + bw * other.by + cw * other.bz + dw * other.bw;
    
    result.cx = ax * other.cx + bx * other.cy + cx * other.cz + dx * other.cw;
    result.cy = ay * other.cx + by * other.cy + cy * other.cz + dy * other.cw;
    result.cz = az * other.cx + bz * other.cy + cz * other.cz + dz * other.cw;
    result.cw = aw * other.cx + bw * other.cy + cw * other.cz + dw * other.cw;
    
    result.dx = ax * other.dx + bx * other.dy + cx * other.dz + dx * other.dw;
    result.dy = ay * other.dx + by * other.dy + cy * other.dz + dy * other.dw;
    result.dz = az * other.dx + bz * other.dy + cz * other.dz + dz * other.dw;
    result.dw = aw * other.dx + bw * other.dy + cw * other.dz + dw * other.dw;
    
    return result;
  }
};

Mat4 mat_4(float value)
{
  Mat4 result = {};
  result[0][0] = value;
  result[1][1] = value;
  result[2][2] = value;
  result[3][3] = value;

  return result;
}

Mat4 orthographic_projection(float left, float right, float top, float bottom)
{
  Mat4 result = {};
  result.aw = -(right + left) / (right - left); 
  result.bw = -(top + bottom) / (top - bottom);
  result.cw = -0.0f; // Near Plane
  result[0][0] = 2.0f / (right - left);
  result[1][1] = 2.0f / (top - bottom);
  result[2][2] = 1.0f / (1.0f - 0.0f);
  result[3][3] = 1.0f;

  return result;
}

// #############################################################################
//                           Easing Functions
// #############################################################################
float ease_out_linear(float t)
{
  if(t < 1.0f)
  {
    return t;
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_quad(float t)
{
  if (t < 1.0f)
  {
    return t * t;
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_quad(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - (1.0f - t) * (1.0f - t);
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_qubic(float t)
{
  if (t < 1.0f)
  {
    return t * t * t * t;
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_qubic(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t) * (1.0f - t);
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_out_qubic(float t)
{
  if (t < 1.0f)
  {
    return t < 0.5f ? 4.0f * t * t * t : 1 - (float)pow(-2 * t + 2, 3) / 2.0f;
  }
  else
  {
    return 1.0f;
  }
}

float ease_wind_slash(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - (float)pow(-2 * (t) + 2, 5) / 33.0f;
  }
  else
  {
    return 1.0f;
  }
}

float ease_arrow(float t)
{
  if (t < 1.0f)
  {
    return t <= 0.3f ? 16.0f * t * t * t : 1 - (float)pow(-2 * (t + 0.111) + 2, 5) / 4.0f;
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_expo(float t)
{
  if (t < 1.0f)
  {
    return (float)pow(2, 8 * t - 8);
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_expo(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - (float)pow(2, -10 * t);
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_quint(float t)
{
  if(t < 1.0f)
  {
    return 1.0f - pow(1.0f - t, 5.0f);
  }
  else
  {
    return 1.0f;
  }
}

float ease_in_circ(float t)
{
  if (t < 1.0f)
  {
    return 1.0f - sqrt(1 - t * t);
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_elastic(float t)
{
  float c4 = (2.0f * 3.14f) / 3.0f;
  
  if (t == 0.0f)
  {
    return 0.0f;
  }
  else if (t < 1.0f)
  {
    return (float)pow(2, -10 * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
  }
  else
  {
    return 1.0f;
  }
}

float ease_out_back(float t)
{
  float c1 = 1.70158f;
  float c3 = c1 + 1.0f;
  if (t < 1.0f)
  {
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
  }
  else
  {
    return 1.0f;
  }
}

float superku_function(float t)
{
  if(t > 1.0f)
  {
    return 1.0f;
  }
  
  return 0.5f * (sqrt(t) + t * t * t * t *t );
}


// #############################################################################
//                           Memeory Management
// #############################################################################
struct BumpAllocator
{
  size_t capacity;
  size_t used;
  char* memory;
};

BumpAllocator make_bump_allocator(size_t size)
{
  BumpAllocator result = {};

  size_t alignedSize = (size + 7) & ~7;
  result.capacity = alignedSize;
  result.memory = (char*)malloc(alignedSize);

  if(result.memory)
  {
    memset(result.memory, 0, alignedSize);
  }
  else
  {
    SM_ASSERT(0, "Failed to malloc memory: %d", size);
  }

  return result;
}

char* bump_alloc(BumpAllocator* allocator, size_t size)
{
  char* result = nullptr;

  size_t alignedSize = (size + 7) & ~7;
  if(allocator->used + alignedSize <= allocator->capacity)
  {
    result = allocator->memory + allocator->used;
    allocator->used += alignedSize;
  }
  else
  {
    SM_ASSERT(0, "Bump allocator is full");
  }

  return result;
}

// #############################################################################
//                           String Stuff
// #############################################################################
template <typename... Args>
char* format_text(char* format, Args... args)
{
  static int bufferIdx = 0;
  static char buffers[2][1024] = {};
  
  char* buffer = buffers[bufferIdx];
  memset(buffer, 0, 1024);
  
  sprintf(buffer, format, args...);
  
  return buffer;
}

// #############################################################################
//                           File I/O
// #############################################################################
long long get_timestamp(char* file)
{
    struct stat file_stat = {};
    stat(file, &file_stat);
    return file_stat.st_mtime;
}

long get_file_size(char* filePath)
{
  SM_ASSERT(filePath, "No filePath supplied!");

  long fileSize = 0;
  auto file = fopen(filePath, "rb");
  if(!file)
  {
    SM_ERROR("Failed opening File: %s", filePath);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  fclose(file);

  return fileSize;
}

/*
* Reads a file into a supplied buffer. We manage our own
* memory and therefore want more control over where it 
* is allocated
*/
char* read_file(char* filePath, int* fileSize, char* buffer)
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

  memset(buffer, 0, *fileSize + 1);
  fread(buffer, sizeof(char), *fileSize, file);

  fclose(file);

  return buffer;
}

void write_file(char* filePath, char* buffer, int size)
{
  SM_ASSERT(filePath, "No filePath supplied!");
  SM_ASSERT(buffer, "No buffer supplied!");
  auto file = fopen(filePath, "wb");
  if(!file)
  {
    SM_ERROR("Failed opening File: %s", filePath);
    return;
  }

  fwrite(buffer, sizeof(char), size, file);
  fclose(file);
}

char* read_file(char* filePath, int* fileSize, BumpAllocator* bumpAllocator)
{
  char* file = 0;
  long fileSize2 = get_file_size(filePath);

  if(fileSize2)
  {
    char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);

    file = read_file(filePath, fileSize, buffer);
  }

  return file; 
}

bool copy_file(char* fileName, char* outputName, char* buffer)
{
  int fileSize = 0;
  char* data = read_file(fileName, &fileSize, buffer);

  auto outputFile = fopen(outputName, "wb");
  if(!outputFile)
  {
    SM_ERROR("Failed opening File: %s", outputName);
    return false;
  }

  int result = fwrite(data, sizeof(char), fileSize, outputFile);
  if(!result)
  {
    SM_ERROR("Failed opening File: %s", outputName);
    return false;
  }
  
  fclose(outputFile);

  return true;
}

bool copy_file(char* fileName, char* outputName, BumpAllocator* bumpAllocator)
{
  char* file = 0;
  long fileSize2 = get_file_size(fileName);

  if(fileSize2)
  {
    char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);

    return copy_file(fileName, outputName, buffer);
  }

  return false;
}

// #############################################################################
//                           WAV File stuff
// #############################################################################
// Wave Files are seperated into chunks, 
// struct chunk
// {
//   unsigned int id;
//   unsigned int size; // In bytes
//   ...
// }
// we are ASSUMING!!!! That we have a "Riff Chunk"
// followed by a "Format Chunk" followed by a
// "Data Chunk", this CAN! be wrong ofcourse
struct WAVHeader
{
  // Riff Chunk
	unsigned int riffChunkId;
	unsigned int riffChunkSize;
	unsigned int format;

  // Format Chunk
	unsigned int formatChunkId;
	unsigned int formatChunkSize;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;

  // Data Chunk
	unsigned char dataChunkId[4];
	unsigned int dataChunkSize;
};

struct WAVFile
{
	WAVHeader header;
	char dataBegin;
};
WAVFile* load_wav(char* path, BumpAllocator* bumpAllocator)
{
	int fileSize = 0;
	WAVFile* wavFile = (WAVFile*)read_file(path, &fileSize, bumpAllocator);
	if(!wavFile) 
  { 
    SM_ASSERT(0, "Failed to load Wave File: %s", path);
    return {}; 
  }

	SM_ASSERT(wavFile->header.numChannels == NUM_CHANNELS, 
            "We only support 2 channels for now!");
	SM_ASSERT(wavFile->header.sampleRate == SAMPLE_RATE, 
            "We only support 44100 sample rate for now!");

	SM_ASSERT(memcmp(&wavFile->header.dataChunkId, "data", 4) == 0, 
						"WAV File not in propper format");

	return wavFile;
}

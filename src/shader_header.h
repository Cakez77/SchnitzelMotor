#ifdef ENGINE // Inside Game / Engine
#pragma once

#include "schnitzel_lib.h"
#define vec2 Vec2
#define ivec2 IVec2
#define vec4 Vec4

#else // Inside Shader

#define BIT(i) 1 << i
#endif // Inside Both

// #############################################################################
//                           Rendering Constants
// #############################################################################
int RENDERING_OPTION_FLIP_X = BIT(0);
int RENDERING_OPTION_FONT = BIT(1);
int RENDERING_OPTION_TRANSPARENT = BIT(2);

// #############################################################################
//                           Rendering Structs
// #############################################################################
struct Transform
{
  vec2 pos; // This is currently the Top Left!!
  vec2 size;
  vec2 atlasOffset;
  vec2 spriteSize;
  int renderOptions;
  float layer;
};

struct Material
{
  vec4 color;
};



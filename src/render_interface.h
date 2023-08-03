#pragma once
#include "schnitzel_lib.h"
#include "assets.h"

// #############################################################################
//                           Render Interface Constants
// #############################################################################
constexpr int RENDERING_OPTION_FLIP_X = BIT(0);
constexpr int MAX_TRANSFORMS = 10000;

// #############################################################################
//                           Render Interface Structs
// #############################################################################

struct Transform
{
  Vec2 pos; // This is currently the Top Left!!
  Vec2 size;
  Vec2 atlasOffset;
  Vec2 spriteSize;
  int renderOptions;
  int padding;
};

struct OrthographicCamera2D
{
  float zoom = 1.0f;
  Vec2 dimensions;
  Vec2 position;
};

struct RenderData
{
  OrthographicCamera2D camera;
  Array<Transform, MAX_TRANSFORMS> transforms;
};

// #############################################################################
//                           Render Interface Globals
// #############################################################################
static RenderData* renderData;

// #############################################################################
//                           Render Interface Functions
// #############################################################################
void draw_quad(Transform transform)
{
  // Screen Space Y grows down by default
  // This inverts all Y Positions so that Y grows UP 
  transform.pos.y *= -1;
  renderData->transforms.add(transform);
}

void draw_quad(Vec2 pos, Vec2 size)
{
  Transform transform = {pos, size, {0.0f, 0.0f}, {1.0f, 1.0f}};
  draw_quad(transform);
}

void draw_quad(IVec2 pos, IVec2 size)
{
  draw_quad(vec_2(pos), vec_2(size));
}

void draw_sprite(SpriteID spriteID, Vec2 pos, int scale = 1,
                 int renderOptions = 0)
{
  Sprite sprite = get_sprite(spriteID);

  Transform transform = {pos, 
                         vec_2(sprite.size) * scale, 
                         vec_2(sprite.atlasOffset), 
                         vec_2(sprite.size),
                         renderOptions};
  draw_quad(transform);
}

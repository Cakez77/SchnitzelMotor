#pragma once
#include "schnitzel_lib.h"
#include "assets.h"

// #############################################################################
//                           Render Interface Constants
// #############################################################################
constexpr int MAX_TRANSFORMS = 100;

// #############################################################################
//                           Render Interface Structs
// #############################################################################
struct Transform
{
  Vec2 pos; // This will be the center!!
  Vec2 size;
  Vec2 atlasOffset;
  Vec2 spriteSize;
};

struct RenderData
{
  Array<Transform, MAX_TRANSFORMS> transforms;
};

// #############################################################################
//                           Render Interface Globals
// #############################################################################
static RenderData* renderData;

// #############################################################################
//                           Render Interface Functions
// #############################################################################
void draw_quad(Vec2 pos, Vec2 size)
{
  Transform transform = {pos, size, {0.0f, 0.0f}, {1.0f, 1.0f}};
  renderData->transforms.add(transform);
}

void draw_quad(IVec2 pos, IVec2 size)
{
  draw_quad(vec_2(pos), vec_2(size));
}

void draw_sprite(SpriteID spriteID, Vec2 pos)
{
  Sprite sprite = get_sprite(spriteID);

  Transform transform = {pos, vec_2(sprite.size) * 6.0f, vec_2(sprite.atlasOffset), vec_2(sprite.size)};
  renderData->transforms.add(transform);
}

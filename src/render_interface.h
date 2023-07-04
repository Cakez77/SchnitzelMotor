#include "schnitzel_lib.h"

struct Transform
{
  Vec2 pos; // This will be the center!!
  Vec2 size;
};

struct RenderData
{
  Array<Transform, MAX_TRANSFORMS> transforms;
};

static RenderData renderData;

void draw_quad(Vec2 pos, Vec2 size)
{
  Transform transform = {pos, size};
  renderData.transforms.add(transform);
}
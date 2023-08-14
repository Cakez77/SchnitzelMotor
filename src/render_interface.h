#pragma once
#include "assets.h"
#include "shader_header.h"
#include "schnitzel_lib.h"

// #############################################################################
//                           Render Interface Constants
// #############################################################################
constexpr int MAX_TRANSFORMS = 10000;

// #############################################################################
//                           Render Interface Structs
// #############################################################################
struct DrawData
{
  // Used to generate an X - Offset based on the 
  // X - Size of the Sprite
  int animationIdx;
  int renderOptions;
  float layer = 1.0f;
};

struct OrthographicCamera2D
{
  float zoom = 1.0f;
  Vec2 dimensions;
  Vec2 position;
};

struct Glyph
{
  Vec2 size;
  Vec2 offset;
  Vec2 advance;
  Vec2 textureCoords;
};

struct RenderData
{
  Vec4 clearColor;
  Glyph glyphs[127];
  OrthographicCamera2D camera;
  Array<Transform, MAX_TRANSFORMS> transforms;
  Array<Transform, MAX_TRANSFORMS> transparent_transforms;
};

// #############################################################################
//                           Render Interface Globals
// #############################################################################
static RenderData* renderData;

// #############################################################################
//                           Render Interface Quad Rendering
// #############################################################################
void draw_quad(Transform transform)
{
  transform.pos = {transform.pos.x - transform.size.x / 2.0f, 
                   transform.pos.y + transform.size.y / 2.0f};

  // Screen Space Y grows down by default
  // This inverts all Y Positions so that Y grows UP 
  transform.pos.y *= -1;
  if(transform.renderOptions & RENDERING_OPTION_TRANSPARENT)
  {
    renderData->transparent_transforms.add(transform);
  }
  else 
  {
    renderData->transforms.add(transform);
  }
}

void draw_quad(Vec2 pos, Vec2 size, DrawData drawData = {})
{
  Transform transform = {};
  transform.pos = pos;
  transform.size = size;
  // References SPRITE_WHITE from the Atlas
  transform.spriteSize = {1.0f, 1.0f}; 
  transform.layer = drawData.layer;

  draw_quad(transform);
}

void draw_quad(IVec2 pos, IVec2 size, DrawData drawData = {})
{
  draw_quad(vec_2(pos), vec_2(size));
}

void draw_sprite(SpriteID spriteID, Vec2 pos, DrawData drawData = {})
{
  Sprite sprite = get_sprite(spriteID);

  // For Anmations, this is a multiple of the sprites size,
  // based on the animationIdx
  sprite.atlasOffset.x += drawData.animationIdx * sprite.size.x;

  Transform transform = {};
  // This centers the Sprite around the position
  transform.pos = pos;
  transform.size = vec_2(sprite.size);
  transform.spriteSize = vec_2(sprite.size);
  transform.renderOptions = drawData.renderOptions;
  transform.atlasOffset = vec_2(sprite.atlasOffset);
  transform.layer = drawData.layer;

  draw_quad(transform);
}

// #############################################################################
//                           Render Interface Font Rendering
// #############################################################################
void load_font(char* filePath, int fontSize);

void draw_text(char* text, Vec2 pos)
{
  SM_ASSERT(text, "No Text Supplied!");
  if(!text)
  {
    return;
  }

  char prev = 0;
  while(char c = *(text++))
  {
    Glyph glyph = renderData->glyphs[c];

    Transform transform = {};
    transform.pos.x = pos.x + glyph.offset.x / 2.0f;
    transform.pos.y = pos.y + glyph.offset.y / 2.0f;
    transform.atlasOffset = glyph.textureCoords;
    transform.spriteSize = glyph.size;
    transform.size = glyph.size;
    transform.renderOptions = RENDERING_OPTION_FONT;
    transform.layer = 1.0f;
    draw_quad(transform);

    prev = c;
    pos.x += glyph.advance.x;
  }
}
template <typename... Args>
void draw_format_text(char* format, Vec2 pos, Args... args)
{
  char* text = format_text(format, args...);
  draw_text(text, pos);
}

void draw_text_drop_shadow(char* text, Vec2 pos)
{
  draw_text(text, pos);
  draw_text(text, pos - 1.0f);
}
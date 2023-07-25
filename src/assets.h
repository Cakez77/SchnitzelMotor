#pragma once
#include "schnitzel_lib.h"

enum SpriteID
{
  SPRITE_WHITE,
  SPRITE_CELESTE_01,
  SPRITE_CELESTE_02,
  SPRITE_TILE_01,
  SPRITE_SPIKE,
  SPRITE_CELESTE_DEATH,

  SPRITE_COUNT
};

struct Sprite
{
  SpriteID ID;
  IVec2 atlasOffset;
  IVec2 size;
  int frameCount = 1;
};

Sprite get_sprite(SpriteID spriteID)
{
  Sprite sprite = {};
  sprite.ID = spriteID;

  switch(spriteID)
  {
    case SPRITE_WHITE:
    {
      sprite.size = {1, 1};
      break;
    }

    case SPRITE_CELESTE_01:
    {
      sprite.atlasOffset = {117, 0};
      sprite.size = {17, 20};
      break;
    }

    case SPRITE_CELESTE_02:
    {
      sprite.atlasOffset = {32, 0};
      sprite.size = {17, 20};
      break;
    }

    case SPRITE_TILE_01:
    {
      sprite.atlasOffset = {192, 0};
      sprite.size = {8, 8};
      break;
    }

    case SPRITE_SPIKE:
    {
      sprite.atlasOffset = {224, 0};
      sprite.size = {8, 8};
      break;
    }

    case SPRITE_CELESTE_DEATH:
    {
      sprite.atlasOffset = {240, 0};
      sprite.size = {32, 32};
      sprite.frameCount = 8;
      break;
    }


    default:
    {
      SM_ASSERT(0, "Unrecognized SpriteID: %d", spriteID);
    }
  }

  return sprite;
}
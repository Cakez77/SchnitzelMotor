#pragma once
#include "schnitzel_lib.h"

enum SpriteID
{
  SPRITE_WHITE,
  SPRITE_CELESTE_01,
  SPRITE_CELESTE_01_BIG,
  SPRITE_CELESTE_01_RUN,
  SPRITE_CELESTE_01_JUMP,
  SPRITE_CELESTE_02,
  SPRITE_CELESTE_02_BIG,
  SPRITE_TILE_01,
  SPRITE_SPIKE,
  SPRITE_CELESTE_DEATH,
  SPRITE_PLAY_BUTTON,
  SPRITE_SAVE_BUTTON,
  SPRITE_SOLID_01,
  SPRITE_SOLID_02,

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

    case SPRITE_CELESTE_01_BIG:
    {
      sprite.atlasOffset = {320, 496};
      sprite.size = {176, 272};
      break;
    }

    case SPRITE_CELESTE_01_RUN:
    {
      sprite.atlasOffset = {496, 0};
      sprite.size = {17, 20};
      sprite.frameCount = 12;
      break;
    }

    case SPRITE_CELESTE_01_JUMP:
    {
      sprite.atlasOffset = {597, 0};
      sprite.size = {17, 20};
      sprite.frameCount = 1;
      break;
    }

    case SPRITE_CELESTE_02:
    {
      sprite.atlasOffset = {32, 0};
      sprite.size = {17, 20};
      break;
    }

    case SPRITE_CELESTE_02_BIG:
    {
      sprite.atlasOffset = {144, 752};
      sprite.size = {176, 272};
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

    case SPRITE_PLAY_BUTTON:
    {
      sprite.atlasOffset = {0, 32};
      sprite.size = {32, 16};
      sprite.frameCount = 1;
      break;
    }

    case SPRITE_SAVE_BUTTON:
    {
      sprite.atlasOffset = {0, 48};
      sprite.size = {32, 16};
      sprite.frameCount = 1;
      break;
    }

    case SPRITE_SOLID_01:
    {
      sprite.atlasOffset = {240, 32};
      sprite.size = {28, 18};
      sprite.frameCount = 1;
      break;
    }

    case SPRITE_SOLID_02:
    {
      sprite.atlasOffset = {272, 32};
      sprite.size = {16, 13};
      sprite.frameCount = 1;
      break;
    }


    default:
    {
      SM_ASSERT(0, "Unrecognized SpriteID: %d", spriteID);
    }
  }

  return sprite;
}
#include "schnitzel_lib.h"

enum SpriteID
{
  SPRITE_WHITE,
  SPRITE_CELESTE_01,

  SPRITE_COUNT
};

struct Sprite
{
  SpriteID ID;
  IVec2 atlasOffset;
  IVec2 size;
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

    default:
    {
      SM_ASSERT(0, "Unrecognized SpriteID: %d", spriteID);
    }
  }

  return sprite;
}
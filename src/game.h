
#include "schnitzel_lib.h"
#include "input.h"
#include "render_interface.h"

struct GameState
{
  Vec2 playerPos;
};

extern "C"
{
  EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, RenderData* renderDataIn);
}
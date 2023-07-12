
#include "schnitzel_lib.h"
#include "input.h"
#include "render_interface.h"

enum GameInputType
{
  INPUT_MOVE_LEFT,
  INPUT_MOVE_RIGHT,
  INPUT_JUMP,
  INPUT_EXTENDED_JUMP,

  GAME_INPUT_COUNT
};

struct GameState
{
  IVec2 playerPos;
  bool playerGrounded = true;

  b8 gameInput[GAME_INPUT_COUNT];
};

extern "C"
{
  EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, RenderData* renderDataIn);
}








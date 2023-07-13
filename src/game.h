
#include "schnitzel_lib.h"
#include "input.h"
#include "render_interface.h"

enum GameInputType
{
  INPUT_MOVE_LEFT,
  INPUT_MOVE_RIGHT,
  INPUT_JUMP,
  INPUT_WALL_GRAB,
  INPUT_MOVE_UP,
  INPUT_MOVE_DOWN,

  GAME_INPUT_COUNT
};

struct GameInput
{
  b8 isDown;
  b8 justPressed;
};

struct GameState
{
  bool initialized;
  IVec2 playerPos;
  GameInput gameInput[GAME_INPUT_COUNT];
  Sound jumpSound;
  Sound wallJumpSound;
};

extern "C"
{
  EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, RenderData* renderDataIn, BumpAllocator* allocator, PlaySoundFunc play_sound);
}








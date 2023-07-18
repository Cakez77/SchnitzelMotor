
#include "schnitzel_lib.h"
#include "input.h"
#include "render_interface.h"
#include "sound.h"

static constexpr int UPDATES_PER_SECOND = 60;
static constexpr double UPDATE_DELAY = 1.0 / UPDATES_PER_SECOND;

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
  double updateTimer;
  IVec2 playerPos;
  IVec2 prevPlayerPos;
  b8 initialized = false;
  GameInput gameInput[GAME_INPUT_COUNT];
  Sound jumpSound;
  Sound deathSound;
};

extern "C"
{
  EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, RenderData* renderDataIn,
                             SoundState* soundStateIn, double frameTime);
}








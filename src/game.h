
#include "schnitzel_lib.h"
#include "input.h"
#include "render_interface.h"
#include "sound.h"

// #############################################################################
//                           Game Constants
// #############################################################################
constexpr int UPDATES_PER_SECOND = 60;
constexpr double UPDATE_DELAY = 1.0 / UPDATES_PER_SECOND;
constexpr int TILESIZE = 8;
constexpr IVec2 ROOM_SIZE = {320 / TILESIZE, 180 / TILESIZE};
constexpr IVec2 WORLD_SIZE = {320 / TILESIZE, 540 / TILESIZE};

// #############################################################################
//                           Game Structs
// #############################################################################
enum GameInputType
{
  INPUT_MOVE_LEFT,
  INPUT_MOVE_RIGHT,
  INPUT_JUMP,
  INPUT_WALL_GRAB,
  INPUT_MOVE_UP,
  INPUT_MOVE_DOWN,
  INPUT_DASH,

  GAME_INPUT_COUNT
};

struct GameInput
{
  b8 isDown;
  b8 justPressed;
  float bufferingTime;
};

enum TileType
{
  TILE_TYPE_NONE,
  TILE_TYPE_SOLID,
  TILE_TYPE_SPIKE
};

struct Tile
{
  TileType type;
  int neighbourMask;
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
  OrthographicCamera2D camera;
  Tile tiles[WORLD_SIZE.x * WORLD_SIZE.y];
};

static int worldScale = 4;

extern "C"
{
  EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, RenderData* renderDataIn,
                             SoundState* soundStateIn, double frameTime);
}








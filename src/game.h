
#include "schnitzel_lib.h"
#include "ui.h"
#include "input.h"
#include "render_interface.h"
#include "sound.h"

// #############################################################################
//                           Game Constants
// #############################################################################
constexpr int UPDATES_PER_SECOND = 60;
constexpr double UPDATE_DELAY = 1.0 / UPDATES_PER_SECOND;
constexpr int TILESIZE = 8;
constexpr int WORLD_HEIGHT = 180 * 3;
constexpr int WORLD_WIDTH = 320;
constexpr int ROOM_HEIGHT = 180;
constexpr int ROOM_WIDTH = WORLD_WIDTH;

// In number of tiles
constexpr IVec2 ROOM_SIZE = {ROOM_WIDTH / TILESIZE, ROOM_HEIGHT / TILESIZE};
constexpr IVec2 WORLD_SIZE = {WORLD_WIDTH / TILESIZE, WORLD_HEIGHT / TILESIZE};

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

enum AnimationState
{
  ANIMATION_STATE_IDLE,
  ANIMATION_STATE_JUMP,
  ANIMATION_STATE_RUN,

  ANIMATION_STATE_COUNT
};

struct Player
{
  IVec2 pos;
  IVec2 prevPos;
  Vec2 solidSpeed;
  int renderOptions;
  float deathAnimTimer;
  float runAnimTimer;
  AnimationState animationState;
  SpriteID animationSprites[ANIMATION_STATE_COUNT]; 
};

struct Tileset
{
  Array<IVec2, 21> tileCoords;
};

struct Keyframe
{
  IVec2 pos;
  float time; // how long to get there
};

struct Solid
{
  SpriteID spriteID;

  // Pixel Movement
  Vec2 prevRemainder;
  Vec2 remainder;

  // Used by "interpolated rendering"
  IVec2 prevPos;
  IVec2 pos;

  int keyframeIdx;
  Array<Keyframe, 10> keyframes;

  // Animation
  float time;
  float waitingTime;
  float waitingDuration;
};

struct TileMap
{
  Tileset tileset;
  Tile tiles[WORLD_SIZE.x * WORLD_SIZE.y];
};

struct Level
{
  int version = 1;
  IVec2 playerStartPos;
  TileMap tileMap;
  TileMap bgTileMap;
  Array<Solid, 50> solids;
};

enum GameStateID
{
  GAME_STATE_MAIN_MENU,
  GAME_STATE_IN_LEVEL
};

struct GameState
{
  GameStateID state;

  double updateTimer;
  bool initialized = false;
  float cameraTimer;
  GameInput gameInput[GAME_INPUT_COUNT];

  Player player;
  Level level;

  Sound jumpSound;
  Sound deathSound;
};

static int worldScale = 5;

extern "C"
{
  EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, RenderData* renderDataIn,
                             SoundState* soundStateIn, UIState* uiStateIn, 
                             BumpAllocator* transientStorageIn, double frameTime);
}








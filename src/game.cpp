#include "game.h"
#include "assets.h"
#include "input.h"
#include "render_interface.h"

constexpr float DEATH_ANIM_TIME = 0.25f;
// #############################################################################
//                           Game Structs
// #############################################################################
  struct Keyframe
  {
    IVec2 pos;
    float time; // how long to get there
  };

  struct Solid
  {
    // Pixel Movement
    Vec2 prevRemainder;
    Vec2 remainder;

    // Used by "interpolated rendering"
    IVec2 prevPos;
    // Current pos & size
    IRect rect;

    int keyframeIdx;
    Array<Keyframe, 10> keyframes;

    // Animation
    float time;
    float waitingTime;
    float waitingDuration;
  };

  struct Tileset
  {
    Array<Vec2, 21> tileCoords;
  };

// #############################################################################
//                           Game Globals
// #############################################################################
static GameState* gameState;
static Vec2 prevRemainder = {};
static float xRemainder = 0.0f;
static float yRemainder = 0.0f;
static bool standingOnPlatform = false;
static Vec2 speed;
static int renderOptions;
static float deathAnimTimer = DEATH_ANIM_TIME;
static Tileset tileset;
static Solid solids [] =
{
  {
    // .rect = {8 * 13, 8 * 2, 8, 8 * 11},
  },
  {
    // .rect = {48 *  8, 48 * 2, 48, 48 * 8}
  },
  {
    .rect = {0, 8 * 13, 8 * 22, 8}
  },
  {
    .rect = {8 * 5, 8 * 12, 8 * 2, 8},
    .keyframes = 
    {
      .count = 5,
      .elements =
      {
        {{8 * 0, 8 * 12}, 0.0f},
        {{8 * 8, 8 * 12}, 0.5f},
        {{8 * 5, 8 *  7}, 1.0f},
        {{8 * 8, 8 * 12}, 1.5f},
        {{8 * 0, 8 * 12}, 2.0f},
      }
    }
  },
};

// #############################################################################
//                           Game Functions
// #############################################################################
bool is_down(GameInputType type);
bool just_pressed(GameInputType type);
void update_game_input(float dt);
void move_x();
IRect get_player_rect();
IRect get_player_jump_rect();
void draw(float interpolatedDT);
void update();
int get_current_room_idx()
{
  int roomIdx = ((gameState->playerPos.y + 8 - 180) / -180) % 
                ArraySize(gameState->rooms);
  return roomIdx;
}
Room* get_current_room()
{
  Room* room = &gameState->rooms[get_current_room_idx()];
  return room;
}

// #############################################################################
//                           Update Game (Exported from DLL)
// #############################################################################
EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, 
                           RenderData* renderDataIn, SoundState* soundStateIn,
                           double frameTime)
{
  if(gameState != gameStateIn)
  {
    input = inputIn;
    renderData = renderDataIn;
    gameState = gameStateIn;
    soundState = soundStateIn;

    // Sounds
    char* jumpSound = "assets/sounds/jump_01.wav";
    char* deathSound = "assets/sounds/died_02.wav";
    memcpy(gameState->jumpSound.path, jumpSound, strlen(jumpSound));
    memcpy(gameState->deathSound.path, deathSound, strlen(deathSound));

    tileset.tileCoords.add({192, 0});
    tileset.tileCoords.add({200, 0});
    tileset.tileCoords.add({208, 0});
    tileset.tileCoords.add({216, 0});

    tileset.tileCoords.add({192, 8});
    tileset.tileCoords.add({200, 8});
    tileset.tileCoords.add({208, 8});
    tileset.tileCoords.add({216, 8});

    tileset.tileCoords.add({192, 16});
    tileset.tileCoords.add({200, 16});
    tileset.tileCoords.add({208, 16});
    tileset.tileCoords.add({216, 16});

    tileset.tileCoords.add({192, 24});
    tileset.tileCoords.add({200, 24});
    tileset.tileCoords.add({208, 24});
    tileset.tileCoords.add({216, 24});

    // Corners
    tileset.tileCoords.add({192, 32});
    tileset.tileCoords.add({200, 32});
    tileset.tileCoords.add({208, 32});
    tileset.tileCoords.add({216, 32});

    // Black inside
    tileset.tileCoords.add({192, 40});
  }

  if(!gameState->initialized)
  {
    gameState->initialized = true;
    gameState->playerPos = {8 * 2, 2 * 8};
  }

  gameState->updateTimer += frameTime;
  while(gameState->updateTimer >= UPDATE_DELAY)
  {
    gameState->updateTimer -= UPDATE_DELAY;
    update();

    // Reset Input
    for(int keyIdx = 0; keyIdx < MAX_KEYCODES; keyIdx++)
    {
      input->keys[keyIdx].justReleased = false;
      input->keys[keyIdx].justPressed = false;
      input->keys[keyIdx].halfTransitionCount = 0;
    }
  }
  float interpolatedDT = (float)(gameState->updateTimer / UPDATE_DELAY);
  draw(interpolatedDT);
}

// #############################################################################
//                           Implementations
// #############################################################################
bool is_down(GameInputType type)
{
  return gameState->gameInput[type].isDown;
}

bool just_pressed(GameInputType type)
{
  return gameState->gameInput[type].justPressed;
}

void update_game_input(float dt)
{
  // Moving
  gameState->gameInput[INPUT_MOVE_LEFT].isDown = input->keys[KEY_A].isDown;
  gameState->gameInput[INPUT_MOVE_RIGHT].isDown = input->keys[KEY_D].isDown;
  gameState->gameInput[INPUT_MOVE_UP].isDown = input->keys[KEY_W].isDown;
  gameState->gameInput[INPUT_MOVE_DOWN].isDown = input->keys[KEY_S].isDown;
  gameState->gameInput[INPUT_MOVE_LEFT].isDown |= input->keys[KEY_LEFT].isDown;
  gameState->gameInput[INPUT_MOVE_RIGHT].isDown |= input->keys[KEY_RIGHT].isDown;
  gameState->gameInput[INPUT_MOVE_UP].isDown |= input->keys[KEY_UP].isDown;
  gameState->gameInput[INPUT_MOVE_DOWN].isDown |= input->keys[KEY_DOWN].isDown;

  // Jumping
  GameInput* jumpInput  = &gameState->gameInput[INPUT_JUMP];
  jumpInput->bufferingTime = max(0.0f, jumpInput->bufferingTime - dt);
  if(input->keys[KEY_SPACE].justPressed)
  {
    gameState->gameInput[INPUT_JUMP].justPressed = true;
    gameState->gameInput[INPUT_JUMP].bufferingTime = 0.125f;
  }

  if(gameState->gameInput[INPUT_JUMP].bufferingTime == 0.0f)
  {
    jumpInput->justPressed = input->keys[KEY_SPACE].justPressed;
  }

  gameState->gameInput[INPUT_JUMP].isDown = 
    input->keys[KEY_SPACE].isDown;

  // Wall Grabbing
  gameState->gameInput[INPUT_WALL_GRAB].isDown =
    input->keys[KEY_E].isDown;
  gameState->gameInput[INPUT_WALL_GRAB].isDown |=
    input->keys[KEY_Q].isDown;

  // Dashing
  gameState->gameInput[INPUT_DASH].justPressed  = input->keys[KEY_Q].justPressed;
  gameState->gameInput[INPUT_DASH].justPressed  = input->keys[KEY_E].justPressed;
  gameState->gameInput[INPUT_DASH].justPressed  = input->keys[KEY_K].justPressed;
  gameState->gameInput[INPUT_DASH].justPressed |= input->keys[KEY_Q].justPressed;
  gameState->gameInput[INPUT_DASH].justPressed |= input->keys[KEY_E].justPressed;
  gameState->gameInput[INPUT_DASH].justPressed |= input->keys[KEY_K].justPressed;
}

IRect get_player_rect()
{
  return 
  {
    gameState->playerPos.x + 6, 
    gameState->playerPos.y + 2, 
    8, 
    16
  };
}

Tile* get_tile(int x, int y)
{
  if(x < 0 || x >= WORLD_SIZE.x || y < 0 || y >= WORLD_SIZE.y) return nullptr;
  return &gameState->tiles[y * WORLD_SIZE.x + x];
}

Tile* get_tile(Vec2 worldPos)
{
  int x = worldPos.x / TILESIZE;
  int y = worldPos.y / TILESIZE;

  return get_tile(x, y);
}

Tile* get_tile(Room* room, int x, int y)
{
  if(x < 0 || x >= WORLD_SIZE.x || y < 0 || y >= WORLD_SIZE.y) return nullptr;
  return &room->tiles[y * WORLD_SIZE.x + x];
}

Tile * get_tile(Room* room, Vec2 worldPos)
{
  int x = worldPos.x / TILESIZE;
  int y = worldPos.y / TILESIZE;

  return get_tile(room, x, y);
}

int animate(float* time, int frameCount, float loopTime = 1.0f)
{
  if(*time > loopTime)
  {
    *time -= loopTime;
  }

  int animationIdx = (int)((*time / loopTime) * (float)frameCount);
  if(animationIdx >= frameCount)
  {
    animationIdx = frameCount - 1;
  }

  return animationIdx;
}

void draw(float interpDT)
{
  // IRect playerCollider = get_player_rect();
  // draw_quad(playerCollider.pos * worldScale, playerCollider.size * worldScale);
  renderData->cameraPos.y = (get_current_room_idx() * 180) * worldScale;
  SM_TRACE("Current Room Idx: %d", get_current_room_idx());

  Vec2 playerPos = lerp(vec_2(gameState->prevPlayerPos), 
                        vec_2(gameState->playerPos), 
                        interpDT);

  // Death Animation
  if(deathAnimTimer < DEATH_ANIM_TIME)
  {
    Sprite sprite = get_sprite(SPRITE_CELESTE_DEATH);
    float t = deathAnimTimer;
    int animationIdx = animate(&t, sprite.frameCount, DEATH_ANIM_TIME);

    Transform transform = {};
    transform.atlasOffset = vec_2(sprite.atlasOffset);
    transform.atlasOffset.x += (float)(animationIdx * sprite.size.x);
    transform.pos = playerPos * worldScale;
    transform.size = Vec2{32.0f * worldScale, 32.0f * worldScale};
    transform.spriteSize = Vec2{32.0f, 32.0f};

    draw_quad(transform);
  }
  else
  {  
    if(speed.x > 0)
    {
      renderOptions = 0;
    }
    if(speed.x < 0)
    {
      renderOptions |= RENDERING_OPTION_FLIP_X ;
    }
    draw_sprite(SPRITE_CELESTE_01, playerPos * worldScale, worldScale, renderOptions);
  }

  // Tiles
  int currentRoomIdx = get_current_room_idx();
  int prevRoomIdx = max(currentRoomIdx - 1, 0);
  int nextRoomIdx = min(currentRoomIdx + 1, ArraySize(gameState->rooms));
  for(int roomIdx = prevRoomIdx; roomIdx < nextRoomIdx; roomIdx++)
  {
    Room* room = &gameState->rooms[roomIdx];

    // Neighbouring Tiles       Top    Left  Right Bottom  
    int neighbourOffsets[24] = { 0,-1,  -1,0,  1,0,  0,1,   
    //                         Topleft Topright Bottomleft Bottomright
                                -1,-1,   1,-1,     -1,1,      1,1,
    //                          Top2   Left2  Right2 Bottom2
                                 0,-2,  -2,0,  2,0,  0,2};

    // Topleft     = BIT(4) = 16
    // Toplright   = BIT(5) = 32
    // Bottomleft  = BIT(6) = 64
    // Bottomright = BIT(7) = 128

    for(int x = 0; x < WORLD_SIZE.x; x++)
    {
      for(int y = 0; y < WORLD_SIZE.y; y++)
      {
        Tile* tile = get_tile(room, x, y);

        if(!tile->type)
        {
          continue;
        }


        if(tile->type == TILE_TYPE_SPIKE)
        {
          draw_sprite(SPRITE_SPIKE, 
                      {float(x * TILESIZE * worldScale), 
                      float(y * TILESIZE * worldScale)}, worldScale);

          continue;
        }

        // Reset mask every frame
        tile->neighbourMask = 0;
        int neighbourCount = 0;
        int extendedNeighbourCount = 0;
        int emptyNeighbourSlot = 0;

        // Look at all 4 Neighbours
        for(int n = 0; n < 12; n++) 
        {
          Tile* neighbour = get_tile(room,
                                     x + neighbourOffsets[n * 2],
                                     y + neighbourOffsets[n * 2 + 1]);


          if(!neighbour || neighbour->type == TILE_TYPE_SOLID)
          {
            tile->neighbourMask |= BIT(n);
            if(n < 8)
            {
              neighbourCount++;
            }
            else
            {
              extendedNeighbourCount++;
            }
          }
          else if(n < 8)
          { 
            emptyNeighbourSlot = n;
          }
        }

        if(neighbourCount == 7 && emptyNeighbourSlot >= 4) // We have a corner
        {
          tile->neighbourMask = 16 + (emptyNeighbourSlot - 4);
        }
        else if(neighbourCount == 8 && extendedNeighbourCount == 4)
        {
          tile->neighbourMask = 20;
        }
        else
        {
          tile->neighbourMask = tile->neighbourMask & 0b1111;
        }

        // Draw Tile
        Transform transform = {};
        transform.pos = Vec2{float(x * TILESIZE * worldScale), 
                             float(-180 * roomIdx + y * TILESIZE * worldScale)};
        transform.size = Vec2{8.0f * worldScale, 8.0f * worldScale};
        transform.spriteSize = Vec2{8.0f, 8.0f};
        transform.atlasOffset = tileset.tileCoords[tile->neighbourMask];
        draw_quad(transform);
      }
    }
  }

  for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
  {
    Solid solid = solids[solidIdx];
    Vec2 solidPos = lerp(vec_2(solid.prevPos), 
                         vec_2(solid.rect.pos), 
                         interpDT);

    draw_quad(solidPos * (float)worldScale, vec_2(solid.rect.size) * (float)worldScale);
  }
}

void update()
{
  // We update the logic at a fixed rate to keep the game stable 
  // and to save on performance
  float dt = UPDATE_DELAY;
  float prevDeathAnimTimer = deathAnimTimer;
  deathAnimTimer  = min(DEATH_ANIM_TIME, deathAnimTimer + dt);
  if (prevDeathAnimTimer < DEATH_ANIM_TIME && deathAnimTimer == DEATH_ANIM_TIME)
  {
    gameState->playerPos = {50, 0};
  }

  update_game_input(dt);
  gameState->prevPlayerPos = gameState->playerPos;
  prevRemainder = {xRemainder, yRemainder};
  standingOnPlatform = false;

  if(key_is_down(KEY_MIDDLE_MOUSE))
  {
    SM_TRACE("MIddle mouse down: %.2f, %.2f", 
      input->relMouseScreen.x, input->relMouseScreen.y);
    // renderData->cameraPos.x += input->relMouseScreen.x? 

    renderData->cameraPos = 
      renderData->cameraPos + input->relMouseScreen * worldScale;

    SM_TRACE("MIddle mouse down: %.2f, %.2f", 
      renderData->cameraPos.x, 
      renderData->cameraPos.y);
  }

  if(key_is_down(KEY_LEFT_MOUSE))
  {
    Room* room = get_current_room();
    Tile* tile = get_tile(room, input->mousePosWorld);
    if(tile)
    {
      if(key_is_down(KEY_SHIFT))
      {
        tile->type = TILE_TYPE_SPIKE;
      }
      else
      {
        tile->type = TILE_TYPE_SOLID;
      }
    }
  }

  if(key_is_down(KEY_RIGHT_MOUSE))
  {
    Room* room = get_current_room();
    Tile* tile = get_tile(room, input->mousePosWorld);
    if(tile)
    {
      tile->type = TILE_TYPE_NONE;
    }
  }

  // Movement Data needed for Celeste
  float maxRunSpeed = 2.0f;
  float wallJumpSpeed = 3.0f;
  float runAcceleration = 50.0f / 3.6f;
  float fallSideAcceleration = 35.0f / 3.6f;
  float maxJumpSpeed = -3.0f;
  float fallSpeed = 18.0f / 3.6f;
  float dashSpeed = 4.2f;
  float gravity = 70.0f / 3.6f;
  float runReduce = 80.0f / 3.6f;
  float wallClimbSpeed = -4.0f / 3.6f;
  float wallSlideDownSpeed = 8.0f / 3.6f;
  Vec2 solidSpeed = {};
  static float highestHeight = input->screenSize.y;
  static float varJumpTimer = 0.0f;
  static float wallJumpTimer = 0.0f;
  static bool playerGrounded = true;
  static bool grabbingWall = false;
  static float dashTimer = 0.0f;
  static int dashCounter = playerGrounded;

  for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
  {
    Solid* solid = &solids[solidIdx];
    solid->prevPos = solid->rect.pos;
    solid->prevRemainder = solid->remainder;

    if(solid->keyframes.count > 1)
    {
      solid->time += dt;

      int nextKeyframeIdx = 1;
      
      bool sdfkljsdfklsjkldfj = false;
      for(int keyframeIdx = 0; keyframeIdx < solid->keyframes.count;
          keyframeIdx++)
      {
        if(solid->keyframes[keyframeIdx].time > solid->time)
        {
          sdfkljsdfklsjkldfj = true;
          nextKeyframeIdx = keyframeIdx;
          break;
        }
      } 

      if(!sdfkljsdfklsjkldfj)
      {
        solid->time -= solid->keyframes[solid->keyframes.count - 1].time;
      }

      int currentKeyframeIdx = nextKeyframeIdx - 1;
      if(currentKeyframeIdx < 0)
      {
        currentKeyframeIdx = solid->keyframes.count - 1;
      }

      Keyframe currentKeyframe = solid->keyframes[currentKeyframeIdx];
      Keyframe nextKeyframe  = solid->keyframes[nextKeyframeIdx];

      float t = (solid->time - currentKeyframe.time) / 
                (nextKeyframe.time - currentKeyframe.time);

      Vec2 nextPos = vec_2(currentKeyframe.pos) + vec_2(nextKeyframe.pos - currentKeyframe.pos) * t;
      float speedX = nextPos.x - (float)solid->rect.pos.x;
      // Move X
      {
        float amount = speedX;
        solid->remainder.x += amount; 
        int move = round(solid->remainder.x);   
        if (move != 0) 
        { 
          solid->remainder.x -= move; 
          int moveSign = sign(move);
          while (move != 0) 
          { 
            bool collisionHappened = false;

            IRect solidRect = solids[solidIdx].rect;
            IRect newSolidRect = solidRect;
            newSolidRect.pos.x += moveSign;
            IRect playerRect = get_player_rect();
            playerRect.pos.y -= 2;
            playerRect.size.y += 4;

            // Is Celeste currently standing on this Solid,
            // or is she getting pushed
            if(rect_collision(playerRect, newSolidRect))
            {
              if(solidRect.pos.y <= playerRect.pos.y + playerRect.size.y)
              {
                gameState->playerPos.x += moveSign;
                solidSpeed.x = speedX;
              }
                
              for(int subSolidIdx = 0; subSolidIdx < ArraySize(solids);
                  subSolidIdx++)
              {
                if(subSolidIdx == solidIdx)
                {
                  continue;
                }

                IRect subSolidRect = solids[subSolidIdx].rect;
                if(rect_collision(get_player_rect(), subSolidRect))
                {
                  gameState->playerPos = {50, 0};
                }
              }
            }

            solid->rect.pos.x += moveSign; 
            move -= moveSign; 
          } 
        }
      }

      // Move Y
      float speedY = nextPos.y - (float)solid->rect.pos.y;
      {
        float amount = nextPos.y - (float)solid->rect.pos.y;
        solid->remainder.y += amount; 
        int move = round(solid->remainder.y);   
        if (move != 0) 
        { 
          solid->remainder.y -= move; 
          int moveSign = sign(move);
          while (move != 0) 
          { 
            bool collisionHappened = false;

            IRect solidRect = solids[solidIdx].rect;
            IRect newSolidRect = solidRect;
            newSolidRect.pos.x += moveSign;
            IRect playerRect = get_player_rect();
            playerRect.pos.y -= 2;
            playerRect.size.y += 4;

            // Is the player currently standing on this Solid
            if(rect_collision(playerRect, newSolidRect))
            {
              standingOnPlatform = true;

              if(solidRect.pos.y <= playerRect.pos.y + playerRect.size.y)
              {
                gameState->playerPos.y += moveSign;
                solidSpeed.y = speedY;
              }

              for(int subSolidIdx = 0; subSolidIdx < ArraySize(solids);
                  subSolidIdx++)
              {
                if(subSolidIdx == solidIdx)
                {
                  continue;
                }

                IRect subSolidRect = solids[subSolidIdx].rect;
                if(rect_collision(get_player_rect(), subSolidRect))
                {
                  gameState->playerPos = {8 * 2, 8 * 2};
                }
              }
            }

            solid->rect.pos.y += moveSign; 
            move -= moveSign; 
          } 
        } 
      }
    }
  }

  if(key_pressed_this_frame(KEY_R))
  {
    gameState->playerPos = {8 * 2, 8 * 2};
  }

  float directionChangeMult = 1.6f;
  if(is_down(INPUT_MOVE_LEFT) &&
     !is_down(INPUT_MOVE_RIGHT))
  {
    float mult = 1.0f;
    if(speed.x > 0.0f)
    {
      mult = directionChangeMult;
    }

    if(playerGrounded)
    {
      speed.x = approach(speed.x, -maxRunSpeed, runAcceleration * mult * dt);
    }
    else
    {
      speed.x = approach(speed.x, -maxRunSpeed, fallSideAcceleration * mult * dt);
    }

  }

  if(is_down(INPUT_MOVE_RIGHT) &&
     !is_down(INPUT_MOVE_LEFT))
  {
    float mult = 1.0f;
    if(speed.x < 0.0f)
    {
      mult = directionChangeMult;
    }

    if(playerGrounded)
    {
      speed.x = approach(speed.x, maxRunSpeed, runAcceleration * mult * dt);
    }
    else
    {
      speed.x = approach(speed.x, maxRunSpeed, fallSideAcceleration * mult * dt);
    }
  }

  // friction
  if(!is_down(INPUT_MOVE_LEFT) &&
     !is_down(INPUT_MOVE_RIGHT))
  {
    speed.x = approach(speed.x, 0, runReduce * dt);
  }

  // Jumping
  {
    if(just_pressed(INPUT_JUMP) && playerGrounded)
    {
      play_sound(gameState->jumpSound);
      varJumpTimer = 0.0f;
      speed.y = maxJumpSpeed + solidSpeed.y * 1.5f;
      float xMulti = 2.5f;
      if(solidSpeed.x)
      {
        if(speed.x < 0 && solidSpeed.x > 0 ||
          speed.x > 0 && solidSpeed.x < 0)
        {
          xMulti = 0.5f;
        }
        speed.x = solidSpeed.x * xMulti;
      }
      playerGrounded = false;
      gameState->gameInput[INPUT_JUMP].justPressed = false;
    }

    if(is_down(INPUT_JUMP) &&
      varJumpTimer < 0.1f)
    {
      speed.y = min(speed.y, maxJumpSpeed);
    }

    if(just_pressed(INPUT_JUMP))
    {
      IRect playerRect = get_player_rect();
      playerRect.pos.x -= 2;
      playerRect.size.x += 4;

      for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
      {
        IRect solidRect = solids[solidIdx].rect;

        if(rect_collision(solidRect, playerRect))
        {
          int playerRectLeft = playerRect.pos.x;
          int playerRectRight = playerRect.pos.x + playerRect.size.x;
          int solidRectLeft = solidRect.pos.x;
          int solidRectRight = solidRect.pos.x + solidRect.size.x;

          // Colliding on the Right
          if(solidRectRight - playerRectLeft <
            playerRectRight - solidRectLeft)
          {
            wallJumpTimer = 0.1f;
            varJumpTimer = 0.0f;
            speed.x = wallJumpSpeed;
            speed.y = maxJumpSpeed;

            play_sound(gameState->jumpSound);
            break;
          }

          // Colliding on the Left
          if(solidRectRight - playerRectLeft >
            playerRectRight - solidRectLeft)
          {
            wallJumpTimer = 0.1f;
            varJumpTimer = 0.0f;
            speed.x = -wallJumpSpeed;
            speed.y = maxJumpSpeed;

            play_sound(gameState->jumpSound);
            break;
          }

        }
      }

      Room* room = get_current_room();
      for(int x = 0; x < WORLD_SIZE.x; x++)
      {
        for(int y = 0; y < WORLD_SIZE.y; y++)
        {
          Tile* tile = get_tile(room, x, y);

          if(tile->type)
          {
            IRect tileRect = IRect{x * 8, y * 8, 8, 8};
            if(rect_collision(tileRect, playerRect))
            {
              int playerRectLeft = playerRect.pos.x;
              int playerRectRight = playerRect.pos.x + playerRect.size.x;
              int tileRectLeft = tileRect.pos.x;
              int tileRectRight = tileRect.pos.x + tileRect.size.x;

              // Colliding on the Right
              if(tileRectRight - playerRectLeft <
                playerRectRight - tileRectLeft)
              {
                wallJumpTimer = 0.1f;
                varJumpTimer = 0.0f;
                speed.x = wallJumpSpeed;
                speed.y = maxJumpSpeed;
                play_sound(gameState->jumpSound);
                break;
              }

              // Colliding on the Left
              if(tileRectRight - playerRectLeft >
                playerRectRight - tileRectLeft)
              {
                wallJumpTimer = 0.1f;
                varJumpTimer = 0.0f;
                speed.x = -wallJumpSpeed;
                speed.y = maxJumpSpeed;
                play_sound(gameState->jumpSound);
                break;
              }

            }
          }
        }
      }
    }

    varJumpTimer += dt;
  }

  // Dash
  if(just_pressed(INPUT_DASH) && dashCounter > 0)
  {
    speed.y = 0.0f;

    Vec2 dir = {0.0f, 0.0f};

    if(!is_down(INPUT_MOVE_LEFT) &&
       !is_down(INPUT_MOVE_RIGHT) &&
       !is_down(INPUT_MOVE_UP) &&
       !is_down(INPUT_MOVE_DOWN))
    {
      if(renderOptions & RENDERING_OPTION_FLIP_X)
      {
        // Left
        dir.x = -1.0f;
      }
      else
      {
        // Right
        dir.x = 1.0f;
      }

    }

    if(is_down(INPUT_MOVE_LEFT) &&
       !is_down(INPUT_MOVE_RIGHT))
    {
      dir.x = -1.0f;
    }

    if(is_down(INPUT_MOVE_RIGHT) &&
       !is_down(INPUT_MOVE_LEFT))
    {
      dir.x = 1.0f;
    }

    if(is_down(INPUT_MOVE_UP) &&
      !is_down(INPUT_MOVE_DOWN))
    {
      dir.y = -1.0f;
    }

    if(is_down(INPUT_MOVE_DOWN) &&
      !is_down(INPUT_MOVE_UP))
    {
      dir.y = 1.0f;
    }

    if(!is_down(INPUT_MOVE_UP) &&
       !is_down(INPUT_MOVE_DOWN))
    {
      dashTimer = 0.1f;
    }


    dir = normalize(dir);
    IVec2 newSpeed = {};
    newSpeed.x = (int)(dir.x * dashSpeed);
    newSpeed.y = (int)(dir.y * dashSpeed);

    // If our current Speed in X is "faster" then we just keep that
    if(sign(newSpeed.x) != sign(newSpeed.x) || abs((long)speed.x) < abs((long)newSpeed.x))
    {     
      speed.x = newSpeed.x;
    }

    speed.y = newSpeed.y;

    dashCounter--;
  }
  else
  {
    dashTimer = max(0.0f, dashTimer - dt);
  }

  // Gravity
  if(!grabbingWall && dashTimer == 0.0f)
  {
    speed.y = approach(speed.y, fallSpeed, gravity * dt);
  }

  // Wall Grabbing
  {
    grabbingWall = false;
    if(is_down(INPUT_WALL_GRAB) && 
       wallJumpTimer == 0.0f)
    {
      IRect playerRect = get_player_rect();
      playerRect.pos.x -= 2;
      playerRect.size.x += 4;

      for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
      {
        IRect solidRect = solids[solidIdx].rect;

        if(rect_collision(solidRect, playerRect))
        {
          int playerRectLeft = playerRect.pos.x;
          int playerRectRight = playerRect.pos.x + playerRect.size.x;
          int solidRectLeft = solidRect.pos.x;
          int solidRectRight = solidRect.pos.x + solidRect.size.x;
          
          // Colliding on the Right
          if(solidRectRight - playerRectLeft <
             playerRectRight - solidRectLeft)
          {
            speed.x = 0;
            grabbingWall = true;
          }

          // Colliding on the Left
          if(solidRectRight - playerRectLeft >
             playerRectRight - solidRectLeft)
          {
            speed.x = 0;
            grabbingWall = true;
          }
        }
      }

      Room* room = get_current_room();
      for(int x = 0; x < WORLD_SIZE.x; x++)
      {
        for(int y = 0; y < WORLD_SIZE.y; y++)
        {
          Tile* tile = get_tile(x, y);

          if(tile->type)
          {
            IRect tileRect = IRect{x * 8, y * 8, 8, 8};
            if(rect_collision(tileRect, playerRect))
            {
              int playerRectLeft = playerRect.pos.x;
              int playerRectRight = playerRect.pos.x + playerRect.size.x;
              int tileRectLeft = tileRect.pos.x;
              int tileRectRight = tileRect.pos.x + tileRect.size.x;
              
              // Colliding on the Right
              if(tileRectRight - playerRectLeft <
                playerRectRight - tileRectLeft)
              {
                speed.x = 0;
                grabbingWall = true;
              }

              // Colliding on the Left
              if(tileRectRight - playerRectLeft >
                playerRectRight - tileRectLeft)
              {
                speed.x = 0;
                grabbingWall = true;
              }
            }
          }
        }
      }
    }

    wallJumpTimer = max(0.0f, wallJumpTimer - dt);

    if(grabbingWall &&
      is_down(INPUT_MOVE_UP))
    {
      float mult = 1.0f;
      if(speed.y > 0.0f)
      {
        mult = directionChangeMult;
      }
      speed.y = approach(speed.y, wallClimbSpeed, runAcceleration * mult * dt);
    }

    if(grabbingWall &&
      is_down(INPUT_MOVE_DOWN))
    {
      float mult = 1.0f;
      if(speed.y < 0.0f)
      {
        mult = directionChangeMult;
      }
      speed.y = approach(speed.y, wallSlideDownSpeed, runAcceleration * mult * dt);
    }

    // friction
    if(grabbingWall &&
      !is_down(INPUT_MOVE_UP) &&
      !is_down(INPUT_MOVE_DOWN))
    {
      speed.y = approach(speed.y, 0, runReduce * dt);
    }
  }

  // Move X from https://maddythorson.medium.com/celeste-and-towerfall-physics-d24bd2ae0fc5
  {
    float amount = speed.x;
    xRemainder += amount; 
    int move = round(xRemainder);   
    if (move != 0) 
    { 
      xRemainder -= move; 
      int moveSign = sign(move);
      bool collisionHappened = false;
      bool spikeCollision = false;
      while (move != 0) 
      { 
        IRect playerRect = get_player_rect();
        IRect newPlayerRect = playerRect;
        newPlayerRect.pos.x += moveSign;

        for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
        {
          IRect solidRect = solids[solidIdx].rect;

          if(rect_collision(solidRect, newPlayerRect))
          {
            collisionHappened = true;
            break;
          }
        }

        if(!collisionHappened)
        {
          Room* room = get_current_room();
          for(int x = 0; x < WORLD_SIZE.x; x++)
          {
            for(int y = 0; y < WORLD_SIZE.y; y++)
            {
              Tile* tile = get_tile(room, x, y);

              if(tile->type)
              {
                IRect tileRect = IRect{x * 8, y * 8, 8, 8};
                if(tile->type == TILE_TYPE_SPIKE)
                {
                  tileRect.pos.y += 4;
                  tileRect.size.y -= 4;
                }

                if(rect_collision(tileRect, newPlayerRect))
                {
                  collisionHappened = true;
                  if(tile->type == TILE_TYPE_SPIKE && 
                     deathAnimTimer == DEATH_ANIM_TIME)
                  {
                    spikeCollision = true;
                    deathAnimTimer = 0.0f;
                    speed = normalize(-speed) * dashSpeed;
                    play_sound(gameState->deathSound);
                  }
                  goto handle_collision;
                }
              }
            }
          }

          handle_collision:

          if(!collisionHappened)
          {
            //There is no Solid immediately beside us, move
            gameState->playerPos.x += moveSign; 
            move -= moveSign; 
          }
          else
          {
            speed.x = 0.0f;
            xRemainder = 0.0f;
            break;
          }

        }
        else
        {
          //Hit a solid! Don't move!
          break;
        }
      } 
    } 
  }

  // Move Y 
  {
    float amount = speed.y;
    yRemainder += amount; 
    int move = round(yRemainder);
    if (move != 0) 
    { 
      yRemainder -= move; 
      int moveSign = sign(move);
      bool jumpingCollisionHappend = false;
      bool collisionHappened = false;
      bool spikeCollision = false;
      while (move != 0) 
      { 
        IRect playerRect = get_player_rect();
        IRect newPlayerRect = playerRect;
        newPlayerRect.pos.y += moveSign;

        for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
        {
          IRect solidRect = solids[solidIdx].rect;

          if(rect_collision(solidRect, newPlayerRect))
          {
            if(speed.y > 0.0f)
            {
              playerGrounded = true;
              dashCounter = 1;
            }

            collisionHappened = true;
            break;
          }
        }

        if(!collisionHappened)
        {
          Room* room = get_current_room();
          for(int x = 0; x < WORLD_SIZE.x; x++)
          {
            for(int y = 0; y < WORLD_SIZE.y; y++)
            {
              Tile* tile = get_tile(room, x, y);

              if(tile->type)
              {
                IRect tileRect = IRect{x * 8, y * 8, 8, 8};
                if(tile->type == TILE_TYPE_SPIKE)
                {
                  tileRect.pos.y += 4;
                  tileRect.size.y -= 4;
                }

                if(rect_collision(tileRect, newPlayerRect))
                {
                  if(speed.y > 0.0f)
                  {
                    playerGrounded = true;
                    dashCounter = 1;
                  }

                  if(tile->type == TILE_TYPE_SPIKE &&
                     deathAnimTimer == DEATH_ANIM_TIME)
                  {
                    spikeCollision = true;
                    deathAnimTimer = 0.0f;
                    speed = normalize(-speed) * dashSpeed;
                    play_sound(gameState->deathSound);
                  }

                  collisionHappened = true;
                  goto handle_collision2;
                }
              }
            }
          }

          handle_collision2:
          if(!collisionHappened)
          {
            // There is no Solid immediately beside us, move
            gameState->playerPos.y += moveSign; 
            move -= moveSign; 
          }
          else
          {
            // Hit a solid! Don't move!
            if(moveSign < 0)
            {
              speed.y = 0.0f;
            }
            break;
          }

          if(jumpingCollisionHappend)
          {
            playerGrounded = true;
          }
        }
        else
        {
          // Hit a solid! Don't move!
          if(moveSign < 0)
          {
            speed.y = 0.0f;
          }
          break;
        }
      } 
    } 
  }
}
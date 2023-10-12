#include "game.h"
#include "assets.h"
#include "input.h"
#include "render_interface.h"

// #############################################################################
//                           Game Constants
// #############################################################################
constexpr float DEATH_ANIM_TIME = 0.25f;

// #############################################################################
//                           Game Globals
// #############################################################################
static BumpAllocator* transientStorage;
static GameState* gameState;

// #############################################################################
//                           Game Functions
// #############################################################################
// Input
void update_game_input(float dt);
bool is_down(GameInputType type);
bool just_pressed(GameInputType type);

// Tiles
IVec2 get_world_pos(Vec2 mousePos);
IVec2 get_player_coords();
Tile* get_tile(Tile* tiles, int x, int y);
Tile* get_tile_fg(int x, int y);
Tile* get_tile_bg(int x, int y);
Tile* get_tile(IVec2 worldPos);
IVec2 get_tile_center(int x, int y);
IRect get_tile_rect(int x, int y);

// Solids
IRect get_solid_rect(Solid solid);

// Player
int get_room_idx();
IRect get_player_rect();
int animate(float* time, int frameCount, float loopTime = 1.0f);
void update_player(float dt);

// Disconnected Rendering and Updating
void draw_tile_map(TileMap* tileMap);
void draw(float interpolatedDT);
void update_level(float dt);
void update();

// #############################################################################
//                           Update Game (Exported from DLL)
// #############################################################################
EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, 
                           RenderData* renderDataIn, SoundState* soundStateIn,
                           UIState* uiStateIn, BumpAllocator* transientStorageIn,
                           double frameTime)
{
  if(gameState != gameStateIn)
  {
    input = inputIn;
    renderData = renderDataIn;
    gameState = gameStateIn;
    soundState = soundStateIn;
    uiState = uiStateIn;
    transientStorage = transientStorageIn;

    // Sounds
    char* jumpSound = "assets/sounds/jump_01.wav";
    char* deathSound = "assets/sounds/died_02.wav";
    memcpy(gameState->jumpSound.path, jumpSound, strlen(jumpSound));
    memcpy(gameState->deathSound.path, deathSound, strlen(deathSound));
  }

  if(!gameState->initialized)
  {
    // Player
    gameState->player.animationSprites [ANIMATION_STATE_IDLE] = SPRITE_CELESTE_01;
    gameState->player.animationSprites [ANIMATION_STATE_JUMP] = SPRITE_CELESTE_01_JUMP;
    gameState->player.animationSprites [ANIMATION_STATE_RUN] = SPRITE_CELESTE_01_RUN;
    gameState->player.deathAnimTimer = DEATH_ANIM_TIME;
    gameState->level.playerStartPos = {0, -4 * 8};
    gameState->player.pos = gameState->level.playerStartPos;
    gameState->player.prevPos = gameState->player.pos;

    bool loadedLevel = false;
    if(file_exists("level.bin"))
    {
      int fileSize;
      Level* level = (Level*)read_file("level.bin", &fileSize, transientStorage);

      if(fileSize == sizeof(Level))
      {
        gameState->level = *level;
        loadedLevel = true;
      }
    }

    // Init Level
    if(!loadedLevel)
    {
      // Solids
      {
        gameState->level.solids.add(
        {
          .spriteID = SPRITE_SOLID_01,
          .prevPos = {2 * 8,  -10 * 8},
          .pos = {2 * 8,  -10 * 8},
        });
        gameState->level.solids.add(
        {
          .spriteID = SPRITE_SOLID_01,
          .prevPos = {0 * 8, -10 * 8},
          .pos = {0 * 8, -10 * 8},
          .keyframes = 
          {
            .count = 3,
            .elements =
            {
              {{8 * 0, -10 * 8}, 0.0f},
              {{8 * 8, -10 * 8}, 0.5f},
              {{8 * 0, -10 * 8}, 1.0f},
              // {{8 * 0, 8 * 0}, 2.0f},
            }
          }
        });
        gameState->level.solids.add(
        {
          .spriteID = SPRITE_SOLID_02,
          .prevPos = {-5 * 8, -10 * 8},
          .pos = {-5 * 8, -10 * 8},
          .keyframes = 
          {
            .count = 3,
            .elements =
            {
              {{-5 * 8, -10 * 8}, 0.0f},
              {{ 0 * 8, -10 * 8}, 0.5f},
              {{-5 * 8, -10 * 8}, 1.0f},
              // {{8 * 0, 8 * 0}, 2.0f},
            }
          }
        });
      }

      // Foreground Tileset
      {
        Tileset* tileset = &gameState->level.tileMap.tileset;
        IVec2 tilesPosition = {192, 0};
        for(int y = 0; y < 5; y++)
        {
          for(int x = 0; x < 4; x++)
          {
            tileset->tileCoords.add({tilesPosition.x +  x * 8, 
                                    tilesPosition.y + y * 8});
          }
        }

        // Black inside
        tileset->tileCoords.add({tilesPosition.x, 
                                tilesPosition.y + 5 * 8});
      }

      // Background Tileset
      {
        Tileset* tileset = &gameState->level.bgTileMap.tileset;
        IVec2 tilesPosition = {192, 48};
        for(int y = 0; y < 5; y++)
        {
          for(int x = 0; x < 4; x++)
          {
            tileset->tileCoords.add({tilesPosition.x +  x * 8, 
                                    tilesPosition.y + y * 8});
          }
        }

        // Black inside
        tileset->tileCoords.add({tilesPosition.x, 
                                tilesPosition.y + 5 * 8});
      }
    }

    // Game Camera
    renderData->gameCamera.position.y = -90.0f;
    renderData->gameCamera.dimensions.x = ROOM_WIDTH;
    renderData->gameCamera.dimensions.y = ROOM_HEIGHT;
    renderData->gameCamera.zoom = 1.0f;
    renderData->gameCamera.position.y = 
      renderData->gameCamera.dimensions.y / 2.0f;
    gameState->cameraTimer = 1.0f;

    // UI Camera
    renderData->uiCamera.dimensions.x = ROOM_WIDTH; // 320
    renderData->uiCamera.dimensions.y = ROOM_HEIGHT; // 180
    // Top Left is going to be 0/0 now
    renderData->uiCamera.position.x = renderData->uiCamera.dimensions.x / 2.0f;
    renderData->uiCamera.position.y = -renderData->uiCamera.dimensions.y / 2.0f;
    renderData->uiCamera.zoom = 1.0f;

    gameState->initialized = true;
  }

  gameState->updateTimer += frameTime;
  while(gameState->updateTimer >= UPDATE_DELAY)
  {
    gameState->updateTimer -= UPDATE_DELAY;
    update();

    // Reset Input
    // input->wheelDelta = 0;
    input->relMouse = {};
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
//                           Implementations Input
// #############################################################################
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
  gameState->gameInput[INPUT_DASH].justPressed  = input->keys[KEY_C].justPressed;
  gameState->gameInput[INPUT_DASH].justPressed |= input->keys[KEY_Q].justPressed;
  gameState->gameInput[INPUT_DASH].justPressed |= input->keys[KEY_E].justPressed;
  gameState->gameInput[INPUT_DASH].justPressed |= input->keys[KEY_C].justPressed;
}

bool is_down(GameInputType type)
{
  return gameState->gameInput[type].isDown;
}

bool just_pressed(GameInputType type)
{
  return gameState->gameInput[type].justPressed;
}

// #############################################################################
//                           Implementations Tiles
// #############################################################################
IVec2 get_world_pos(IVec2 mousePos)
{
  Vec2 localPos = vec_2(mousePos) / worldScale;
  localPos.x += -renderData->gameCamera.dimensions.x / 2.0f + renderData->gameCamera.position.x;
  localPos.y = -(localPos.y - renderData->gameCamera.dimensions.y / 2.0f + renderData->gameCamera.position.y);
  return ivec_2(localPos);
}

IVec2 get_player_coords()
{
  // return {gameState->player.pos
  return {};
}

Tile* get_tile(Tile* tiles, int x, int y)
{
  if(x < 0 || x >= WORLD_SIZE.x || y < 0 || y >= WORLD_SIZE.y) return nullptr;
  return &tiles[y * WORLD_SIZE.x + x];
}

Tile* get_tile_fg(int x, int y)
{
  return get_tile(gameState->level.tileMap.tiles, x, y);
}

Tile* get_tile_bg(int x, int y)
{
  return get_tile(gameState->level.bgTileMap.tiles, x, y);
}

Tile* get_tile(IVec2 worldPos)
{
  int x = (worldPos.x + renderData->gameCamera.dimensions.x / 2.0f)/ TILESIZE;
  int y = (-worldPos.y + TILESIZE / 2) / TILESIZE;

  SM_TRACE("X: %d, Y: %d", x, y);
  SM_TRACE("Player Pos: X: %d, Y: %d", gameState->player.pos.x, 
                                       gameState->player.pos.y);

  if(key_is_down(KEY_CONTROL))
  {
    return get_tile_bg(x, y);
  }

  return get_tile_fg(x, y);
}

IVec2 get_tile_center(int x, int y)
{
  return IVec2{x * TILESIZE - ROOM_WIDTH / 2 + TILESIZE / 2, 
               -y * TILESIZE};
}

IRect get_tile_rect(int x, int y)
{
  IVec2 tileCenter = get_tile_center(x, y);
  return {tileCenter.x - TILESIZE / 2, 
          tileCenter.y - TILESIZE / 2, 
          TILESIZE, TILESIZE};
}

// #############################################################################
//                           Implementations Solids
// #############################################################################
IRect get_solid_rect(Solid solid)
{
  Sprite sprite = get_sprite(solid.spriteID);
  return {solid.pos.x - sprite.size.x / 2, 
          solid.pos.y - sprite.size.y / 2, 
          sprite.size};
}

// #############################################################################
//                           Implementations Player
// #############################################################################
int get_room_idx()
{
  int roomIdx = -gameState->player.pos.y / 180;
  roomIdx = clamp(roomIdx, 0, WORLD_HEIGHT / ROOM_HEIGHT - 1);
  return roomIdx;
}

IRect get_player_rect()
{
  return 
  {
    gameState->player.pos.x - 4, 
    gameState->player.pos.y - 8, 
    8, 
    16
  };
}

int animate(float* time, int frameCount, float loopTime)
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

void update_player(float dt)
{
  // Movement Data needed for Celeste
  float maxRunSpeed = 2.0f;
  float wallJumpSpeed = 3.0f;
  float runAcceleration = 12.0f;
  float fallSideAcceleration = 10.0f;
  float maxJumpSpeed = -3.0f;
  float fallSpeed = 3.6f;
  float dashSpeed = 4.2f;
  float gravity = 13.0f;
  float runReduce = 22.0f;
  float flyReduce = 12.0f;
  float wallClimbSpeed = 1.2f;
  float wallSlideDownSpeed = 2.2f;
  float directionChangeMult = 1.6f;

  // Static variables that keep the state of the data
  // Other functions don't need access to this data
  // which is why it's here
  static Vec2 speed;
  static float xRemainder;
  static float yRemainder;
  static float varJumpTimer;
  static float wallJumpTimer;
  static float dashTimer;
  static bool playerGrounded = true;
  static bool grabbingWall = false;
  static int dashCounter = playerGrounded;

  gameState->player.prevPos = gameState->player.pos;
  gameState->player.animationState = ANIMATION_STATE_IDLE;

  // Make Celeste face into the direction that she is walking in
  {
    if(speed.x > 0)
    {
      gameState->player.renderOptions = 0;
    }

    if(speed.x < 0)
    {
      gameState->player.renderOptions |= RENDERING_OPTION_FLIP_X ;
    }
  }

  // Death Animation
  {
    float prevDeathAnimTimer = gameState->player.deathAnimTimer;
    gameState->player.deathAnimTimer  = 
      min(DEATH_ANIM_TIME, gameState->player.deathAnimTimer + dt);
    if (prevDeathAnimTimer < DEATH_ANIM_TIME && 
        gameState->player.deathAnimTimer == DEATH_ANIM_TIME)
    {
      gameState->player.pos = gameState->level.playerStartPos;
    }
  }

  // Running
  {
    if(is_down(INPUT_MOVE_LEFT) &&
      !is_down(INPUT_MOVE_RIGHT))
    {
      if(!playerGrounded)
      {
        gameState->player.animationState = ANIMATION_STATE_JUMP;
      }
      else
      {
        gameState->player.runAnimTimer += dt;
        gameState->player.animationState = ANIMATION_STATE_RUN;
      }

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
      if(!playerGrounded)
      {
        gameState->player.animationState = ANIMATION_STATE_JUMP;
      }
      else
      {
        gameState->player.runAnimTimer += dt;
        gameState->player.animationState = ANIMATION_STATE_RUN;
      }

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

    // Friction
    if(!is_down(INPUT_MOVE_LEFT) &&
      !is_down(INPUT_MOVE_RIGHT))
    {
      if(playerGrounded)
      {
        speed.x = approach(speed.x, 0, runReduce * dt);
      }
      else
      {
        speed.x = approach(speed.x, 0, flyReduce * dt);
      }
    }
  }

  // Jumping
  {
    if(just_pressed(INPUT_JUMP) && playerGrounded)
    {
      play_sound(gameState->jumpSound);
      varJumpTimer = 0.0f;
      speed.y = maxJumpSpeed + gameState->player.solidSpeed.y * 1.5f;
      float xMulti = 2.5f;
      if(gameState->player.solidSpeed.x)
      {
        if(speed.x < 0 && gameState->player.solidSpeed.x > 0 ||
          speed.x > 0 && gameState->player.solidSpeed.x < 0)
        {
          xMulti = 0.5f;
        }
        speed.x = gameState->player.solidSpeed.x * xMulti;
      }
      playerGrounded = false;
      gameState->gameInput[INPUT_JUMP].justPressed = false;
    }

    if(is_down(INPUT_JUMP) &&
      varJumpTimer < 0.1f)
    {
      speed.y = max(speed.y, maxJumpSpeed);
    }

    if(just_pressed(INPUT_JUMP))
    {
      IRect playerRect = get_player_rect();
      playerRect.pos.x -= 2;
      playerRect.size.x += 4;

      for(int solidIdx = 0; solidIdx < gameState->level.solids.count; solidIdx++)
      {
        IRect solidRect = get_solid_rect(gameState->level.solids[solidIdx]);

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

            // Consume input
            gameState->gameInput[INPUT_JUMP].bufferingTime = 0.0f;
            gameState->gameInput[INPUT_JUMP].justPressed = false;
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

            // Consume input
            gameState->gameInput[INPUT_JUMP].bufferingTime = 0.0f;
            gameState->gameInput[INPUT_JUMP].justPressed = false;
            break;
          }

        }
      }

      for(int x = 0; x < WORLD_SIZE.x; x++)
      {
        for(int y = 0; y < WORLD_SIZE.y; y++)
        {
          Tile* tile = get_tile_fg(x, y);

          if(tile->type)
          {
            IVec2 tilePos = get_tile_center(x, y);
            IRect tileRect = get_tile_rect(x, y);
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

                // Consume input
                gameState->gameInput[INPUT_JUMP].bufferingTime = 0.0f;
                gameState->gameInput[INPUT_JUMP].justPressed = false;
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

                // Consume input
                gameState->gameInput[INPUT_JUMP].bufferingTime = 0.0f;
                gameState->gameInput[INPUT_JUMP].justPressed = false;
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
      // Without Input, dash into the direction the player is facing
      if(gameState->player.renderOptions & RENDERING_OPTION_FLIP_X)
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

      for(int solidIdx = 0; solidIdx < gameState->level.solids.count; solidIdx++)
      {
        IRect solidRect = get_solid_rect(gameState->level.solids[solidIdx]);

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

      for(int x = 0; x < WORLD_SIZE.x; x++)
      {
        for(int y = 0; y < WORLD_SIZE.y; y++)
        {
          Tile* tile = get_tile_fg(x, y);

          if(tile->type)
          {
            IVec2 tilePos = get_tile_center(x, y);
            IRect tileRect = get_tile_rect(x, y);
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

        for(int solidIdx = 0; solidIdx < gameState->level.solids.count; solidIdx++)
        {
          IRect solidRect = get_solid_rect(gameState->level.solids[solidIdx]);

          if(rect_collision(solidRect, newPlayerRect))
          {
            collisionHappened = true;
            break;
          }
        }

        if(!collisionHappened)
        {
          for(int x = 0; x < WORLD_SIZE.x; x++)
          {
            for(int y = 0; y < WORLD_SIZE.y; y++)
            {
              Tile* tile = get_tile_fg(x, y);

              if(tile->type)
              {
                IVec2 tilePos = get_tile_center(x, y);
                IRect tileRect = get_tile_rect(x, y);
                if(tile->type == TILE_TYPE_SPIKE)
                {
                  tileRect.pos.y += 4;
                  tileRect.size.y = 4;
                }

                if(rect_collision(tileRect, newPlayerRect))
                {
                  collisionHappened = true;
                  if(tile->type == TILE_TYPE_SPIKE && 
                     gameState->player.deathAnimTimer == DEATH_ANIM_TIME)
                  {
                    spikeCollision = true;
                    gameState->player.deathAnimTimer = 0.0f;
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
            gameState->player.pos.x += moveSign; 
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
    yRemainder += speed.y; 
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

        for(int solidIdx = 0; solidIdx < gameState->level.solids.count; solidIdx++)
        {
          IRect solidRect = get_solid_rect(gameState->level.solids[solidIdx]);

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
          for(int x = 0; x < WORLD_SIZE.x; x++)
          {
            for(int y = 0; y < WORLD_SIZE.y; y++)
            {
              Tile* tile = get_tile_fg(x, y);

              if(tile->type)
              {
                IVec2 tilePos = get_tile_center(x, y);
                IRect tileRect = get_tile_rect(x, y);
                if(tile->type == TILE_TYPE_SPIKE)
                {
                  tileRect.pos.y += 4;
                  tileRect.size.y = 4;
                }

                if(rect_collision(tileRect, newPlayerRect))
                {
                  if(speed.y > 0.0f)
                  {
                    playerGrounded = true;
                    dashCounter = 1;
                  }

                  if(tile->type == TILE_TYPE_SPIKE &&
                     gameState->player.deathAnimTimer == DEATH_ANIM_TIME)
                  {
                    spikeCollision = true;
                    gameState->player.deathAnimTimer = 0.0f;
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
            gameState->player.pos.y += moveSign; 
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

// #############################################################################
//                 Implementations Rendering and Updating
// #############################################################################
void draw_tile_map(TileMap* tileMap)
{
  // BG Tiles
  {
    Tileset tileset = tileMap->tileset;

    // Neighbouring Tiles       Top    Left  Right Bottom  
    int neighbourOffsets[24] = { 0,1,  -1,0,  1,0,  0,-1,   
    //                         Topleft Topright Bottomleft Bottomright
                                -1,1,   1,1,     -1,-1,      1,-1,
    //                          Top2   Left2  Right2 Bottom2
                                 0,2,  -2,0,  2,0,  0,-2};

    // Topleft     = BIT(4) = 16
    // Toplright   = BIT(5) = 32
    // Bottomleft  = BIT(6) = 64
    // Bottomright = BIT(7) = 128

    for(int x = 0; x < WORLD_SIZE.x; x++)
    {
      for(int y = 0; y < WORLD_SIZE.y; y++)
      {
        Tile* tile = get_tile(tileMap->tiles, x, y);

        if(!tile->type)
        {
          continue;
        }

        if(tile->type == TILE_TYPE_SPIKE)
        {
          draw_sprite(SPRITE_SPIKE, vec_2(get_tile_center(x, y)));
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
          Tile* neighbour = get_tile(tileMap->tiles,
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
        // Draw the Tile around the center
        IRect tileRect = get_tile_rect(x, y);
        transform.pos = vec_2(tileRect.pos);
        transform.size = vec_2(tileRect.size);
        transform.spriteSize = vec_2(tileRect.size);
        transform.atlasOffset = vec_2(tileset.tileCoords[tile->neighbourMask]);
        draw_quad(transform);
      }
    }
  }
}

void draw(float interpDT)
{
  Vec4 clearColor = {79.0f / 255.0f, 140.0f / 255.0f, 235.0f / 255.0f, 1.0f};
  renderData->clearColor = clearColor * (renderData->gameCamera.position.y / ((float)ROOM_HEIGHT * 100.0f));

  // Game ortho projection
  {
    OrthographicCamera2D camera = renderData->gameCamera;
    float zoom = camera.zoom? camera.zoom : 1.0f;
    Vec2 dimensions = camera.dimensions * 1.0f / zoom;
    Vec2 position = camera.position * (float)max((int)(1.0f / zoom), 1);
    renderData->orthoProjectionGame = 
      orthographic_projection(position.x - dimensions.x / 2.0f, 
                              position.x + dimensions.x / 2.0f, 
                              position.y - dimensions.y / 2.0f, 
                              position.y + dimensions.y / 2.0f);
  }

  // UI ortho projection
  {
    OrthographicCamera2D camera = renderData->uiCamera;
    float zoom = camera.zoom? camera.zoom : 1.0f;
    Vec2 dimensions = camera.dimensions * 1.0f / zoom;
    Vec2 position = camera.position * (float)max((int)(1.0f / zoom), 1);
    renderData->orthoProjectionUI = 
    orthographic_projection(position.x - dimensions.x / 2.0f, 
                          position.x + dimensions.x / 2.0f, 
                          position.y - dimensions.y / 2.0f, 
                          position.y + dimensions.y / 2.0f);
  }
 
  // Draw UI
  {
    for(int eleIdx = 0; eleIdx < uiState->uiElements.count; eleIdx++)
    {
      UIElement uiElement = uiState->uiElements[eleIdx];
      draw_ui_sprite(uiElement.spriteID, uiElement.pos);
    }
  }

  // Don't like the double switch statement much tbh,
  // but it's the easiest solution right now!
  switch(gameState->state)
  {
    case GAME_STATE_MAIN_MENU:
    {
      // Only draw UI
      renderData->clearColor = {79.0f / 255.0f, 140.0f / 255.0f, 235.0f / 255.0f, 1.0f};
      draw_text("Celeste Clone",{-44, 170});
      draw_ui_sprite(SPRITE_CELESTE_01_BIG, {80, 120}, 
                  {
                    .renderOptions = RENDERING_OPTION_TRANSPARENT,
                    .layer = 0.7f,
                  });

      draw_ui_sprite(SPRITE_CELESTE_02_BIG, {240, 120}, 
                  {
                    .renderOptions = RENDERING_OPTION_TRANSPARENT,
                    .layer = 0.7f,
                  });

      break;
    }

    case GAME_STATE_IN_LEVEL:
    {
      // Draw Level
      {
        // Draw Solids
        {
          for(int solidIdx = 0; solidIdx < gameState->level.solids.count; solidIdx++)
          {
            Solid solid = gameState->level.solids[solidIdx];
            Vec2 solidPos = lerp(vec_2(solid.prevPos), 
                                vec_2(solid.pos), 
                                interpDT);

            draw_sprite(solid.spriteID, solidPos);
          }
        }

        // Forground Tiles
        draw_tile_map(&gameState->level.tileMap);

        // Draw Celeste
        {
          IRect playerRect = get_player_rect();
          Vec2 playerPos = lerp(vec_2(gameState->player.prevPos), 
                                vec_2(gameState->player.pos), 
                                interpDT);

          // Death Animation
          if(gameState->player.deathAnimTimer < DEATH_ANIM_TIME)
          {
            Sprite sprite = get_sprite(SPRITE_CELESTE_DEATH);
            float t = gameState->player.deathAnimTimer;
            int animationIdx = animate(&t, sprite.frameCount, DEATH_ANIM_TIME);
            draw_sprite(SPRITE_CELESTE_DEATH,  playerPos, {.animationIdx = animationIdx});
          }
          else
          {  
            SpriteID spriteID = gameState->player.animationSprites[gameState->player.animationState];
            Sprite sprite = get_sprite(spriteID);
            int animationIdx = animate(&gameState->player.runAnimTimer, sprite.frameCount, 0.5f);
            draw_quad(playerPos, vec_2(1.0f));
            draw_sprite(spriteID, playerPos, 
                        {
                          .animationIdx = animationIdx,
                          .renderOptions = gameState->player.renderOptions
                        });
          }
        }

        // BG Tiles
        draw_tile_map(&gameState->level.bgTileMap);
      }
    }
  }
}

void update_level(float dt)
{
  update_player(dt);

  // Change Solids
  if(key_pressed_this_frame(KEY_1))
  {
    gameState->level.solids.clear();

    // Solid 1
    Solid solid = {};
    solid.spriteID = SPRITE_SOLID_01;
    solid.keyframes.add({.pos = { 2 * 8, -10 * 8}, .time = 0.0f});
    solid.keyframes.add({.pos = {-4 * 8, -10 * 8}, .time = 0.5f});
    solid.keyframes.add({.pos = { 2 * 8, -10 * 8}, .time = 1.0f});
    gameState->level.solids.add(solid);

    // Solid 2
    solid = {}; // Init to zero
    solid.spriteID = SPRITE_SOLID_02;
    solid.keyframes.add({.pos = {-9 * 8, -30 * 8}, .time = 0.0f});
    solid.keyframes.add({.pos = {-9 * 8, -40 * 8}, .time = 1.0f});
    solid.keyframes.add({.pos = {-9 * 8, -30 * 8}, .time = 2.0f});
    gameState->level.solids.add(solid);

    solid = {}; // Init to zero
    solid.spriteID = SPRITE_SOLID_02;
    solid.keyframes.add({.pos = {-12 * 8, -45 * 8}, .time = 0.0f});
    solid.keyframes.add({.pos = {  4 * 8, -45 * 8}, .time = 1.2f});
    solid.keyframes.add({.pos = {-12 * 8, -45 * 8}, .time = 2.4f});
    gameState->level.solids.add(solid);

  }

  if(key_pressed_this_frame(KEY_R))
  {
    // gameState->player.pos = gameState->level.playerStartPos;
    gameState->player.pos = {-18 * 8, - 60 * 8};
  }

  if(do_button(SPRITE_SAVE_BUTTON, {WORLD_WIDTH - 20, 12}, line_id(1)))
  {
    write_file("level.bin", (char*)&gameState->level, sizeof(Level));
  }


  if(key_pressed_this_frame(KEY_ESCAPE))
  {
    gameState->state = GAME_STATE_MAIN_MENU;
  }

  if(key_pressed_this_frame(KEY_K))
  {
    write_file("gamestate.bin", (char*)gameState, sizeof(GameState));
  }

  if(key_pressed_this_frame(KEY_L))
  {
    int fileSize;
    GameState* emulatedState = (GameState*)read_file("gamestate.bin", &fileSize, transientStorage);
    SM_ASSERT(sizeof(GameState) == fileSize, "Penis");
    if(emulatedState)
    {
      *gameState = *emulatedState;
    }
  }

  // Leveleditor
  {
    if(!ui_is_hot() && key_is_down(KEY_LEFT_MOUSE))
    {
      IVec2 worldPos = ivec_2(screen_to_world(input->mousePos));

      Tile* tile = get_tile(worldPos);
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
      IVec2 worldPos = ivec_2(screen_to_world(input->mousePos));
      Tile* tile = get_tile(worldPos);
      if(tile)
      {
        tile->type = TILE_TYPE_NONE;
      }
    }
  }

  // Updating Solids
  {
    gameState->player.solidSpeed = {};

    for(int solidIdx = 0; solidIdx < gameState->level.solids.count; solidIdx++)
    {
      Solid* solid = &gameState->level.solids[solidIdx];
      solid->prevPos = solid->pos;
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
        float speedX = nextPos.x - (float)solid->pos.x;
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

              IRect solidRect = get_solid_rect(gameState->level.solids[solidIdx]);
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
                  gameState->player.pos.x += moveSign;
                  gameState->player.solidSpeed.x = speedX;
                }
                  
                for(int subSolidIdx = 0; 
                    subSolidIdx < gameState->level.solids.count;
                    subSolidIdx++)
                {
                  if(subSolidIdx == solidIdx)
                  {
                    continue;
                  }

                  IRect subSolidRect = 
                    get_solid_rect(gameState->level.solids[subSolidIdx]);
                  if(rect_collision(get_player_rect(), subSolidRect))
                  {
                    gameState->player.pos = gameState->level.playerStartPos;
                  }
                }
              }

              solid->pos.x += moveSign; 
              move -= moveSign; 
            } 
          }
        }

        // Move Y
        float speedY = nextPos.y - (float)solid->pos.y;
        {
          float amount = nextPos.y - (float)solid->pos.y;
          solid->remainder.y += amount; 
          int move = round(solid->remainder.y);   
          if (move != 0) 
          { 
            solid->remainder.y -= move; 
            int moveSign = sign(move);
            while (move != 0) 
            { 
              bool collisionHappened = false;

              IRect solidRect = get_solid_rect(gameState->level.solids[solidIdx]);
              IRect newSolidRect = solidRect;
              newSolidRect.pos.x += moveSign;
              IRect playerRect = get_player_rect();
              playerRect.pos.y -= 2;
              playerRect.size.y += 4;

              // Is the player currently standing on this Solid
              if(rect_collision(playerRect, newSolidRect))
              {
                if(solidRect.pos.y <= playerRect.pos.y + playerRect.size.y)
                {
                  gameState->player.pos.y += moveSign;
                  gameState->player.solidSpeed.y = speedY;
                }

                for(int subSolidIdx = 0; 
                    subSolidIdx < gameState->level.solids.count;
                    subSolidIdx++)
                {
                  if(subSolidIdx == solidIdx)
                  {
                    continue;
                  }

                  IRect subSolidRect = get_solid_rect(gameState->level.solids[subSolidIdx]);
                  if(rect_collision(get_player_rect(), subSolidRect))
                  {
                    gameState->player.pos = gameState->level.playerStartPos;
                  }
                }
              }

              solid->pos.y += moveSign; 
              move -= moveSign; 
            } 
          } 
        }
      }
    }
  }
}

void update()
{
  // We update the logic at a fixed rate to keep the game stable 
  // and to save on performance
  float dt = UPDATE_DELAY;

  // Update Camera
  {
    static Vec2 camEndPos = renderData->gameCamera.position;
    static Vec2 camStartPos = renderData->gameCamera.position;
    static float t = 0.0f;

    // Camera Position is a multiple of 180(Room Height)
    {
      if(camEndPos.y != renderData->gameCamera.position.y)
      {
        if(t == 1.0f)
        {
          t = 0.0f;
        }
        t = min(t + dt, 1.0f);
        renderData->gameCamera.position = lerp(camStartPos, camEndPos, ease_out_quad(t));
        return;
      }

      t = 1.0f; // Hmm
      camStartPos = camEndPos;
      int roomIdx = get_room_idx();
      camEndPos.y = 90.0f + 176.0f * (float)roomIdx;
      camEndPos.x = renderData->gameCamera.position.x;

      if(key_pressed_this_frame(KEY_H))
      {
        renderData->gameCamera.position.y -= 1.0f;
      }

      if(key_pressed_this_frame(KEY_J))
      {
        renderData->gameCamera.position.y += 1.0f;
      }
    } 

    if(key_is_down(KEY_Z))
    {
      renderData->gameCamera.zoom += dt;
    }

    if(key_is_down(KEY_T))
    {
      renderData->gameCamera.zoom -= dt;
    }
  }

  update_game_input(dt);
  update_ui();

  switch(gameState->state)
  {
    case GAME_STATE_MAIN_MENU:
    {
      if(do_button(SPRITE_PLAY_BUTTON, {160, 90}, line_id(1)))
      {
        gameState->state = GAME_STATE_IN_LEVEL;
      }
      break;
    }

    case GAME_STATE_IN_LEVEL:
    {
      update_level(dt);
      break;
    }
  }
}
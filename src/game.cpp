#include "game.h"
#include "assets.h"
#include "input.h"
#include "render_interface.h"

// #############################################################################
//                           Game Globals
// #############################################################################
static GameState* gameState;

// #############################################################################
//                           Game Functions
// #############################################################################
bool is_down(GameInputType type);
bool just_pressed(GameInputType type);
void update_game_input();
void move_x();
IRect get_player_rect();

// #############################################################################
//                           Update Game (Exported from DLL)
// #############################################################################
EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, 
                           RenderData* renderDataIn, SoundState* soundStateIn)
{
  if(gameState != gameStateIn)
  {
    input = inputIn;
    renderData = renderDataIn;
    gameState = gameStateIn;
    soundState = soundStateIn;
  }

  if(!gameState->initialized)
  {
    gameState->initialized = true;
    gameState->playerPos = {50, -100};
    gameState->jumpSound.path = "assets/sounds/jump_01.wav";
    gameState->deathSound.path = "assets/sounds/died_01.wav";
  }

  update_game_input();

  // This is inside of the data section of the memory, int the exe
  float dt = 1.0f / 60.0f;
  float maxRunSpeed = 10.0f;
  float wallJumpSpeed = 15.0f;
  float runAcceleration = 50.0f;
  float fallSideAcceleration = 35.0f;
  float maxJumpSpeed = -14.0f;
  float fallSpeed = 18.0f;
  float gravity = 70.0f;
  float runReduce = 80.0f;
  float wallClimbSpeed = -4.0f;
  float wallSlideDownSpeed = 8.0f;
  static Vec2 speed;
  static float highestHeight = input->screenSize.y;
  static float varJumpTimer = 0.0f;
  static float xRemainder = 0.0f;
  static float yRemainder = 0.0f;
  static float wallJumpTimer = 0.0f;
  static bool playerGrounded = true;
  static bool grabbingWall = false;

  struct Keyframe
  {
    IVec2 pos;
    float time; // how long to get there
  };

  struct Solid
  {
    // Pixel Movement
    float remainderX;
    float remainderY;

    // Current pos & size
    IRect rect;

    int keyframeIdx;
    Array<Keyframe, 10> keyframes;

    // Animation
    float time;
    float waitingTime;
    float waitingDuration;
  };

  static Solid solids [] =
  {
    // {48 * 2, 48, 48 * 8, 48},
    // {624, 96, 480, 48},
    {
      .rect = {48 * 13, 48 * 2, 48, 48 * 8},
    },
    {
      .rect = {48 *  8, 48 * 2, 48, 48 * 8}
    },
    {
      .rect = {0, 624, 1048, 48}
    },
    // {1048 + 240, 624, 1048, 48},
    // {1040 - 240, 96 * 4, 480, 48},
    // {48 * 3, 48 * 10, 48 * 3, 48},
    {
      .rect = {48 * 5, 48 * 7, 48 * 2, 48},
      .keyframes = 
      {
        .count = 5,
        .elements =
        {
          {{48 * 0, 48 * 7}, 0.0f},
          {{48 * 5, 48 * 7}, 1.0f},
          {{48 * 5, 48 * 3}, 2.0f},
          {{48 * 5, 48 * 7}, 3.0f},
          {{48 * 0, 48 * 7}, 4.0f},
        }
      }
    },
    // {48 * 9, 48 * 5, 48 * 3, 48},
  };

  for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
  {
    Solid* solid = &solids[solidIdx];

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
      // Move X
      {
        float amount = nextPos.x - (float)solid->rect.pos.x;
        xRemainder += amount; 
        int move = round(xRemainder);   
        if (move != 0) 
        { 
          xRemainder -= move; 
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
              if(solidRect.pos.y <= playerRect.pos.y + playerRect.size.y)
              {
                gameState->playerPos.x += moveSign;
              }
            }

            solid->rect.pos.x += moveSign; 
            move -= moveSign; 
          } 
        } 
      }

      // Move Y
      {
        float amount = nextPos.y - (float)solid->rect.pos.y;
        xRemainder += amount; 
        int move = round(xRemainder);   
        if (move != 0) 
        { 
          xRemainder -= move; 
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
              if(solidRect.pos.y <= playerRect.pos.y + playerRect.size.y)
              {
                gameState->playerPos.y += moveSign;
              }
            }

            solid->rect.pos.y += moveSign; 
            move -= moveSign; 
          } 
        } 
      }
    }

    draw_quad(solids[solidIdx].rect.pos, solids[solidIdx].rect.size);
  }

  if(key_pressed_this_frame(KEY_R))
  {
    gameState->playerPos = {50, 50};
  }

  float directionChangeMult = 1.6f;
  if(is_down(INPUT_MOVE_LEFT))
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

  if(is_down(INPUT_MOVE_RIGHT))
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

  // firction
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
      speed.y = maxJumpSpeed;
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
      for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
      {
        IRect playerRect = get_player_rect();
        playerRect.pos.x -= 2 * 6;
        playerRect.size.x += 4 * 6;
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
          }

          // Colliding on the Left
          if(solidRectRight - playerRectLeft >
            playerRectRight - solidRectLeft)
          {
            wallJumpTimer = 0.1f;
            varJumpTimer = 0.0f;
            speed.x = -wallJumpSpeed;
            speed.y = maxJumpSpeed;
          }

          play_sound(gameState->jumpSound);
        }
      }
    }

    varJumpTimer += dt;
  }

  // Gravity
  if(!grabbingWall)
  {
    speed.y = approach(speed.y, fallSpeed, gravity * dt);
  }

  // Wall Grabbing
  {
    grabbingWall = false;
    if(is_down(INPUT_WALL_GRAB) && 
       wallJumpTimer == 0.0f)
    {
      for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
      {
        IRect playerRect = get_player_rect();
        playerRect.pos.x -= 2 * 6;
        playerRect.size.x += 4 * 6;
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

    // firction
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
      while (move != 0) 
      { 
        bool collisionHappened = false;
        for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
        {
          IRect playerRect = get_player_rect();
          IRect newPlayerRect = playerRect;
          newPlayerRect.pos.x += moveSign;
          IRect solidRect = solids[solidIdx].rect;

          if(rect_collision(solidRect, newPlayerRect))
          {
            collisionHappened = true;
            break;
          }
        }

        if(!collisionHappened)
        {
          //There is no Solid immediately beside us, move
          gameState->playerPos.x += moveSign; 
          move -= moveSign; 
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
      while (move != 0) 
      { 
        bool collisionHappened = false;
        for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
        {
          IRect playerRect = get_player_rect();
          IRect newPlayerRect = playerRect;
          newPlayerRect.pos.y += moveSign;
          IRect solidRect = solids[solidIdx].rect;

          if(rect_collision(solidRect, newPlayerRect))
          {
            collisionHappened = true;
            break;
          }
        }

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
          if(moveSign > 0)
          {
            playerGrounded = true;
          }
          break;
        }
      } 
    } 
  }

  IRect playerRect = get_player_rect();
  draw_quad(playerRect.pos, playerRect.size);
  draw_sprite(SPRITE_CELESTE_01, vec_2(gameState->playerPos));
  // draw_quad(gameState.playerPos, {50.0f, 50.0f});
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

void update_game_input()
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
  gameState->gameInput[INPUT_JUMP].justPressed = 
    input->keys[KEY_SPACE].justPressed;
  gameState->gameInput[INPUT_JUMP].isDown = 
    input->keys[KEY_SPACE].isDown;

  // Wall Grabbing
  gameState->gameInput[INPUT_WALL_GRAB].isDown =
    input->keys[KEY_E].isDown;
  gameState->gameInput[INPUT_WALL_GRAB].isDown |=
    input->keys[KEY_Q].isDown;
}

void move_x(float amount) 
{
}

IRect get_player_rect()
{
  return 
  {
    gameState->playerPos.x + 30, 
    gameState->playerPos.y + 12, 
    9 * 6, 
    16 * 6
  };
}
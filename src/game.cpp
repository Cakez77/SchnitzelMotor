#include "game.h"
#include "assets.h"
#include "input.h"
#include "render_interface.h"

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

// #############################################################################
//                           Game Globals
// #############################################################################
static int worldScale = 5;
static GameState* gameState;
static Vec2 prevRemainder = {};
static float xRemainder = 0.0f;
static float yRemainder = 0.0f;
static bool standingOnPlatofrm = false;

static Solid solids [] =
{
  {
    // .rect = {48 * 13, 48 * 2, 48, 48 * 8},
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
void update_game_input();
void move_x();
IRect get_player_rect();
void draw(float interpolatedDT);
void update();

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

    gameState->jumpSound.path = "assets/sounds/jump_01.wav";
    gameState->deathSound.path = "assets/sounds/died_01.wav";
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
    gameState->playerPos.x + 6, 
    gameState->playerPos.y + 2, 
    9, 
    16
  };
}

void draw(float interpDT)
{
  Vec2 prevSolidRemainder = {};
  Vec2 solidRemainder = {};

  for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
  {
    Solid solid = solids[solidIdx];
    if(solid.remainder.x && solid.remainder.y)
    {
      solidRemainder = solid.remainder;
    }

    Vec2 solidPos = lerp(vec_2(solid.prevPos)  + solid.prevRemainder, 
                         vec_2(solid.rect.pos) + solid.remainder, 
                         interpDT);

    draw_quad(solidPos * (float)worldScale, vec_2(solid.rect.size) * (float)worldScale);
  }

  IRect playerRect = get_player_rect();  

  Vec2 playerPos = lerp(vec_2(gameState->prevPlayerPos) + 
                          (standingOnPlatofrm? prevSolidRemainder : prevRemainder), 
                        vec_2(gameState->playerPos) +
                          (standingOnPlatofrm? solidRemainder : Vec2{xRemainder, yRemainder}), 
                        interpDT);

  draw_quad(playerPos * worldScale, vec_2(playerRect.size) * worldScale);
  draw_sprite(SPRITE_CELESTE_01, playerPos * worldScale, worldScale);
}

void update()
{
  update_game_input();
  gameState->prevPlayerPos = gameState->playerPos;
  prevRemainder = {xRemainder, yRemainder};
  standingOnPlatofrm = false;

  // We update the logic at a fixed rate to keep the game stable 
  // and to save on performance
  float dt = UPDATE_DELAY;

  // Movement Data needed for Celeste
  float maxRunSpeed = 10.0f / 3.6f;
  float wallJumpSpeed = 15.0f / 3.6f;
  float runAcceleration = 50.0f / 3.6f;
  float fallSideAcceleration = 35.0f / 3.6f;
  float maxJumpSpeed = -14.0f / 3.6f;
  float fallSpeed = 18.0f / 3.6f;
  float gravity = 70.0f / 3.6f;
  float runReduce = 80.0f / 3.6f;
  float wallClimbSpeed = -4.0f / 3.6f;
  float wallSlideDownSpeed = 8.0f / 3.6f;
  static Vec2 speed;
  Vec2 solidSpeed = {};
  static float highestHeight = input->screenSize.y;
  static float varJumpTimer = 0.0f;
  static float wallJumpTimer = 0.0f;
  static bool playerGrounded = true;
  static bool grabbingWall = false;

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
              standingOnPlatofrm = true;

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
      for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
      {
        IRect playerRect = get_player_rect();
        playerRect.pos.x -= 2;
        playerRect.size.x += 4;
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
        playerRect.pos.x -= 2;
        playerRect.size.x += 4;
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
      bool jumpingCollisionHappend = false;
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

          IRect jumpGraceRect = newPlayerRect;
          jumpGraceRect.size.y += 2; // Jumping Grace???

          if(rect_collision(solidRect, jumpGraceRect) &&
             moveSign > 0)
          {
            jumpingCollisionHappend = true;
          }

          if(rect_collision(solidRect, newPlayerRect))
          {
            collisionHappened = true;
            break;
          }

        }

        if(jumpingCollisionHappend)
        {
          playerGrounded = true;
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
          break;
        }
      } 
    } 
  }
}
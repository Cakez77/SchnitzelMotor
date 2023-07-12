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
bool game_input(GameInputType type);
void update_game_input();
void move_x();
IRect get_player_rect();

// #############################################################################
//                           Update Game (Exported from DLL)
// #############################################################################
EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, RenderData* renderDataIn)
{
  if(gameState != gameStateIn)
  {
    input = inputIn;
    renderData = renderDataIn;
    gameState = gameStateIn;

    gameState->playerPos = {50, -100};
  }

  update_game_input();

  // This is inside of the data section of the memory, int the exe
  float dt = 1.0f / 60.0f;
  float maxRunSpeed = 10.0f;
  float runAcceleration = 50.0f;
  float maxJumpSpeed = -18.0f;
  float fallSpeed = 18.0f;
  float gravity = 70.0f;
  float runReduce = 80.0f;
  static Vec2 speed;
  static float highestHeight = input->screenSize.y;
  static float varJumpTimer = 0.0f;
  static float xRemainder = 0.0f;
  static float yRemainder = 0.0f;

  static IRect solids [] =
  {
    {48 * 2, 48, 48 * 8, 48},
    {624, 96, 480, 48},
    {624, 96, 48, 288},
    {0, 624, 1048, 48},
    {1048 + 240, 624, 1048, 48},
    {1040 - 240, 96 * 4, 480, 48},
    {48 * 3, 48 * 10, 48 * 3, 48},
    {48 * 5, 48 * 7, 48 * 2, 48},
    {48 * 9, 48 * 5, 48 * 3, 48},
  };

  for(int solidIdx = 0; solidIdx < ArraySize(solids); solidIdx++)
  {
    draw_quad(solids[solidIdx].pos, solids[solidIdx].size);
  }

  if(key_pressed_this_frame(KEY_R))
  {
    gameState->playerPos = {0, 200};
  }

  float directionChangeMult = 1.6f;
  if(game_input(INPUT_MOVE_LEFT))
  {
    float mult = 1.0f;
    if(speed.x > 0.0f)
    {
      mult = directionChangeMult;
    }
    speed.x = approach(speed.x, -maxRunSpeed, runAcceleration * mult * dt);
  }

  if(game_input(INPUT_MOVE_RIGHT))
  {
    float mult = 1.0f;
    if(speed.x < 0.0f)
    {
      mult = directionChangeMult;
    }
    speed.x = approach(speed.x, maxRunSpeed, runAcceleration * mult * dt);
  }

  if(game_input(INPUT_JUMP))
  {
    varJumpTimer = 0.0f;
    speed.y = maxJumpSpeed;
    gameState->playerGrounded = false;
  }

  if(game_input(INPUT_EXTENDED_JUMP) &&
     varJumpTimer < 0.1f)
  {
    speed.y = min(speed.y, maxJumpSpeed);
  }
  varJumpTimer += dt;

  speed.y = approach(speed.y, fallSpeed, gravity * dt);

  // firction
  if(!key_is_down(KEY_A) &&
     !key_is_down(KEY_D))
  {
    speed.x = approach(speed.x, 0, runReduce * dt);
  }

  if(key_is_down(KEY_W))
  {
  }
  if(key_is_down(KEY_S))
  {
  }

  SM_TRACE("Speed.x: %.2f", speed.x);

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
          IRect solidRect = solids[solidIdx];

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
          IRect solidRect = solids[solidIdx];

          if(rect_collision(solidRect, newPlayerRect))
          {
            collisionHappened = true;
            break;
          }
        }

        if(!collisionHappened)
        {
          //There is no Solid immediately beside us, move
          gameState->playerPos.y += moveSign; 
          move -= moveSign; 
        }
        else
        {
          //Hit a solid! Don't move!
          if(moveSign < 0)
          {
            speed.y = 0.0f;
          }
          if(moveSign > 0)
          {
            gameState->playerGrounded = true;
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
bool game_input(GameInputType type)
{
  return gameState->gameInput[type];
}

void update_game_input()
{
  // Moving
  gameState->gameInput[INPUT_MOVE_LEFT] = input->keys[KEY_A].isDown;
  gameState->gameInput[INPUT_MOVE_RIGHT] = input->keys[KEY_D].isDown;

  // Jumping
  gameState->gameInput[INPUT_JUMP] = 
    input->keys[KEY_SPACE].justPressed && gameState->playerGrounded;
  gameState->gameInput[INPUT_EXTENDED_JUMP] = input->keys[KEY_SPACE].isDown;
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
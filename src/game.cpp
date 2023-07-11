#include "game.h"
#include "assets.h"
#include "input.h"
#include "render_interface.h"

static GameState* gameState;

EXPORT_FN void update_game(GameState* gameStateIn, Input* inputIn, RenderData* renderDataIn)
{
  if(gameState != gameStateIn)
  {
    input = inputIn;
    renderData = renderDataIn;
    gameState = gameStateIn;
  }

  // This is on the stack
  Vec2 speed1;

  // This is inside of the data section of the memory, int the exe
  float dt = 1.0f / 60.0f;
  float maxRunSpeed = 10.0f;
  float runAcceleration = 100.0f;
  float maxJumpSpeed = -20.0f;
  float fallSpeed = 20.0f;
  float gravity = 90.0f;
  float runReduce = 40.0f;
  static Vec2 speed;
  static bool grounded = true;
  static float highestHeight = input->screenSize.y;
  static float varJumpTimer = 0.0f;

  if(key_pressed_this_frame(KEY_R))
  {
    gameState->playerPos = {};
  }

  if(key_is_down(KEY_A))
  {
    speed.x = approach(speed.x, -maxRunSpeed, runAcceleration * dt);
  }

  if(key_is_down(KEY_D))
  {
    speed.x = approach(speed.x, maxRunSpeed, runAcceleration * dt);
  }

  if(key_pressed_this_frame(KEY_SPACE) &&
     grounded)
  {
    varJumpTimer = 0.0f;
    speed.y = maxJumpSpeed;
    grounded = false;
  }

  if(key_is_down(KEY_SPACE) &&
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
    gameState->playerPos.y -= speed.y;
  }
  if(key_is_down(KEY_S))
  {
    gameState->playerPos.y += speed.y;
  }

  gameState->playerPos.x += speed.x;
  gameState->playerPos.y += speed.y;


  gameState->playerPos.y += 6.0f;
  if(gameState->playerPos.y > 500.0f - 4.0f * 17.0f)
  {
    gameState->playerPos.y = 500.0f - 4.0f * 17.0f;
    grounded = true;
  }

  if(!grounded &&
     gameState->playerPos.y < highestHeight)
  {
    highestHeight = gameState->playerPos.y;
  }

  draw_quad({0.0f, highestHeight + 2.0f}, {input->screenSize.x, 2.0});

  draw_sprite(SPRITE_CELESTE_02, gameState->playerPos);
  // draw_quad(gameState.playerPos, {50.0f, 50.0f});

  draw_quad({0.0f, 500.0f}, {input->screenSize.x / 2.0f, 10.0f});
  draw_quad({input->screenSize.x / 2.0f + 50.0f, 500.0f}, 
            {input->screenSize.x / 2.0f, 10.0f});
}



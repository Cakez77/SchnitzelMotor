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

  float speed = 5.0f;

  if(key_is_down(KEY_A))
  {
    gameState->playerPos.x -= speed;
  }
  if(key_is_down(KEY_D))
  {
    gameState->playerPos.x += speed;
  }
  if(key_is_down(KEY_W))
  {
    gameState->playerPos.y -= speed;
  }
  if(key_is_down(KEY_S))
  {
    gameState->playerPos.y += speed;
  }

  draw_sprite(SPRITE_CELESTE_02, gameState->playerPos);
  // draw_quad(gameState.playerPos, {50.0f, 50.0f});

  gameState->playerPos.y += 6.0f;
  if(gameState->playerPos.y > 500.0f - 4.0f * 17.0f)
  {
    gameState->playerPos.y = 500.0f - 4.0f * 17.0f;
  }

  if(key_pressed_this_frame(KEY_SPACE))
  {
    gameState->playerPos.y -= 100.0f;
  }

  draw_quad({0.0f, 500.0f}, {input->screenSize.x / 2.0f, 10.0f});
  draw_quad({input->screenSize.x / 2.0f + 50.0f, 500.0f}, 
            {input->screenSize.x / 2.0f, 10.0f});
}



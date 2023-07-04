
struct GameState
{
  Vec2 playerPos;
};

static GameState gameState;

void update_game()
{
  float speed = 5.0f;

  if(key_is_down(KEY_A))
  {
    gameState.playerPos.x -= speed;
  }
  if(key_is_down(KEY_D))
  {
    gameState.playerPos.x += speed;
  }
  if(key_is_down(KEY_W))
  {
    gameState.playerPos.y -= speed;
  }
  if(key_is_down(KEY_S))
  {
    gameState.playerPos.y += speed;
  }

  draw_quad(gameState.playerPos, {50.0f, 50.0f});
}



#pragma once
#include "assets.h"
#include "input.h"
#include "render_interface.h"

// #############################################################################
//                           UI Constants
// #############################################################################
constexpr int MAX_UI_ELEMENTS = 100;
constexpr int MAX_TEXT_CHARS = 256;

// #############################################################################
//                           UI Structs
// #############################################################################
struct UIID
{
  int ID;
  int layer;
};

struct UIElement
{
  SpriteID spriteID;
  Vec2 pos;
};

struct UIText
{
  int charCount;
  char text[MAX_TEXT_CHARS];
  Vec2 pos;
};

struct UIState
{
  UIID hotLastFrame;
  UIID hotThisFrame;
  UIID active;

  Array<UIText, 100> texts;
  Array<UIElement, MAX_UI_ELEMENTS> uiElements;
};

// #############################################################################
//                           UI Globals
// #############################################################################
static UIState* uiState;

// #############################################################################
//                           UI Functions
// #############################################################################
void update_ui()
{
  if (!key_is_down(KEY_MOUSE_LEFT) && !key_released_this_frame(KEY_MOUSE_LEFT))
  {
    uiState->active = {};
  }

  uiState->uiElements.clear();
  uiState->hotLastFrame = uiState->hotThisFrame;
  uiState->hotThisFrame = {};
}

void set_active(int ID)
{
  uiState->active = {ID, 0};
}

void set_hot(int ID, int layer = 0)
{
  if (uiState->hotThisFrame.layer <= layer)
  {
    uiState->hotThisFrame.ID = ID;
    uiState->hotThisFrame.layer = layer;
  }
}
bool is_active(int ID)
{
  return uiState->active.ID && uiState->active.ID == ID;
}

bool is_hot(int ID)
{
  return uiState->hotLastFrame.ID && uiState->hotLastFrame.ID == ID;
}

bool ui_is_hot()
{
  return uiState->hotLastFrame.ID || uiState->hotLastFrame.ID;
}

bool do_button(SpriteID spriteID, IVec2 pos, int ID)
{
  Sprite sprite = get_sprite(spriteID);
  IVec2 mousePosWold = screen_to_ui(input->mousePos);
  // Draw UI Element (Adds to an array of elements to draw during render())
  {
    UIElement uiElement = {
        .spriteID = spriteID,
        .pos = vec_2(pos),
    };
    uiState->uiElements.add(uiElement);
  }

  IRect rect = {pos.x - sprite.size.x / 2, pos.y - sprite.size.y / 2, sprite.size};
  if (is_active(ID))
  {
    if (key_released_this_frame(KEY_MOUSE_LEFT) && point_in_rect(mousePosWold, rect))
    {
      // Set inactive
      uiState->active = {};
      return true;
    }
  }
  else if (is_hot(ID))
  {
    if (key_pressed_this_frame(KEY_MOUSE_LEFT))
    {
      set_active(ID);
    }
  }

  if (point_in_rect(mousePosWold, rect))
  {
    set_hot(ID);
  }

  return false;
}

void do_ui_text(char* text, Vec2 pos)
{
  UIText uiText = {};
  memcpy(uiText.text, text, strlen(text));
  uiText.charCount = strlen(text);
  uiText.pos = pos;

  uiState->texts.add(uiText);
}

template <typename... Args> void do_format_ui_text(char* format, Vec2 pos, Args... args)
{
  char* text = format_text(format, args...);
  do_ui_text(text, pos);
}

#include "schnitzel_lib.h"
#include "input.h"
#include "game.h"
#include "sound.h"
#include "render_interface.h"

// This is so glcorearb does not include windows.h on Windows
#define APIENTRY
#define GL_GLEXT_PROTOTYPES // This is so we get the function declarations
#include "glcorearb.h"

// #############################################################################
//                           Platform defines
// #############################################################################
// This is the function pointer to update_game in game.cpp
typedef decltype(update_game) update_game_type;

// #############################################################################
//                           Platform Constants
// #############################################################################
constexpr int TRANSIENT_STORAGE_SIZE = MB(128);
constexpr int PERSISTENT_STORAGE_SIZE = MB(256);
constexpr int MAX_KEYCODES = 256;

// #############################################################################
//                           Platform Globals
// #############################################################################
static bool running = true;
static KeyCodes KeyCodeLookupTable[MAX_KEYCODES];
static update_game_type* update_game_ptr;
static BumpAllocator transientStorage;
static BumpAllocator persistentStorage;

// #############################################################################
//                           Platform Functions
// #############################################################################
void platform_fill_keycode_lookup_table();
bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_func(char* funName);
void platform_swap_buffers();
void platform_reaload_dynamic_library();
bool platform_init_audio();
void platform_update_audio(float dt);

// #############################################################################
//                           Windows Platform
// #############################################################################
#ifdef _WIN32
#include "win32_platform.cpp"
#elif defined(__APPLE__)
#include "mac_platform.cpp"
#else // Linux
#include "linux_platform.cpp"
#endif

// #############################################################################
//                           Renderer
// #############################################################################
#include "gl_renderer.cpp"

int main()
{
  transientStorage = make_bump_allocator(TRANSIENT_STORAGE_SIZE);
  persistentStorage = make_bump_allocator(PERSISTENT_STORAGE_SIZE);

  GameState * gameState = (GameState*)bump_alloc(&persistentStorage, sizeof(GameState));

  // Defined in the file render_interface.h
  renderData = (RenderData*)bump_alloc(&persistentStorage, sizeof(RenderData));

  // Defiend in "input.h"
  input = (Input*)bump_alloc(&persistentStorage, sizeof(Input));

  // Defines in "sound.h"
  soundState = (SoundState*)bump_alloc(&persistentStorage, sizeof(SoundState));
  soundState->transientStorage = &transientStorage;

  // Allocating Data for Sounds
  soundState->allocatedsoundsBuffer = bump_alloc(&persistentStorage, SOUNDS_BUFFER_SIZE);

  if(!platform_create_window(1200, 720, "Schnitzel Motor"))
  {
    SM_ERROR("Failed to create Windows Window");
    return -1;
  }

  platform_fill_keycode_lookup_table();

  if(!gl_init(&transientStorage))
  {
    SM_ERROR("Failed to initialize OpenGL");
    return -1;
  }

  if(!platform_init_audio())
  {
    SM_ERROR("Failed to initialize Audio");
    return -1;
  }

  while(running)
  {
    // Resent transient Storage
    transientStorage.used = 0;

    // Load the update_game function pointer from the DLL
    platform_reaload_dynamic_library();

    // Reset Input
    for(int keyIdx = 0; keyIdx < MAX_KEYCODES; keyIdx++)
    {
      input->keys[keyIdx].justReleased = false;
      input->keys[keyIdx].justPressed = false;
      input->keys[keyIdx].halfTransitionCount = 0;
    }

    platform_update_window();
    update_game(gameState, input, renderData, soundState);
    gl_render();

    // This is platform specific!
    platform_update_audio(1.0f/60.0f);
    platform_swap_buffers();
  }

  return 0;
}

void update_game(GameState* gameState, Input* inputIn, 
                 RenderData* renderDataIn, SoundState* soundStateIn)
{
  update_game_ptr(gameState, inputIn, renderDataIn, soundStateIn);
}
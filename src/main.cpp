#include "schnitzel_lib.h"
#include "input.h"
#include "ui.h"
#include "game.h"
#include "sound.h"
#include "render_interface.h"

// This is so glcorearb does not include windows.h on Windows
#define APIENTRY
#define GL_GLEXT_PROTOTYPES // This is so we get the function declarations
#include "glcorearb.h"

// #############################################################################
//                           Game DLL Stuff(Hot Code Reloading)
// #############################################################################
// This is the function pointer to update_game in game.cpp
typedef decltype(update_game) update_game_type;
static update_game_type* update_game_ptr;

// #############################################################################
//                           Platform Includes
// #############################################################################
#include "platform.h"
#ifdef _WIN32
#include "win32_platform.cpp"
char* gameDLLName = "game.dll";
char* gameLoadDLLName = "game_load.dll";
#elif defined(__APPLE__)
#include "mac_platform.cpp"
char* gameDLLName = "game.so"; // ?????
char* gameLoadDLLName = "game_load.so";
#else // Linux
#include "linux_platform.cpp"
char* gameDLLName = "game.so";
char* gameLoadDLLName = "game_load.so";
#endif

// #############################################################################
//                           Renderer
// #############################################################################
#include "gl_renderer.cpp"

// #############################################################################
//                           Cross Platform
// #############################################################################
// Used to get Delta Time
#include <chrono>

double get_delta_time();
void reload_game_dll();


int main()
{
  // Init lastTime
  get_delta_time();

  transientStorage = make_bump_allocator(TRANSIENT_STORAGE_SIZE);
  persistentStorage = make_bump_allocator(PERSISTENT_STORAGE_SIZE);

  GameState * gameState = (GameState*)bump_alloc(&persistentStorage, sizeof(GameState));

  // Defined in the file render_interface.h
  renderData = (RenderData*)bump_alloc(&persistentStorage, sizeof(RenderData));

  // Defiend in "input.h"
  input = (Input*)bump_alloc(&persistentStorage, sizeof(Input));

  uiState = (UIState*)bump_alloc(&persistentStorage, sizeof(UIState));

  // Defines in "sound.h"
  soundState = (SoundState*)bump_alloc(&persistentStorage, sizeof(SoundState));
  soundState->transientStorage = &transientStorage;

  // Allocating Data for Sounds
  soundState->allocatedsoundsBuffer = bump_alloc(&persistentStorage, SOUNDS_BUFFER_SIZE);

  if(!platform_create_window(ROOM_WIDTH * worldScale, 
                             ROOM_HEIGHT * worldScale, 
                             "Schnitzel Motor"))
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

  platform_set_vsync(true);

  while(running)
  {
    // In seconds
    double dt = get_delta_time();

    // Resent transient Storage
    transientStorage.used = 0;

    // Load the update_game function pointer from the DLL
    reload_game_dll();
    platform_update_window();

    update_game(gameState, input, renderData, soundState, uiState, &transientStorage, dt);
    gl_render();

    // This is platform specific!
    platform_update_audio(dt);
    platform_swap_buffers();
  }

  return 0;
}

void update_game(GameState* gameState, Input* inputIn, 
                 RenderData* renderDataIn, SoundState* soundStateIn,
                 UIState* uiStateIn, BumpAllocator* transientStorageIn, double dt)
{
  update_game_ptr(gameState, inputIn, renderDataIn, soundStateIn, uiStateIn, transientStorageIn, dt);
}

// #############################################################################
//                           Cross Platform
// #############################################################################
double get_delta_time()
{
  // Only executed once when entering the function (static)
  static auto lastTime = std::chrono::steady_clock::now();
  auto currentTime = std::chrono::steady_clock::now();

  // seconds
  double delta = std::chrono::duration<double>(currentTime - lastTime).count(); 
  lastTime = currentTime; 

  return delta;
}

void reload_game_dll()
{
  static void* gameDLL;
  static long long lastTimestampGameDLL;

  long long currentTimestampGameDLL = get_timestamp(gameDLLName);
  if(currentTimestampGameDLL > lastTimestampGameDLL)
  {
    if(gameDLL)
    {
      bool freeResult = platform_free_dynamic_library(gameDLL);
      SM_ASSERT(freeResult, "Failed to free game.dll");
      gameDLL = nullptr;
      SM_TRACE("Freed %s", gameDLLName);
    }

    while(!copy_file(gameDLLName, gameLoadDLLName, &transientStorage))
    {
      Sleep(10);
    }
    SM_TRACE("Copied %s", gameDLLName);

    gameDLL =  platform_load_dynamic_library(gameLoadDLLName);
    SM_ASSERT(gameDLL, "Failed to load %s", gameDLLName);

    update_game_ptr = (update_game_type*)platform_load_dynamic_function(gameDLL, "update_game");
    SM_ASSERT(update_game_ptr, "Failed to load update_game function");
    lastTimestampGameDLL = currentTimestampGameDLL;
  }
}
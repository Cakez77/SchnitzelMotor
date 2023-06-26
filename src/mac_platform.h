#pragma once

// Cocoa API specific code (modern)
#ifdef __cplusplus
extern "C" {
#endif
bool platform_create_window(int width, int height, char* title);
void platform_update_window();

#ifdef __cplusplus
}
#endif

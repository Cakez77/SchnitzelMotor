// mac_platform.cpp
// Bridge to Objective-C code
extern bool running;  // <- declare, but do not define

extern "C" {
    bool platform_create_window_objc(int width, int height, char* title);
    void platform_update_window_objc();
}

inline bool platform_create_window(int width, int height, char* title) {
    return platform_create_window_objc(width, height, title);
}

inline void platform_update_window() {
    platform_update_window_objc();
}
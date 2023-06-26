#include <Cocoa/Cocoa.h>

static NSWindow* window;

extern "C" {
    bool running = true; // time to signal exit from main loop
}

extern "C" bool platform_create_window(int width, int height, char* title)
{
    // Start the shared application
    [NSApplication sharedApplication];
    
    // Set the activation policy to regular before creating the window
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // Create the main window
    CGFloat screenWidth = [[NSScreen mainScreen] frame].size.width;
    CGFloat screenHeight = [[NSScreen mainScreen] frame].size.height;
    CGFloat windowX = (screenWidth - width) / 2;
    CGFloat windowY = (screenHeight - height) / 2; 
    NSRect frame = NSMakeRect(windowX, windowY, width, height);
   
    NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
    window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
    [window setTitle:[NSString stringWithUTF8String:title]];
    [window setLevel:NSStatusWindowLevel]; // Set window level top-most (above all other windows)
    [window makeKeyAndOrderFront:nil];
    [window orderFrontRegardless]; // Bring the main window into focus
    
    // Activate the window to give it focus
    [NSApp activateIgnoringOtherApps:YES];
    
    return true;
}

extern "C" void platform_update_window()
{
    while (running) {
        NSEvent* event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES]) && running) {
            // Handle other events like window resizing, closing, etc.
            [NSApp sendEvent:event];
            
            // Check for keyboard events and handle Cmd-X shortcut
            if ([event type] == NSEventTypeKeyDown) {
                NSString *characters = [event charactersIgnoringModifiers];
                NSEventModifierFlags flags = [event modifierFlags];
                
                if (flags & NSEventModifierFlagCommand && [characters isEqualToString:@"x"]) {
                    running = false; // Exit the application
                    break; // Stop processing events
                }
            }
        }
        
        [NSApp updateWindows];
        if (!running) break;
    }
}

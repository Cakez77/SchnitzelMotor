# Schnitzel Motor Engine: Mac Build Notes

The goal in bring the Schnitzel Motor Engine to the Mac platform was to provide a seamless and consistent integration with the supported platforms, focusing on simplicity and foundational elements that make this engine a terrific choice for students learning to write games in C++ for the first time. This document details the technical considerations, challenges, and rationale we encountered and the solution we settled upon.

## OpenGL Support on macOS

The Schnitzel Motor Engine utilizes OpenGL 4.1, supported by Apple up to macOS 10.11 Mojave. Key factors influenced this choice:

- **Ease of Adoption**: OpenGL 4.1 is native to macOS from version 10.11 Mojave onwards, eliminating additional software installations. Including necessary headers and libraries is straightforward.
- **Portability**: Transitioning the OpenGL Schnitzel Motor Engine to macOS necessitates minimal code alterations, ensuring efficiency.
- **Beginner-Friendly**: OpenGL is an intuitive graphics API, suitable for those new to game development.

However, there are limitations:
- **Deprecation**: OpenGL 4.1 is deprecated, risking removal in future macOS versions.
- **Apple's Metal Recommendation**: Apple advocates Metal for graphics rendering on macOS and iOS, hinting at a potential phase-out of OpenGL support.

## OpenGL Implementation Challenges on Mac

Support for OpenGL 4.3 and above is absent on macOS, requiring reliance on OpenGL 4.1. This alignment allows progression with minimal codebase adjustments, albeit with certain missing features:

- **Shader Storage Buffer Objects (SSBOs)**: Enable shaders to read from and write to buffer objects.
- **Debug Output**: Furnishes debugging and performance information from the OpenGL driver.
- **Texture Views**: Permit creating multiple views of a texture.
- **Vertex Attribute Binding**: Manages binding between vertex attributes and buffer objects.

These missing features necessitate alternative approaches or extensions for robust engine operation on OpenGL 4.1.

## Shader Adaptation from OpenGL 4.3 to 4.1

Transitioning the OpenGL 4.3 shader code to comply with OpenGL 4.1 standards posed challenges due to certain missing features. Various strategies ensured a successful transition with performance remaining close to the original version.

### 1. **Buffer Management**:
   - The `USE_OPENGL410` directive segregates buffer data handling for OpenGL 4.1 and 4.3.
   - For OpenGL 4.1:
      - A Vertex Buffer Object (VBO) stores vertex data, with `glBufferData` allocating buffer space, and `glVertexAttribPointer` setting up attribute pointers for interleaved data.
   - For OpenGL 4.3:
      - A Shader Storage Buffer Object (SSBO) manages transform data, a feature absent in OpenGL 4.1, requiring a different buffer management strategy.

### **Key Differences**:
   - **Buffer Management**: Use of VBOs in OpenGL 4.1 versus SSBOs in OpenGL 4.3 for handling buffer data.
   - **Vertex Processing**: Explicit vertex data processing in OpenGL 4.1 compared to the more streamlined approach in OpenGL 4.3.
   - **Shader Compilation**: Consistent shader compilation and error handling across both versions aids in debugging and ensures shader correctness.

## Solution

We opted for OpenGL as it aligns with the graphics API used on Windows and Linux, despite certain caveats on Mac due to its limitation to OpenGL 4.1 and the API's deprecation on this platform.

### **Pros**:
   - **Performance**: Employed strategies upheld performance despite OpenGL 4.1 restrictions.
   - **Learning Opportunity**: Porting shader code across OpenGL versions enlightens on the API's evolution.

Due to the absence of SSBOs in OpenGL 4.1, we opted for Vertex Buffer Objects (VBO) due to their speed, performance, simplicity, and alignment with our educational goals. VBOs, present since OpenGL 1.5, cater to our engine's needs efficiently. While the Schnitzel Motor Engine minimizes third-party library dependencies, we employed GLFW for its robust window management and streamlined OpenGL setup. GLFW simplifies build configurations, enabling a consistent build process across macOS, Windows, and Linux using the same "Single Compilation Unit" (SCU).

## Summary

The utilization of OpenGL 4.1 for the Schnitzel Motor Engine on macOS is driven by its ease of adoption, portability, and beginner-friendly nature, aligning with our educational goals. The deprecation of OpenGL 4.1 and the absence of certain features necessitate alternative strategies for robust engine operation. The use of GLFW enables a streamlined OpenGL setup on macOS, aligning with the engine's design principle of minimizing third-party dependencies.

Looking ahead, the aging OpenGL 4.1 may necessitate exploring other graphics APIs or projects to ensure the engine's longevity and adaptability to modern graphics programming standards.

### Resources
- [OpenGL Official Website](https://www.opengl.org/)
- [GLFW Official Website](https://www.glfw.org/)
- [Vulkan Official Website](https://www.khronos.org/vulkan/)
- [Metal - Apple Developer](https://developer.apple.com/metal/)
- [MGL Project on GitHub](https://github.com/openglonmetal/MGL)
- [Shader Storage Buffer Object - OpenGL Wiki](https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object)

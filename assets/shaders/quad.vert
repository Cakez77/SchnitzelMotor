#version 430

void main()
{
    // Creating Vertices on the GPU (2D Engine)
    // OpenGL Device Coordinates
    // -1 / 1                          1 / 1
    // -1 /-1                          1 /-1

    vec2 vertices[6] =
    {
      // Top Left
      vec2(-0.5,  0.5),

      // Bottom Left
      vec2(-0.5, -0.5),

      // Top Right
      vec2( 0.5,  0.5),

      // Top Right
      vec2( 0.5,  0.5),

      // Bottom Left
      vec2(-0.5, -0.5),

      // Bottom Right
      vec2( 0.5, -0.5)
    };

    // gl_VertexID is the index into the vertices when calling glDraw
    gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}




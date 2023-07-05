#version 430

struct Transform
{
  vec2 pos;
  vec2 size;
  vec2 atlasOffset;
  vec2 spriteSize;
};

// Input Uniforms
uniform vec2 screenSize;

// Input Buffers
layout(std430, binding = 0) buffer TransformSBO
{
  Transform transforms[];
};

// Output
layout (location = 0) out vec2 textureCoordsOut;

void main()
{
  Transform t = transforms[gl_InstanceID];

  // Creating Vertices on the GPU (2D Engine)
  // OpenGL Device Coordinates
  // -1 / 1                          1 / 1
  // -1 /-1                          1 /-1

  vec2 vertices[6] =
  {
    t.pos,                                // Top Left
    vec2(t.pos + vec2(0.0, t.size.y)),    // Bottom Left
    vec2(t.pos + vec2(t.size.x, 0.0)),    // Top Right
    vec2(t.pos + vec2(t.size.x, 0.0)),    // Top Right
    vec2(t.pos + vec2(0.0, t.size.y)),    // Bottom Left
    t.pos + t.size                        // Bottom Right
  };

  float left = t.atlasOffset.x;
  float top = t.atlasOffset.y;
  float right = t.atlasOffset.x + t.spriteSize.x;
  float bottom = t.atlasOffset.y + t.spriteSize.y;

  vec2 textureCoords[6] =
  {
    vec2(left,  top),
    vec2(left,  bottom),
    vec2(right, top),
    vec2(right, top),
    vec2(left,  bottom),
    vec2(right, bottom),
  };

  // Normalize Position
  vec2 vertexPos = vertices[gl_VertexID];
  vertexPos.y = -vertexPos.y + screenSize.y;
  vertexPos = 2.0 * (vertexPos / screenSize) - 1.0;

  // gl_VertexID is the index into the vertices when calling glDraw
  gl_Position = vec4(vertexPos, 0.0, 1.0);

  textureCoordsOut = textureCoords[gl_VertexID];
}





// Input Uniforms
uniform vec2 screenSize;
uniform mat4 orthoProjection;

// Input Buffers
layout(std430, binding = 0) buffer TransformSBO
{
  Transform transforms[];
};

// Output
layout (location = 0) out vec2 textureCoordsOut;
layout (location = 1) out flat int renderOptionsOut;

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

  if(bool(t.renderOptions & RENDERING_OPTION_FLIP_X))
  {
    float tmpLeft = left;
    left = right;
    right = tmpLeft;
  }

  vec2 textureCoords[6] =
  {
    vec2(left,  top),
    vec2(left,  bottom),
    vec2(right, top),
    vec2(right, top),
    vec2(left,  bottom),
    vec2(right, bottom),
  };

  vec2 vertexPos = vertices[gl_VertexID];
  // gl_VertexID is the index into the vertices when calling glDraw
  gl_Position = orthoProjection * vec4(vertexPos, 1.0, 1.0);

  textureCoordsOut = textureCoords[gl_VertexID];
  renderOptionsOut = t.renderOptions;
}














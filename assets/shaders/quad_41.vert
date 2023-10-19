// Input Uniforms
uniform vec2 screenSize;
uniform mat4 orthoProjection;

// Input Attributes from VBO
layout (location = 0) in vec2 positionIn;
layout (location = 1) in vec2 sizeIn;
layout (location = 2) in vec2 atlasOffsetIn;
layout (location = 3) in vec2 spriteSizeIn;
layout (location = 4) in int  renderOptionsIn;
layout (location = 5) in float layerIn;

// Output
layout (location = 0) out vec2 textureCoords;
layout (location = 1) flat out int renderOptions;

void main()
{
    // Creating Vertices on the GPU (2D Engine)
    // OpenGL Device Coordinates
    // -1 / 1                          1 / 1
    // -1 /-1                          1 /-1
    vec2 vertices[6];
    vertices[0] = positionIn;
    vertices[1] = positionIn + vec2(0.0, sizeIn.y);
    vertices[2] = positionIn + vec2(sizeIn.x, 0.0);
    vertices[3] = positionIn + vec2(sizeIn.x, 0.0);
    vertices[4] = positionIn + vec2(0.0, sizeIn.y);
    vertices[5] = positionIn + sizeIn;

    // Initialize textureCoords
    float left = atlasOffsetIn.x;
    float top = atlasOffsetIn.y;
    float right = atlasOffsetIn.x + spriteSizeIn.x;
    float bottom = atlasOffsetIn.y + spriteSizeIn.y;

    if (bool(renderOptionsIn & RENDERING_OPTION_FLIP_X))
    {
        float tmpLeft = left;
        left = right;
        right = tmpLeft;
    }

    vec2 localTextureCoords[6];
    localTextureCoords[0] = vec2(left, top);
    localTextureCoords[1] = vec2(left, bottom);
    localTextureCoords[2] = vec2(right, top);
    localTextureCoords[3] = vec2(right, top);
    localTextureCoords[4] = vec2(left, bottom);
    localTextureCoords[5] = vec2(right, bottom);

    // Compute final positions and outputs
    vec2 vertexPos = vertices[gl_VertexID];
    gl_Position = orthoProjection * vec4(vertexPos, layerIn, 1.0);
    
    textureCoords = localTextureCoords[gl_VertexID];
    renderOptions = renderOptionsIn;
}

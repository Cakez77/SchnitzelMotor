
// Input
layout (location = 0) in vec2 textureCoordsIn;
layout (location = 1) in flat int renderOptions;

// Output
layout (location = 0) out vec4 fragColor;

// Bindings
layout (binding = 0) uniform sampler2D textureAtlas;
layout (binding = 1) uniform sampler2D fontAtlas;

void main()
{
  vec4 textureColor;
  
  if(bool(renderOptions & RENDERING_OPTION_FONT))
  {
    textureColor = texelFetch(fontAtlas, ivec2(textureCoordsIn), 0);
    if(textureColor.r == 0.0)
    {
      discard;
    }

    textureColor = vec4(1);
  }
  else
  {
    textureColor = texelFetch(textureAtlas, ivec2(textureCoordsIn), 0);
    if(textureColor.a == 0.0)
    {
      discard;
    }
  }

  // Fill with White
  fragColor = textureColor;
}

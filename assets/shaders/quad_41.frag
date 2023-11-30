// Input
layout (location = 0) in vec2 textureCoords;  
layout (location = 1) flat in int renderOptions;

// Output
layout (location = 0) out vec4 fragColor;

// Uniforms (no binding)
uniform sampler2D textureAtlas;
uniform sampler2D fontAtlas;

void main()
{
    vec4 textureColor;

    if(bool(renderOptions & RENDERING_OPTION_FONT))  
    {
        textureColor = texelFetch(fontAtlas, ivec2(textureCoords), 0);  
        if(textureColor.r == 0.0)
        {
            discard;
        }
        textureColor = vec4(1.0, 1.0, 1.0, 1.0); // Filling with white
    }
    else
    {
        textureColor = texelFetch(textureAtlas, ivec2(textureCoords), 0);  
        if(textureColor.a == 0.0)
        {
            discard;
        }
    }

    fragColor = textureColor;
}

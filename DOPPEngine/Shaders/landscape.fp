
#version 420

uniform sampler2D baseMap;

varying vec2 Texcoord;

void main( void )
{
    vec2 tex = Texcoord;

    if (tex.y < 0.5)
    {
        tex.x = 1.0 - tex.x;
        tex.y = 0.5 - tex.y;
    }

    vec4 texColor = texture2D( baseMap, tex );
    
    gl_FragColor = vec4(texColor.r, texColor.g, texColor.b, 0.0);
}
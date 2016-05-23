
#version 420

uniform sampler2D baseMap;

in vec2 Texcoord;

void main( void )
{
    vec4 texColor = texture2D( baseMap, Texcoord );
    
    gl_FragColor = vec4(texColor.r, texColor.g, texColor.b, texColor.a);
}
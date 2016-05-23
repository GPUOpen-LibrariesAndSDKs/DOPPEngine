
#version 420

uniform sampler2D baseMap;

varying vec2 Texcoord;

void main( void )
{
    vec4 texColor = texture2D( baseMap, Texcoord );
    
    gl_FragColor = vec4(1.0) - vec4(texColor.r, texColor.g, texColor.b, 0.0);
}
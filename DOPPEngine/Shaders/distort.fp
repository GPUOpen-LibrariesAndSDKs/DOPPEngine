#version 420

uniform float     fTime;
uniform sampler2D baseMap;

varying vec2 Texcoord;

void main( void )
{
    float d;
    ivec2  texSize = textureSize(baseMap, 0);     
       
    d = distance(texSize/2.0, vec2(gl_FragCoord.x, gl_FragCoord.y));
    
    float s = sin(d/(texSize.x/8.0)) * sin(fTime)/10.0;
     
    gl_FragColor = texture2D( baseMap, vec2(Texcoord.s + s, Texcoord.t) );
   
    
}
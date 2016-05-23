
#version 420

uniform sampler2D baseMap;
varying vec2 Texcoord;

void main(void)
{
   float  scale = 4.0;
   vec4   laplace;
   vec2   samples0,samples1,samples2,samples3;
   ivec2  texSize = textureSize(baseMap, 0);       
       
   float pixelSize_S = 1.0f/ texSize.s;   
   float pixelSize_T = 1.0f/ texSize.t;    
 
  
   samples0 =  Texcoord + pixelSize_T *vec2( 0.0, -1.0);
   samples1 =  Texcoord + pixelSize_S *vec2(-1.0,  0.0);
   samples2 =  Texcoord + pixelSize_S *vec2( 1.0,  0.0);
   samples3 =  Texcoord + pixelSize_T *vec2( 0.0,  1.0);

 
   laplace = -4.0 * texture2D(baseMap, Texcoord);
   laplace += texture2D(baseMap, samples0);
   laplace += texture2D(baseMap, samples1);
   laplace += texture2D(baseMap, samples2);
   laplace += texture2D(baseMap, samples3);
   

   gl_FragColor = 1.0 - scale * laplace;
   
}
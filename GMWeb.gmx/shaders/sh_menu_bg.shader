//
// Simple passthrough vertex shader
//
attribute vec3 in_Position;                  // (x,y,z)
//attribute vec3 in_Normal;                  // (x,y,z)     unused in this shader.
attribute vec4 in_Colour;                    // (r,g,b,a)
attribute vec2 in_TextureCoord;              // (u,v)

varying vec2 v_vTexcoord;
varying vec4 v_vColour;

varying vec2 fragCoord;

void main()
{
    vec4 object_space_pos = vec4( in_Position.x, in_Position.y, in_Position.z, 1.0);
    gl_Position = gm_Matrices[MATRIX_WORLD_VIEW_PROJECTION] * object_space_pos;
    
    v_vColour = in_Colour;
    v_vTexcoord = in_TextureCoord;
    
    fragCoord = in_Position.xy;
}

//######################_==_YOYO_SHADER_MARKER_==_######################@~//Basic ripple shader
varying vec2 v_vTexcoord;
varying vec4 v_vColour;

varying vec2 fragCoord;

uniform vec2        iResolution;    //Viewport resolution (in pixels)
uniform float       iGlobalTime;    //Shader playback time (in seconds)
uniform sampler2D   iChannel0;      //Image input

void main()
{
    
    //Tweakable parameters
    float waveStrength = 0.18;
    float frequency = 560.0;
    float waveSpeed = 8.0;
    vec4 sunlightColor = vec4(0.0,0.0,0.0, 1.0);
    float sunlightStrength = 0.1;
    
    vec2 tapPoint = vec2(0.11,0.14);
    vec2 uv = fragCoord.xy / iResolution.xy;
    float modifiedTime = iGlobalTime * waveSpeed;
    float aspectRatio = (iResolution.x * 0.8)/iResolution.y;
    vec2 distVec = uv - tapPoint;
    distVec.x *= aspectRatio;
    float distance = length(distVec) * 0.5;
    vec2 newTexCoord = uv;
    
    float multiplier = (distance < 1.0) ? ((distance-1.0)*(distance-1.0)) : 0.0;
    float addend = (sin(frequency*distance-modifiedTime)+1.0) * waveStrength * multiplier;
    newTexCoord += addend;    
    
    vec4 colorToAdd = sunlightColor * sunlightStrength * addend;
    
    gl_FragColor = texture2D(iChannel0, newTexCoord) + colorToAdd;
}

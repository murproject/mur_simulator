#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vScreenPos;
varying float depth;
varying float time;

#ifdef COMPILEPS
uniform float cNoiseStrength;
#endif

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
    depth = GetDepth(gl_Position);
    time = cElapsedTime;
}

void PS()
{
    gl_FragColor = texture2D(sDiffMap, vScreenPos);
    float noiseFactor = cNoiseStrength;
    gl_FragColor -= rand(vScreenPos + sin(time)) * noiseFactor - noiseFactor/2.0;
}


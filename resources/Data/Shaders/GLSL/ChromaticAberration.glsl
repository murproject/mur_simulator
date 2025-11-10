#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vScreenPos;
varying float depth;
varying float time;

#ifdef COMPILEPS
uniform float cAberrationStrength;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
    depth = GetDepth(gl_Position);
    time = cElapsedTime;
}

vec4 aberration(sampler2D tex, vec2 pos, float strength) {
    return vec4(
        texture2D(tex, pos * (1.0 + 0.0075 * strength)).r,
        texture2D(tex, pos * (1.0 + 0.0000 * strength)).g,
        texture2D(tex, pos * (1.0 - 0.0045 * strength)).b,
        1.0
    );
}

void PS()
{
    gl_FragColor = aberration(sDiffMap, vScreenPos, cAberrationStrength);
}


#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vTexCoord;
varying vec2 vScreenPos;

varying float time;

#ifdef COMPILEPS
uniform float cDistortionFactor;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
    time = cElapsedTime;
}

void PS()
{
    float factor = cDistortionFactor;
    float zoom = 1.0 + abs(factor / 2);
    vec2 uResolution = vec2(1.0, 1.0);

    vec2 uv = vScreenPos;
    vec2 center = vec2(0.5);
    vec2 pos = uv - center;

    float r = pow(length(pos), 2);
    float rDistorted = r + factor * r * r;
    rDistorted /= zoom;

    vec2 dir = pos / r;
    vec2 posDistorted = dir * rDistorted;

    vec2 uvDistorted = posDistorted + center;
    gl_FragColor = texture2D(sDiffMap, uvDistorted);
}


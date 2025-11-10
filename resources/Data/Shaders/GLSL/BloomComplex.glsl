#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vScreenPos;
varying float depth;
varying float time;

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
    vec4 original = texture2D(sDiffMap, vScreenPos);

    #ifdef COPY
        gl_FragColor = original;
    #else
        vec4 unblurred = texture2D(sEmissiveMap, vScreenPos);
        vec4 blurAngular = texture2D(sNormalMap, vScreenPos);
        gl_FragColor = original + max(blurAngular - original, 0.0) * 1.0;
    #endif
}


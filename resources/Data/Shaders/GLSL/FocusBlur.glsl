#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vScreenPos;
varying float depth;
varying float time;

#ifdef COMPILEPS
uniform float cFocusBlurDepthFactor;
uniform float cFocusBlurDepthOffset;
uniform float cFocusBlurDepthMin;
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

void PS()
{
    vec4 depthInput = texture2D(sDepthBuffer, vScreenPos);
    float depth = DecodeDepth(depthInput.rgb);

    depth *= cFocusBlurDepthFactor;
    depth += cFocusBlurDepthOffset;
    depth = abs(depth);

    depth = clamp(depth, cFocusBlurDepthMin, 1.0);

    vec4 original = texture2D(sDiffMap, vScreenPos);
    vec4 blurred = texture2D(sEnvMap, vScreenPos);

    gl_FragColor = mix(original, blurred, depth);
}


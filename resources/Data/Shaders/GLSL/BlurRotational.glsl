#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vTexCoord;
varying vec2 vScreenPos;

#ifdef COMPILEPS
uniform vec2 cBlurDir;
uniform float cBlurRadius;
uniform float cBlurSigma;
uniform vec2 cBlurHInvSize;
uniform vec2 cBlurDirInv;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

void PS()
{
    vec2 blurDir;
    vec2 BlurHInvSize = cGBufferInvSize * 0.5;

    #ifdef BLUR_RADIAL
        float radialDistance = 3.0;
        blurDir = (radialDistance * (vTexCoord.xy)) - (radialDistance / 2.0);
        vec2 blurCentral = (1.0 - length(blurDir)) * cBlurDir * cBlurDirInv + 0.0;
        blurCentral = clamp(blurCentral, 0.0, 1.0);
        blurDir = mix(blurDir, cBlurDir, blurCentral);
    #else
        blurDir = (2.0 * cBlurDirInv) - 1.0;
        blurDir *= cBlurDir;
    #endif

    #ifdef BLUR3
        gl_FragColor = GaussianBlur(3, blurDir, BlurHInvSize * cBlurRadius, cBlurSigma, sDiffMap, vTexCoord);
    #endif

    #ifdef BLUR5
        gl_FragColor = GaussianBlur(5, blurDir, BlurHInvSize * cBlurRadius, cBlurSigma, sDiffMap, vTexCoord);
    #endif

    #ifdef BLUR7
        gl_FragColor = GaussianBlur(7, blurDir, BlurHInvSize * cBlurRadius, cBlurSigma, sDiffMap, vTexCoord);
    #endif

    #ifdef BLUR9
        gl_FragColor = GaussianBlur(9, blurDir, BlurHInvSize * cBlurRadius, cBlurSigma, sDiffMap, vTexCoord);
    #endif
}

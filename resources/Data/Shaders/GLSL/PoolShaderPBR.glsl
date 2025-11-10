#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Lighting.glsl"
#include "Constants.glsl"
#include "Fog.glsl"
#include "PBR.glsl"
#include "IBL.glsl"
#line 30010

#ifndef COMPILEPS
    uniform sampler2D sEmissiveMap;
#endif

varying float cNormalMapFactor;

#if defined(NORMALMAP)
    varying vec4 vTexCoord;
    varying vec4 vTangent;
#else
    varying vec2 vTexCoord;
#endif
varying vec3 vNormal;
varying vec4 vWorldPos;
#ifdef VERTEXCOLOR
    varying vec4 vColor;
#endif
#ifdef PERPIXEL
    #ifdef SHADOW
        #ifndef GL_ES
            varying vec4 vShadowPos[NUMCASCADES];
        #else
            varying highp vec4 vShadowPos[NUMCASCADES];
        #endif
    #endif
    #ifdef SPOTLIGHT
        varying vec4 vSpotPos;
    #endif
    #ifdef POINTLIGHT
        varying vec3 vCubeMaskVec;
    #endif
#else
    varying vec3 vVertexLight;
    varying vec4 vScreenPos;
    #ifdef ENVCUBEMAP
        varying vec3 vReflectionVec;
    #endif
    #if defined(LIGHTMAP) || defined(AO)
        varying vec2 vTexCoord2;
    #endif
#endif

varying vec4 mUOffset;
varying vec4 mVOffset;
varying vec4 vTexCoordNoScale;
varying float time;
varying float NormalMapFactor;

vec3 chooseAxis(vec3 n) {
    vec3 ax = vec3(1.0, 1.0, 0.0);
    vec3 ay = vec3(0.0, -1.0, 0.0);
    vec3 az = vec3(0.0, 1.0, 0.0);
    float dx = abs(dot(n, ax));
    float dy = abs(dot(n, ay));
    float dz = abs(dot(n, az));
    if (dx <= dy && dx <= dz) return ax;
    if (dy <= dx && dy <= dz) return ay;
    return az;
}


vec2 GetTexCoordTilingNormal(vec2 texCoord, vec2 scale)
{
    vec3 n = normalize(vNormal);
    vec3 axis = chooseAxis(n);
    vec3 tangent = normalize(cross(axis, n));
    vec3 bitangent = normalize(cross(n, tangent));
    vec2 uv = vec2(dot(vWorldPos.xyz, tangent), dot(vWorldPos.xyz, bitangent));
    return uv / scale;
}

vec2 GetTexCoordTilingWorld(vec2 texCoord, vec2 scale)
{
    #if defined(FOLLOW_UV)
        return GetTexCoordTilingNormal(texCoord, scale);
    #else
        vec2 uv = vWorldPos.xy - vWorldPos.yz;
        return uv / scale;
    #endif
}

vec4 triplanarTexture(sampler2D tex, vec3 pos, vec3 n, float tileSize, vec2 offset) {
    vec3 absN = n;
    vec3 w = absN / (absN.x + absN.y + absN.z);

    tileSize *= 1.0;
    vec2 tileSizeVec = vec2(tileSize, tileSize);

    vec2 uvXY = vec2(1.0, 1.0);
    vec2 uvXZ = vec2(1.0, 1.0);
    vec2 uvYZ = vec2(1.0, 1.0);

    vec4 tx = texture(tex, uvYZ * offset * tileSize);
    vec4 ty = texture(tex, uvXZ * offset * tileSize);
    vec4 tz = texture(tex, uvXY * offset * tileSize);

    return tx * w.x + ty * w.y + tz * w.z;
}


void VS()
{
    mUOffset = cUOffset;
    mVOffset = cVOffset;

    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vNormal = GetWorldNormal(modelMatrix);
    vWorldPos = vec4(worldPos, GetDepth(gl_Position));

    #ifdef VERTEXCOLOR
        vColor = iColor;
    #endif

    vec2 scale = vec2(mUOffset.x, mVOffset.y);

    #if defined(NORMALMAP) || defined(DIRBILLBOARD)
        vec4 tangent = GetWorldTangent(modelMatrix);
        vec3 bitangent = cross(tangent.xyz, vNormal) * tangent.w;
        #if defined(TILING)
            vTexCoord = vec4(GetTexCoordTilingNormal(iTexCoord, scale), bitangent.xy);
        #else
            vTexCoord = vec4(GetTexCoord(iTexCoord), bitangent.xy);
        #endif
        vTangent = vec4(tangent.xyz, bitangent.z);
        vTexCoordNoScale = vec4(GetTexCoordTilingWorld(iTexCoord, vec2(1.0,1.0)), bitangent.xy);
    #else
        #if defined(TILING)
            vTexCoord = GetTexCoordTilingNormal(iTexCoord, scale);
        #else
            vTexCoord = GetTexCoord(iTexCoord);
        #endif
        vTexCoordNoScale = vec4(GetTexCoordTilingWorld(iTexCoord, vec2(1.0,1.0)), vNormal.xy);
    #endif

    #ifdef PERPIXEL
        vec4 projWorldPos = vec4(worldPos, 1.0);

        #ifdef SHADOW
            for (int i = 0; i < NUMCASCADES; i++) {
                vec4 pos = projWorldPos;
                vShadowPos[i] = GetShadowPos(i, vNormal, pos);
            }
        #endif

        #ifdef SPOTLIGHT
            vSpotPos = projWorldPos * cLightMatrices[0];
        #endif

        #ifdef POINTLIGHT
            vCubeMaskVec = (worldPos - cLightPos.xyz) * mat3(cLightMatrices[0][0].xyz, cLightMatrices[0][1].xyz, cLightMatrices[0][2].xyz);
        #endif
    #else
        #if defined(LIGHTMAP) || defined(AO)
            vVertexLight = vec3(0.0, 0.0, 0.0);
            vTexCoord2 = iTexCoord1;
        #else
            vVertexLight = GetAmbient(GetZonePos(worldPos));
        #endif

        #ifdef NUMVERTEXLIGHTS
            for (int i = 0; i < NUMVERTEXLIGHTS; ++i)
                vVertexLight += GetVertexLight(i, worldPos, vNormal) * cVertexLights[i * 3].rgb;
        #endif

        vScreenPos = GetScreenPos(gl_Position);

        #ifdef ENVCUBEMAP
            vReflectionVec = worldPos - cCameraPos;
        #endif
    #endif

    time = cElapsedTime;
    NormalMapFactor = cNormalMapFactor;
}

void PS()
{
    #ifdef DIFFMAP
        vec4 diffInput = texture2D(sDiffMap, vTexCoord.xy);
        #ifdef ALPHAMASK
            if (diffInput.a < 0.5)
                discard;
        #endif
        vec4 diffColor = cMatDiffColor * diffInput;
    #else
        vec4 diffColor = cMatDiffColor;
    #endif

    #ifdef VERTEXCOLOR
        diffColor *= vColor;
    #endif

    #ifdef METALLIC
        vec4 roughMetalSrc = texture2D(sSpecMap, vTexCoord.xy);

        float roughness = roughMetalSrc.r + cRoughness;
        float metalness = roughMetalSrc.g + cMetallic;
    #else
        float roughness = cRoughness;
        float metalness = cMetallic;
    #endif

    roughness *= roughness;

    roughness = clamp(roughness, ROUGHNESS_FLOOR, 1.0);
    metalness = clamp(metalness, METALNESS_FLOOR, 1.0);

    vec3 specColor = mix(0.08 * cMatSpecColor.rgb, diffColor.rgb, metalness);
    diffColor.rgb = diffColor.rgb - diffColor.rgb * metalness;

    vec3 fogColor = vec3(0.0, 0.25, 0.3);

    #if defined(NORMALMAP) || defined(DIRBILLBOARD)
        vec3 tangent = vTangent.xyz;
        vec3 bitangent = vec3(vTexCoord.zw, vTangent.w);
        mat3 tbn = mat3(tangent, bitangent, vNormal);
    #endif

    #ifdef NORMALMAP
        vec3 nn = DecodeNormal(texture2D(sNormalMap, vTexCoord.xy));
        vec3 normal = normalize(tbn * nn);
    #else
        vec3 normal = normalize(vNormal);
    #endif

    #ifdef HEIGHTFOG
        float fogFactor = GetHeightFogFactor(vWorldPos.w, vWorldPos.y);
    #else
        float fogFactor = GetFogFactor(vWorldPos.w);
    #endif

    if (vWorldPos.y > 0) {
        fogFactor = 1.0;
    }

    fogFactor = min(fogFactor, 1.0);
    fogFactor = max(fogFactor, 0.0);

    #if defined(PERPIXEL)
        vec3 lightColor;
        vec3 lightDir;
        vec3 finalColor;

        float atten = 1;

        #if defined(DIRLIGHT)
            atten = GetAtten(normal, vWorldPos.xyz, lightDir);
        #elif defined(SPOTLIGHT)
            atten = GetAttenSpot(normal, vWorldPos.xyz, lightDir);
        #else
            atten = GetAttenPoint(normal, vWorldPos.xyz, lightDir);
        #endif

        float shadow = 1.0;
        #ifdef SHADOW
            shadow = GetShadow(vShadowPos, vWorldPos.w);
        #endif

        #if defined(SPOTLIGHT)
            lightColor = vSpotPos.w > 0.0 ? texture2DProj(sLightSpotMap, vSpotPos).rgb * cLightColor.rgb : vec3(0.0, 0.0, 0.0);
        #elif defined(CUBEMASK)
            lightColor = textureCube(sLightCubeMap, vCubeMaskVec).rgb * cLightColor.rgb;
        #else
            lightColor = cLightColor.rgb;
        #endif
        vec3 toCamera = normalize(cCameraPosPS - vWorldPos.xyz);
        vec3 lightVec = normalize(lightDir);
        float ndl = clamp((dot(normal, lightVec)), M_EPSILON, 1.0);

        vec3 BRDF = GetBRDF(vWorldPos.xyz, lightDir, lightVec, toCamera, normal, roughness, diffColor.rgb, specColor);

        finalColor.rgb = BRDF * lightColor * (atten * shadow) / M_PI;

        #ifdef AMBIENT
            finalColor += cAmbientColor.rgb * diffColor.rgb;
            finalColor += cMatEmissiveColor;
            gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
        #else
            gl_FragColor = vec4(GetLitFog(finalColor, fogFactor), diffColor.a);
        #endif
    #elif defined(DEFERRED)
        const vec3 spareData = vec3(0,0,0); 
        gl_FragData[0] = vec4(specColor, spareData.r);
        gl_FragData[1] = vec4(diffColor.rgb, spareData.g);
        gl_FragData[2] = vec4(normal * roughness, spareData.b);
        gl_FragData[3] = vec4(EncodeDepth(vWorldPos.w), 0.0);
    #else
        vec3 finalColor = vVertexLight * diffColor.rgb;
        #ifdef AO
            finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * cAmbientColor.rgb * diffColor.rgb;
        #endif

        #ifdef MATERIAL
            vec4 lightInput = 2.0 * texture2DProj(sLightBuffer, vScreenPos);
            vec3 lightSpecColor = lightInput.a * lightInput.rgb / max(GetIntensity(lightInput.rgb), 0.001);

            finalColor += lightInput.rgb * diffColor.rgb + lightSpecColor * specColor;
        #endif

        vec3 toCamera = normalize(vWorldPos.xyz - cCameraPosPS);
        vec3 reflection = normalize(reflect(toCamera, normal));

        vec3 cubeColor = vVertexLight.rgb;

        #ifdef IBL
          vec3 iblColor = ImageBasedLighting(reflection, normal, toCamera, diffColor.rgb, specColor.rgb, roughness, cubeColor);
          float gamma = 0.0;
          finalColor.rgb += iblColor;
        #endif

        #ifdef ENVCUBEMAP
            finalColor += cMatEnvMapColor * textureCube(sEnvCubeMap, reflect(vReflectionVec, normal)).rgb;
        #endif
        #ifdef LIGHTMAP
            finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * diffColor.rgb;
        #endif
        #ifdef EMISSIVEMAP
            finalColor += cMatEmissiveColor * texture2D(sEmissiveMap, vTexCoord.xy).rgb;
        #else
            finalColor += cMatEmissiveColor;
        #endif

        gl_FragColor = vec4(mix(fogColor, finalColor, max(0.0, pow(fogFactor * 1.0, 100.0) - 0.1)), diffColor.a);
    #endif

    vec4 waterBlissInput;
    vec4 waterDispalcement;
    float power = 10.0;
    float waterBlissFactor = 0.0;

    waterBlissFactor = (normalize(vNormal).y * 1.0) + 1.1;
    waterBlissFactor = clamp(waterBlissFactor, 0.0, 1.0) * 2.0;


    waterDispalcement = texture2D(sEmissiveMap, vTexCoordNoScale.xy * 0.2) * 2.0;
    waterDispalcement.y = waterDispalcement.y + time;

    waterBlissInput = triplanarTexture(
        sEnvMap,
        vWorldPos.xyz,
        vNormal,
        0.5,
        vTexCoordNoScale.xy / 1.0 + waterDispalcement.xy / 10.0
    );

    waterBlissInput.rgb = vec3(pow(waterBlissInput.g, power));

    waterBlissInput *= 0.025;
    waterBlissInput *= min(abs(normalize(vNormal).y) + 0.5, 1.0);
    waterBlissInput *= min(abs(min(vWorldPos.y, 0.0)), 1.0);

    gl_FragColor += waterBlissInput * waterBlissFactor;


    waterDispalcement = texture2D(sEmissiveMap, vTexCoordNoScale.yx * 0.01) * 100.0;
    waterDispalcement.x = waterDispalcement.x + time * 2;

    waterBlissInput = triplanarTexture(
        sEnvMap,
        vWorldPos.xyz,
        vNormal,
        0.1,
        vTexCoordNoScale.xy / 1.0 + waterDispalcement.xy / 15.0
    );

    waterBlissInput.rgb = vec3(pow(waterBlissInput.b, power));

    waterBlissInput *= 0.015;
    waterBlissInput *= min(abs(normalize(vNormal).y) + 0.5, 1.0);
    waterBlissInput *= min(abs(min(vWorldPos.y, 0.0)), 1.0);

    gl_FragColor += waterBlissInput * waterBlissFactor;
}

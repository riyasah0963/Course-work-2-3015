#version 460 core

const float PI = 3.14159265359;
const int PointLightCount = 2;

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec3 ObjectPosition;
in vec2 TexCoord;
in vec4 LightSpacePosition;

uniform vec3 CameraPosition;
uniform vec3 DirectionalLightDir;
uniform vec3 DirectionalLightColor;
uniform vec3 PointLightPositions[PointLightCount];
uniform vec3 PointLightColors[PointLightCount];
uniform vec3 Albedo;
uniform vec3 HairColor;
uniform float Metallic;
uniform float Roughness;
uniform float Ao;
uniform float HairLayer;
uniform float HairLayers;
uniform bool UseTexture;
uniform bool UseHairShell;
uniform bool UseNoiseMaterial;
uniform float NoiseScale;
uniform float NoiseStrength;
uniform vec3 NoiseDarkColor;
uniform vec3 NoiseLightColor;

uniform sampler2D DiffuseTex;
uniform sampler2D ShadowMap;

layout (location = 0) out vec4 FragColor;

float distributionGGX(vec3 n, vec3 h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float nDotH = max(dot(n, h), 0.0);
    float nDotH2 = nDotH * nDotH;
    float denom = (nDotH2 * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 0.0001);
}

float geometrySchlickGGX(float nDotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return nDotV / max(nDotV * (1.0 - k) + k, 0.0001);
}

float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
    float ggx2 = geometrySchlickGGX(max(dot(n, v), 0.0), roughness);
    float ggx1 = geometrySchlickGGX(max(dot(n, l), 0.0), roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float hash13(vec3 p)
{
    p = fract(p * 0.1031);
    p += dot(p, p.zyx + 31.32);
    return fract((p.x + p.y) * p.z);
}

float valueNoise(vec3 p)
{
    vec3 base = floor(p);
    vec3 fracPart = fract(p);
    vec3 smoothPart = fracPart * fracPart * (3.0 - 2.0 * fracPart);

    float n000 = hash13(base + vec3(0.0, 0.0, 0.0));
    float n100 = hash13(base + vec3(1.0, 0.0, 0.0));
    float n010 = hash13(base + vec3(0.0, 1.0, 0.0));
    float n110 = hash13(base + vec3(1.0, 1.0, 0.0));
    float n001 = hash13(base + vec3(0.0, 0.0, 1.0));
    float n101 = hash13(base + vec3(1.0, 0.0, 1.0));
    float n011 = hash13(base + vec3(0.0, 1.0, 1.0));
    float n111 = hash13(base + vec3(1.0, 1.0, 1.0));

    float nx00 = mix(n000, n100, smoothPart.x);
    float nx10 = mix(n010, n110, smoothPart.x);
    float nx01 = mix(n001, n101, smoothPart.x);
    float nx11 = mix(n011, n111, smoothPart.x);
    float nxy0 = mix(nx00, nx10, smoothPart.y);
    float nxy1 = mix(nx01, nx11, smoothPart.y);

    return mix(nxy0, nxy1, smoothPart.z);
}

float fbm(vec3 p)
{
    float sum = 0.0;
    float amplitude = 0.5;

    for (int i = 0; i < 5; i++) {
        sum += amplitude * valueNoise(p);
        p = p * 2.02 + vec3(17.1, 11.7, 5.3);
        amplitude *= 0.5;
    }

    return sum;
}

float calculateShadow(vec3 normal, vec3 lightDir)
{
    vec3 projCoords = LightSpacePosition.xyz / max(LightSpacePosition.w, 0.0001);
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float bias = max(0.0025 * (1.0 - dot(normal, lightDir)), 0.0008);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(ShadowMap, 0));

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float closestDepth = texture(ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    vec3 n = normalize(WorldNormal);
    vec3 v = normalize(CameraPosition - WorldPosition);
    vec3 albedo = Albedo;
    float metallic = Metallic;
    float roughness = clamp(Roughness, 0.05, 1.0);
    float alpha = 1.0;

    if (UseTexture) {
        albedo *= texture(DiffuseTex, TexCoord).rgb;
    }

    if (UseNoiseMaterial) {
        vec3 noisePos = ObjectPosition * NoiseScale;
        float turbulence = fbm(noisePos * vec3(1.0, 0.55, 1.0));
        float rings = fract(length(ObjectPosition.xz) * NoiseScale * 3.2 + turbulence * NoiseStrength);
        float grain = fbm(noisePos * vec3(2.0, 10.0, 2.0));
        float woodMask = smoothstep(0.15, 0.85, rings + (grain - 0.5) * 0.35);

        albedo *= mix(NoiseDarkColor, NoiseLightColor, woodMask);
        roughness = clamp(roughness + (1.0 - woodMask) * 0.12 + grain * 0.08, 0.05, 1.0);
        metallic *= 0.15;
    }

    if (UseHairShell) {
        float layerFrac = HairLayer / max(HairLayers, 1.0);
        vec2 uv = vec2(TexCoord.x * 24.0, TexCoord.y * 70.0 + layerFrac * 9.0);
        vec2 cell = floor(uv * vec2(1.5, 0.35));
        float rnd = hash12(cell + layerFrac * 17.0);
        float strand = abs(fract(uv.y + rnd * 8.0 + TexCoord.x * 11.0) - 0.5);
        float width = mix(0.34, 0.06, layerFrac);
        float strandMask = 1.0 - smoothstep(width, width + 0.05, strand);
        float densityMask = step(layerFrac * 0.82, rnd + 0.18);
        float coverage = strandMask * densityMask;

        if (coverage < 0.22) {
            discard;
        }

        albedo = mix(albedo, HairColor, 0.9);
        roughness = 0.96;
        metallic = 0.0;
        alpha = coverage * (1.0 - layerFrac * 0.25);
    }

    vec3 f0 = mix(vec3(0.04), albedo, metallic);
    vec3 lo = vec3(0.0);

    vec3 dirLight = normalize(-DirectionalLightDir);
    vec3 h = normalize(v + dirLight);
    float ndf = distributionGGX(n, h, roughness);
    float g = geometrySmith(n, v, dirLight, roughness);
    vec3 f = fresnelSchlick(max(dot(h, v), 0.0), f0);
    vec3 numerator = ndf * g * f;
    float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, dirLight), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    vec3 kS = f;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    float nDotL = max(dot(n, dirLight), 0.0);
    float shadow = calculateShadow(n, dirLight);
    lo += (kD * albedo / PI + specular) * DirectionalLightColor * nDotL * (1.0 - shadow);

    for (int i = 0; i < PointLightCount; i++) {
        vec3 l = normalize(PointLightPositions[i] - WorldPosition);
        vec3 halfVector = normalize(v + l);
        float distance = length(PointLightPositions[i] - WorldPosition);
        float attenuation = 1.0 / max(distance * distance, 0.001);
        vec3 radiance = PointLightColors[i] * attenuation;

        float pointNdf = distributionGGX(n, halfVector, roughness);
        float pointG = geometrySmith(n, v, l, roughness);
        vec3 pointF = fresnelSchlick(max(dot(halfVector, v), 0.0), f0);
        vec3 pointNumerator = pointNdf * pointG * pointF;
        float pointDenominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001;
        vec3 pointSpecular = pointNumerator / pointDenominator;
        vec3 pointKs = pointF;
        vec3 pointKd = (vec3(1.0) - pointKs) * (1.0 - metallic);
        float pointNdotL = max(dot(n, l), 0.0);

        lo += (pointKd * albedo / PI + pointSpecular) * radiance * pointNdotL;
    }

    vec3 ambient = vec3(0.028) * albedo * Ao;
    vec3 color = ambient + lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, alpha);
}

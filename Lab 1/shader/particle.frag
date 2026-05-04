#version 460 core

in float ParticleLifeRatio;

layout (location = 0) out vec4 FragColor;

void main()
{
    vec2 centered = gl_PointCoord * 2.0 - 1.0;
    float radial = dot(centered, centered);

    if (radial > 1.0) {
        discard;
    }

    float softEdge = 1.0 - smoothstep(0.2, 1.0, radial);
    vec3 innerColor = vec3(1.0, 0.93, 0.72);
    vec3 outerColor = vec3(1.0, 0.42, 0.08);
    vec3 color = mix(outerColor, innerColor, ParticleLifeRatio);
    float alpha = softEdge * ParticleLifeRatio * 0.9;

    FragColor = vec4(color, alpha);
}

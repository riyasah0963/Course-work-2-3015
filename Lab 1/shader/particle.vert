#version 460 core

layout (location = 0) in vec3 ParticlePosition;
layout (location = 1) in float LifeRatio;

uniform mat4 View;
uniform mat4 Projection;

out float ParticleLifeRatio;

void main()
{
    vec4 viewPos = View * vec4(ParticlePosition, 1.0);
    float distanceScale = clamp(22.0 / max(-viewPos.z, 0.35), 6.0, 28.0);

    ParticleLifeRatio = LifeRatio;
    gl_Position = Projection * viewPos;
    gl_PointSize = distanceScale * mix(0.45, 1.0, LifeRatio);
}

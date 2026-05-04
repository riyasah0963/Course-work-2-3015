#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat3 NormalMatrix;
uniform mat4 LightSpaceMatrix;
uniform float HairShellOffset;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec3 ObjectPosition;
out vec2 TexCoord;
out vec4 LightSpacePosition;

void main()
{
    vec3 displacedPosition = VertexPosition + VertexNormal * HairShellOffset;
    vec4 worldPos = Model * vec4(displacedPosition, 1.0);

    WorldPosition = worldPos.xyz;
    WorldNormal = normalize(NormalMatrix * VertexNormal);
    ObjectPosition = displacedPosition;
    TexCoord = VertexTexCoord;
    LightSpacePosition = LightSpaceMatrix * worldPos;

    gl_Position = Projection * View * worldPos;
}

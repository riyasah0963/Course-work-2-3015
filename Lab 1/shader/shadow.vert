#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform mat4 Model;
uniform mat4 LightSpaceMatrix;

void main()
{
    gl_Position = LightSpaceMatrix * Model * vec4(VertexPosition, 1.0);
}

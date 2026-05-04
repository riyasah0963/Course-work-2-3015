#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    TexCoords = aPos;

    // Remove translation from view matrix for skybox
    mat4 viewNoTranslate = mat4(mat3(View));

    vec4 pos = Projection * viewNoTranslate * vec4(aPos, 1.0);
    gl_Position = pos.xyww;  // Important trick for skybox depth
}

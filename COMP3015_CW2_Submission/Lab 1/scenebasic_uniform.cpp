#include "scenebasic_uniform.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "helper/glutils.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

using glm::mat3;
using glm::mat4;
using glm::vec3;
using std::cerr;
using std::endl;

SceneBasic_Uniform::SceneBasic_Uniform()
    : torus(nullptr),
      ground(nullptr),
      angle(0.0f),
      tPrev(0.0f),
      checkerTex(0),
      shadowFbo(0),
      shadowTex(0),
      lightSpaceMatrix(1.0f)
{
}

SceneBasic_Uniform::~SceneBasic_Uniform()
{
    delete torus;
    delete ground;

    if (checkerTex != 0) {
        glDeleteTextures(1, &checkerTex);
    }

    if (shadowTex != 0) {
        glDeleteTextures(1, &shadowTex);
    }

    if (shadowFbo != 0) {
        glDeleteFramebuffers(1, &shadowFbo);
    }
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.055f, 0.07f, 0.10f, 1.0f);

    torus = new Torus(0.95f, 0.30f, 64, 64);
    ground = new Plane(18.0f, 18.0f, 1, 1, 4.0f, 4.0f);

    initTextures();
    initShadowMap();
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();

        shadowProg.compileShader("shader/shadow.vert");
        shadowProg.compileShader("shader/shadow.frag");
        shadowProg.link();
    }
    catch (GLSLProgramException &e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::initTextures()
{
    const int texSize = 256;
    std::vector<unsigned char> data(texSize * texSize * 3);

    for (int y = 0; y < texSize; y++) {
        for (int x = 0; x < texSize; x++) {
            int idx = (y * texSize + x) * 3;
            bool evenTile = ((x / 32) + (y / 32)) % 2 == 0;
            unsigned char value = evenTile ? 214 : 62;
            data[idx + 0] = value;
            data[idx + 1] = static_cast<unsigned char>(value * 0.92f);
            data[idx + 2] = static_cast<unsigned char>(value * 0.74f);
        }
    }

    glGenTextures(1, &checkerTex);
    glBindTexture(GL_TEXTURE_2D, checkerTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SceneBasic_Uniform::initShadowMap()
{
    glGenFramebuffers(1, &shadowFbo);

    glGenTextures(1, &shadowTex);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, ShadowMapSize, ShadowMapSize, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Shadow framebuffer is incomplete.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::update(float t)
{
    if (tPrev == 0.0f) {
        tPrev = t;
    }

    float deltaT = t - tPrev;
    tPrev = t;

    if (animating()) {
        angle += deltaT;
    }
}

void SceneBasic_Uniform::render()
{
    vec3 directionalLightDir = glm::normalize(vec3(-0.65f, -1.0f, -0.25f));
    vec3 lightPos = -directionalLightDir * 12.0f;
    mat4 lightView = glm::lookAt(lightPos, vec3(0.0f, 0.2f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    mat4 lightProj = glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, 2.0f, 30.0f);
    lightSpaceMatrix = lightProj * lightView;

    vec3 cameraPosition = vec3(8.2f * cos(angle * 0.25f) - 0.8f, 3.6f, 9.4f * sin(angle * 0.25f) + 1.4f);
    view = glm::lookAt(cameraPosition, vec3(0.3f, 0.5f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

    glViewport(0, 0, ShadowMapSize, ShadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    shadowProg.use();
    renderScene(shadowProg, true);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.use();
    setLightUniforms(cameraPosition, directionalLightDir);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, checkerTex);
    prog.setUniform("DiffuseTex", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    prog.setUniform("ShadowMap", 1);

    renderScene(prog, false);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    projection = glm::perspective(glm::radians(50.0f), float(w) / float(h), 0.3f, 100.0f);
}

void SceneBasic_Uniform::setMatrices(GLSLProgram &shader)
{
    shader.setUniform("Model", model);
    shader.setUniform("View", view);
    shader.setUniform("Projection", projection);
    shader.setUniform("NormalMatrix", mat3(glm::transpose(glm::inverse(model))));
    shader.setUniform("LightSpaceMatrix", lightSpaceMatrix);
}

void SceneBasic_Uniform::setLightUniforms(const vec3 &cameraPosition, const vec3 &directionalLightDir)
{
    vec3 pointLightPositions[2] = {
        vec3(-4.0f, 4.6f, 4.0f) + vec3(cos(angle * 0.7f), 0.0f, sin(angle * 0.7f)),
        vec3(4.2f, 3.8f, -4.5f) + vec3(sin(angle * 0.5f), 0.5f * cos(angle * 0.8f), cos(angle * 0.5f))
    };

    vec3 pointLightColors[2] = {
        vec3(14.0f, 13.0f, 11.0f),
        vec3(10.0f, 12.0f, 16.0f)
    };

    prog.setUniform("CameraPosition", cameraPosition);
    prog.setUniform("DirectionalLightDir", directionalLightDir);
    prog.setUniform("DirectionalLightColor", vec3(5.5f, 5.0f, 4.8f));
    prog.setUniform("PointLightPositions[0]", pointLightPositions[0]);
    prog.setUniform("PointLightPositions[1]", pointLightPositions[1]);
    prog.setUniform("PointLightColors[0]", pointLightColors[0]);
    prog.setUniform("PointLightColors[1]", pointLightColors[1]);
}

void SceneBasic_Uniform::renderScene(GLSLProgram &shader, bool shadowPass)
{
    renderGround(shader, shadowPass);

    renderTorusInstance(shader, shadowPass, vec3(-4.2f, 0.3f, -1.8f), vec3(0.92f, 0.16f, 0.12f),
        0.06f, 0.18f, 0.0f, false);
    renderTorusInstance(shader, shadowPass, vec3(-1.4f, 0.35f, -0.7f), vec3(0.96f, 0.71f, 0.18f),
        0.24f, 0.32f, 0.9f, true);
    renderTorusInstance(shader, shadowPass, vec3(1.4f, 0.3f, 0.5f), vec3(0.93f, 0.66f, 0.56f),
        0.74f, 0.24f, 1.8f, false);
    renderTorusInstance(shader, shadowPass, vec3(4.0f, 0.35f, 1.7f), vec3(0.92f, 0.92f, 0.90f),
        1.0f, 0.58f, 2.5f, false);

    renderHairObject(shader, shadowPass);
}

void SceneBasic_Uniform::renderGround(GLSLProgram &shader, bool shadowPass)
{
    model = glm::translate(mat4(1.0f), vec3(0.0f, -1.15f, 0.0f));
    setMatrices(shader);

    if (!shadowPass) {
        shader.setUniform("UseTexture", true);
        shader.setUniform("UseHairShell", false);
        shader.setUniform("HairShellOffset", 0.0f);
        shader.setUniform("HairLayer", 0.0f);
        shader.setUniform("HairLayers", 1.0f);
        shader.setUniform("Albedo", vec3(0.78f, 0.72f, 0.62f));
        shader.setUniform("Metallic", 0.0f);
        shader.setUniform("Roughness", 0.95f);
        shader.setUniform("Ao", 1.0f);
        shader.setUniform("HairColor", vec3(0.0f));
    }

    ground->render();
}

void SceneBasic_Uniform::renderTorusInstance(GLSLProgram &shader, bool shadowPass, const vec3 &position,
    const vec3 &albedo, float metallic, float roughness, float spinOffset, bool useTexture)
{
    model = glm::translate(mat4(1.0f), position);
    model = glm::rotate(model, angle * 0.85f + spinOffset, vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(65.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices(shader);

    if (!shadowPass) {
        shader.setUniform("UseTexture", useTexture);
        shader.setUniform("UseHairShell", false);
        shader.setUniform("HairShellOffset", 0.0f);
        shader.setUniform("HairLayer", 0.0f);
        shader.setUniform("HairLayers", 1.0f);
        shader.setUniform("Albedo", albedo);
        shader.setUniform("Metallic", metallic);
        shader.setUniform("Roughness", roughness);
        shader.setUniform("Ao", 1.0f);
        shader.setUniform("HairColor", vec3(0.0f));
    }

    torus->render();
}

void SceneBasic_Uniform::renderHairObject(GLSLProgram &shader, bool shadowPass)
{
    const vec3 hairBasePosition(-6.0f, 0.28f, -0.8f);
    model = glm::translate(mat4(1.0f), hairBasePosition);
    model = glm::rotate(model, angle * 0.75f, vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(88.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(1.05f, 0.88f, 1.05f));
    setMatrices(shader);

    if (!shadowPass) {
        shader.setUniform("UseTexture", false);
        shader.setUniform("UseHairShell", false);
        shader.setUniform("HairShellOffset", 0.0f);
        shader.setUniform("HairLayer", 0.0f);
        shader.setUniform("HairLayers", 1.0f);
        shader.setUniform("Albedo", vec3(0.22f, 0.16f, 0.10f));
        shader.setUniform("Metallic", 0.0f);
        shader.setUniform("Roughness", 0.86f);
        shader.setUniform("Ao", 1.0f);
        shader.setUniform("HairColor", vec3(0.80f, 0.61f, 0.28f));
    }

    torus->render();

    if (shadowPass) {
        return;
    }

    const int hairLayers = 24;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (int layer = 1; layer <= hairLayers; layer++) {
        float shellOffset = 0.016f * float(layer) / float(hairLayers);
        shader.setUniform("UseTexture", false);
        shader.setUniform("UseHairShell", true);
        shader.setUniform("HairShellOffset", shellOffset);
        shader.setUniform("HairLayer", float(layer));
        shader.setUniform("HairLayers", float(hairLayers));
        shader.setUniform("Albedo", vec3(0.26f, 0.18f, 0.10f));
        shader.setUniform("Metallic", 0.0f);
        shader.setUniform("Roughness", 0.92f);
        shader.setUniform("Ao", 1.0f);
        shader.setUniform("HairColor", vec3(0.92f, 0.74f, 0.36f));
        torus->render();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

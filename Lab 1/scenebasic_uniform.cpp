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

namespace {
float hashFloat(float value)
{
    return glm::fract(sinf(value * 12.9898f) * 43758.5453f);
}
}

SceneBasic_Uniform::SceneBasic_Uniform()
    : torus(nullptr),
      ground(nullptr),
      angle(0.0f),
      tPrev(0.0f),
      elapsedTime(0.0f),
      checkerTex(0),
      shadowFbo(0),
      shadowTex(0),
      particleVao(0),
      particlePosVbo(0),
      particleLifeVbo(0),
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

    if (particlePosVbo != 0) {
        glDeleteBuffers(1, &particlePosVbo);
    }

    if (particleLifeVbo != 0) {
        glDeleteBuffers(1, &particleLifeVbo);
    }

    if (particleVao != 0) {
        glDeleteVertexArrays(1, &particleVao);
    }
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glCullFace(GL_BACK);
    glClearColor(0.055f, 0.07f, 0.10f, 1.0f);

    torus = new Torus(0.95f, 0.30f, 64, 64);
    ground = new Plane(18.0f, 18.0f, 1, 1, 4.0f, 4.0f);
    modelMesh = ObjMesh::load("media/simple_totem.obj", true, false);

    initTextures();
    initShadowMap();
    initParticles();
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

        particleProg.compileShader("shader/particle.vert");
        particleProg.compileShader("shader/particle.frag");
        particleProg.link();
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

void SceneBasic_Uniform::initParticles()
{
    particles.resize(ParticleCount);
    particlePositions.resize(ParticleCount);
    particleLifeRatios.resize(ParticleCount);

    for (int i = 0; i < ParticleCount; i++) {
        resetParticle(particles[i], i, -0.02f * float(i));
        particlePositions[i] = particles[i].position;
        particleLifeRatios[i] = 1.0f;
    }

    glGenVertexArrays(1, &particleVao);
    glBindVertexArray(particleVao);

    glGenBuffers(1, &particlePosVbo);
    glBindBuffer(GL_ARRAY_BUFFER, particlePosVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * particlePositions.size(), particlePositions.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &particleLifeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, particleLifeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * particleLifeRatios.size(), particleLifeRatios.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void SceneBasic_Uniform::resetParticle(Particle &particle, int index, float cycleOffset)
{
    const float baseSeed = float(index) * 17.0f + cycleOffset * 13.0f + elapsedTime * 0.37f;
    const float angleSeed = hashFloat(baseSeed + 1.2f) * glm::two_pi<float>();
    const float radialSpeed = 0.18f + hashFloat(baseSeed + 2.4f) * 0.38f;
    const float verticalSpeed = 2.3f + hashFloat(baseSeed + 4.8f) * 1.1f;
    const float lifeJitter = hashFloat(baseSeed + 7.1f);
    const vec3 emitterPosition(-2.4f, -0.78f, 2.6f);

    particle.position = emitterPosition;
    particle.velocity = vec3(cos(angleSeed) * radialSpeed, verticalSpeed, sin(angleSeed) * radialSpeed);
    particle.age = 0.0f;
    particle.life = 1.2f + lifeJitter * 1.0f;
}

void SceneBasic_Uniform::updateParticles(float deltaT)
{
    const vec3 gravity(0.0f, -4.8f, 0.0f);

    for (int i = 0; i < ParticleCount; i++) {
        Particle &particle = particles[i];
        particle.age += deltaT;

        if (particle.age >= particle.life) {
            resetParticle(particle, i, particle.age);
        }

        particle.velocity += gravity * deltaT;
        particle.position += particle.velocity * deltaT;

        particlePositions[i] = particle.position;
        particleLifeRatios[i] = 1.0f - particle.age / particle.life;
    }

    glBindBuffer(GL_ARRAY_BUFFER, particlePosVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * particlePositions.size(), particlePositions.data());

    glBindBuffer(GL_ARRAY_BUFFER, particleLifeVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * particleLifeRatios.size(), particleLifeRatios.data());
}

void SceneBasic_Uniform::update(float t)
{
    elapsedTime = t;

    if (tPrev == 0.0f) {
        tPrev = t;
    }

    float deltaT = t - tPrev;
    tPrev = t;
    if (deltaT < 0.0f) {
        deltaT = 0.0f;
    }
    if (deltaT > 0.033f) {
        deltaT = 0.033f;
    }

    if (animating()) {
        angle += deltaT;
        updateParticles(deltaT);
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
    renderParticles();
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

void SceneBasic_Uniform::setSurfaceUniforms(GLSLProgram &shader, bool useTexture, bool useHairShell,
    bool useNoiseMaterial, float hairShellOffset, float hairLayer, float hairLayers, const vec3 &albedo,
    float metallic, float roughness, float ao, const vec3 &hairColor, float noiseScale, float noiseStrength,
    const vec3 &noiseDarkColor, const vec3 &noiseLightColor)
{
    shader.setUniform("UseTexture", useTexture);
    shader.setUniform("UseHairShell", useHairShell);
    shader.setUniform("UseNoiseMaterial", useNoiseMaterial);
    shader.setUniform("HairShellOffset", hairShellOffset);
    shader.setUniform("HairLayer", hairLayer);
    shader.setUniform("HairLayers", hairLayers);
    shader.setUniform("Albedo", albedo);
    shader.setUniform("Metallic", metallic);
    shader.setUniform("Roughness", roughness);
    shader.setUniform("Ao", ao);
    shader.setUniform("HairColor", hairColor);
    shader.setUniform("NoiseScale", noiseScale);
    shader.setUniform("NoiseStrength", noiseStrength);
    shader.setUniform("NoiseDarkColor", noiseDarkColor);
    shader.setUniform("NoiseLightColor", noiseLightColor);
}

void SceneBasic_Uniform::renderScene(GLSLProgram &shader, bool shadowPass)
{
    renderGround(shader, shadowPass);

    renderTorusInstance(shader, shadowPass, vec3(-4.2f, 0.3f, -1.8f), vec3(0.92f, 0.16f, 0.12f),
        0.06f, 0.18f, 0.0f, false, false);
    renderTorusInstance(shader, shadowPass, vec3(-1.4f, 0.35f, -0.7f), vec3(0.96f, 0.71f, 0.18f),
        0.24f, 0.32f, 0.9f, true, false);
    renderTorusInstance(shader, shadowPass, vec3(1.4f, 0.3f, 0.5f), vec3(0.88f, 0.73f, 0.48f),
        0.08f, 0.56f, 1.8f, false, true);
    renderTorusInstance(shader, shadowPass, vec3(4.0f, 0.35f, 1.7f), vec3(0.92f, 0.92f, 0.90f),
        1.0f, 0.58f, 2.5f, false, false);

    model = glm::translate(mat4(1.0f), vec3(2.2f, -0.15f, -3.0f));
    model = glm::rotate(model, glm::radians(-18.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, vec3(1.15f));
    setMatrices(shader);
    if (!shadowPass) {
        setSurfaceUniforms(shader, false, false, false, 0.0f, 0.0f, 1.0f, vec3(0.28f, 0.48f, 0.74f),
            0.08f, 0.42f, 1.0f, vec3(0.0f), 0.0f, 0.0f, vec3(0.0f), vec3(0.0f));
    }
    modelMesh->render();

    renderHairObject(shader, shadowPass);
}

void SceneBasic_Uniform::renderParticles()
{
    particleProg.use();
    particleProg.setUniform("View", view);
    particleProg.setUniform("Projection", projection);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glBindVertexArray(particleVao);
    glDrawArrays(GL_POINTS, 0, ParticleCount);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void SceneBasic_Uniform::renderGround(GLSLProgram &shader, bool shadowPass)
{
    model = glm::translate(mat4(1.0f), vec3(0.0f, -1.15f, 0.0f));
    setMatrices(shader);

    if (!shadowPass) {
        setSurfaceUniforms(shader, true, false, false, 0.0f, 0.0f, 1.0f, vec3(0.78f, 0.72f, 0.62f),
            0.0f, 0.95f, 1.0f, vec3(0.0f), 0.0f, 0.0f, vec3(0.0f), vec3(0.0f));
    }

    ground->render();
}

void SceneBasic_Uniform::renderTorusInstance(GLSLProgram &shader, bool shadowPass, const vec3 &position,
    const vec3 &albedo, float metallic, float roughness, float spinOffset, bool useTexture, bool useNoiseMaterial)
{
    model = glm::translate(mat4(1.0f), position);
    model = glm::rotate(model, angle * 0.85f + spinOffset, vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(65.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices(shader);

    if (!shadowPass) {
        float noiseScale = useNoiseMaterial ? 3.4f : 0.0f;
        float noiseStrength = useNoiseMaterial ? 0.85f : 0.0f;
        vec3 noiseDarkColor = useNoiseMaterial ? vec3(0.28f, 0.13f, 0.05f) : vec3(0.0f);
        vec3 noiseLightColor = useNoiseMaterial ? vec3(0.82f, 0.62f, 0.22f) : vec3(0.0f);

        setSurfaceUniforms(shader, useTexture, false, useNoiseMaterial, 0.0f, 0.0f, 1.0f, albedo,
            metallic, roughness, 1.0f, vec3(0.0f), noiseScale, noiseStrength, noiseDarkColor, noiseLightColor);
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
        setSurfaceUniforms(shader, false, false, false, 0.0f, 0.0f, 1.0f, vec3(0.22f, 0.16f, 0.10f),
            0.0f, 0.86f, 1.0f, vec3(0.80f, 0.61f, 0.28f), 0.0f, 0.0f, vec3(0.0f), vec3(0.0f));
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
        setSurfaceUniforms(shader, false, true, false, shellOffset, float(layer), float(hairLayers),
            vec3(0.26f, 0.18f, 0.10f), 0.0f, 0.92f, 1.0f, vec3(0.92f, 0.74f, 0.36f), 0.0f, 0.0f,
            vec3(0.0f), vec3(0.0f));
        torus->render();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

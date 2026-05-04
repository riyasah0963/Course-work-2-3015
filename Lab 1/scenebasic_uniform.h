#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include "helper/objmesh.h"
#include "helper/plane.h"
#include "helper/torus.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class SceneBasic_Uniform : public Scene
{
private:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        float age;
        float life;
    };

    GLSLProgram prog;
    GLSLProgram shadowProg;
    GLSLProgram particleProg;
    std::unique_ptr<ObjMesh> modelMesh;
    Torus *torus;
    Plane *ground;
    float angle;
    float tPrev;
    float elapsedTime;
    GLuint checkerTex;
    GLuint shadowFbo;
    GLuint shadowTex;
    GLuint particleVao;
    GLuint particlePosVbo;
    GLuint particleLifeVbo;
    glm::mat4 lightSpaceMatrix;
    static const int ShadowMapSize = 2048;
    static const int ParticleCount = 96;
    std::vector<Particle> particles;
    std::vector<glm::vec3> particlePositions;
    std::vector<float> particleLifeRatios;

    void compile();
    void setMatrices(GLSLProgram &shader);
    void setLightUniforms(const glm::vec3 &cameraPosition, const glm::vec3 &directionalLightDir);
    void setSurfaceUniforms(GLSLProgram &shader, bool useTexture, bool useHairShell, bool useNoiseMaterial,
        float hairShellOffset, float hairLayer, float hairLayers, const glm::vec3 &albedo, float metallic,
        float roughness, float ao, const glm::vec3 &hairColor, float noiseScale, float noiseStrength,
        const glm::vec3 &noiseDarkColor, const glm::vec3 &noiseLightColor);
    void initTextures();
    void initShadowMap();
    void initParticles();
    void resetParticle(Particle &particle, int index, float cycleOffset);
    void updateParticles(float deltaT);
    void renderScene(GLSLProgram &shader, bool shadowPass);
    void renderParticles();
    void renderGround(GLSLProgram &shader, bool shadowPass);
    void renderTorusInstance(GLSLProgram &shader, bool shadowPass, const glm::vec3 &position,
        const glm::vec3 &albedo, float metallic, float roughness, float spinOffset, bool useTexture,
        bool useNoiseMaterial);
    void renderHairObject(GLSLProgram &shader, bool shadowPass);

public:
    SceneBasic_Uniform();
    ~SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H

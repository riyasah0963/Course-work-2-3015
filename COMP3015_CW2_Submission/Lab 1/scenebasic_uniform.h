#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include "helper/plane.h"
#include "helper/torus.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;
    GLSLProgram shadowProg;
    Torus *torus;
    Plane *ground;
    float angle;
    float tPrev;
    GLuint checkerTex;
    GLuint shadowFbo;
    GLuint shadowTex;
    glm::mat4 lightSpaceMatrix;
    static const int ShadowMapSize = 2048;

    void compile();
    void setMatrices(GLSLProgram &shader);
    void setLightUniforms(const glm::vec3 &cameraPosition, const glm::vec3 &directionalLightDir);
    void initTextures();
    void initShadowMap();
    void renderScene(GLSLProgram &shader, bool shadowPass);
    void renderGround(GLSLProgram &shader, bool shadowPass);
    void renderTorusInstance(GLSLProgram &shader, bool shadowPass, const glm::vec3 &position,
        const glm::vec3 &albedo, float metallic, float roughness, float spinOffset, bool useTexture);
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

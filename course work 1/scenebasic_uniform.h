#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "scene.h"
#include "glslprogram.h"
#include "helper/skybox.h"

#include <glm/glm.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;

    SkyBox* skybox;   // 🔥 Pointer (important!)

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    float angle;

    void setMatrices();

public:
    SceneBasic_Uniform();
    ~SceneBasic_Uniform();   // destructor

    void initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int, int) override;
};

#endif

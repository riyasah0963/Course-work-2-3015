#include "scenebasic_uniform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>

SceneBasic_Uniform::SceneBasic_Uniform()
    : angle(0.0f), skybox(nullptr)
{
}

SceneBasic_Uniform::~SceneBasic_Uniform()
{
    delete skybox;
}

void SceneBasic_Uniform::initScene()
{
    try {
        prog.compileShader("shaders/basic_uniform.vert");
        prog.compileShader("shaders/basic_uniform.frag");
        prog.link();
        prog.use();
    }
    catch (GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);

    // 🔥 Create SkyBox AFTER OpenGL context exists
    skybox = new SkyBox();
}

void SceneBasic_Uniform::update(float t)
{
    angle += 0.5f * t;
}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.use();

    view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    model = glm::mat4(1.0f);

    setMatrices();

    prog.setUniform("skybox", 0);

    if (skybox)
        skybox->render();
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);

    projection = glm::perspective(
        glm::radians(60.0f),
        (float)w / h,
        0.1f,
        100.0f
    );
}

void SceneBasic_Uniform::setMatrices()
{
    prog.setUniform("Model", model);
    prog.setUniform("View", view);
    prog.setUniform("Projection", projection);
}

#include "scenerunner.h"
#include "scenebasic_uniform.h"

int main(int argc, char* argv[])
{
    SceneBasic_Uniform scene;
    SceneRunner runner("COMP3015 Lab 3" , 800, 600);
    runner.run(scene);
    return 0;
}
#include "ParticleDemo3D.h"
#include <glut.h>

static BaseParticleDemo* gDemo = nullptr;

static void renderCallback()
{
    gDemo->step();
    gDemo->render();
}

int main(int argc, char** argv)
{
    // ここで2D / 3Dを切り替える！
    bool use2D = false;
    if (use2D) {
        //gDemo = new ParticleDemo2D();
    }
    else {
        gDemo = new ParticleDemo3D();
    }

    gDemo->init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow(use2D ? "PhysX 2D Demo" : "PhysX 3D Demo");

    setupDefaultRenderState();
    glutDisplayFunc(renderCallback);
    glutIdleFunc(renderCallback);
    glutMainLoop();

    delete gDemo;
    return 0;
}

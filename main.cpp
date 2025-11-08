#define GLEW_STATIC        // (静的リンクの場合のみ必要)
#include <GL/glew.h>       // ★ 必ず最初に！

#include <glut.h>
#include "ParticleDemo3D.h"
#include "ParticleDemo2d.h"

static BaseParticleDemo* gDemo = nullptr;

static void renderCallback()
{
    gDemo->step();
    gDemo->render();
}

int main(int argc, char** argv)
{
    // ここで2D / 3Dを切り替える！
    bool use2D = true;
    if (use2D) {
        gDemo = new ParticleDemo2D();
    }
    else {
        gDemo = new ParticleDemo3D();
    }


    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glutInitWindowSize(1280, 720);
    glutCreateWindow(use2D ? "PhysX 2D Demo" : "PhysX 3D Demo");

    // --- GLEW 初期化（これをやらないと glCreateShader が nullptr） ---
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "GLEW init failed\n");
        return -1;
    }

    gDemo->init();

    setupDefaultRenderState();
    glutDisplayFunc(renderCallback);
    glutIdleFunc(renderCallback);
    glutMainLoop();

    delete gDemo;
    return 0;
}

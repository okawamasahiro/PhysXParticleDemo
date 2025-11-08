#define GLEW_STATIC        // (静的リンクの場合のみ必要)
#include <GL/glew.h>       // ★ 必ず最初に！

#include <glut.h>
#include "ParticleDemo3D.h"
#include "ParticleDemo2d.h"

static BaseParticleDemo* gDemo = nullptr;

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        printf("Shader compile error: %s\n", log);
    }
    return shader;
}

GLuint createShaderProgram(const char* vsSource, const char* fsSource)
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        printf("Program link error: %s\n", log);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}
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

    gDemo->init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glutInitWindowSize(1280, 720);
    glutCreateWindow(use2D ? "PhysX 2D Demo" : "PhysX 3D Demo");

    setupDefaultRenderState();
    glutDisplayFunc(renderCallback);
    glutIdleFunc(renderCallback);
    glutMainLoop();

    delete gDemo;
    return 0;
}

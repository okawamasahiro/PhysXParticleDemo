#pragma once
#define GLEW_STATIC        // (静的リンクの場合のみ必要)
#include <GL/glew.h>       // ★ 必ず最初に！
#include "ParticleDemo.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

class ParticleDemo2D : public BaseParticleDemo {
public:
    // ファイルを文字列として読み込む
    std::string LoadShaderSource(const char* filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cout << "Current path: " << std::filesystem::current_path() << std::endl;
            std::cerr << "Failed to open shader file: " << filePath << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
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
    void init() override {
        gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
        gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, nullptr);

        PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
        sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);
        gDispatcher = PxDefaultCpuDispatcherCreate(2);
        sceneDesc.cpuDispatcher = gDispatcher;
        sceneDesc.filterShader = PxDefaultSimulationFilterShader;
        gScene = gPhysics->createScene(sceneDesc);

        gMaterial = gPhysics->createMaterial(0.4f, 0.4f, 0.6f);

        // 床を作る
        PxRigidStatic* floorBox = PxCreateStatic(
            *gPhysics,
            PxTransform(PxVec3(0, 0.0f, 0)),                // 床の位置
            PxBoxGeometry(2.5f, 0.2f, 2.5f),              // 幅X=5, 高さY=1, 奥行Z=5
            *gMaterial
        );
        gScene->addActor(*floorBox);

        // === ★ 固定パイプライン互換 GLSL シェーダ ===
        std::string vsrc = LoadShaderSource("./shader/vertex.glsl");
        std::string fsrc = LoadShaderSource("./shader/fragment.glsl");
        gFloorShader = createShaderProgram(vsrc.data(), fsrc.data());
    }

    void spawnParticle() {
        float x = (rand() % 100 / 100.0f - 0.5f) * 4.0f;
        float y = 3.0f + (rand() % 100) / 200.0f;

        PxRigidDynamic* sphere = PxCreateDynamic(
            *gPhysics,
            PxTransform(PxVec3(x, y, 0.0f)),   // Z=0固定
            PxSphereGeometry(0.15f),
            *gMaterial,
            0.2f
        );

        // Z軸方向を完全ロックして2D化
        sphere->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, true);
        sphere->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);
        sphere->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, true);

        gScene->addActor(*sphere);
        gParticles.push_back(sphere);

        if (gParticles.size() > MAX_PARTICLES) {
            PxRigidDynamic* old = gParticles.front();
            gScene->removeActor(*old);
            old->release();
            gParticles.erase(gParticles.begin());
        }
    }

    void step() override {
        static int frame = 0;
        if (frame++ % 6 == 0)
            spawnParticle();
        gScene->simulate(1.0f / 60.0f);
        gScene->fetchResults(true);
    }

    void render() override {
        PxVec3 eye(0, 5, 5);
        PxVec3 target(0, 0, 0);
        PxVec3 dir = (target - eye).getNormalized();
        // カメラ位置を近づけて注視点を中央に
        startRender(eye, dir);

        // === 床の可視化 ===
        //glPushMatrix();
        //glColor3f(0.4f, 0.4f, 0.45f); // 少し明るいグレー

        //glBegin(GL_QUADS);
        //glNormal3f(0, 1, 0);          // 上向き法線（PhysXの床と同じ）

        //const float size = 5.0f;     // 床の広さ（10m四方）
        //const float y = 0.0f;         // PhysXの床と同じ高さ
        //glVertex3f(-size, y, -size);
        //glVertex3f(-size, y, size);
        //glVertex3f(size, y, size);
        //glVertex3f(size, y, -size);
        //glEnd();

        //glPopMatrix();

        // === 物理オブジェクト描画 ===
        PxU32 dynamicActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC);
        std::vector<PxRigidActor*> actors(dynamicActors);
        if (dynamicActors)
        {
            gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC,
                reinterpret_cast<PxActor**>(actors.data()), dynamicActors);
        }
        PxU32 staticActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC);
        std::vector<PxRigidActor*> sActors(staticActors);
        if (staticActors)
        {
            gScene->getActors(PxActorTypeFlag::eRIGID_STATIC,
                reinterpret_cast<PxActor**>(sActors.data()), staticActors);
        }
        // 描画（PhysX SnippetRender）

        // パーティクル描画

        glUseProgram(gFloorShader); // ★ シェーダ有効
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);          // 上向き法線（PhysXの床と同じ）

        const float size = 2.5f;  // 床の広さ（10m四方）
        const float y = 0.0f;         // PhysXの床と同じ高さ
        glVertex3f(-size, y, -size);
        glVertex3f(-size, y, size);
        glVertex3f(size, y, size);
        glVertex3f(size, y, -size);
        glEnd();        // ★ 固定パイプラインに戻す
        
        glUseProgram(0);
        renderActors(actors.data(), dynamicActors, true, PxVec3(0.8f, 0.7f, 0.6f), nullptr);

        finishRender();
    }
};
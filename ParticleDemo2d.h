#pragma once
#define GLEW_STATIC        // (静的リンクの場合のみ必要)
#include <GL/glew.h>       // ★ 必ず最初に！
#include "ParticleDemo.h"
#include "Shader.h"

class ParticleDemo2D : public BaseParticleDemo {
public:
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
        //std::string vsrc = Shader::LoadShaderSource("./shader/vertex.glsl");
        //std::string fsrc = Shader::LoadShaderSource("./shader/fragment.glsl");

        const char* vertexSrc = R"(
#version 120
attribute float aRadius;  // 半径を受け取る

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_PointSize = aRadius * 300.0; // 半径から画面サイズへ変換
    gl_FrontColor = gl_Color;
}
        )";
        const char* fragmentSrc = R"(
#version 120

void main()
{
    // 点スプライト内のUV [-1, 1]
    vec2 uv = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(uv, uv);
    if (r2 > 1.0) discard; // 丸い粒だけ残す

    // ★ 単一色（例：水色系）
    vec3 color = vec3(0.3, 0.7, 1.0); // ← 好きな色に変更可

    float alpha = smoothstep(1.0, 0.0, r2) * 0.8;

    gl_FragColor = vec4(color, alpha);
}
        )";
        gShader = Shader::createShaderProgram(vertexSrc, fragmentSrc);
    }

    void spawnParticle() {
        //float x = (rand() % 100 / 100.0f - 0.5f) * 4.0f;
        //float y = 3.0f + (rand() % 100) / 200.0f;
        float x = (rand() % 100 / 100.0f - 0.5f) * 1.0f;  // ← 横方向を狭める
        float y = 3.0f + (rand() % 100) / 500.0f;         // ← 縦方向も狭める

        float r = 0.05f + (rand() % 100 / 100.0f) * 0.02f; // 0.05〜0.07
        PxRigidDynamic* sphere = PxCreateDynamic(
            *gPhysics,
            PxTransform(PxVec3(x, y, 0.0f)),   // Z=0固定
            PxSphereGeometry(0.04f),
            *gMaterial,
            0.01f
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
        ptcl.resize(gParticles.size());

    }

    void step() override {
        static int frame = 0;
        for (int i = 0; i < 3; ++i) // 1フレームで3つ生成
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
        

        for (PxU32 i = 0; i < dynamicActors; ++i) {
            ptcl[i].position = actors[i]->getGlobalPose().p;
            ptcl[i].radius = 0.05f; // ここでサイズを統一
        }
        // 描画（PhysX SnippetRender）

        // パーティクル描画

        glUseProgram(0); // ★ シェーダ有効
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);          // 上向き法線（PhysXの床と同じ）

        const float size = 2.5f;  // 床の広さ（10m四方）
        const float y = 0.0f;         // PhysXの床と同じ高さ
        glVertex3f(-size, y, -size);
        glVertex3f(-size, y, size);
        glVertex3f(size, y, size);
        glVertex3f(size, y, -size);
        glEnd();        // ★ 固定パイプラインに戻す
        
        // ②パーティクル（あなたの“流体っぽい”シェーダ）
        glDisable(GL_LIGHTING);                  // SnippetRenderの影響を避ける
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_POINT_SPRITE);              // GLSL 1.20でgl_PointCoordを使うなら有効化
        glEnable(GL_PROGRAM_POINT_SIZE_ARB);        // ARBじゃなくてこっちでOK

        glUseProgram(gShader);
        GLuint vbo;
        glGenBuffers(1, &vbo);          // VBOを作成して

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, ptcl.size() * sizeof(ParticleData), ptcl.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0); // 位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)0);

        glEnableVertexAttribArray(1); // 半径
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)offsetof(ParticleData, radius));


        // 実際に点群を描く！
        glDrawArrays(GL_POINTS, 0, (GLsizei)ptcl.size());

        // 後片付け
        glDisableClientState(GL_VERTEX_ARRAY);
        glUseProgram(0);
        glDisable(GL_POINT_SPRITE);

        //glUseProgram(gShader);
        //renderActors(actors.data(), dynamicActors, true, PxVec3(0.8f, 0.7f, 0.6f), nullptr);
        finishRender();
    }
    struct ParticleData {
        PxVec3 position;
        float radius;
    };
    std::vector<ParticleData> ptcl;

};
#include "PxPhysicsAPI.h"
#include "SnippetRender.h"
#include <vector>
#include <glut.h>

using namespace physx;
using namespace Snippets;

// === グローバル ===
static PxDefaultErrorCallback      gErrorCallback;
static PxDefaultAllocator          gAllocator;
static PxFoundation* gFoundation = nullptr;
static PxPhysics* gPhysics = nullptr;
static PxDefaultCpuDispatcher* gDispatcher = nullptr;
static PxScene* gScene = nullptr;
static PxMaterial* gMaterial = nullptr;

// === パーティクル ===
static std::vector<PxRigidDynamic*> gParticles;
static constexpr int MAX_PARTICLES = 150;

// =========================================================
// PhysX 初期化
// =========================================================
static void initPhysX()
{
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, nullptr);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    // === 床 ===
    PxRigidStatic* plane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    gScene->addActor(*plane);
}

// =========================================================
// パーティクル生成
// =========================================================
static void spawnParticle()
{
    float x = (rand() % 100 / 50.0f - 1.0f) * 1.0f;
    float y = 3.0f + (rand() % 100) / 150.0f;
    float z = (rand() % 100 / 50.0f) * 1.0f;

    PxRigidDynamic* sphere = PxCreateDynamic(
        *gPhysics,
        PxTransform(PxVec3(x, y, z)),
        PxSphereGeometry(0.5f), // 半径50cm
        *gMaterial,
        1.0f
    );

    sphere->setLinearDamping(0.1f);
    sphere->setAngularDamping(0.2f);

    sphere->setLinearVelocity(PxVec3(
        (rand() % 100 / 100.0f - 0.5f) * 2.0f,
        (rand() % 100 / 100.0f) * 3.0f,
        (rand() % 100 / 100.0f - 0.5f) * 2.0f
    ));

    gScene->addActor(*sphere);
    gParticles.push_back(sphere);

    //printf("Actor count = %u\n", gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC));

    // 古いパーティクルを削除
    if (gParticles.size() > MAX_PARTICLES)
    {
        PxRigidDynamic* old = gParticles.front();
        gScene->removeActor(*old);
        old->release();
        gParticles.erase(gParticles.begin());
    }
}

// =========================================================
// 更新
// =========================================================
static void stepPhysX()
{
    static int frameCount = 0;
    if (frameCount++ % 6 == 0)
        spawnParticle();

    gScene->simulate(1.0f / 60.0f);
    gScene->fetchResults(true);
}

// =========================================================
// 描画
// =========================================================
static void renderCallback()
{
    stepPhysX();

    // カメラ位置を近づけて注視点を中央に
    startRender(PxVec3(0, 2, 10), PxVec3(0, 0, 1), 0.1f, 50.0f);


    // === 床の可視化 ===
    glPushMatrix();
    glColor3f(0.4f, 0.4f, 0.45f); // 少し明るいグレー

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);          // 上向き法線（PhysXの床と同じ）

    const float size = 100.0f;     // 床の広さ（10m四方）
    const float y = 0.0f;         // PhysXの床と同じ高さ
    glVertex3f(-size, y, -size);
    glVertex3f(-size, y, size);
    glVertex3f(size, y, size);
    glVertex3f(size, y, -size);
    glEnd();

    glPopMatrix();

    // === 物理オブジェクト描画 ===
    PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
    std::vector<PxRigidActor*> actors(nbActors);
    if (nbActors)
        gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC,
            reinterpret_cast<PxActor**>(actors.data()), nbActors);

    // 描画（PhysX SnippetRender）
    renderActors(actors.data(), nbActors, true, PxVec3(0.8f, 0.7f, 0.6f), nullptr);

    finishRender();
}

// =========================================================
// メイン
// =========================================================
int main(int argc, char** argv)
{
    initPhysX();

    setupDefaultWindow("PhysX Particle Visible Demo");
    setupDefaultRenderState();
    glutDisplayFunc(renderCallback);
    glutIdleFunc(renderCallback);
    glutMainLoop();

    return 0;
}

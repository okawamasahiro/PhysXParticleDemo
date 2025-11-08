#pragma once
#include "ParticleDemo.h"

class ParticleDemo3D : public BaseParticleDemo {
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

        gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

        PxRigidStatic* plane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
        gScene->addActor(*plane);
    }

    void spawnParticle() {
        float x = (rand() % 100 / 50.0f - 1.0f) * 2.0f;
        float y = 3.0f + (rand() % 100) / 100.0f;
        float z = (rand() % 100 / 50.0f - 1.0f) * 2.0f;

        PxRigidDynamic* sphere = PxCreateDynamic(
            *gPhysics,
            PxTransform(PxVec3(x, y, z)),
            PxSphereGeometry(0.25f),
            *gMaterial,
            0.5f
        );
        sphere->setLinearDamping(0.1f);
        sphere->setAngularDamping(0.2f);
        // ★ 初期速度設定（これがないと遅い）
        sphere->setLinearVelocity(PxVec3(
            (rand() % 100 / 100.0f - 0.5f) * 8.0f,  // X: -4〜+4 m/s
            (rand() % 100 / 100.0f) * 10.0f,        // Y: 0〜+10 m/s
            (rand() % 100 / 100.0f) * 8.0f   // Z: 0〜+8 m/s
        ));

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
        startRender(PxVec3(0, 2, 10), PxVec3(0, 0, 1), 0.1f, 50.0f);
        PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC);
        std::vector<PxRigidActor*> actors(nbActors);
        if (nbActors)
            gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC,
                reinterpret_cast<PxActor**>(actors.data()), nbActors);
        renderActors(actors.data(), nbActors, true, PxVec3(0.8f, 0.7f, 0.6f), nullptr);
        finishRender();
    }
};
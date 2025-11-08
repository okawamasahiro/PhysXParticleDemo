#pragma once
#include "PxPhysicsAPI.h"
#include "SnippetRender.h"
#include <vector>

using namespace physx;
using namespace Snippets;

class BaseParticleDemo {
protected:
    PxDefaultErrorCallback gErrorCallback;
    PxDefaultAllocator gAllocator;
    PxFoundation* gFoundation = nullptr;
    PxPhysics* gPhysics = nullptr;
    PxDefaultCpuDispatcher* gDispatcher = nullptr;
    PxScene* gScene = nullptr;
    PxMaterial* gMaterial = nullptr;
    std::vector<PxRigidDynamic*> gParticles;
    GLuint gFloorShader = 0;

    static constexpr int MAX_PARTICLES = 200;

public:
    virtual ~BaseParticleDemo() {}
    virtual void init() = 0;
    virtual void step() = 0;
    virtual void render() = 0;
};

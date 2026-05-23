#include "gepch.h"
#include "Physics.h"
#include <glm/gtc/quaternion.hpp>

#include "GlassEngine/Scene/Entity.h"
#include "GlassEngine/Scene/Scene.h"

using namespace physx;

namespace ge::physics {
    PhysicsSystem::PhysicsSystem() {
        _foundation = PxCreateFoundation(PX_PHYSICS_VERSION, _allocator, _errorCallback);
        GE_ASSERT(_foundation, "PxCreateFoundation failed");

        _pvd = PxCreatePvd(*_foundation);
        PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        _pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

        _physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation, PxTolerancesScale(), true, _pvd);
        GE_ASSERT(_physics, "PxCreatePhysics failed");

        PxInitExtensions(*_physics, _pvd);

        _dispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency() - 1);

        PxSceneDesc sceneDesc(_physics->getTolerancesScale());
        sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
        sceneDesc.cpuDispatcher = _dispatcher;
        sceneDesc.filterShader = PxDefaultSimulationFilterShader;
        sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;

        _pxScene = _physics->createScene(sceneDesc);
        GE_ASSERT(_pxScene, "PxScene creation failed");

        PxPvdSceneClient* pvdClient = _pxScene->getScenePvdClient();
        if (pvdClient) {
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
    }

    PhysicsSystem::~PhysicsSystem() {
        if (_pxScene) { _pxScene->release(); _pxScene = nullptr; }
        if (_dispatcher) { _dispatcher->release(); _dispatcher = nullptr; }
        PxCloseExtensions();
        if (_physics) { _physics->release(); _physics = nullptr; }
        if (_pvd) { _pvd->release(); _pvd = nullptr; }
        if (_foundation) { _foundation->release(); _foundation = nullptr; }
    }

    void PhysicsSystem::Step(float deltaTime) {
        _pxScene->simulate(deltaTime);
        _pxScene->fetchResults(true);
        Scene_SyncECSTransformToPhysicsSystem();
    }

    void PhysicsSystem::Scene_SetEffectedScene(Scene* scene) {
        _effectedScene = scene;
    }

    void PhysicsSystem::Scene_SyncECSTransformToPhysicsSystem() {
        if (!_effectedScene) return;

        auto& registry = _effectedScene->GetRegistry();
        auto view = registry.view<RigidBodyComponent, TransformComponent>();

        for (auto [handle, rbc, tc] : view.each()) {
            auto& rb = registry.get<RigidBodyComponent>(handle);
            auto actor = _cachedActors.at(rb.actorID);

            if (!rb.actorID && !actor) continue;
            if (rb.bodyType == RigidBodyType::Static) continue;

            PxRigidDynamic* dynamic = actor->is<PxRigidDynamic>();
            if (!dynamic) continue;
            if (dynamic->isSleeping()) continue;

            PxTransform t = dynamic->getGlobalPose();
            auto& transform = registry.get<TransformComponent>(handle);

            transform.position = math::PxVec3ToGLMVec3(t.p);
            transform.rotation = math::PxQuatToGLMEuler(t.q);
        }
    }

    void PhysicsSystem::Scene_DropAllBodiesFromEffectedScene() {
        if (!_effectedScene) return;

        PxActorTypeFlags flags = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;

        PxU32 count = _pxScene->getNbActors(flags);
        if (count == 0) return;

        std::vector<PxActor*> actors(count);
        _pxScene->getActors(flags, actors.data(), count);

        for (PxActor* actor : actors)
            _pxScene->removeActor(*actor);
    }

    static physx::PxShape* CreateShapeForEntity(Entity* entity, physx::PxPhysics* physics, physx::PxMaterial* material) {
        using namespace physx;
        if (entity->HasComponent<BoxColliderComponent>()) {
            auto& col = entity->GetComponent<BoxColliderComponent>();
            return physics->createShape(
                PxBoxGeometry(col.halfExtents.x, col.halfExtents.y, col.halfExtents.z),
                *material);
        }

        if (entity->HasComponent<SphereColliderComponent>()) {
            auto& col = entity->GetComponent<SphereColliderComponent>();
            return physics->createShape(PxSphereGeometry(col.radius), *material);
        }

        if (entity->HasComponent<CapsuleColliderComponent>()) {
            auto& col = entity->GetComponent<CapsuleColliderComponent>();
            return physics->createShape(
                PxCapsuleGeometry(col.radius, col.halfHeight),
                *material);
        }

        return physics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *material);
    }

    void PhysicsSystem::CreateBody(Entity* entity) {
        using namespace physx;

        if (!entity) return;
        if (!entity->HasComponent<RigidBodyComponent>()) return;

        auto& rb = entity->GetComponent<RigidBodyComponent>();
        auto& transform = entity->GetComponent<TransformComponent>();

        PxTransform pxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z), math::GLMEulerToPxQuat(transform.rotation));
        PxMaterial* material = _physics->createMaterial(0.5f, 0.5f, 0.6f);

        PxShape* shape = CreateShapeForEntity(entity, _physics, material);

        PxRigidActor* actor = nullptr;

        if (rb.bodyType == RigidBodyType::Static) {
            actor = _physics->createRigidStatic(pxTransform);
        }
        else {
            PxRigidDynamic* dynamic = _physics->createRigidDynamic(pxTransform);
            dynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !rb.useGravity);
            PxRigidBodyExt::updateMassAndInertia(*dynamic, rb.mass);

            dynamic->setLinearDamping(rb.linearDamping);
            dynamic->setAngularDamping(rb.angularDamping);
            dynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rb.lockAngularX);
            dynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rb.lockAngularY);
            dynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rb.lockAngularZ);

            actor = dynamic;
        }

        actor->attachShape(*shape);
        shape->release();

        EntityID id = entity->GetComponent<IdentityComponent>().id;
        actor->userData = reinterpret_cast<void*>(static_cast<uint64_t>(id));

        PhysicsActorID actorID = PhysicsActorID();
        _cachedActors[actorID] = actor;
        rb.actorID = actorID;
        _pxScene->addActor(*actor);

        if (rb.bodyType == RigidBodyType::Dynamic) {
            PxRigidDynamic* dynamic = actor->is<PxRigidDynamic>();
            if (dynamic) dynamic->wakeUp();
        }

        material->release();
    }

    void PhysicsSystem::DestroyBody(Entity* entity) {
        if (!entity) return;
        if (!entity->HasComponent<RigidBodyComponent>()) return;

        auto& rb = entity->GetComponent<RigidBodyComponent>();
        if (!_cachedActors.contains(rb.actorID)) return;

        auto body = _cachedActors.at(rb.actorID);
        _pxScene->removeActor(*body);
        body->release();
        body = nullptr;
        rb.actorID = 0;
    }
}
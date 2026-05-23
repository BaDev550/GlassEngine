#pragma once

#include <PxPhysicsAPI.h>
#include "GlassEngine/Utilities/UUID.h"

namespace ge { class Scene; class Entity; }


namespace ge::physics {
    using PhysicsActorID = UUID;

    class PhysicsSystem final {
    public:
        PhysicsSystem();
        ~PhysicsSystem();

        void Step(float deltaTime);

        void CreateBody(Entity* entity);
        void DestroyBody(Entity* entity);

        void Scene_SetEffectedScene(Scene* scene);
        void Scene_SyncECSTransformToPhysicsSystem();
        void Scene_DropAllBodiesFromEffectedScene();

        inline physx::PxScene* GetPxScene() { return _pxScene; }
        inline physx::PxPhysics* GetPxPhysics() { return _physics; }
    private:
        physx::PxDefaultAllocator      _allocator;
        physx::PxDefaultErrorCallback  _errorCallback;
        physx::PxFoundation* _foundation = nullptr;
        physx::PxPhysics* _physics = nullptr;
        physx::PxScene* _pxScene = nullptr;
        physx::PxDefaultCpuDispatcher* _dispatcher = nullptr;
        physx::PxPvd* _pvd = nullptr;

        GEUnorderedMap<uint64_t, physx::PxRigidActor*> _cachedActors;

        Scene* _effectedScene = nullptr;
    };
}
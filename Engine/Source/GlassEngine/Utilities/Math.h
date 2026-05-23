#pragma once
#include <assimp/types.h>
#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ge::math {
    inline glm::vec3 PxQuatToGLMEuler(const physx::PxQuat& q) noexcept {
        glm::quat glmQ(q.w, q.x, q.y, q.z);
        return glm::degrees(glm::eulerAngles(glmQ));
    }

    inline physx::PxQuat GLMEulerToPxQuat(const glm::vec3& eulerDegrees) noexcept {
        glm::quat glmQ = glm::quat(glm::radians(eulerDegrees));
        return physx::PxQuat(glmQ.x, glmQ.y, glmQ.z, glmQ.w);
    }

    inline glm::vec3 PxVec3ToGLMVec3(const physx::PxVec3& v) { return { v.x, v.y, v.z }; }
    inline glm::vec3 AssimpVec3ToGLMVec3(const aiVector3D& v) { return { v.x, v.y, v.z }; }
    inline glm::vec2 AssimpVec2ToGLMVec2(const aiVector2D& v) { return { v.x, v.y }; }
    inline physx::PxVec3 GLMVec3ToPxVec3(const glm::vec3& v) { return physx::PxVec3(v.x, v.y, v.z); }
    inline aiVector3D GLMVec3ToAssimpVec3(const glm::vec3& v) { return aiVector3D(v.x, v.y, v.z); }
    inline aiVector2D GLMVec2ToAssimpVec2(const glm::vec2& v) { return aiVector2D(v.x, v.y); }
}
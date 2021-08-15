#include "Aurora.h"
#include "RigidBody.h"

namespace Aurora
{
    RigidBody::RigidBody(EngineContext* engineContext, Entity* entity, uint32_t componentID) : IComponent(engineContext, entity, componentID)
    {

    }

    RigidBody::~RigidBody()
    {

    }

    std::string RigidBody::GetCollisionShapeToString() const
    {
        switch (m_CollisionShape)
        {
            case CollisionShape::CollisionShape_Box:
                return "Box";

            case CollisionShape::CollisionShape_Capsule:
                return "Capsule";

            case CollisionShape::CollisionShape_Sphere:
                return "Sphere";

            case CollisionShape::CollisionShape_ConvexHull:
                return "Convex Hull";

            case CollisionShape::CollisionShape_TriangleMesh:
                return "Triangle Mesh";

            case CollisionShape::CollisionShape_Empty:
                return "Empty";
        }

        return "Null Shape";
    }
}
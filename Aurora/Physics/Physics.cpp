#include "Aurora.h"
#include "Physics.h"
#include "../Scene/World.h"

namespace Aurora
{
    std::vector<btVector3> g_Collisions;

    Physics::Physics(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    Physics::~Physics()
    {

    }

    void TickCallback(btDynamicsWorld* dynamicsWorld, btScalar timeStep)
    {
        g_Collisions.clear();

        int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
        for (int i = 0; i < numManifolds; i++) 
        {
            btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
            {
                const btCollisionObject* objA = contactManifold->getBody0();
                const btCollisionObject* objB = contactManifold->getBody1();
            }
            int numContacts = contactManifold->getNumContacts();
            for (int j = 0; j < numContacts; j++) 
            {
                btManifoldPoint& pt = contactManifold->getContactPoint(j);
                const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& ptB = pt.getPositionWorldOnB();
                const btVector3& normalOnB = pt.m_normalWorldOnB;
                g_Collisions.push_back(ptA);
                g_Collisions.push_back(ptB);
                g_Collisions.push_back(normalOnB);
            }
        }
    }

    bool Physics::Initialize()
    {        
        m_CollisionDispatcher = std::make_unique<btCollisionDispatcher>(&m_CollisionConfiguration);
        m_PhysicsWorld = std::make_unique<btDiscreteDynamicsWorld>(m_CollisionDispatcher.get(), &m_OverlappingPairCache, &m_Solver, &m_CollisionConfiguration);
        m_PhysicsWorld->setInternalTickCallback(TickCallback);

        m_PhysicsWorld->setGravity(btVector3(0, -10, 0));

        AURORA_INFO(LogLayer::Physics, "Initialized Physics Engine.");
        return true;
    }

    void Physics::Tick(float deltaTime)
    {
        if (!GetPhysicsEnabled() || deltaTime <= 0)
        {
            return;
        }

        Stopwatch physicsProfiler("Physics");

        // Our system will register rigidbodies to objects and update the physics engine state for kinematics.
        std::vector<std::shared_ptr<Entity>> sceneEntities = m_EngineContext->GetSubsystem<World>()->EntityGetAll();
        std::vector<std::shared_ptr<Entity>> rigidbodyEntities;
        for (auto& entity : sceneEntities)
        {
            if (entity->HasComponent<RigidBody>())
            {
                rigidbodyEntities.push_back(entity);
            }
        }

        for (uint32_t i = 0; i < static_cast<uint32_t>(rigidbodyEntities.size()); i++)
        {
            RigidBody* rigidBodyComponent = rigidbodyEntities[i]->GetComponent<RigidBody>();
            Entity* entity = rigidbodyEntities[i].get();

            if (rigidBodyComponent->m_PhysicsInternal == nullptr)
            {
                Transform* transformComponent = entity->GetComponent<Transform>();
                Mesh* meshComponent = entity->GetComponent<Mesh>();

                AddRigidBody(entity, rigidBodyComponent, transformComponent, meshComponent);
            }

            if (rigidBodyComponent->m_PhysicsInternal != nullptr)
            {
                btRigidBody* rigidBody = (btRigidBody*)rigidBodyComponent->m_PhysicsInternal;

                int activationState = rigidBody->getActivationState();
                if (rigidBodyComponent->GetDeactivationState())
                {
                    activationState |= DISABLE_DEACTIVATION;
                }
                else
                {
                    activationState &= ~DISABLE_DEACTIVATION;
                }
                rigidBody->setActivationState(activationState);

                rigidBody->setDamping(rigidBodyComponent->GetDamplingLinear(), rigidBodyComponent->GetDamplingAngular());
                rigidBody->setFriction(rigidBodyComponent->GetFriction());
                rigidBody->setRestitution(rigidBodyComponent->GetRestitution());

                // For kinematic objects, system updates physics state, else the physics updates system state.
                if (rigidBodyComponent->GetKinematicState() || !GetPhysicsSimulationEnabled())
                {
                    Transform* transformComponent = entity->GetComponent<Transform>();
                    btMotionState* motionState = rigidBody->getMotionState();
                    btTransform physicsTransform;

                    XMFLOAT3 position = transformComponent->GetPosition();
                    XMFLOAT4 rotation = transformComponent->GetRotation();

                    btVector3 T(position.x, position.y, position.z);
                    btQuaternion R(rotation.x, rotation.y, rotation.z, rotation.w);

                    physicsTransform.setOrigin(T);
                    physicsTransform.setRotation(R);
                    
                    motionState->setWorldTransform(physicsTransform);

                    if (!GetPhysicsSimulationEnabled())
                    {
                        // This is a more direct way of manipulating rigid body.
                        rigidBody->setWorldTransform(physicsTransform);
                    }

                    btCollisionShape* shape = rigidBody->getCollisionShape();
                    XMFLOAT3 scale = transformComponent->GetScale();
                    btVector3 S(scale.x, scale.y, scale.z);
                    shape->setLocalScaling(S);
                }
            }
        }

        // Perform internal simulation step.
        if (GetPhysicsSimulationEnabled())
        {
            m_PhysicsWorld->stepSimulation(deltaTime, m_Accuracy);
        }

        // Feedback physics engine state to system.
        for (int i = 0; i < m_PhysicsWorld->getCollisionObjectArray().size(); ++i)
        {
            btCollisionObject* collisionObject = m_PhysicsWorld->getCollisionObjectArray()[i];
            Entity* entity = m_EngineContext->GetSubsystem<World>()->GetEntityByID(collisionObject->getUserIndex()).get();
            btRigidBody* rigidBody = btRigidBody::upcast(collisionObject);

            if (rigidBody != nullptr)
            {
                RigidBody* rigidBodyComponent = entity->GetComponent<RigidBody>();
                if (rigidBodyComponent == nullptr || rigidBodyComponent->m_PhysicsInternal != rigidBody)
                {
                    m_PhysicsWorld->removeRigidBody(rigidBody);
                    i--;
                    continue;
                }

                if (g_Collisions.empty()) 
                {
                    AURORA_INFO(LogLayer::Physics, "No Collisions!");
                }
                else 
                {
                    AURORA_INFO(LogLayer::Physics, "Collision Detected!");
                }

                // Feedback non-kinematic objects to the system.
                if (GetPhysicsSimulationEnabled() && !rigidBodyComponent->GetKinematicState())
                {
                    Transform* transformComponent = entity->GetComponent<Transform>();

                    btMotionState* motionState = rigidBody->getMotionState();
                    btTransform physicsTransform;

                    motionState->getWorldTransform(physicsTransform);
                    btVector3 T = physicsTransform.getOrigin();
                    btQuaternion R = physicsTransform.getRotation();

                    transformComponent->m_TranslationLocal = XMFLOAT3(T.x(), T.y(), T.z());
                    transformComponent->m_RotationLocal = XMFLOAT4(R.x(), R.y(), R.z(), R.w());
                    transformComponent->SetDirty();
                }
            }
        }
    }

    void Physics::AddRigidBody(Entity* entity, RigidBody* rigidBodyComponent, const Transform* transformComponent, const Mesh* meshComponent)
    {
        btCollisionShape* shape = nullptr;

        switch (rigidBodyComponent->GetCollisionShape())
        {
            case CollisionShape::CollisionShape_Box:
                shape = new btBoxShape(btVector3(rigidBodyComponent->m_BoxParameters.m_HalfExtents.x, rigidBodyComponent->m_BoxParameters.m_HalfExtents.y, rigidBodyComponent->m_BoxParameters.m_HalfExtents.z));
                break;

            case CollisionShape::CollisionShape_Sphere:
                shape = new btSphereShape(btScalar(rigidBodyComponent->m_SphereParameters.m_Radius));
                break;

            case CollisionShape::CollisionShape_Capsule:
                shape = new btCapsuleShape(btScalar(rigidBodyComponent->m_CapsuleParameters.m_Radius), btScalar(rigidBodyComponent->m_CapsuleParameters.m_Height));
                break;

            case CollisionShape::CollisionShape_ConvexHull:
                if (meshComponent != nullptr)
                {
                    shape = new btConvexHullShape();
                    for (auto& position : meshComponent->m_VertexPositions)
                    {
                        ((btConvexHullShape*)shape)->addPoint(btVector3(position.x, position.y, position.z));
                    }
                    btVector3 S(transformComponent->m_ScaleLocal.x, transformComponent->m_ScaleLocal.y, transformComponent->m_ScaleLocal.z);
                    shape->setLocalScaling(S);
                }
                else
                {
                    AURORA_ERROR(LogLayer::Physics, "Convex Hull Collider requested, but no MeshComponent was provided.");
                }

                break;

            case CollisionShape::CollisionShape_TriangleMesh:
                // AURORA_WARNING(LogLayer::Physics, "TriangleMesh collider currently not supported. No detections will take place");
                break;

            case CollisionShape::CollisionShape_Empty:
                // AURORA_WARNING(LogLayer::Physics, "RigidBody collider currently set as Empty. No detections will take place");
                break;
        }

        if (shape == nullptr)
        {
            return;
        }

        btScalar objectMass = rigidBodyComponent->GetMass();
        bool isDynamic = (objectMass != 0.0f && !rigidBodyComponent->GetKinematicState()); // If an object is Kinematic, it will not be driven by the physics engine, and can only be manipulated by its Transform.
        
        btVector3 localInertia(0.0f, 0.0f, 0.0f); // Inertia is the resistance of any agency that attempts to put the object in motion, or if its moving, to change the magnitude or direction of its velocity.
        if (isDynamic)
        {
            shape->calculateLocalInertia(objectMass, localInertia);
        }
        else
        {
            objectMass = 0.0f;
        }

        // Using motion state is recommended. It provides interpolation capabilities, and only synchronizes "active" objects. 
        btTransform shapeTransform;
        shapeTransform.setIdentity();
        shapeTransform.setOrigin(btVector3(transformComponent->m_TranslationLocal.x, transformComponent->m_TranslationLocal.y, transformComponent->m_TranslationLocal.z));
        shapeTransform.setRotation(btQuaternion(transformComponent->m_RotationLocal.x, transformComponent->m_RotationLocal.y, transformComponent->m_RotationLocal.z, transformComponent->m_RotationLocal.w));
        btDefaultMotionState* motionState = new btDefaultMotionState(shapeTransform);

        btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo(objectMass, motionState, shape, localInertia);

        btRigidBody* rigidBody = new btRigidBody(rigidBodyInfo);
        rigidBody->setUserIndex(entity->GetObjectID());

        if (rigidBodyComponent->GetKinematicState())
        {
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        }

        if (rigidBodyComponent->GetDeactivationState())
        {
            rigidBody->setActivationState(DISABLE_DEACTIVATION);
        }
        
        m_PhysicsWorld->addRigidBody(rigidBody);
        rigidBodyComponent->m_PhysicsInternal = rigidBody;
    }

    void Physics::ApplyForce(const RigidBody* rigidBodyComponent, const XMFLOAT3& force)
    {
        if (rigidBodyComponent->m_PhysicsInternal != nullptr)
        {
            btRigidBody* rigidBody = (btRigidBody*)rigidBodyComponent->m_PhysicsInternal;
            rigidBody->applyCentralForce(btVector3(force.x, force.y, force.z));
        }
    }

    void Physics::ApplyForceAt(const RigidBody* rigidBodyComponent, const XMFLOAT3& force, const XMFLOAT3& atPosition)
    {
        if (rigidBodyComponent->m_PhysicsInternal != nullptr)
        {
            btRigidBody* rigidBody = (btRigidBody*)rigidBodyComponent->m_PhysicsInternal;
            rigidBody->applyForce(btVector3(force.x, force.y, force.x), btVector3(atPosition.x, atPosition.y, atPosition.z));
        }
    }

    void Physics::ApplyImpulse(const RigidBody* rigidBodyComponent, const XMFLOAT3& impulse)
    {
        if (rigidBodyComponent->m_PhysicsInternal != nullptr)
        {
            btRigidBody* rigidBody = (btRigidBody*)rigidBodyComponent->m_PhysicsInternal;
            rigidBody->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
        }
    }

    void Physics::ApplyImpulseAt(const RigidBody* rigidBodyComponent, const XMFLOAT3& impulse, const XMFLOAT3& atPosition)
    {
        if (rigidBodyComponent->m_PhysicsInternal != nullptr)
        {
            btRigidBody* rigidBody = (btRigidBody*)rigidBodyComponent->m_PhysicsInternal;
            rigidBody->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z), btVector3(atPosition.x, atPosition.y, atPosition.z));
        }
    }
}
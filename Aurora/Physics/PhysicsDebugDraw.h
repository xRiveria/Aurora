#pragma once

// Hide warnings that belong to Bullet.
#pragma warning(push, 0)   
#include <Bullet/LinearMath/btIDebugDraw.h>
#pragma warning(pop)

namespace Aurora
{
    class Renderer;

    class PhysicsDebugDraw : public btIDebugDraw
    {
    public:
        PhysicsDebugDraw(Renderer* renderer);
        ~PhysicsDebugDraw() = default;

        // === btIDebugDraw ===
        void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor) override;
        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
        void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
        void reportErrorWarning(const char* errorWarning) override;
        void draw3dText(const btVector3& location, const char* textString) override {}
        void setDebugMode(const int debugMode) override { m_DebugMode = debugMode; }
        int getDebugMode() const override { return m_DebugMode; }

    private:
        Renderer* m_Renderer;
        int m_DebugMode;
    };
}

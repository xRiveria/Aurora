#pragma once
#include "BoundingBox.h"

namespace Aurora
{
    class Ray
    {
    public:
        Ray() = default;
        Ray(const Vector3& rayStart, const Vector3& rayEnd);
        ~Ray() = default;

        // Returns hit distance or infinitely if there is no hit.
        float HitDistance(const BoundingBox& boundingBox) const;

        const auto& GetStart() const { return m_Start; }
        const auto& GetEnd() const { return m_End; }
        const auto& GetLength() const { return m_Length; }
        const auto& GetDirection() const { return m_Direction; }
        
    private:
        Vector3 m_Start;
        Vector3 m_End;
        Vector3 m_Direction;
        float m_Length = 0.0f;
    };
}
#include "Aurora.h"
#include "Ray.h"

namespace Aurora
{
    Ray::Ray(const Vector3& rayStart, const Vector3& rayEnd)
    {
        m_Start = rayStart;
        m_End = rayEnd;
        const Vector3 startToEnd = (rayEnd - rayStart);
        m_Length = startToEnd.Length();
        m_Direction = startToEnd.Normalized();
    }

    float Ray::HitDistance(const BoundingBox& boundingBox) const
    {
        // If undefined, no hit (infinite distance).
        if (!boundingBox.Defined())
        {
            return Helper::Infinity;
        }

        // Check for ray origin being inside the box.
        if (boundingBox.IsInside(m_Start))
        {
            return 0.0f;
        }

        float distance = Helper::Infinity;

        // Check for intersection in the X direction.
        if (m_Start.x < boundingBox.GetMinimum().x && m_Direction.x > 0.0f) 
        {
            const float x = (boundingBox.GetMinimum().x - m_Start.x) / m_Direction.x;
            if (x < distance)
            {
                const Vector3 point = m_Start + x * m_Direction;
                if (point.y >= boundingBox.GetMinimum().y && point.y <= boundingBox.GetMaximum().y && point.z >= boundingBox.GetMinimum().z && point.z <= boundingBox.GetMaximum().z)
                {
                    distance = x;
                }
            }
        }

        if (m_Start.x > boundingBox.GetMaximum().x && m_Direction.x < 0.0f)
        {

        }

        // Check for intersection in the Y direction.

        // Check for intersection in the Z direction.


        return distance;
    }
}
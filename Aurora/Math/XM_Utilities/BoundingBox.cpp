#include "Aurora.h"
#include "BoundingBox.h"

namespace Aurora
{
    BoundingBox::BoundingBox()
    {
        m_Minimum = Vector3::Infinity;
        m_Maximum = Vector3::InfinityNegative;
    }

    BoundingBox::BoundingBox(const Vector3& minimum, const Vector3& maximum)
    {
        this->m_Minimum = minimum;
        this->m_Maximum = maximum;
    }

    Intersection BoundingBox::IsInside(const Vector3& point) const
    {
        if (point.x < m_Minimum.x || point.x > m_Maximum.x ||
            point.y < m_Minimum.y || point.y > m_Maximum.y ||
            point.z < m_Minimum.z || point.z > m_Maximum.z)
        {
            return Intersection::Outside;
        }
        else
        {
            return Intersection::Inside;
        }
    }

    Intersection BoundingBox::IsInside(const BoundingBox& boundingBox) const
    {
        if (boundingBox.m_Maximum.x < m_Minimum.x || boundingBox.m_Minimum.x > m_Maximum.x ||
            boundingBox.m_Maximum.y < m_Minimum.y || boundingBox.m_Minimum.y > m_Maximum.y ||
            boundingBox.m_Maximum.z < m_Minimum.z || boundingBox.m_Minimum.z > m_Maximum.z)
        {
            return Intersection::Outside;
        }
        else if (
            boundingBox.m_Minimum.x < m_Minimum.x || boundingBox.m_Maximum.x > m_Maximum.x ||
            boundingBox.m_Minimum.y < m_Minimum.y || boundingBox.m_Maximum.y > m_Maximum.y ||
            boundingBox.m_Minimum.z < m_Minimum.z || boundingBox.m_Maximum.z > m_Maximum.z)
        {
            return Intersection::Intersects;
        }
        else
        {
            return Intersection::Inside;
        }
    }

    BoundingBox BoundingBox::Transform(const XMMATRIX& transform) const
    {
        return BoundingBox();
    }


}
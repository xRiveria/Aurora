#pragma once
#include <limits>
#include "../Vector3.h"

using namespace Aurora::Math;

namespace Aurora
{
    enum Intersection
    {
        Outside,
        Inside,
        Intersects
    };

    class BoundingBox
    {
    public:
        // Construct with zero size.
        BoundingBox();

        // Construct from minimum and maximum vectors.
        BoundingBox(const Vector3& minimum, const Vector3& maximum);

        ~BoundingBox() = default;

        // Assign from bounding box;
        BoundingBox& operator =(const BoundingBox& otherBoundingBox) = default;

        // Returns the center.
        Vector3 GetCenter() const { return (m_Maximum + m_Minimum) * 0.5f; }

        // Returns the size.
        Vector3 GetSize() const { return m_Maximum - m_Minimum; }

        // Returns extents.
        Vector3 GetExtents() const { return (m_Maximum - m_Minimum) * 0.5f; }

        // Test if a point is inside the bounding box.
        Intersection IsInside(const Vector3& point) const;

        // Test if a bounding box is inside.
        Intersection IsInside(const BoundingBox& boundingBox) const;

        // Returns a transformed bounding box.
        BoundingBox Transform(const XMMATRIX& transform) const;

        // ====================================================================================

        const Vector3& GetMinimum() const { return m_Minimum; }
        const Vector3& GetMaximum() const { return m_Maximum; }

        void Undefine() 
        {
            m_Minimum = Vector3::InfinityNegative; 
            m_Maximum = Vector3::Infinity; 
        }

        bool Defined() const { return m_Minimum.x != INFINITY; }

        static const BoundingBox Zero;

    private:
        Vector3 m_Minimum;
        Vector3 m_Maximum;
    };
}
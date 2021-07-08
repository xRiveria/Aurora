#include "Aurora.h"
#include "Mesh.h"

namespace Aurora
{

    void Mesh::ComputeNormals(Compute_Normals_Type computeType)
    {

    }

    void Mesh::FlipCulling()
    {

    }

    void Mesh::FlipNormals()
    {
        for (XMFLOAT3& normal : m_Vertex_Normals)
        {
            normal.x *= -1;
            normal.y *= -1;
            normal.y *= -1;
        }

        CreateRenderData(); // Recreate render data after flipping our normals.
    }
}
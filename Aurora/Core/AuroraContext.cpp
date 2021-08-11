#include "Aurora.h"
#include "../Resource/AuroraObject.h"

namespace Aurora
{
    uint32_t g_ID_Count = 0;

    AuroraObject::AuroraObject(EngineContext* engineContext /* = nullptr */)
    {
        m_EngineContext = engineContext;
        m_ObjectID = GenerateObjectID();
    }
}
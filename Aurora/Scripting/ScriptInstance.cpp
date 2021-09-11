#include "Aurora.h"
#include "ScriptInstance.h"
#include "Scripting.h"

namespace Aurora
{
    void ScriptClassData::InitializeClassMethods(MonoImage* monoImage)
    {
        m_Constructor = Scripting::GetMonoMethod(monoImage, m_FullReferralName + ":.ctor()");
        m_OnStartMethod = Scripting::GetMonoMethod(monoImage, m_FullReferralName + ":OnStart()");
        m_OnUpdateMethod = Scripting::GetMonoMethod(monoImage, m_FullReferralName + ":OnUpdate(single)");
    }
}
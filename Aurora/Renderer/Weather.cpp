#include "Aurora.h"
#include "Weather.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    Weather::Weather(EngineContext* engineContext)
    {
        m_EngineContext = engineContext;
        m_Renderer = m_EngineContext->GetSubsystem<Renderer>();
    }

    void Weather::SetPreset_Cloudy()
    {
        m_AmbientColor = XMFLOAT3(0.1f, 0.1f, 0.1f);
        m_HorizonColor = XMFLOAT3(0.38f, 0.38f, 0.38f);
        m_ZenithColor = XMFLOAT3(0.42f, 0.42f, 0.42f);
        m_Cloudiness = 0.75f;
        m_FogStart = 0;
        m_FogEnd = 500;
        m_FogHeight = 0;

        /// Invalidate environment probes to reflect sky changes.
    }
}

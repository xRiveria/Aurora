#include "Aurora.h"
#include "AuroraResource.h"
#include "../Graphics/DX11_Refactored/DX11_Texture.h"
#include "../Renderer/Model.h"
#include "../Renderer/Material.h"
#include "../Audio/AudioClip.h"

namespace Aurora
{
    AuroraResource::AuroraResource(EngineContext* engineContext, ResourceType resourceType)
    {
        m_EngineContext = engineContext;
        m_ResourceType = resourceType;
        m_LoadState = LoadState::LoadState_Idle;
    }

    template <typename T>
    inline constexpr ResourceType AuroraResource::TypeToEnum() { return ResourceType::ResourceType_Empty; }

    template <typename T>
    inline constexpr void ValidateResourceType() { static_assert(std::is_base_of<AuroraResource, T>::value, "Provided type does not implement AuroraResource."); }

    // Explicit template instantiation.
    #define INSTANTIATE_TO_RESOURCE_TYPE(T, enumT) template<> ResourceType AuroraResource::TypeToEnum<T>() { ValidateResourceType<T>(); return enumT; }

    // To add a new resource type to the engine, simply register it here.
    INSTANTIATE_TO_RESOURCE_TYPE(DX11_Texture, ResourceType::ResourceType_Image)
    INSTANTIATE_TO_RESOURCE_TYPE(Model, ResourceType::ResourceType_Model)
    INSTANTIATE_TO_RESOURCE_TYPE(Material, ResourceType::ResourceType_Material)
    INSTANTIATE_TO_RESOURCE_TYPE(AudioClip, ResourceType::ResourceType_Audio)
}
#pragma once
#include <string>
#include <memory>
#include "../Resource/AuroraResource.h"

namespace Aurora
{
    enum SpriteFlag
    {
        SpriteFlag_Empty            = 0,
        SpriteFlag_Hidden           = 1 << 0,
        SpriteFlag_DisableUpdate    = 1 << 1
    };

    class Sprite
    {
    public:
        Sprite(const std::string& newTexture = "", const std::string& newMask = "");

        virtual void FixedTick();
        virtual void Tick(float deltaTime);
        virtual void Draw() const;

        void SetHidden(bool value = true) { if (value) { m_Flags |= SpriteFlag::SpriteFlag_Hidden; } else { m_Flags &= ~SpriteFlag::SpriteFlag_DisableUpdate; } }
        bool GetIsHidden() const { return m_Flags & SpriteFlag::SpriteFlag_Hidden; }
        void SetDisableUpdate(bool value = true) { if (value) { m_Flags |= SpriteFlag::SpriteFlag_DisableUpdate; } else { m_Flags &= ~SpriteFlag::SpriteFlag_DisableUpdate; } }
        bool GetDisableUpdate() const { return m_Flags & SpriteFlag::SpriteFlag_DisableUpdate; }
        
    private:
        uint32_t m_Flags = SpriteFlag::SpriteFlag_Empty;
        std::string m_TextureName;
        std::string m_MaskName;
        // =============================
        std::shared_ptr<AuroraResource> m_TextureResource;
        std::shared_ptr<AuroraResource> m_MaskResource;
    };
}
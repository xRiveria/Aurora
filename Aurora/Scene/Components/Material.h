#pragma once
#include "IComponent.h"
#include "ShaderInternals.h"
#include "MaterialUtilities.h"

/*
    Properties that define a material, such as color, textures etc.
*/

namespace Aurora
{
    class Material : public IComponent
    {
    public:
        Material(EngineContext* engineContext, Entity* entity, uint32_t componentID = 0);

        void SetDirty(bool value = true) { if (value) { m_Flags |= Material_Flags::Material_Flag_Dirty; } else { m_Flags &= ~Material_Flags::Material_Flag_Dirty; } }
        bool IsDirty() const { return m_Flags & Material_Flags::Material_Flag_Dirty; }

        void SetCastShadow(bool value) { SetDirty(); if (value) { m_Flags |= Material_Flags::Material_Flag_Cast_Shadow; } else { m_Flags &= ~Material_Flags::Material_Flag_Cast_Shadow; } }
        void SetReceiveShadow(bool value) { SetDirty(); if (value) { m_Flags &= ~Material_Flags::Material_Flag_Disable_Receive_Shadow; } else { m_Flags |= Material_Flags::Material_Flag_Disable_Receive_Shadow; } }

        void SetBaseColor(const XMFLOAT4& value) { SetDirty(); m_BaseColor = value; }
        void SetSpecularColor(const XMFLOAT4& value) { SetDirty(); m_SpecularColor = value; }
        void SetEmissiveColor(const XMFLOAT4& value) { SetDirty(); m_EmissiveColor = value; }
        void SetRoughness(float value) { SetDirty(); m_Roughness = value; }
        void SetMetalness(float value) { SetDirty(); m_Metalness = value; }
        void SetEmissiveStrength(float value) { SetDirty(); m_EmissiveColor.w = value; }
        void SetNormalMapStrength(float value) { SetDirty(); m_NormalMapStrength = value; }
        void SetDisplacementMapping(float value) { SetDirty(); m_DisplacementMapping = value; }

        void SetOpacity(float value) { SetDirty(); m_BaseColor.w = value; }
        void SetUseVertexColors(bool value) { SetDirty(); if (value) { m_Flags |= Material_Flags::Material_Flag_Use_VertexColors; } else { m_Flags &= ~Material_Flags::Material_Flag_Use_VertexColors; } }
        void SetDoubleSided(bool value = true) { if (value) { m_Flags |= Material_Flags::Material_Flag_Double_Sided; } else { m_Flags &= ~Material_Flags::Material_Flag_Double_Sided; } }
        
        BlendMode_Types GetBlendMode() const { if (m_UserBlendMode == BlendMode_Types::BlendMode_Opaque && (GetRenderTypes() & Render_Types::Render_Type_Transparent)) { return BlendMode_Types::BlendMode_Alpha; } else { return m_UserBlendMode; } }
        bool IsCastingShadow() const { return m_Flags & Material_Flags::Material_Flag_Cast_Shadow; }
        bool IsReceivingShadow() const { return (m_Flags & Material_Flags::Material_Flag_Disable_Receive_Shadow) == 0; }
        bool IsDoubleSided() const { return m_Flags & Material_Flags::Material_Flag_Double_Sided; }
        bool IsUsingVertexColors() const { return m_Flags & Material_Flags::Material_Flag_Use_VertexColors; }
        float GetOpacity() const { return m_BaseColor.w; }
        float GetEmissiveStrength() const { return m_EmissiveColor.w; }

        uint32_t GetRenderTypes() const;  // Returns the bitwise OR of all the Render Type flags applicable to this material (opaque, transparent etc).

        void WriteShaderMaterial(ShaderMaterial* destination ) const; // Our Material Component will be written to ShaderMaterial, a struct that is optimized for GPU use.
        void WriteTextures(const RHI_GPU_Resource** destination, int count) const; // Retrieve the array of textures from the material.
        void CreateRenderData(); // Create constant buffer and texture resources for the GPU.

    public:
        BlendMode_Types m_UserBlendMode = BlendMode_Types::BlendMode_Opaque;

        XMFLOAT4 m_BaseColor = XMFLOAT4(1, 1, 1, 1);
        XMFLOAT4 m_SpecularColor = XMFLOAT4(1, 1, 1, 1);
        XMFLOAT4 m_EmissiveColor = XMFLOAT4(1, 1, 1, 0);

        float m_NormalMapStrength = 1.0f;   // "How 3D?" https://www.youtube.com/watch?v=aVmgWneIj24
        float m_Roughness = 0.2f;
        float m_Metalness = 0.0f;
        float m_DisplacementMapping = 0.0f; // https://www.youtube.com/watch?v=EWTEJZqGLK0

        TextureMap m_Textures[Texture_Slot_Count];

        /// Subsurface Scattering?
        /// Parallax Occlusion Mapping
        /// Displacement Mapping
        /// Refraction
        /// Reflectance
        /// Transmission
        /// Alpha Reference
        
        // Texture Animations
        /// TexCoord Multiplier 
        /// Texture Animation Direction
        /// Texture Animation Frame Rate
        /// Texture Animation Elapsed Time

        Material_Shader_Type m_ShadingType = Material_Shader_Type::Material_Shader_Type_PBR;
        uint32_t m_Flags = Material_Flags::Material_Flag_Cast_Shadow;

        RHI_GPU_Buffer m_ConstantBuffer; // Our material GPU constant buffer.
        mutable bool m_IsBufferDirty = false;
    };
}
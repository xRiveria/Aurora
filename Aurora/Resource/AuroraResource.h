#pragma once
#include "FileSystem.h"
#include "../Resource/AuroraObject.h"
#include "../Graphics/DX11_Refactored/DX11_VertexBuffer.h"
#include "../Graphics/DX11_Refactored/DX11_IndexBuffer.h"
#include "../Scene/Entity.h"
#include <memory>
#include <variant>

namespace Aurora
{
    class DX11_Texture;
    class DX11_MeshData;

    enum ResourceType
    {
        ResourceType_Empty,
        ResourceType_Image,
        ResourceType_Audio,
        ResourceType_Model,
        ResourceType_Material
    };

    enum class LoadState
    {
        LoadState_Idle,
        LoadState_Started,
        LoadState_Completed,
        LoadState_Failed
    };

    // Simply add additional resource layer to objects.
    class AuroraResource : public AuroraObject
    {
    public:
        AuroraResource(EngineContext* engineContext, ResourceType resourceType);
        virtual ~AuroraResource() = default;

        void SetResourceFilePath(const std::string& filePath)
        {
            const bool isNativeFile = FileSystem::IsEngineMaterialFile(filePath) || FileSystem::IsEngineModelFile(filePath) || FileSystem::IsEngineTextureFile(filePath);

            // If this is a native engine file, don't do a file check as no actual foreign material exists (it was created on the fly).
            if (!isNativeFile)
            {
                if (!FileSystem::IsFile(filePath))
                {
                    AURORA_ERROR(LogLayer::Engine, "The provided path \"%s\" is not valid.", filePath.c_str());
                    return;
                }
            }

            const std::string filePathRelative = FileSystem::GetRelativePath(filePath);

            // Foreign file.
            if (!FileSystem::IsEngineFile(filePath))
            {
                m_ResourceFilePathForeign = filePathRelative;

                if (FileSystem::IsSupportedImageFile(filePath) || FileSystem::IsSupportedAudioFile(filePath))
                {
                    m_ResourceFilePathNative = filePathRelative;
                }
                else
                {
                    m_ResourceFilePathNative = FileSystem::NativizeFilePath(filePathRelative);
                }
            }
            // Native File
            else
            {
                m_ResourceFilePathForeign.clear();
                m_ResourceFilePathNative = filePathRelative;
            }

            m_ResourceName = FileSystem::GetFileNameWithoutExtensionFromFilePath(filePathRelative);
            m_ResourceDirectory = FileSystem::GetDirectoryFromFilePath(filePathRelative);
        }

        void SetResourceType(ResourceType resourceType) { m_ResourceType = resourceType; }
        ResourceType GetResourceType() const { return m_ResourceType; }

        const char* GetResourceTypeInCString() const { return typeid(*this).name(); }
        const std::string& GetResourceFilePath() const { return m_ResourceFilePathForeign; }
        bool HasFilePathNative() const { return !m_ResourceFilePathNative.empty(); }
        const std::string& GetResourceFilePathNative() const { return m_ResourceFilePathNative; }
        const std::string& GetResourceName() const { return m_ResourceName; }
        const std::string& GetResourceDirectory() const { return m_ResourceDirectory; }

        // Misc
        LoadState GetLoadState() const { return m_LoadState; }

        // IO
        virtual bool SaveToFile(const std::string& filePath) { return true; }
        virtual bool LoadFromFile(const std::string& filePath) { return true; }

        // Type
        template<typename T>
        static constexpr ResourceType TypeToEnum();

    public:
        std::shared_ptr<DX11_Texture> m_Texture;
        Entity* m_Entity = nullptr; // Texture and Models involve entities. We store them here for reference.

    protected:
        ResourceType m_ResourceType = ResourceType::ResourceType_Empty;
        std::atomic<LoadState> m_LoadState = LoadState::LoadState_Idle;

    private:
        std::string m_ResourceName;
        std::string m_ResourceDirectory;
        std::string m_ResourceFilePathNative;
        std::string m_ResourceFilePathForeign;
    };
}
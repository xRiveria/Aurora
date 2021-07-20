#include "IconLibrary.h"
#include "EngineContext.h"
#include "FileSystem.h"
#include "../Editor.h"
#include "../Resource/ResourceCache.h"

static Icon g_NoIcon;

IconLibrary::IconLibrary()
{
    m_EngineContext = nullptr;
    m_EditorContext = nullptr;
}

IconLibrary::~IconLibrary()
{
    m_Icons.clear();
}

void IconLibrary::Initialize(Aurora::EngineContext* engineContext, Editor* editorContext)
{
    m_EngineContext = engineContext;
    m_EditorContext = editorContext;
    const std::string resourceDirectory = m_EngineContext->GetSubsystem<Aurora::ResourceCache>()->GetResourceDirectory() + "/";

    // Console
    LoadIcon_(resourceDirectory + "Icons/Console_Info.png", IconType::IconType_Console_Info);
    LoadIcon_(resourceDirectory + "Icons/Console_Warning.png", IconType::IconType_Console_Warning);
    LoadIcon_(resourceDirectory + "Icons/Console_Error.png", IconType::IconType_Console_Error);

    // Toolbar
    LoadIcon_(resourceDirectory + "Icons/Toolbar_Play.png", IconType::IconType_Toolbar_Play);
    LoadIcon_(resourceDirectory + "Icons/Toolbar_Pause.png", IconType::IconType_Toolbar_Pause);
    LoadIcon_(resourceDirectory + "Icons/Toolbar_Stop.png", IconType::IconType_Toolbar_Stop);

    // Assets
    LoadIcon_(resourceDirectory + "Icons/Assets_Cube.png", IconType::IconType_ObjectPanel_Cube);

    // Default
    g_NoIcon = LoadIcon_(resourceDirectory + "Icons/AssetBrowser_Unknown.png", IconType::IconType_Custom);
}

Aurora::AuroraResource* IconLibrary::GetTextureByType(IconType iconType)
{
    return LoadIcon_("", iconType).m_Texture.get();
}

Aurora::AuroraResource* IconLibrary::GetTextureByFilePath(const std::string& filePath)
{
    return LoadIcon_(filePath).m_Texture.get();
}

Aurora::AuroraResource* IconLibrary::GetTextureByIcon(const Icon& icon)
{
    for (const Icon& storedIcon : m_Icons)
    {
        if (storedIcon.m_Texture->GetObjectID() == icon.m_Texture->GetObjectID())
        {
            return storedIcon.m_Texture.get();
        }
    }

    return nullptr;
}

const Icon& IconLibrary::LoadIcon_(const std::string& filePath, IconType iconType, int iconSize)
{
    // Check if we already have this thumbnail (by type).
    if (iconType != IconType::IconType_Custom)
    {
        for (Icon& icon : m_Icons)
        {
            if (icon.m_IconType == iconType)
            {
                return icon;
            }
        }
    }
    else // Check if we already have this thumbnail (by path).
    {
        for (Icon& icon : m_Icons)
        {
            if (icon.m_Texture->m_FilePath == filePath)
            {
                return icon;
            }
        }
    }

    // Deduce File Path Type
    if (Aurora::FileSystem::IsSupportedImageFile(filePath))
    {
        // Make a cheap textures.
        std::shared_ptr<Aurora::AuroraResource> texture = m_EngineContext->GetSubsystem<Aurora::ResourceCache>()->LoadTexture(filePath, Aurora::FileSystem::GetFileNameFromFilePath(filePath));
        m_Icons.emplace_back(iconType, texture);

        return m_Icons.back();
    }

    return GetIconByType(IconType::IconType_Custom); // Can't find anything. Hence, we desperate retrieve something...
}

const Icon& IconLibrary::GetIconByType(IconType iconType)
{
    for (Icon& icon : m_Icons)
    {
        if (icon.m_IconType == iconType)
        {
            return icon;
        }
    }

    return g_NoIcon;
}
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../Resource/AuroraResource.h"

enum IconType
{
    IconType_NotAssigned,
    IconType_Custom,
    IconType_Console_Info,
    IconType_Console_Warning,
    IconType_Console_Error,
    IconType_AssetBrowser_Folder,
    IconType_AssetBrowser_Script,
    IconType_Toolbar_Play,
    IconType_Toolbar_Pause,
    IconType_Toolbar_Stop,
    IconType_ObjectPanel_Cube
};

namespace Aurora
{
    class EngineContext;
    struct RHI_Texture;
}
class Editor;

struct Icon
{
    Icon() = default;
    Icon(IconType iconType, std::shared_ptr<Aurora::AuroraResource> texture)
    {
        this->m_IconType = iconType;
        this->m_Texture = texture;
    }

    IconType m_IconType = IconType::IconType_NotAssigned;
    std::shared_ptr<Aurora::AuroraResource> m_Texture;
};

class IconLibrary
{
public:
    static IconLibrary& GetInstance()
    {
        static IconLibrary m_LibraryInstance;
        return m_LibraryInstance;
    }

    IconLibrary();
    ~IconLibrary();

    void Initialize(Aurora::EngineContext* engineContext, Editor* editorContext);

    Aurora::AuroraResource* GetTextureByType(IconType iconType);
    Aurora::AuroraResource* GetTextureByFilePath(const std::string& filePath);
    Aurora::AuroraResource* GetTextureByIcon(const Icon& icon);

    const Icon& LoadIcon_(const std::string& filePath, IconType iconType = IconType::IconType_Custom, int iconSize = 100);

private:
    const Icon& GetIconByType(IconType iconType);

private:
    std::vector<Icon> m_Icons;
    Editor* m_EditorContext;
    Aurora::EngineContext* m_EngineContext;
};
#pragma once
#include "IconLibrary.h"
#include "Extensions.h"
#include "FileSystem.h"
#include <chrono>
#include <functional>

// Keeps track of directory navigation.
class FileDialogNavigation
{
public:
    bool NavigateTo(std::string directory, bool updateHistory = true)
    {
        if (!Aurora::FileSystem::IsDirectory(directory))
        {
            return false;
        }

        // If the directory ends with a slash, remove it to simplify operations below.
        if (directory.back() == '\\')
        {
            directory = directory.substr(0, directory.size() - 1);
        }

        // Ensure we don't renavigate.
        if (m_CurrentPath == directory)
        {
            return false;
        }

        // Update current path.
        m_CurrentPath = directory;

        // Update history.
        if (updateHistory)
        {
            m_PathHistory.emplace_back(m_CurrentPath);
            m_PathHistoryIndex++;
        }

        // Clear hierarchy.
        m_CurrentPathHierarchy.clear();
        m_CurrentPathHierarchyLabels.clear();

        // Is there a slash?
        std::size_t slashPositionIndex = m_CurrentPath.find('\\'); // Remember that .find returns the first occurance of the given character found.

        // If there are no further slashes and no nesting, we are done.
        if (slashPositionIndex == std::string::npos)
        {
            m_CurrentPathHierarchy.emplace_back(m_CurrentPath);
        }
        // If there is a slash, get the individual directories between the slashs. ../Resources/Herpaderp/Derp etc.
        else
        {
            std::size_t previousSlashPosition = 0;

            // Recursive.
            while (true)
            {
                // Save everything before the slash.
                m_CurrentPathHierarchy.emplace_back(m_CurrentPath.substr(0, slashPositionIndex));

                // Attempt to find a slash after the one we already found.
                previousSlashPosition = slashPositionIndex;
                slashPositionIndex = m_CurrentPath.find('\\', slashPositionIndex + 1);

                // If there are no more slashes...
                if (slashPositionIndex == std::string::npos)
                {
                    // Save the complete path to the hierarchy.
                    m_CurrentPathHierarchy.emplace_back(m_CurrentPath);
                    break;
                }
            }
        }

        // Create a proper looking label to show in the editor for each path.
        for (const std::string& filePath : m_CurrentPathHierarchy)
        {
            slashPositionIndex = filePath.find('\\');
            if (slashPositionIndex == std::string::npos)
            {
                m_CurrentPathHierarchyLabels.emplace_back(filePath);
            }
            else
            {
                m_CurrentPathHierarchyLabels.emplace_back(filePath.substr(filePath.find_last_of('\\') + 1)); // Do not include the /.              
            }
        }

        return true;
    }

    bool Backward()
    {
        if (m_PathHistory.empty() || (m_PathHistoryIndex - 1) < 0)
        {
            return false;
        }

        NavigateTo(m_PathHistory[--m_PathHistoryIndex], false);

        return true;
    }

    bool Forward()
    {
        if (m_PathHistory.empty() || (m_PathHistoryIndex + 1) >= static_cast<int>(m_PathHistory.size())) // Ensure that we don't go over the currently saved path history.
        {
            return false;
        }

        NavigateTo(m_PathHistory[++m_PathHistoryIndex], false);

        return true;
    }

public:
    std::string m_CurrentPath;
    std::vector<std::string> m_CurrentPathHierarchy;        // The list of paths to the current directory.
    std::vector<std::string> m_CurrentPathHierarchyLabels;  // The list of labels for each path leading to the current directory.
    std::vector<std::string> m_PathHistory;                 // Allows us to head forward/backwards from the current directory.
    int m_PathHistoryIndex = -1;                            // The current index of the path in m_PathHistory.
};

// A class representing each item in our current asset browser hierarchy.
class FileDialogItem
{
public:
    FileDialogItem(const std::string& filePath, const Icon& icon)
    {
        m_Path = filePath;
        m_Icon = icon;
        m_ID = Aurora::AuroraObject::GenerateObjectID();
        m_IsDirectory = Aurora::FileSystem::IsDirectory(filePath); // Some file dialogs can be items, not directories.
        m_Label = Aurora::FileSystem::GetFileNameFromFilePath(filePath);
    }

    const std::string& GetPath() const { return m_Path; }
    const std::string& GetLabel() const { return m_Label; }
    uint32_t GetID() const { return m_ID; }
    Aurora::DX11_Texture* GetTexture() const { return IconLibrary::GetInstance().GetTextureByIcon(m_Icon); }
    bool IsDirectory() const { return m_IsDirectory; }
    float GetTimeSinceLastClickInMilliseconds() const { return static_cast<float>(m_TimeSinceLastClick.count()); }

    void Click()
    {
        const std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        m_TimeSinceLastClick = currentTime - m_LastClickTime;
        m_LastClickTime = currentTime;
    }

public:
    bool m_IsRenaming = false;
 
private:
    Icon m_Icon;
    uint32_t m_ID;
    std::string m_Path;
    std::string m_Label;
    bool m_IsDirectory;
    std::chrono::duration<double, std::milli> m_TimeSinceLastClick;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_LastClickTime;
};

class FileDialog
{
public:
    FileDialog(Aurora::EngineContext* engineContext, Editor* editorContext, bool isStandaloneWindow);

    // Master that encomapsses ShowTop, Middle and Bottom UI.
    bool ShowDialog(bool* isVisible, std::string* directory = nullptr, std::string* filePath = nullptr);
    void ShowDialogDirectory();

    // Callbacks
    void SetOnItemClickedCallback(const std::function<void(const std::string&)>& callback) { m_OnItemClickedCallback = callback; }
    void SetOnItemDoubleClickedCallback(const std::function<void(const std::string&)>& callback) { m_OnItemDoubleClickedCallback = callback; }

    // Creation of New Content
    void CreateNewFolder(const std::string& filePath);
    void CreateNewMaterial(const std::string& filePath);

    // Misc
    void SetDirty(bool isDirty) { m_IsDirty = isDirty; }

private:
    void ShowTopUI(bool* isVisible);
    void ShowMiddleUI();
    void ShowBottomUI(bool* isVisible);

    // Item functionality handling.
    void OnItemDrag(FileDialogItem* item) const;
    void OnItemClick(FileDialogItem* item) const;
    void ItemContextMenu(FileDialogItem* item);
    void AddDirectoryListItem(const std::string& filePath, bool isProjectRoot = false);

    // Misc
    bool DialogUpdateFromDirectory(const std::string& directoryPath);
    void EmptyAreaContextMenu();

private:

    // Options
    /// Position
    /// Drop Shadows?
    const float m_HierarchyItemSizeMinimum = 50.0f;
    const float m_HierarchyItemSizeMaximum = 200.0f;
    const Aurora::Math::Vector4 m_ContentBackgroundColor = { 0.0f, 0.0f, 0.0f, 50.0f };

    // Flags
    bool m_IsWindow;
    bool m_SelectionMade;
    bool m_IsDirty;
    bool m_IsHoveringItem;
    bool m_IsHoveringWindow;
    std::string m_Title;
    FileDialogNavigation m_NavigationContext;
    std::string m_InputBox;
    std::string m_HoveredItemPath;
    uint32_t m_DisplayedItemCount;
    bool m_ShowCachedFiles = false;

    // Internal
    mutable uint32_t m_ContextMenuID;
    mutable EditorExtensions::DragDropPayload m_DragDropPayload;
    float m_OffsetBottom = 0.0f;
    ImGuiTextFilter m_SearchFilter; 

    std::vector<FileDialogItem> m_HierarchyItems;
    Aurora::Math::Vector2 m_HierarchyItemSize;
    Aurora::EngineContext* m_EngineContext;
    Editor* m_EditorContext;
    FileDialogItem* m_CurrentlyRenamingItem = nullptr;
    char m_RenameBuffer[256];

    // Directory List
    bool m_ShowDirectoryList = true;
    bool m_IsDirectoryListDirty = true;
    std::vector<std::string> m_DirectoryList;
    std::string m_CurrentlySelectedDirectoryItem = "";

    // Callbacks
    std::function<void(const std::string&)> m_OnItemClickedCallback;
    std::function<void(const std::string&)> m_OnItemDoubleClickedCallback;
};
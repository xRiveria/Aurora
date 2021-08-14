#include "FileDialog.h"
#include "Settings.h"
#include "../Source/imgui_internal.h"

FileDialog::FileDialog(Aurora::EngineContext* engineContext, Editor* editorContext, bool isStandaloneWindow)
{
    m_EngineContext = engineContext;
    m_EditorContext = editorContext;
    m_Title = "File Dialog";
    m_IsWindow = isStandaloneWindow;
    m_HierarchyItemSize = Aurora::Math::Vector2(90.0f, 90.0f);
    m_IsDirty = true;
    m_SelectionMade = false;
    m_OnItemClickedCallback = nullptr;
    m_OnItemDoubleClickedCallback = nullptr;

    m_NavigationContext.NavigateTo(m_EngineContext->GetSubsystem<Aurora::Settings>()->GetProjectDirectory());

    /// External file dialogs can open to display it at the center of the screen.
}

bool FileDialog::ShowDialog(bool* isVisible, std::string* directory, std::string* filePath)
{
    if (!(*isVisible)) // If the dialog isn't visible...
    {
        m_IsDirty = true; // Set as dirty as things can change until the next time it becomes visible.
        return false;
    }

    m_SelectionMade = false;
    m_IsHoveringItem = false;
    m_IsHoveringWindow = false;

    ShowTopUI(isVisible);       // Top menu.
    ShowMiddleUI();             // Contents of the current directory.
    ShowBottomUI(isVisible);    // Bottom menu.

    if (m_IsWindow) 
    {
        ImGui::End();
    }

    if (m_IsDirty)
    {
        DialogUpdateFromDirectory(m_NavigationContext.m_CurrentPath);
        m_IsDirty = false;
    }

    if (m_SelectionMade)
    {
        if (directory)
        {
            (*directory) = m_NavigationContext.m_CurrentPath;
        }

        if (filePath)
        {
            (*filePath) = m_NavigationContext.m_CurrentPath + "/" + std::string(m_InputBox);
        }
    }

    EmptyAreaContextMenu();

    return m_SelectionMade;
}

void FileDialog::ShowTopUI(bool* isVisible)
{
    if (m_IsWindow)
    {
        /// Window Dialog Shenanigans.
    }

    // Directory Navigation Buttons
    if (ImGui::Button("<"))
    {
        m_IsDirty = m_NavigationContext.Backward();
    }

    ImGui::SameLine();

    if (ImGui::Button(">"))
    {
        m_IsDirty = m_NavigationContext.Forward();
    }

    // Search filter.
    ImGui::SameLine();
    const float labelWidth = 170.0f;
    m_SearchFilter.Draw("##SearchFilter", labelWidth);

    // Text
    ImGui::SameLine();

    // Turn them into buttons?
    char buffer[1024] = "";
    for (uint32_t i = 0; i < m_NavigationContext.m_CurrentPathHierarchyLabels.size(); i++)
    {
        strcat_s(buffer, sizeof(buffer), m_NavigationContext.m_CurrentPathHierarchyLabels[i].c_str());
        strcat_s(buffer, sizeof(buffer), " / ");
    }

    ImGui::Text(buffer);

    ImGui::Separator();
}

void FileDialog::ShowMiddleUI()
{
    // Compute some stuff.
    const auto window = ImGui::GetCurrentWindowRead(); // Get current window.
    const float contentWidth = ImGui::GetContentRegionAvail().x;
    const float contentHeight = ImGui::GetContentRegionAvail().y - m_OffsetBottom;
    m_DisplayedItemCount = 0;

    ImGuiContext& context = *GImGui;
    ImGuiStyle& style = ImGui::GetStyle();
    const float fontSize = context.FontSize;
    const float labelSize = fontSize;
    const float textOffset = 3.0f;
    bool newLine = true;
    ImRect rectButton;
    ImRect rectLabel;
    float penXMinimum = 0.0f;
    float penX = 0.0f;

    // Remove border.
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0);

    // Make background slightly darker.
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(
        static_cast<int>(m_ContentBackgroundColor.x),
        static_cast<int>(m_ContentBackgroundColor.y),
        static_cast<int>(m_ContentBackgroundColor.z),
        static_cast<int>(m_ContentBackgroundColor.w)));
    
    if (ImGui::BeginChild("##FileDialog_ContentRegion", ImVec2(contentWidth, contentHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        m_IsHoveringWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) ? true : m_IsHoveringWindow;

        // Set starting position.
        float offset = ImGui::GetStyle().ItemSpacing.x;
        penXMinimum = ImGui::GetCursorPosX() + offset;
        ImGui::SetCursorPosX(penXMinimum);

        // Go through all items.
        for (int i = 0; i < m_HierarchyItems.size(); i++)
        {
            // Get item to be displayed.
            FileDialogItem& item = m_HierarchyItems[i];

            // Apply search filter.
            if (!m_SearchFilter.PassFilter(item.GetLabel().c_str()))
            {
                continue;
            }

            m_DisplayedItemCount++;

            // Start a new line?
            if (newLine)
            {
                ImGui::BeginGroup();
                newLine = false;
            }

            ImGui::BeginGroup();
            {
                {
                    // Compute rectangles for elements that make up an item.
                    rectButton = ImRect
                    (
                        ImGui::GetCursorScreenPos().x,
                        ImGui::GetCursorScreenPos().y,
                        ImGui::GetCursorScreenPos().x + m_HierarchyItemSize.x,
                        ImGui::GetCursorScreenPos().y + m_HierarchyItemSize.y
                    );

                    rectLabel = ImRect
                    (
                        rectButton.Min.x,
                        rectButton.Max.y - labelSize - style.FramePadding.y,
                        rectButton.Max.x,
                        rectButton.Max.y
                    );
                }

                /// Drop shadow effect?

                // Icon
                {
                    ImGui::PushID(i);
                    // ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.25f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

                    if (EditorExtensions::Button("##Dummy", m_HierarchyItemSize))
                    {
                        // Determine type of click.
                        item.Click();
                        const bool isSingleClick = item.GetTimeSinceLastClickInMilliseconds() > 500;

                        if (isSingleClick)
                        {
                            // Updated input box.
                            m_InputBox = item.GetLabel();

                            // Callback
                            if (m_OnItemClickedCallback)
                            {
                                m_OnItemClickedCallback(item.GetPath());
                            }
                        }
                        else // Double Click
                        {
                            m_IsDirty = m_NavigationContext.NavigateTo(item.GetPath());
                            m_SelectionMade = !item.IsDirectory();

                            // When browsing files, open them on double click.
                            if (!item.IsDirectory())
                            {
                                Aurora::FileSystem::OpenItem(item.GetPath());
                            }

                            // Callback
                            if (m_OnItemDoubleClickedCallback)
                            {
                                m_OnItemDoubleClickedCallback(m_NavigationContext.m_CurrentPath);
                            }
                        }
                    }

                    // Item Functionality
                    {
                        // Detect some useful states.
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
                        {
                            m_IsHoveringItem = true;
                            m_HoveredItemPath = item.GetPath();
                        }

                        OnItemClick(&item);
                        ItemContextMenu(&item);
                        OnItemDrag(&item);
                    }

                    // Image
                    {
                        // Compute thumbnail size.
                        ImVec2 imageSizeMax = ImVec2(rectButton.Max.x - rectButton.Min.x - style.FramePadding.x * 2.0f, rectButton.Max.y - rectButton.Min.y - style.FramePadding.y - labelSize - 5.0f);
                        ImVec2 imageSize = item.GetTexture() ? ImVec2(static_cast<float>(item.GetTexture()->GetWidth()), static_cast<float>(item.GetTexture()->GetHeight())) : imageSizeMax;
                        ImVec2 imageSizeDelta = ImVec2(0.0f, 0.0f);

                        // Scale the image size to fit the max size avaliable while respecting its aspect ratio.
                        {
                            // Clamp Width
                            if (imageSize.x != imageSizeMax.x)
                            {
                                float scale = imageSizeMax.x / imageSize.x;
                                imageSize.x = imageSizeMax.x;
                                imageSize.y = imageSize.y * scale;
                            }

                            // Clamp Height
                            if (imageSize.y != imageSizeMax.y)
                            {
                                float scale = imageSizeMax.y / imageSize.y;
                                imageSize.x = imageSize.x * scale;
                                imageSize.y = imageSizeMax.y;
                            }

                            imageSizeDelta.x = imageSizeMax.x - imageSize.x;
                            imageSizeDelta.y = imageSizeMax.y - imageSize.y;
                        }

                        ImGui::SetCursorScreenPos(ImVec2(rectButton.Min.x + style.FramePadding.x + imageSizeDelta.x * 0.5f, rectButton.Min.y + style.FramePadding.y + imageSizeDelta.y * 0.5f));
                        EditorExtensions::Image(item.GetTexture(), imageSize);
                    }

                    ImGui::PopStyleColor(1);
                    ImGui::PopID();
                }

                // Label
                {
                    const char* labelText = item.GetLabel().c_str();
                    const ImVec2 labelSize = ImGui::CalcTextSize(labelText, nullptr, true);

                    // Draw text background.
                    ImGui::SetCursorScreenPos(ImVec2(rectLabel.Min.x + textOffset, rectLabel.Min.y + textOffset));
                    if (labelSize.x <= m_HierarchyItemSize.x && labelSize.y <= m_HierarchyItemSize.y)
                    {
                        ImGui::TextUnformatted(labelText);
                    }
                    else
                    {
                        ImGui::RenderTextClipped(ImVec2((rectLabel.Min.x + textOffset), rectLabel.Min.y + textOffset), ImVec2((rectLabel.Min.x + textOffset), rectLabel.Min.y + textOffset), labelText, nullptr, &labelSize, ImVec2(0, 0), &rectLabel);
                    }
                }

                ImGui::EndGroup();
            }

            // Decide whether we should switch to the next column or row.
            penX += m_HierarchyItemSize.x + ImGui::GetStyle().ItemSpacing.x;
            if (penX >= contentWidth - m_HierarchyItemSize.x)
            {
                ImGui::EndGroup();
                penX = penXMinimum;
                ImGui::SetCursorPosX(penX);
                newLine = true;
            }
            else
            {
                ImGui::SameLine();
            }
        }

        if (!newLine)
        {
            ImGui::EndGroup();
        }
    }
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void FileDialog::ShowBottomUI(bool* isVisible)
{
    const auto SizeSlider = [this]()
    {
        // Size Slider
        const float sliderWidth = 150.0f;
        ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - sliderWidth);
        ImGui::PushItemWidth(sliderWidth);

        const float previousWidth = m_HierarchyItemSize.x;
        ImGui::SetCursorPosY((ImGui::GetWindowSize().y - m_OffsetBottom - 2.3f));
        ImGui::SliderFloat("##FileDialog_Slider", &m_HierarchyItemSize.x, m_HierarchyItemSizeMinimum, m_HierarchyItemSizeMaximum, "%.3g");
        m_HierarchyItemSize.y += m_HierarchyItemSize.x - previousWidth; // Main aspect ratio.

        ImGui::PopItemWidth();
    };

    m_OffsetBottom = 23.0f;
    ImGui::SetCursorPosY(ImGui::GetWindowSize().y - m_OffsetBottom);

    const char* text = (m_DisplayedItemCount == 1) ? "%d Item" : "%d Items";
    ImGui::Text(text, m_DisplayedItemCount);

    SizeSlider();
}

void FileDialog::OnItemDrag(FileDialogItem* item) const
{

}

void FileDialog::OnItemClick(FileDialogItem* item) const
{
    if (!item || !m_IsHoveringItem)
    {
        return;
    }

    // Item context menu on right click.
    if (ImGui::IsItemClicked(1))
    {
        m_ContextMenuID = item->GetID();
        ImGui::OpenPopup("##FileDialog_HierarchyItem");
    }
}

void FileDialog::ItemContextMenu(FileDialogItem* item)
{
    if (m_ContextMenuID != item->GetID()) // Ensures that every item has a different ID so as to not allow one operation from another.
    {
        return;
    }

    if (!ImGui::BeginPopup("##FileDialog_HierarchyItem"))
    {
        return;
    }

    if (ImGui::MenuItem("Delete"))
    {
        if (item->IsDirectory())
        {
            Aurora::FileSystem::Delete(item->GetPath());
            m_IsDirty = true;
        }
        else
        {
            Aurora::FileSystem::Delete(item->GetPath());
            m_IsDirty = true;
        }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Open in File Explorer"))
    {
        Aurora::FileSystem::OpenItem(item->GetPath());
    }

    ImGui::EndPopup();
}

bool FileDialog::DialogUpdateFromDirectory(const std::string& directoryPath)
{
    if (!Aurora::FileSystem::IsDirectory(directoryPath))
    {
        AURORA_ERROR(Aurora::LogLayer::Editor, "File dialog could not be updated. Provided path \"%s\" does not point to a directory.", directoryPath.c_str());
        return false;
    }

    m_HierarchyItems.clear(); // Clearing items does not guarantee a change in vector capacity. 
    m_HierarchyItems.shrink_to_fit(); // Hence, we will request for a removal of unused capacity.

    // We will show directories first, then items. Naturally, we can allow some sort of sorting function in the future and sort items based on key parameters.

    // Get directories.
    std::vector<std::string> childDirectories = Aurora::FileSystem::GetDirectoriesInDirectory(directoryPath);
    for (const std::string& childDirectory : childDirectories)
    {
        m_HierarchyItems.emplace_back(childDirectory, IconLibrary::GetInstance().LoadIcon_(childDirectory, IconType::IconType_AssetBrowser_Folder, static_cast<int>(m_HierarchyItemSize.x)));
    }

    // Get items.
    std::vector<std::string> childItems = Aurora::FileSystem::GetFilesInDirectory(directoryPath);
    for (const std::string& childItem : childItems)
    {
        if (Aurora::FileSystem::IsSupportedImageFile(childItem) || Aurora::FileSystem::IsEngineFile(childItem))
        {
            m_HierarchyItems.emplace_back(childItem, IconLibrary::GetInstance().LoadIcon_(childItem, IconType::IconType_Custom, static_cast<int>(m_HierarchyItemSize.x)));
        }
    }

    return true;
}

void FileDialog::EmptyAreaContextMenu()
{
    if (ImGui::IsMouseClicked(1) && m_IsHoveringWindow && !m_IsHoveringItem)
    {
        ImGui::OpenPopup("##FileDialog_EmptyContextMenu");
    }

    if (!ImGui::BeginPopup("##FileDialog_EmptyContextMenu"))
    {
        return;
    }

    if (ImGui::MenuItem("Create Folder"))
    {
        Aurora::FileSystem::CreateDirectory_(m_NavigationContext.m_CurrentPath + "/New Folder");
        m_IsDirty = true;
    }

    /// Other shenanigans.
    ImGui::Separator();

    if (ImGui::MenuItem("Open Directory in Explorer"))
    {
        Aurora::FileSystem::OpenItem(m_NavigationContext.m_CurrentPath);
    }

    ImGui::EndPopup();
}

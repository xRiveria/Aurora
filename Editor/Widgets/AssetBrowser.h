#pragma once
#include "../Backend/Widget.h"
#include "../Backend/Utilities/Extensions.h"

namespace Aurora
{
	class Settings;
}
class FileDialog;

// === Asset Browser ===

// There are two types of asset browsers - standalone ones similar to our classic file dialog, and the one that sits as a widget in our editor.

using namespace EditorExtensions;
class AssetBrowser : public Widget
{
public:
	AssetBrowser(Editor* editorContext, Aurora::EngineContext* engineContext);

	void OnTickVisible() override;

private:
	void OnPathClicked(const std::string& filePath) const;

private:
	std::unique_ptr<FileDialog> m_FileDialog;
	Aurora::Settings* m_SettingsSubsystem;
};
